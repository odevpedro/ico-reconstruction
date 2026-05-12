#!/usr/bin/env python3
"""ELF Extractor for ICO - Extract ELF from disc image for disassembly.

This tool extracts the main executable (SCUS_971.13) from the BIN/CUE disc image
for use with external disassemblers like Ghidra, radare2, or IDA Pro.

NOTE: This extracts the raw ELF bytes. Handle with care - this is for local
analysis only, not for distribution.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


TOOL_NAME = "elf-extractor"
TOOL_VERSION = "0.1.0"
PAYLOAD_SIZE = 2048


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def read_image_region(image_path: Path, lba: int, size: int, sector_size: int, data_offset: int) -> bytes:
    chunks: list[bytes] = []
    remaining = size
    current_lba = lba
    with image_path.open("rb") as handle:
        while remaining > 0:
            handle.seek(current_lba * sector_size + data_offset)
            chunk = handle.read(min(PAYLOAD_SIZE, remaining))
            if not chunk:
                break
            chunks.append(chunk)
            remaining -= len(chunk)
            current_lba += 1
    data = b"".join(chunks)
    if len(data) != size:
        raise RuntimeError(f"Expected {size} bytes from image region, read {len(data)} bytes.")
    return data


def parse_elf_header(data: bytes) -> dict[str, Any]:
    if len(data) < 52 or data[:4] != b"\x7fELF":
        raise RuntimeError("Input is not an ELF file.")
    if data[4] != 1 or data[5] != 1:
        raise RuntimeError("Only ELF32 little-endian inputs are supported.")
    return {
        "entry_point": f"0x{int.from_bytes(data[24:28], 'little'):08x}",
        "machine": int.from_bytes(data[18:20], "little"),
    }


def calculate_hashes(data: bytes) -> dict[str, str]:
    return {
        "md5": hashlib.md5(data).hexdigest(),
        "sha256": hashlib.sha256(data).hexdigest(),
    }


def build_report(args: argparse.Namespace, output_path: Path, hashes: dict[str, str]) -> dict[str, Any]:
    return {
        "tool": TOOL_NAME,
        "tool_version": TOOL_VERSION,
        "generated_at_utc": utc_now(),
        "source": {
            "image_path": str(args.image.expanduser().resolve()),
            "source_name": args.source_name,
            "extent_lba": args.lba,
            "size_bytes": args.size,
            "sector_size": args.sector_size,
            "data_offset": args.data_offset,
        },
        "output": {
            "extracted_path": str(output_path),
            "hashes": hashes,
        },
        "legal_notice": "Extracted ELF is for local analysis only. Do not distribute.",
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Extract ELF from disc image for disassembly."
    )
    parser.add_argument("--image", type=Path, required=True, help="Local disc image containing the ELF.")
    parser.add_argument("--lba", type=int, required=True, help="Start LBA of the ELF.")
    parser.add_argument("--size", type=int, required=True, help="Size in bytes of the ELF.")
    parser.add_argument("--sector-size", type=int, default=2352, help="Physical sector size. Default: 2352.")
    parser.add_argument("--data-offset", type=int, default=24, help="Data payload offset. Default: 24.")
    parser.add_argument("--source-name", default="SCUS_971.13", help="Display name for the source.")
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(".local/extracted"),
        help="Directory for extracted ELF. Default: .local/extracted",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    image = args.image.expanduser().resolve()
    if not image.exists():
        print(f"error: Image path does not exist: {args.image}")
        return 1

    try:
        print(f"Reading ELF from disc image...")
        data = read_image_region(image, args.lba, args.size, args.sector_size, args.data_offset)

        header = parse_elf_header(data)
        print(f"ELF detected: {header['entry_point']}, Machine: {header['machine']}")

        output_dir = args.output_dir
        output_dir.mkdir(parents=True, exist_ok=True)

        output_path = output_dir / f"{args.source_name}.elf"
        output_path.write_bytes(data)
        print(f"ELF extracted to: {output_path}")

        hashes = calculate_hashes(data)

        import json
        report = build_report(args, output_path, hashes)
        report_path = output_dir / f"{args.source_name}-extract-report.json"
        report_path.write_text(json.dumps(report, indent=2) + "\n")
        print(f"Extraction report written to: {report_path}")

        print(f"\nSHA256: {hashes['sha256']}")
        print(f"MD5: {hashes['md5']}")
        print(f"\nLegal notice: Extracted ELF is for local analysis only. Do not distribute.")

    except Exception as error:
        print(f"error: {error}")
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())