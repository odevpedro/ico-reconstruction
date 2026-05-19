#!/usr/bin/env python3
"""Summarize Rev.093b-style halfword writer probe logs.

The script streams a JSONL or JSONL.GZ probe log and groups events by
`halfword_entry`. It does not require valid JSON parsing; regex extraction is
intentional because very large PCSX2 logs may end with a partial line.
"""

from __future__ import annotations

import argparse
import collections
import gzip
import re
from pathlib import Path
from typing import BinaryIO


LABEL_RE = re.compile(rb'"label":"([^"]+)"')
CYCLE_RE = re.compile(rb'"cycle":([0-9]+)')
INFO_A0_RE = re.compile(rb'"info":\{[^\n]*?"a0":"(0x[0-9a-fA-F]+)"')
COUNTER_RE = re.compile(rb'"counter":"(0x[0-9a-fA-F]+)"')
ROW_RE = re.compile(rb'"a2_row":"(0x[0-9a-fA-F]+)"')
COL_RE = re.compile(rb'"t0_col":"(0x[0-9a-fA-F]+)"')
WORLD_RE = re.compile(rb'"world_state_raw":"(0x[0-9a-fA-F]+)"')
RA_RE = re.compile(rb'"ra":"(0x[0-9a-fA-F]+)"')


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


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("log", type=Path)
    parser.add_argument("--top", type=int, default=20)
    args = parser.parse_args()

    labels = collections.Counter()
    ras = collections.Counter()
    write_labels = collections.Counter()
    write_counts = collections.Counter()
    final_counters = collections.Counter()
    counter_vs_writes = collections.Counter()
    cells = collections.Counter()
    sequences = collections.Counter()
    a0_total = collections.Counter()
    a0_with_writes = collections.Counter()
    a0_write_events = collections.Counter()
    worlds = collections.Counter()

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
        current = None

    with _open(args.log) as f:
        for line in f:
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
                current = {
                    "a0": _text(INFO_A0_RE.search(line)),
                    "cells": [],
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
    show("world_state_raw", worlds)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
