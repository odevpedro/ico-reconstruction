#!/usr/bin/env python3
"""Summarize Rev.093b/Rev.094-style halfword writer probe logs.

The script streams a JSONL or JSONL.GZ probe log and groups events by
`halfword_entry`. It adds caller-side cuts for the inferred fast path and the
second caller, which makes the next runtime capture easier to analyze without
re-running ad hoc scripts. It does not require valid JSON parsing; regex
extraction is intentional because very large PCSX2 logs may end with a partial
line. Optional tail analysis can summarize the recent log window by lines or
approximate megabytes without changing the default full-log report. The recent
window report includes labels, PCs, return addresses, `a0`, cell writes, and
`world_state_raw` when present.
"""

from __future__ import annotations

import argparse
import collections
import gzip
import re
from pathlib import Path
from typing import BinaryIO


LABEL_RE = re.compile(rb'"label":"([^"]+)"')
PC_RE = re.compile(rb'"pc":"(0x[0-9a-fA-F]+)"')
CYCLE_RE = re.compile(rb'"cycle":([0-9]+)')
REGS_A0_RE = re.compile(rb'"regs":\{[^\n]*?"a0":"(0x[0-9a-fA-F]+)"')
INFO_A0_RE = re.compile(rb'"info":\{[^\n]*?"a0":"(0x[0-9a-fA-F]+)"')
COUNTER_RE = re.compile(rb'"counter":"(0x[0-9a-fA-F]+)"')
ROW_RE = re.compile(rb'"a2_row":"(0x[0-9a-fA-F]+)"')
COL_RE = re.compile(rb'"t0_col":"(0x[0-9a-fA-F]+)"')
WORLD_RE = re.compile(rb'"world_state_raw":"(0x[0-9a-fA-F]+)"')
RA_RE = re.compile(rb'"ra":"(0x[0-9a-fA-F]+)"')
CATALOG_RE = re.compile(
    r'\{\s*"([^"]+)",\s*"([^"]+)",\s*"([^"]+)",\s*"([^"]+)",\s*'
    r'\(ico_ptr32\)0x([0-9A-Fa-f]{8}),\s*([0-9]+)u,\s*\}',
    re.M,
)
CATALOG_PATH = Path(__file__).resolve().parents[1] / "src" / "entity" / "halfword_runtime_catalog.c"


def _open(path: Path) -> BinaryIO:
    if path.suffix == ".gz":
        return gzip.open(path, "rb")
    return path.open("rb")


def _hex(match: re.Match[bytes] | None) -> int | None:
    if not match:
        return None
    return int(match.group(1).decode(), 16)


def _int(match: re.Match[bytes] | None) -> int | None:
    if not match:
        return None
    return int(match.group(1).decode(), 10)


def _text(match: re.Match[bytes] | None, default: str = "<missing>") -> str:
    if not match:
        return default
    return match.group(1).decode()


def _recent_a0(line: bytes) -> str | None:
    value = _text(REGS_A0_RE.search(line), "")
    if value:
        return value
    value = _text(INFO_A0_RE.search(line), "")
    if value:
        return value
    return None


def probable_callsite_from_ra(ra: int | None) -> int | None:
    if ra is None:
        return None
    return (ra - 8) & 0xFFFFFFFF


def fmt_addr(value: int | None) -> str:
    if value is None:
        return "<missing>"
    return f"0x{value:08x}"


def _load_catalog_entries() -> list[dict[str, object]]:
    try:
        text = CATALOG_PATH.read_text(encoding="utf-8")
    except OSError:
        return []

    entries: list[dict[str, object]] = []
    for match in CATALOG_RE.finditer(text):
        entries.append(
            {
                "label": match.group(1),
                "status": match.group(2),
                "detail": match.group(3),
                "next_probe": match.group(4),
                "address": int(match.group(5), 16),
                "hits": int(match.group(6), 10),
            }
        )
    return entries


def _catalog_snapshot(entries: list[dict[str, object]]) -> str:
    if not entries:
        return "halfword_catalog_snapshot{count=0,next=continue_capture,latest=none,latest_status=none}"

    latest = entries[-1]
    return (
        "halfword_catalog_snapshot{"
        f"count={len(entries)},"
        f"next={latest['next_probe']},"
        f"latest={latest['label']},"
        f"latest_status={latest['status']}"
        "}"
    )


