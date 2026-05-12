#!/usr/bin/env python3
"""Generate a metadata-only ELF32 index for a local file or disc-image region."""

from __future__ import annotations

import argparse
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


TOOL_NAME = "elf-index"
TOOL_VERSION = "0.1.0"
PAYLOAD_SIZE = 2048


SECTION_TYPES = {
    0: "SHT_NULL",
    1: "SHT_PROGBITS",
    2: "SHT_SYMTAB",
    3: "SHT_STRTAB",
    4: "SHT_RELA",
    8: "SHT_NOBITS",
    9: "SHT_REL",
    11: "SHT_DYNSYM",
}


PROGRAM_TYPES = {
    0: "PT_NULL",
    1: "PT_LOAD",
    2: "PT_DYNAMIC",
    3: "PT_INTERP",
    4: "PT_NOTE",
    5: "PT_SHLIB",
    6: "PT_PHDR",
}


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
        "elf_class": "ELF32",
        "endian": "little",
        "type": int.from_bytes(data[16:18], "little"),
        "machine": int.from_bytes(data[18:20], "little"),
        "version": int.from_bytes(data[20:24], "little"),
        "entry_point": f"0x{int.from_bytes(data[24:28], 'little'):08x}",
        "program_header_offset": int.from_bytes(data[28:32], "little"),
        "section_header_offset": int.from_bytes(data[32:36], "little"),
        "flags": f"0x{int.from_bytes(data[36:40], 'little'):08x}",
        "elf_header_size": int.from_bytes(data[40:42], "little"),
        "program_header_entry_size": int.from_bytes(data[42:44], "little"),
        "program_header_count": int.from_bytes(data[44:46], "little"),
        "section_header_entry_size": int.from_bytes(data[46:48], "little"),
        "section_header_count": int.from_bytes(data[48:50], "little"),
        "section_name_string_table_index": int.from_bytes(data[50:52], "little"),
    }


def parse_program_headers(data: bytes, header: dict[str, Any]) -> list[dict[str, Any]]:
    offset = header["program_header_offset"]
    entry_size = header["program_header_entry_size"]
    count = header["program_header_count"]
    headers: list[dict[str, Any]] = []
    for index in range(count):
        start = offset + index * entry_size
        entry = data[start : start + entry_size]
        if len(entry) < 32:
            raise RuntimeError(f"Program header {index} is truncated.")
        p_type = int.from_bytes(entry[0:4], "little")
        headers.append(
            {
                "index": index,
                "type": p_type,
                "type_name": PROGRAM_TYPES.get(p_type, f"PT_UNKNOWN_{p_type}"),
                "offset": int.from_bytes(entry[4:8], "little"),
                "virtual_address": f"0x{int.from_bytes(entry[8:12], 'little'):08x}",
                "physical_address": f"0x{int.from_bytes(entry[12:16], 'little'):08x}",
                "file_size": int.from_bytes(entry[16:20], "little"),
                "memory_size": int.from_bytes(entry[20:24], "little"),
                "flags": f"0x{int.from_bytes(entry[24:28], 'little'):08x}",
                "alignment": int.from_bytes(entry[28:32], "little"),
            }
        )
    return headers


def parse_raw_section_headers(data: bytes, header: dict[str, Any]) -> list[dict[str, Any]]:
    offset = header["section_header_offset"]
    entry_size = header["section_header_entry_size"]
    count = header["section_header_count"]
    sections: list[dict[str, Any]] = []
    for index in range(count):
        start = offset + index * entry_size
        entry = data[start : start + entry_size]
        if len(entry) < 40:
            raise RuntimeError(f"Section header {index} is truncated.")
        section_type = int.from_bytes(entry[4:8], "little")
        sections.append(
            {
                "index": index,
                "name_offset": int.from_bytes(entry[0:4], "little"),
                "type": section_type,
                "type_name": SECTION_TYPES.get(section_type, f"SHT_UNKNOWN_{section_type}"),
                "flags": f"0x{int.from_bytes(entry[8:12], 'little'):08x}",
                "address": f"0x{int.from_bytes(entry[12:16], 'little'):08x}",
                "offset": int.from_bytes(entry[16:20], "little"),
                "size": int.from_bytes(entry[20:24], "little"),
                "link": int.from_bytes(entry[24:28], "little"),
                "info": int.from_bytes(entry[28:32], "little"),
                "alignment": int.from_bytes(entry[32:36], "little"),
                "entry_size": int.from_bytes(entry[36:40], "little"),
            }
        )
    return sections


