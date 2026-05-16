#!/usr/bin/env python3
"""Monitor PCSX2 runtime probe JSONL logs for ICO reconstruction."""

from __future__ import annotations

import argparse
import json
import time
from collections import Counter
from pathlib import Path
from typing import Any, Iterable


KNOWN_CALLSITES = {
    0x001B7A80: "descriptor +0x58 jalr caller for cloth_payload_init",
    0x001B7A6C: "descriptor iterator pre-registration path",
    0x001B7AB0: "entry +0x24 callback registration",
    0x001B7ACC: "descriptor_label +0x40 fallback registration",
    0x00201ED4: "runtime-dependent 0x00201e70 registration candidate",
    0x00203080: "caller of 0x00201e70 using table-loaded s0 as callback",
    0x00203EA0: "caller of 0x00201e70 using [s0+4] as callback",
    0x00240F90: "literal t0 callback registration path",
    0x00082618: "ELF entry sentinel caller",
}


def parse_hex(value: Any) -> int | None:
    if value is None:
        return None
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        try:
            return int(value, 16)
        except ValueError:
            return None
    return None


def fmt_addr(value: int | None) -> str:
    if value is None:
        return "unknown"
    return f"0x{value:08x}"


def probable_callsite_from_ra(ra: int | None) -> int | None:
    if ra is None:
        return None
    return (ra - 8) & 0xFFFFFFFF


def event_summary(event: dict[str, Any], index: int) -> str:
    regs = event.get("regs", {})
    memory = event.get("memory", {})

    pc = parse_hex(event.get("pc"))
    ra = parse_hex(regs.get("ra"))
    a0 = parse_hex(regs.get("a0"))
    a1 = parse_hex(regs.get("a1"))
    sp = parse_hex(regs.get("sp"))
    s0 = parse_hex(regs.get("s0"))
    s3 = parse_hex(regs.get("s3"))
    variant = parse_hex(memory.get("a1_plus_30_word"))
    callsite = probable_callsite_from_ra(ra)
    callsite_label = KNOWN_CALLSITES.get(callsite, "unclassified")

    stack_note = "a1==sp" if a1 is not None and a1 == sp else "a1!=sp"

    return (
        f"hit={index} pc={fmt_addr(pc)} label={event.get('label', 'unknown')} "
        f"ra={fmt_addr(ra)} callsite={fmt_addr(callsite)} "
        f"caller={callsite_label} a0={fmt_addr(a0)} a1={fmt_addr(a1)} "
        f"sp={fmt_addr(sp)} {stack_note} s0={fmt_addr(s0)} s3={fmt_addr(s3)} "
        f"a1+0x30={fmt_addr(variant)} cycle={event.get('cycle', 'unknown')}"
    )


def read_events(path: Path) -> Iterable[dict[str, Any]]:
    with path.open("r", encoding="utf-8") as handle:
        for line_number, line in enumerate(handle, 1):
            line = line.strip()
            if not line:
                continue
            try:
                event = json.loads(line)
            except json.JSONDecodeError as exc:
                print(f"skip malformed line={line_number}: {exc}")
                continue
            if isinstance(event, dict):
                yield event


def follow_events(path: Path, poll_seconds: float) -> Iterable[dict[str, Any]]:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.touch(exist_ok=True)

    with path.open("r", encoding="utf-8") as handle:
        handle.seek(0, 2)
        while True:
            line = handle.readline()
            if not line:
                time.sleep(poll_seconds)
                continue
            line = line.strip()
            if not line:
                continue
            try:
                event = json.loads(line)
            except json.JSONDecodeError as exc:
                print(f"skip malformed live line: {exc}")
                continue
            if isinstance(event, dict):
                yield event


def print_summary(events: list[dict[str, Any]]) -> None:
    callsites: Counter[int] = Counter()
    variants: Counter[int] = Counter()
    a1_by_callsite: dict[int, Counter[int]] = {}
    a0_by_pc: dict[int, Counter[int]] = {}
    stack_matches = 0

    for event in events:
        regs = event.get("regs", {})
        memory = event.get("memory", {})
        pc = parse_hex(event.get("pc"))
        ra = parse_hex(regs.get("ra"))
        a0 = parse_hex(regs.get("a0"))
        a1 = parse_hex(regs.get("a1"))
        sp = parse_hex(regs.get("sp"))
        variant = parse_hex(memory.get("a1_plus_30_word"))
        callsite = probable_callsite_from_ra(ra)

        if callsite is not None:
            callsites[callsite] += 1
            if a1 is not None:
                a1_by_callsite.setdefault(callsite, Counter())[a1] += 1
        if pc is not None and a0 is not None:
            a0_by_pc.setdefault(pc, Counter())[a0] += 1
        if variant is not None:
            variants[variant] += 1
        if a1 is not None and a1 == sp:
            stack_matches += 1

    print(f"events={len(events)}")
    print(f"a1_equals_sp={stack_matches}")

    print("callsites:")
    for callsite, count in callsites.most_common():
        label = KNOWN_CALLSITES.get(callsite, "unclassified")
        print(f"  {fmt_addr(callsite)} count={count} label={label}")

    print("a1_plus_30_values:")
    for value, count in variants.most_common():
        print(f"  {fmt_addr(value)} count={count}")

    print("unique_a1_by_callsite:")
    for callsite, values in sorted(a1_by_callsite.items()):
        label = KNOWN_CALLSITES.get(callsite, "unclassified")
        rendered_values = ", ".join(
            f"{fmt_addr(value)}({count})" for value, count in values.most_common()
        )
        print(
            f"  {fmt_addr(callsite)} unique={len(values)} label={label} "
            f"values={rendered_values}"
        )

    print("unique_a0_by_pc:")
    for pc, values in sorted(a0_by_pc.items()):
        rendered_values = ", ".join(
            f"{fmt_addr(value)}({count})" for value, count in values.most_common()
        )
        print(f"  {fmt_addr(pc)} unique={len(values)} values={rendered_values}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--input",
        type=Path,
        default=Path("/tmp/ico-pcsx2-probe-events.jsonl"),
        help="Path to the PCSX2 probe JSONL log.",
    )
    parser.add_argument("--follow", action="store_true", help="Monitor new events.")
    parser.add_argument("--summary", action="store_true", help="Print aggregate summary.")
    parser.add_argument("--poll-seconds", type=float, default=0.25)
    args = parser.parse_args()

    if args.follow:
        print(f"monitoring {args.input}")
        for index, event in enumerate(follow_events(args.input, args.poll_seconds), 1):
            print(event_summary(event, index), flush=True)
        return 0

    if not args.input.exists():
        print(f"input not found: {args.input}")
        return 1

    events = list(read_events(args.input))
    for index, event in enumerate(events, 1):
        print(event_summary(event, index))

    if args.summary:
        print_summary(events)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
