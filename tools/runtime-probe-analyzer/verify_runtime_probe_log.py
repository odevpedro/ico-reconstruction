#!/usr/bin/env python3
"""Validate an ICO PCSX2 JSONL runtime capture without loading it into memory."""

from __future__ import annotations

import argparse
import json
from collections import Counter
from pathlib import Path


REQUIRED_LABELS = {"elf_entry_sentinel", "isys_gobj_init", "ios_om_main"}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("log", type=Path)
    parser.add_argument(
        "--allow-incomplete",
        action="store_true",
        help="Only validate JSON/schema; do not require boot/runtime sentinel labels.",
    )
    args = parser.parse_args()

    if not args.log.is_file():
        print(f"invalid: log not found: {args.log}")
        return 1

    labels: Counter[str] = Counter()
    sessions: Counter[str] = Counter()
    malformed: list[str] = []
    events = 0

    with args.log.open("r", encoding="utf-8") as handle:
        for line_number, raw_line in enumerate(handle, 1):
            line = raw_line.strip()
            if not line:
                continue
            try:
                event = json.loads(line)
            except json.JSONDecodeError as exc:
                malformed.append(f"line {line_number}: {exc}")
                continue

            if not isinstance(event, dict):
                malformed.append(f"line {line_number}: event is not an object")
                continue

            label = event.get("label")
            session = event.get("session")
            regs = event.get("regs")
            info = event.get("info")
            if not isinstance(label, str) or not isinstance(session, str):
                malformed.append(f"line {line_number}: missing string label/session")
                continue
            if not isinstance(regs, dict) or not isinstance(info, dict):
                malformed.append(f"line {line_number}: missing regs/info objects")
                continue

            labels[label] += 1
            sessions[session] += 1
            events += 1

    missing = sorted(REQUIRED_LABELS - labels.keys())
    valid = not malformed and events > 0 and len(sessions) == 1
    if not args.allow_incomplete and missing:
        valid = False

    print(f"status={'valid' if valid else 'invalid'}")
    print(f"events={events}")
    print(f"sessions={','.join(sorted(sessions)) or 'none'}")
    print("labels:")
    for label, count in labels.most_common():
        print(f"  {label}={count}")
    if missing:
        print(f"missing_required={','.join(missing)}")
    if malformed:
        print(f"malformed={len(malformed)}")
        for issue in malformed[:10]:
            print(f"  {issue}")

    return 0 if valid else 1


if __name__ == "__main__":
    raise SystemExit(main())
