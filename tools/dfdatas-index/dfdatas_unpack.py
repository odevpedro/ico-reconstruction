#!/usr/bin/env python3
"""Decode all PAL DFDATAS .DF containers and catalog their named subfiles.

Metadata + extraction: PAL .DF payloads are raw deflate (RFC 1951) streams
decoding to a DF container (DfHeader count + 0x224-byte entries + blobs).
This tool decodes every .DF listed in a TOC catalog, parses the container,
lists subfiles (name/size/off/global_id), and optionally extracts textures
or specific extensions.

Usage:
  dfdatas_unpack.py --image <PAL.iso> --lba 19771 --base 0x269D800 \
      --toc /tmp/pal-dfdatas-toc.json \
      --out /tmp/pal-unpacked [--ext tm2] [--list]
"""

from __future__ import annotations

import argparse
import json
import struct
import zlib
from pathlib import Path

ENTRY_BYTES = 0x224
NAME_BYTES = 0x200
OFF_FIELD = 0x214
GID_FIELD = 0x210
NAME_FIELD = 0x10
SIZE_FIELD = 0x0C


def parse_container(raw: bytes) -> dict:
    """Parse a decoded DF container; returns {'count', 'entries', 'slack'}."""
    if len(raw) < 4:
        return {"count": 0, "entries": [], "slack": 0}
    count = struct.unpack_from("<I", raw, 0)[0]
    if count > 0x4000:  # sanity: real containers are well under 16K entries
        return {"count": count, "entries": [], "slack": 0}
    entries = []
    pos = 0x10
    for i in range(count):
        if pos + ENTRY_BYTES > len(raw):
            break
        e = raw[pos:pos + ENTRY_BYTES]
        (size,) = struct.unpack_from("<I", e, SIZE_FIELD)
        (gid,) = struct.unpack_from("<I", e, GID_FIELD)
        (off,) = struct.unpack_from("<I", e, OFF_FIELD)
        name = e[NAME_FIELD:NAME_FIELD + NAME_BYTES].split(b"\x00", 1)[0]
        try:
            nm = name.decode("ascii")
        except UnicodeDecodeError:
            nm = name.decode("latin1", errors="replace")
        entries.append({
            "index": i,
            "name": nm,
            "size": size,
            "offset": off,
            "global_id": gid,
        })
        pos += ENTRY_BYTES
    return {"count": count, "entries": entries, "slack": len(raw) - pos}


def read_payload(image: Path, base: int, off: int, size: int) -> bytes:
    with image.open("rb") as f:
        f.seek(base + off)
        return f.read(size)


def extract_entry(raw: bytes, entry: dict) -> bytes:
    o, s = entry["offset"], entry["size"]
    if o + s > len(raw):
        return b""
    return raw[o:o + s]


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--image", type=Path, required=True)
    ap.add_argument("--toc", type=Path, required=True)
    ap.add_argument("--base", type=lambda v: int(v, 0), required=True)
    ap.add_argument("--out", type=Path, default=Path("/tmp/pal-unpacked"))
    ap.add_argument("--ext", action="append", default=None,
                    help="only extract subfiles with these extensions (repeatable)")
    ap.add_argument("--only", action="append", default=None,
                    help="only decode these top-level .DF files (repeatable)")
    ap.add_argument("--list", action="store_true",
                    help="print full subfile catalog without extracting")
    args = ap.parse_args()

    toc = json.loads(args.toc.read_text())
    records = toc if isinstance(toc, list) else toc.get("records", [])
    if isinstance(records[0], dict):
        rec = [{"name": r["name"], "offset": r["offset"], "size": r["size"]}
               for r in records]
    else:
        rec = [{"name": n, "offset": o, "size": s} for n, o, s in records]

    ext_set = {e.lower().lstrip(".") for e in args.ext} if args.ext else None
    only_set = set(args.only) if args.only else None

    total_df = 0
    total_sub = 0
    summary = []
    for r in rec:
        name = r["name"]
        if not name.upper().endswith(".DF"):
            continue
        if only_set and name not in only_set:
            continue
        total_df += 1
        blob = read_payload(args.image, args.base, r["offset"], r["size"])
        raw = blob
        codec = "raw"
        if blob[:1] == b"\xec":
            raw = blob
        try:
            d = zlib.decompressobj(-15)
            decoded = d.decompress(blob) + d.flush()
            if decoded:
                raw = decoded
                codec = "deflate"
            else:
                try:
                    d2 = zlib.decompressobj(-15)
                    really = d2.decompress(blob, 1)
                except zlib.error:
                    codec = "raw"
        except zlib.error:
            codec = "raw"

        cont = parse_container(raw)
        entries = cont["entries"]
        total_sub += len(entries)
        summary.append({"df": name, "codec": codec,
                        "stored": r["size"], "decoded": len(raw),
                        "count": cont["count"], "entries": len(entries)})

        if args.list:
            print(f"\n== {name} [{codec}] stored={r['size']} decoded={len(raw)} "
                  f"count={cont['count']} parsed={len(entries)} slack={cont['slack']}")
            for e in entries[:20]:
                print(f"   [{e['index']:4d}] gid={e['global_id']:6d} "
                      f"off=0x{e['offset']:08x} size=0x{e['size']:08x} {e['name']}")
            if len(entries) > 20:
                print(f"   ... ({len(entries)} total)")

        if ext_set:
            for e in entries:
                ext = e["name"].rsplit(".", 1)[-1].lower() if "." in e["name"] else ""
                if ext not in ext_set:
                    continue
                data = extract_entry(raw, e)
                if not data:
                    continue
                dst = args.out / name / e["name"].lstrip("/")
                dst.parent.mkdir(parents=True, exist_ok=True)
                dst.write_bytes(data)

    print(f"\ntotal .DF: {total_df}, subfiles: {total_sub}")
    by_codec = {}
    for s in summary:
        by_codec[s["codec"]] = by_codec.get(s["codec"], 0) + 1
    print("codec distribution:", by_codec)
    for s in summary:
        print(f"  {s['df']:20s} {s['codec']:7s} stored={s['stored']:10d} "
              f"decoded={s['decoded']:10d} count={s['count']:5d} parsed={s['entries']:5d}")

    json_out = args.out / "catalog.json"
    if ext_set:
        json_out.parent.mkdir(parents=True, exist_ok=True)
        json_out.write_text(json.dumps(summary, indent=2))
        print(f"summary -> {json_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())