def _format_top(title: str, counter: collections.Counter, limit: int) -> None:
    print(f"\n{title}")
    for key, value in counter.most_common(limit):
        print(f"{value:>10}  {key}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("log", type=Path)
    parser.add_argument("--top", type=int, default=20)
    recent_group = parser.add_mutually_exclusive_group()
    recent_group.add_argument(
        "--recent-lines",
        type=int,
        help="summarize only the last N log lines after the full report",
    )
    recent_group.add_argument(
        "--recent-mb",
        type=float,
        help="summarize only the recent tail covering approximately N MiB of raw log lines",
    )
    args = parser.parse_args()

    labels = collections.Counter()
    ras = collections.Counter()
    write_labels = collections.Counter()
    write_counts = collections.Counter()
    final_counters = collections.Counter()
    counter_vs_writes = collections.Counter()
    cells = collections.Counter()
    sequences = collections.Counter()
    entry_callsites = collections.Counter()
    entry_callsite_buckets = collections.Counter()
    entry_world_states = collections.Counter()
    a0_total = collections.Counter()
    a0_with_writes = collections.Counter()
    a0_write_events = collections.Counter()
    worlds = collections.Counter()
    recent_mode = None
    recent_buffer: collections.deque[bytes] | None = None
    recent_bytes_budget = 0
    recent_total_bytes = 0

    if args.recent_lines is not None:
        if args.recent_lines <= 0:
            parser.error("--recent-lines must be greater than zero")
        recent_mode = "lines"
        recent_buffer = collections.deque(maxlen=args.recent_lines)
    elif args.recent_mb is not None:
        if args.recent_mb <= 0:
            parser.error("--recent-mb must be greater than zero")
        recent_mode = "bytes"
        recent_bytes_budget = max(1, int(args.recent_mb * 1024 * 1024))
        recent_buffer = collections.deque()

    current: dict | None = None
    invocations = 0
    first_cycle = None
    last_cycle = None

    def finish(next_counter: int | None) -> None:
        nonlocal current, invocations
        if current is None:
            return
        invocations += 1
        write_count = len(current["cells"])
        write_counts[write_count] += 1
        if next_counter is not None:
            final_counters[next_counter] += 1
            counter_vs_writes[(next_counter, write_count)] += 1
        if current["cells"]:
            sequences[tuple(current["cells"][:8])] += 1
            a0_with_writes[current["a0"]] += 1
            a0_write_events[current["a0"]] += write_count
        a0_total[current["a0"]] += 1
        callsite = current.get("callsite")
        if callsite is not None:
            entry_callsites[callsite] += 1
            if next_counter is not None:
                entry_callsite_buckets[(callsite, next_counter, write_count)] += 1
            world = current.get("world_state")
            if world:
                entry_world_states[(callsite, world)] += 1
        current = None

    with _open(args.log) as f:
        for line in f:
            if recent_buffer is not None:
                recent_buffer.append(line)
                if recent_mode == "bytes":
                    recent_total_bytes += len(line)
                    while recent_total_bytes > recent_bytes_budget and len(recent_buffer) > 1:
                        removed = recent_buffer.popleft()
                        recent_total_bytes -= len(removed)

            label_match = LABEL_RE.search(line)
            if not label_match:
                continue
            label = label_match.group(1).decode()
            labels[label] += 1

            ra = _text(RA_RE.search(line), "")
            if ra:
                ras[ra] += 1

            cycle = _int(CYCLE_RE.search(line))
            if cycle is not None:
                first_cycle = cycle if first_cycle is None else min(first_cycle, cycle)
                last_cycle = cycle if last_cycle is None else max(last_cycle, cycle)

            if label == "halfword_entry":
                finish(_hex(COUNTER_RE.search(line)))
                ra = _hex(RA_RE.search(line))
                current = {
                    "a0": _text(INFO_A0_RE.search(line)),
                    "cells": [],
                    "callsite": probable_callsite_from_ra(ra),
                    "world_state": _text(WORLD_RE.search(line), ""),
                }
            elif label in ("halfword_write_A", "halfword_write_B"):
                write_labels[label] += 1
                row = _hex(ROW_RE.search(line))
                col = _hex(COL_RE.search(line))
                if row is not None and col is not None:
                    cell = (row, col)
                    cells[cell] += 1
                    if current is not None:
                        current["cells"].append(cell)
            elif label == "world_state_load":
                world = _text(WORLD_RE.search(line), "")
                if world:
                    worlds[world] += 1

    finish(None)

    print(f"log: {args.log}")
    print(f"invocations: {invocations}")
    if first_cycle is not None and last_cycle is not None:
        print(f"cycle_range: {first_cycle}..{last_cycle} delta={last_cycle - first_cycle}")

    def show(title: str, counter: collections.Counter, limit: int | None = None) -> None:
        print(f"\n{title}")
        for key, value in counter.most_common(limit or args.top):
            print(f"{value:>10}  {key}")

    show("labels", labels)
    show("return_addresses", ras)
    show("write_labels", write_labels)
    show("write_count_per_invocation", write_counts)
    show("final_counter_at_next_entry", final_counters)
    show("final_counter_vs_observed_ab_writes", counter_vs_writes)
    show("top_cells", cells)
    show("top_sequences", sequences)
    show("top_a0_total", a0_total)
    show("top_a0_with_writes", a0_with_writes)
    show("top_a0_write_events", a0_write_events)
    show("entry_callsites", entry_callsites)
    print("\nentry_callsite_vs_final_counter_and_writes")
    for (callsite, final_counter, write_count), value in entry_callsite_buckets.most_common(
        args.top
    ):
        print(
            f"{value:>10}  {fmt_addr(callsite)} "
            f"final_counter={final_counter} writes={write_count}"
        )
    print("\nentry_world_state_raw")
    for (callsite, world), value in entry_world_states.most_common(args.top):
        print(f"{value:>10}  {fmt_addr(callsite)} world_state_raw={world}")
    show("world_state_raw", worlds)

    catalog_entries = _load_catalog_entries()
    print("\nhalfword_catalog_snapshot")
    print(_catalog_snapshot(catalog_entries))
    print("halfword_catalog_entries")
    for entry in catalog_entries:
        print(
            f"{entry['label']:>20}  status={entry['status']} "
            f"address={fmt_addr(int(entry['address']))} hits={entry['hits']} "
            f"next={entry['next_probe']}"
        )

    if recent_buffer is not None:
        recent_labels = collections.Counter()
        recent_pcs = collections.Counter()
        recent_ras = collections.Counter()
        recent_a0 = collections.Counter()
        recent_write_labels = collections.Counter()
        recent_cells = collections.Counter()
        recent_worlds = collections.Counter()
        recent_total_lines = 0

        for line in recent_buffer:
            recent_total_lines += 1
            label_match = LABEL_RE.search(line)
            if not label_match:
                continue
            label = label_match.group(1).decode()
            recent_labels[label] += 1

            pc = _text(PC_RE.search(line), "")
            if pc:
                recent_pcs[pc] += 1

            ra = _text(RA_RE.search(line), "")
            if ra:
                recent_ras[ra] += 1

            a0 = _recent_a0(line)
            if a0:
                recent_a0[a0] += 1

            if label in ("halfword_write_A", "halfword_write_B"):
                recent_write_labels[label] += 1
                row = _hex(ROW_RE.search(line))
                col = _hex(COL_RE.search(line))
                if row is not None and col is not None:
                    recent_cells[(row, col)] += 1

            world = _text(WORLD_RE.search(line), "")
            if world:
                recent_worlds[world] += 1

        print("\nrecent_tail")
        if recent_mode == "lines":
            print(
                f"window: last {args.recent_lines} lines "
                f"(retained={recent_total_lines})"
            )
        else:
            print(
                "window: recent approx "
                f"{args.recent_mb:g} MiB "
                f"(retained_lines={recent_total_lines}, retained_bytes={recent_total_bytes})"
            )
        print("a0_source: regs.a0 -> info.a0 fallback")
        _format_top("recent_labels", recent_labels, args.top)
        _format_top("recent_pcs", recent_pcs, args.top)
        _format_top("recent_return_addresses", recent_ras, args.top)
        _format_top("recent_a0", recent_a0, args.top)
        _format_top("recent_write_labels", recent_write_labels, args.top)
        _format_top("recent_cells", recent_cells, args.top)
        _format_top("recent_world_state_raw", recent_worlds, args.top)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
