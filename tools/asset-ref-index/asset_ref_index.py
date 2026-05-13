#!/usr/bin/env python3
"""Generate a metadata-only asset reference index from a local ICO executable."""

from __future__ import annotations

import argparse
import json
import re
from collections import Counter
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


TOOL_NAME = "asset-ref-index"
TOOL_VERSION = "0.1.0"
PAYLOAD_SIZE = 2048

KNOWN_EXTENSIONS = {
    ".bga",
    ".bin",
    ".cam",
    ".df",
    ".gcm",
    ".irx",
    ".mob",
    ".p2o",
    ".smb",
    ".tm2",
}


SECTION_TYPES = {
    0: "SHT_NULL",
    1: "SHT_PROGBITS",
    2: "SHT_SYMTAB",
    3: "SHT_STRTAB",
    8: "SHT_NOBITS",
}


PRINTABLE_RE = re.compile(rb"[\x20-\x7e]{4,}")


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


def read_c_string(table: bytes, offset: int) -> str:
    if offset < 0 or offset >= len(table):
        return ""
    end = table.find(b"\x00", offset)
    if end == -1:
        end = len(table)
    return table[offset:end].decode("ascii", errors="replace")


def parse_elf_header(data: bytes) -> dict[str, Any]:
    if len(data) < 52 or data[:4] != b"\x7fELF":
        raise RuntimeError("Input is not an ELF file.")
    if data[4] != 1 or data[5] != 1:
        raise RuntimeError("Only ELF32 little-endian inputs are supported.")
    return {
        "entry_point": f"0x{int.from_bytes(data[24:28], 'little'):08x}",
        "program_header_offset": int.from_bytes(data[28:32], "little"),
        "section_header_offset": int.from_bytes(data[32:36], "little"),
        "program_header_entry_size": int.from_bytes(data[42:44], "little"),
        "program_header_count": int.from_bytes(data[44:46], "little"),
        "section_header_entry_size": int.from_bytes(data[46:48], "little"),
        "section_header_count": int.from_bytes(data[48:50], "little"),
        "section_name_string_table_index": int.from_bytes(data[50:52], "little"),
    }


def parse_program_headers(data: bytes, header: dict[str, Any]) -> list[dict[str, Any]]:
    output: list[dict[str, Any]] = []
    for index in range(header["program_header_count"]):
        start = header["program_header_offset"] + index * header["program_header_entry_size"]
        entry = data[start : start + header["program_header_entry_size"]]
        if len(entry) < 32:
            raise RuntimeError(f"Program header {index} is truncated.")
        output.append(
            {
                "index": index,
                "type": int.from_bytes(entry[0:4], "little"),
                "file_offset": int.from_bytes(entry[4:8], "little"),
                "virtual_address": int.from_bytes(entry[8:12], "little"),
                "file_size": int.from_bytes(entry[16:20], "little"),
            }
        )
    return output


def parse_sections(data: bytes, header: dict[str, Any]) -> list[dict[str, Any]]:
    sections: list[dict[str, Any]] = []
    for index in range(header["section_header_count"]):
        start = header["section_header_offset"] + index * header["section_header_entry_size"]
        entry = data[start : start + header["section_header_entry_size"]]
        if len(entry) < 40:
            raise RuntimeError(f"Section header {index} is truncated.")
        section_type = int.from_bytes(entry[4:8], "little")
        sections.append(
            {
                "index": index,
                "name_offset": int.from_bytes(entry[0:4], "little"),
                "type": section_type,
                "type_name": SECTION_TYPES.get(section_type, f"SHT_UNKNOWN_{section_type}"),
                "address": int.from_bytes(entry[12:16], "little"),
                "offset": int.from_bytes(entry[16:20], "little"),
                "size": int.from_bytes(entry[20:24], "little"),
            }
        )
    add_section_names(data, header, sections)
    return sections


def add_section_names(data: bytes, header: dict[str, Any], sections: list[dict[str, Any]]) -> None:
    table_index = header["section_name_string_table_index"]
    if table_index >= len(sections):
        return
    table_section = sections[table_index]
    table = data[table_section["offset"] : table_section["offset"] + table_section["size"]]
    for section in sections:
        section["name"] = read_c_string(table, section["name_offset"])


def file_offset_to_virtual(offset: int, program_headers: list[dict[str, Any]]) -> str | None:
    for header in program_headers:
        if header["type"] != 1:
            continue
        start = header["file_offset"]
        end = start + header["file_size"]
        if start <= offset < end:
            return f"0x{header['virtual_address'] + (offset - start):08x}"
    return None


def containing_section(offset: int, sections: list[dict[str, Any]]) -> dict[str, Any] | None:
    for section in sections:
        if section["size"] == 0 or section["type"] == 8:
            continue
        if section["offset"] <= offset < section["offset"] + section["size"]:
            return section
    return None


