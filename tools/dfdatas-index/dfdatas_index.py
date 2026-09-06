#!/usr/bin/env python3
"""Parse the fixed-record directory at the head of DFDATAS/DATA.DF.

Metadata-only: reads TOC records and validates offset/size windows. It
writes no payload bytes and no extracted assets.

Disc layout (USA BIN): raw 2352-byte sectors, 2048-byte payload at +24.
DATA.DF starts at the payload offset given by --payload-start.
"""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path

ENTRY_BYTES = 40
HEADER_BYTES = 4  # u32 file count before the first record
TOC_REGION = 8 * 2048  # directory lives in the first 8 payload sectors
OFFSET_OFFSET = 32
SIZE_OFFSET = 36
NAME_BYTES = 12


def parse_name(raw: bytes) -> str:
    name = raw[:NAME_BYTES].split(b"\x00", 1)[0]
    return name.decode("ascii", errors="replace").strip()


def readable(name: bytes) -> bool:
    return bool(name) and all(32 <= b < 127 for b in name)


def parse_record(raw: bytes) -> dict | None:
    name_raw = raw[:NAME_BYTES]
    name_b = name_raw.split(b"\x00", 1)[0]
    if not readable(name_b):
        return None
    (offset,) = struct.unpack_from("<I", raw, OFFSET_OFFSET)
    (size,) = struct.unpack_from("<I", raw, SIZE_OFFSET)
    if offset < 1 or size == 0 or size > 0x10000000:
        return None
    return {"name": name_b.decode("ascii"), "offset": offset, "size": size}


def compute_iso_offset(lba: int, sector_size: int, data_offset: int,
                       payload_byte: int) -> int:
    sector = payload_byte // 2048
    within = payload_byte % 2048
    return (lba + sector) * sector_size + data_offset + within


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--image", type=Path, required=True)
    ap.add_argument("--lba", type=int, required=True)
    ap.add_argument("--size", type=int, required=True)
    ap.add_argument("--sector-size", type=int, default=2352)
    ap.add_argument("--data-offset", type=int, default=24)
    ap.add_argument("--source-name", default="DFDATAS/DATA.DF")
    ap.add_argument("--output", type=Path)
    args = ap.parse_args()

    payload_start = compute_iso_offset(args.lba, args.sector_size,
                                       args.data_offset, 0)
    window = min(TOC_REGION, args.size)
    with args.image.open("rb") as handle:
        handle.seek(payload_start)
        toc = handle.read(window)
        if len(toc) < HEADER_BYTES:
            print("error: cannot read TOC")
            return 2
        declared, = struct.unpack_from("<I", toc, 0)
        print(f"head tag 0x0000 = 0x{declared:08x} (={declared})")

        records: list[dict] = []
        pos = HEADER_BYTES
        while pos + ENTRY_BYTES <= len(toc):
            rec = parse_record(toc[pos:pos + ENTRY_BYTES])
            if rec:
                rec["toc_offset"] = pos
                records.append(rec)
                pos += ENTRY_BYTES
            else:
                pos += 4  # resync on asymmetric records
        records.sort(key=lambda r: r["offset"], reverse=False)
        # drop duplicates (same offset+name) that the resync can double-read
        seen = set()
        uniq = []
        for r in records:
            key = (r["name"], r["offset"], r["size"])
            if key not in seen:
                seen.add(key)
                uniq.append(r)
        records = uniq

    print(f"records parsed: {len(records)}")
    by_name = {r["name"]: r for r in records}
    print(f"unique names : {len(by_name)}")

    bad = 0
    prev_hi = 0
    print("\npacking check (offset delta vs size):")
    for r in records:
        lo, size = r["offset"], r["size"]
        hi = lo + size
        if hi > args.size or lo > args.size:
            print(f"  !! out-of-file: {r['name']}") 
            bad += 1
            continue
        if lo < prev_hi:
            print(f"  !! overlap: {r['name']} off=0x{lo:08x} size=0x{size:08x} (prev end 0x{prev_hi:08x})")
            bad += 1
        prev_hi = max(prev_hi, hi)

    print(f"\nbads: {bad}, ok: {len(records) - bad}")
    print("coverage: first=0x%08x last=0x%08x / file=0x%08x" % (
        records[0]["offset"] if records else 0,
        prev_hi, args.size))

    print("\ncatalog:")
    for r in records:
        print(f"  {r['toc_offset']:04x} {r['name']:16s} off=0x{r['offset']:08x} size=0x{r['size']:08x}")

    if args.output:
        report = {
            "source": args.source_name,
            "image": str(args.image),
            "lba": args.lba,
            "file_size": args.size,
            "declared_count": declared,
            "record_stride_bytes": ENTRY_BYTES,
            "records": records,
        }
        args.output.write_text(json.dumps(report, indent=2))
        print(f"\nreport -> {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())