def add_section_names(data: bytes, header: dict[str, Any], sections: list[dict[str, Any]]) -> None:
    shstr_index = header["section_name_string_table_index"]
    if shstr_index >= len(sections):
        return
    table_section = sections[shstr_index]
    start = table_section["offset"]
    end = start + table_section["size"]
    table = data[start:end]
    for section in sections:
        section["name"] = read_c_string(table, section["name_offset"])


def symbol_table_summary(sections: list[dict[str, Any]]) -> list[dict[str, Any]]:
    summaries: list[dict[str, Any]] = []
    for section in sections:
        if section["type"] not in (2, 11):
            continue
        entry_size = section["entry_size"]
        count = section["size"] // entry_size if entry_size else 0
        linked_name = ""
        if section["link"] < len(sections):
            linked_name = sections[section["link"]].get("name", "")
        summaries.append(
            {
                "section_index": section["index"],
                "section_name": section.get("name", ""),
                "type_name": section["type_name"],
                "entry_size": entry_size,
                "entry_count": count,
                "linked_string_table_index": section["link"],
                "linked_string_table_name": linked_name,
            }
        )
    return summaries


def load_input(args: argparse.Namespace) -> tuple[bytes, dict[str, Any]]:
    if args.elf:
        resolved = args.elf.expanduser().resolve()
        if not resolved.exists():
            raise FileNotFoundError(f"ELF path does not exist: {args.elf}")
        return resolved.read_bytes(), {"mode": "direct-file", "path": str(resolved), "size_bytes": resolved.stat().st_size}

    image = args.image.expanduser().resolve()
    if not image.exists():
        raise FileNotFoundError(f"Image path does not exist: {args.image}")
    data = read_image_region(image, args.lba, args.size, args.sector_size, args.data_offset)
    return data, {
        "mode": "disc-image-region",
        "image_path": str(image),
        "source_name": args.source_name,
        "extent_lba": args.lba,
        "size_bytes": args.size,
        "sector_size": args.sector_size,
        "data_offset": args.data_offset,
    }


def build_report(args: argparse.Namespace) -> dict[str, Any]:
    data, source = load_input(args)
    header = parse_elf_header(data)
    program_headers = parse_program_headers(data, header)
    sections = parse_raw_section_headers(data, header)
    add_section_names(data, header, sections)

    return {
        "tool": TOOL_NAME,
        "tool_version": TOOL_VERSION,
        "generated_at_utc": utc_now(),
        "legal_boundary": "metadata-only; no executable contents extracted into this report",
        "source": source,
        "elf_header": header,
        "summary": {
            "program_header_count": len(program_headers),
            "section_header_count": len(sections),
            "symbol_table_count": len(symbol_table_summary(sections)),
            "load_segment_count": sum(1 for item in program_headers if item["type_name"] == "PT_LOAD"),
        },
        "program_headers": program_headers,
        "sections": sections,
        "symbol_tables": symbol_table_summary(sections),
    }


def safe_report_name(args: argparse.Namespace) -> str:
    if args.elf:
        stem = args.elf.name
    else:
        stem = args.source_name or args.image.name
    safe_stem = "".join(char if char.isalnum() or char in ("-", "_", ".") else "_" for char in stem)
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    return f"{timestamp}-{safe_stem}-elf-index.json"


def write_report(report: dict[str, Any], output_dir: Path, args: argparse.Namespace) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    report_path = output_dir / safe_report_name(args)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return report_path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate a metadata-only ELF32 index.")
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--elf", type=Path, help="Direct local ELF file to inspect.")
    source.add_argument("--image", type=Path, help="Local disc image containing the ELF region.")
    parser.add_argument("--lba", type=int, help="Start LBA of the ELF inside --image.")
    parser.add_argument("--size", type=int, help="Size in bytes of the ELF inside --image.")
    parser.add_argument("--sector-size", type=int, default=2352, help="Physical sector size. Default: 2352.")
    parser.add_argument("--data-offset", type=int, default=24, help="Data payload offset in each sector. Default: 24.")
    parser.add_argument("--source-name", help="Display name for an embedded ELF source.")
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(".local/reports"),
        help="Directory for the generated JSON report. Default: .local/reports",
    )
    args = parser.parse_args()
    if args.image and (args.lba is None or args.size is None):
        parser.error("--image requires --lba and --size")
    return args


def main() -> int:
    args = parse_args()
    try:
        report = build_report(args)
        report_path = write_report(report, args.output_dir, args)
    except Exception as error:
        print(f"error: {error}")
        return 1
    print(f"elf index written: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