def classify_reference(value: str) -> dict[str, str]:
    lower = value.lower()
    extension = ""
    for candidate in sorted(KNOWN_EXTENSIONS, key=len, reverse=True):
        if lower.endswith(candidate):
            extension = candidate
            break
    prefix = value.split("/", 1)[0] if "/" in value else ""
    return {"extension": extension, "prefix": prefix}


def is_asset_reference(value: str) -> bool:
    if "/" not in value and "." not in value:
        return False
    lower = value.lower()
    return any(lower.endswith(extension) for extension in KNOWN_EXTENSIONS)


def scan_asset_references(
    data: bytes,
    program_headers: list[dict[str, Any]],
    sections: list[dict[str, Any]],
    max_records: int,
) -> dict[str, Any]:
    records: list[dict[str, Any]] = []
    seen: set[tuple[int, str]] = set()
    for match in PRINTABLE_RE.finditer(data):
        value = match.group(0).decode("ascii", errors="replace")
        if not is_asset_reference(value):
            continue
        offset = match.start()
        key = (offset, value)
        if key in seen:
            continue
        seen.add(key)
        classification = classify_reference(value)
        section = containing_section(offset, sections)
        record: dict[str, Any] = {
            "value": value,
            "file_offset": offset,
            "file_offset_hex": f"0x{offset:08x}",
            "virtual_address": file_offset_to_virtual(offset, program_headers),
            "extension": classification["extension"],
            "prefix": classification["prefix"],
        }
        if section:
            record["section_index"] = section["index"]
            record["section_name"] = section.get("name", "")
        records.append(record)

    extension_counts = Counter(record["extension"] or "(none)" for record in records)
    prefix_counts = Counter(record["prefix"] or "(none)" for record in records)
    section_counts = Counter(record.get("section_name", "(none)") for record in records)
    records.sort(key=lambda item: (item["extension"], item["value"], item["file_offset"]))

    return {
        "summary": {
            "total_references": len(records),
            "unique_values": len({record["value"] for record in records}),
            "extension_counts": dict(sorted(extension_counts.items())),
            "prefix_counts": dict(sorted(prefix_counts.items())),
            "section_counts": dict(sorted(section_counts.items())),
            "records_included": len(records) if max_records == 0 else min(len(records), max_records),
            "records_truncated": max_records != 0 and len(records) > max_records,
        },
        "records": records if max_records == 0 else records[:max_records],
    }


def load_input(args: argparse.Namespace) -> tuple[bytes, str]:
    if args.elf:
        path = Path(args.elf)
        return path.read_bytes(), str(path)
    image = Path(args.image)
    data = read_image_region(image, args.lba, args.size, args.sector_size, args.data_offset)
    return data, args.source_name


def write_report(report: dict[str, Any], output_dir: Path, source_name: str) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    safe_name = source_name.replace("/", "_").replace(" ", "_")
    path = output_dir / f"{timestamp}-{safe_name}-asset-ref-index.json"
    path.write_text(json.dumps(report, indent=2, sort_keys=True), encoding="utf-8")
    return path


def build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    input_group = parser.add_mutually_exclusive_group(required=True)
    input_group.add_argument("--elf", help="Path to a local extracted ELF file")
    input_group.add_argument("--image", help="Path to a local BIN/ISO image")
    parser.add_argument("--lba", type=int, help="Starting LBA when reading from --image")
    parser.add_argument("--size", type=int, help="Region size when reading from --image")
    parser.add_argument("--sector-size", type=int, default=2352)
    parser.add_argument("--data-offset", type=int, default=24)
    parser.add_argument("--source-name", default="SCUS_971.13")
    parser.add_argument("--max-records", type=int, default=500, help="Maximum records to include, 0 for all")
    parser.add_argument("--output-dir", default=".local/reports")
    return parser


def validate_args(args: argparse.Namespace) -> None:
    if args.image and (args.lba is None or args.size is None):
        raise SystemExit("--image requires --lba and --size")
    if args.max_records < 0:
        raise SystemExit("--max-records must be >= 0")


def main() -> int:
    parser = build_arg_parser()
    args = parser.parse_args()
    validate_args(args)

    data, source_label = load_input(args)
    header = parse_elf_header(data)
    program_headers = parse_program_headers(data, header)
    sections = parse_sections(data, header)
    scan = scan_asset_references(data, program_headers, sections, args.max_records)

    report = {
        "tool": {"name": TOOL_NAME, "version": TOOL_VERSION, "generated_at": utc_now()},
        "source": {
            "name": source_label,
            "input_kind": "elf" if args.elf else "image-region",
            "size": len(data),
        },
        "elf": {"entry_point": header["entry_point"]},
        **scan,
    }
    report_path = write_report(report, Path(args.output_dir), args.source_name)

    print(f"Asset references: {report['summary']['total_references']}")
    print(f"Unique values: {report['summary']['unique_values']}")
    print(f"Report: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
