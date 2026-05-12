#!/usr/bin/env python3
"""Enhanced ELF symbol table scanner for SCUS_971.13.

This tool performs deeper analysis to find symbol tables that may be:
- .symtab (standard symbol table)
- .dynsym (dynamic symbol table)
- Stripped or omitted symbols
- Alternative section types
"""

from __future__ import annotations

import argparse
import json
import struct
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


TOOL_NAME = "elf-symbol-scan"
TOOL_VERSION = "0.1.0"
PAYLOAD_SIZE = 2048

SHT_SYMTAB = 2
SHT_DYNSYM = 11
SHT_STRTAB = 3
SHT_HASH = 5
SHT_DYNHASH = 9


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
        "entry_point": int.from_bytes(data[24:28], "little"),
        "program_header_offset": int.from_bytes(data[28:32], "little"),
        "section_header_offset": int.from_bytes(data[32:36], "little"),
        "program_header_entry_size": int.from_bytes(data[42:44], "little"),
        "program_header_count": int.from_bytes(data[44:46], "little"),
        "section_header_entry_size": int.from_bytes(data[46:48], "little"),
        "section_header_count": int.from_bytes(data[48:50], "little"),
        "section_name_string_table_index": int.from_bytes(data[50:52], "little"),
    }


def parse_sections(data: bytes, header: dict[str, Any]) -> list[dict[str, Any]]:
    sections: list[dict[str, Any]] = []
    for index in range(header["section_header_count"]):
        start = header["section_header_offset"] + index * header["section_header_entry_size"]
        entry = data[start : start + header["section_header_entry_size"]]
        if len(entry) < 40:
            raise RuntimeError(f"Section header {index} is truncated.")
        sections.append(
            {
                "index": index,
                "name_offset": int.from_bytes(entry[0:4], "little"),
                "type": int.from_bytes(entry[4:8], "little"),
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
        name_offset = section["name_offset"]
        end = table.find(b"\x00", name_offset)
        if end == -1:
            end = len(table)
        section["name"] = table[name_offset:end].decode("ascii", errors="replace")


def section_type_name(type_id: int) -> str:
    names = {
        0: "SHT_NULL",
        1: "SHT_PROGBITS",
        2: "SHT_SYMTAB",
        3: "SHT_STRTAB",
        4: "SHT_RELA",
        5: "SHT_HASH",
        6: "SHT_NOTE",
        7: "SHT_NOBITS",
        8: "SHT_REL",
        9: "SHT_DYNHASH",
        10: "SHT_LOPROC",
        11: "SHT_DYNSYM",
        12: "SHT_HIPROC",
        14: "SHT_NUM",
    }
    return names.get(type_id, f"SHT_UNKNOWN_{type_id}")


def find_symbol_table(data: bytes, sections: list[dict[str, Any]]) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []

    for section in sections:
        if section["size"] == 0:
            continue
        section_type = section["type"]

        if section_type == SHT_SYMTAB:
            results.append(
                {
                    "section_name": section.get("name", ""),
                    "section_index": section["index"],
                    "section_type": section_type,
                    "section_type_name": "SHT_SYMTAB",
                    "file_offset": section["offset"],
                    "virtual_address": f"0x{section['address']:08x}",
                    "size_bytes": section["size"],
                    "estimated_entry_count": section["size"] // 16,
                    "status": "found",
                    "note": "Standard symbol table (usually stripped in release builds)",
                }
            )

        elif section_type == SHT_DYNSYM:
            results.append(
                {
                    "section_name": section.get("name", ""),
                    "section_index": section["index"],
                    "section_type": section_type,
                    "section_type_name": "SHT_DYNSYM",
                    "file_offset": section["offset"],
                    "virtual_address": f"0x{section['address']:08x}",
                    "size_bytes": section["size"],
                    "estimated_entry_count": section["size"] // 12,
                    "status": "found",
                    "note": "Dynamic symbol table (for runtime linking)",
                }
            )

        elif section_type == SHT_HASH or section_type == SHT_DYNHASH:
            results.append(
                {
                    "section_name": section.get("name", ""),
                    "section_index": section["index"],
                    "section_type": section_type,
                    "section_type_name": section_type_name(section_type),
                    "file_offset": section["offset"],
                    "virtual_address": f"0x{section['address']:08x}",
                    "size_bytes": section["size"],
                    "status": "potential_symbol_index",
                    "note": "Hash table - may index dynamic symbols",
                }
            )

    return results


def scan_for_symbol_strings(data: bytes, sections: list[dict[str, Any]]) -> list[dict[str, Any]]:
    potential_strings: list[dict[str, Any]] = []

    for section in sections:
        if section["type"] != SHT_STRTAB:
            continue

        name = section.get("name", "")
        if name in (".shstrtab", ".dynstr"):
            continue

        if section["size"] < 64 or section["size"] > 1024 * 1024:
            continue

        try:
            str_data = data[section["offset"] : section["offset"] + section["size"]]
        except Exception:
            continue

        c_strings = []
        pos = 0
        while pos < len(str_data) - 8:
            if str_data[pos] == 0:
                pos += 1
                continue

            end = str_data.find(b"\x00", pos)
            if end == -1:
                break

            s = str_data[pos:end]
            if 3 <= len(s) <= 64 and all(32 <= b <= 126 for b in s):
                c_strings.append(s.decode("ascii"))

            pos = end + 1

        if len(c_strings) >= 3:
            potential_strings.append(
                {
                    "section_name": name,
                    "section_index": section["index"],
                    "file_offset": section["offset"],
                    "size_bytes": section["size"],
                    "string_count": len(c_strings),
                    "sample_strings": c_strings[:10],
                    "note": "Potential symbol string table",
                }
            )

    return potential_strings


def check_program_headers_for_dynamic(data: bytes, header: dict[str, Any]) -> list[dict[str, Any]]:
    dynamic: list[dict[str, Any]] = []

    for index in range(header["program_header_count"]):
        start = header["program_header_offset"] + index * header["program_header_entry_size"]
        entry = data[start : start + header["program_header_entry_size"]]
        if len(entry) < 32:
            continue

        p_type = int.from_bytes(entry[0:4], "little")
        p_offset = int.from_bytes(entry[4:8], "little")
        p_vaddr = int.from_bytes(entry[8:12], "little")
        p_filesz = int.from_bytes(entry[16:20], "little")

        if p_type == 2:
            dynamic.append(
                {
                    "type": "PT_DYNAMIC",
                    "file_offset": p_offset,
                    "virtual_address": f"0x{p_vaddr:08x}",
                    "size_bytes": p_filesz,
                    "status": "found",
                    "note": "Dynamic linking information present",
                }
            )

    return dynamic


def build_report(args: argparse.Namespace) -> dict[str, Any]:
    image = args.image.expanduser().resolve()
    if not image.exists():
        raise FileNotFoundError(f"Image path does not exist: {args.image}")

    data = read_image_region(image, args.lba, args.size, args.sector_size, args.data_offset)
    header = parse_elf_header(data)
    sections = parse_sections(data, header)

    symbol_tables = find_symbol_table(data, sections)
    potential_strings = scan_for_symbol_strings(data, sections) if args.scan_strings else []
    dynamic_segments = check_program_headers_for_dynamic(data, header)

    return {
        "tool": TOOL_NAME,
        "tool_version": TOOL_VERSION,
        "generated_at_utc": utc_now(),
        "legal_boundary": "metadata-only; no executable contents extracted",
        "source": {
            "mode": "disc-image-region",
            "image_path": str(image),
            "source_name": args.source_name,
            "extent_lba": args.lba,
            "size_bytes": args.size,
            "sector_size": args.sector_size,
            "data_offset": args.data_offset,
        },
        "summary": {
            "symbol_tables_found": len(symbol_tables),
            "potential_string_tables": len(potential_strings),
            "dynamic_segments": len(dynamic_segments),
        },
        "symbol_tables": symbol_tables,
        "potential_string_tables": potential_strings,
        "dynamic_segments": dynamic_segments,
    }


def safe_report_name(args: argparse.Namespace) -> str:
    stem = args.source_name or args.image.name
    safe_stem = "".join(char if char.isalnum() or char in ("-", "_", ".") else "_" for char in stem)
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    return f"{timestamp}-{safe_stem}-elf-symbol-scan.json"


def write_report(report: dict[str, Any], output_dir: Path, args: argparse.Namespace) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    report_path = output_dir / safe_report_name(args)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return report_path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Enhanced ELF symbol table scanner for PS2 executables."
    )
    parser.add_argument("--image", type=Path, required=True, help="Local disc image containing the ELF.")
    parser.add_argument("--lba", type=int, required=True, help="Start LBA of the ELF.")
    parser.add_argument("--size", type=int, required=True, help="Size in bytes of the ELF.")
    parser.add_argument("--sector-size", type=int, default=2352, help="Physical sector size. Default: 2352.")
    parser.add_argument("--data-offset", type=int, default=24, help="Data payload offset. Default: 24.")
    parser.add_argument("--source-name", default="SCUS_971.13", help="Display name for the source.")
    parser.add_argument(
        "--scan-strings",
        action="store_true",
        help="Scan for potential symbol string tables (slower).",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(".local/reports"),
        help="Directory for the generated JSON report. Default: .local/reports",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    try:
        report = build_report(args)
        report_path = write_report(report, args.output_dir, args)
    except Exception as error:
        print(f"error: {error}")
        return 1
    print(f"elf symbol scan report written: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())