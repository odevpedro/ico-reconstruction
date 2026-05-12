#!/usr/bin/env python3
"""Generate a metadata-only exact reference report for SCUS_971.13."""

from __future__ import annotations

import argparse
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


TOOL_NAME = "exe-ref-index"
TOOL_VERSION = "0.1.0"
PAYLOAD_SIZE = 2048


SECTION_TYPES = {
    0: "SHT_NULL",
    1: "SHT_PROGBITS",
    2: "SHT_SYMTAB",
    3: "SHT_STRTAB",
    8: "SHT_NOBITS",
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
                "memory_size": int.from_bytes(entry[20:24], "little"),
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
            value = header["virtual_address"] + (offset - start)
            return f"0x{value:08x}"
    return None


def containing_section(offset: int, sections: list[dict[str, Any]]) -> dict[str, Any] | None:
    for section in sections:
        if section["size"] == 0 or section["type"] == 8:
            continue
        if section["offset"] <= offset < section["offset"] + section["size"]:
            return {
                "section_index": section["index"],
                "section_name": section.get("name", ""),
                "section_type": section["type_name"],
            }
    return None


def find_all(data: bytes, needle: bytes) -> list[int]:
    if not needle:
        return []
    offsets: list[int] = []
    start = 0
    while True:
        found = data.find(needle, start)
        if found == -1:
            return offsets
        offsets.append(found)
        start = found + 1


def match_record(offset: int, program_headers: list[dict[str, Any]], sections: list[dict[str, Any]]) -> dict[str, Any]:
    record: dict[str, Any] = {
        "file_offset": offset,
        "file_offset_hex": f"0x{offset:08x}",
        "virtual_address": file_offset_to_virtual(offset, program_headers),
    }
    section = containing_section(offset, sections)
    if section:
        record.update(section)
    return record


def query_matches(
    data: bytes,
    queries: list[str],
    program_headers: list[dict[str, Any]],
    sections: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    output: list[dict[str, Any]] = []
    for query in queries:
        needle = query.encode("ascii")
        offsets = find_all(data, needle)
        output.append(
            {
                "query": query,
                "encoding": "ascii",
                "match_count": len(offsets),
                "matches": [match_record(offset, program_headers, sections) for offset in offsets],
            }
        )
    return output


def constant_matches(
    data: bytes,
    constants: list[int],
    program_headers: list[dict[str, Any]],
    sections: list[dict[str, Any]],
) -> list[dict[str, Any]]:
    output: list[dict[str, Any]] = []
    for value in constants:
        encodings = []
        for endian in ("little", "big"):
            needle = value.to_bytes(4, endian, signed=False)
            offsets = find_all(data, needle)
            encodings.append(
                {
                    "endian": endian,
                    "match_count": len(offsets),
                    "matches": [
                        match_record(offset, program_headers, sections) for offset in offsets
                    ],
                }
            )
        output.append({"value": value, "hex": f"0x{value:08x}", "encodings": encodings})
    return output


def build_report(args: argparse.Namespace) -> dict[str, Any]:
    image = args.image.expanduser().resolve()
    if not image.exists():
        raise FileNotFoundError(f"Image path does not exist: {args.image}")
    data = read_image_region(image, args.lba, args.size, args.sector_size, args.data_offset)
    header = parse_elf_header(data)
    program_headers = parse_program_headers(data, header)
    sections = parse_sections(data, header)
    return {
        "tool": TOOL_NAME,
        "tool_version": TOOL_VERSION,
        "generated_at_utc": utc_now(),
        "legal_boundary": "metadata-only; exact query/constant references only",
        "source": {
            "mode": "disc-image-region",
            "image_path": str(image),
            "source_name": args.source_name,
            "extent_lba": args.lba,
            "size_bytes": args.size,
            "sector_size": args.sector_size,
            "data_offset": args.data_offset,
        },
        "elf_header": header,
        "summary": {
            "query_count": len(args.query),
            "constant_count": len(args.constant),
        },
        "string_queries": query_matches(data, args.query, program_headers, sections),
        "constants": constant_matches(data, args.constant, program_headers, sections),
    }


def safe_report_name(args: argparse.Namespace) -> str:
    stem = args.source_name or args.image.name
    safe_stem = "".join(char if char.isalnum() or char in ("-", "_", ".") else "_" for char in stem)
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    return f"{timestamp}-{safe_stem}-exe-ref-index.json"


def write_report(report: dict[str, Any], output_dir: Path, args: argparse.Namespace) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    report_path = output_dir / safe_report_name(args)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return report_path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate a metadata-only executable reference report.")
    parser.add_argument("--image", type=Path, required=True, help="Local disc image containing the ELF.")
    parser.add_argument("--lba", type=int, required=True, help="Start LBA of the ELF.")
    parser.add_argument("--size", type=int, required=True, help="Size in bytes of the ELF.")
    parser.add_argument("--sector-size", type=int, default=2352, help="Physical sector size. Default: 2352.")
    parser.add_argument("--data-offset", type=int, default=24, help="Data payload offset. Default: 24.")
    parser.add_argument("--source-name", default="SCUS_971.13", help="Display name for the source.")
    parser.add_argument("--query", action="append", default=[], help="ASCII string query. Repeatable.")
    parser.add_argument(
        "--constant",
        action="append",
        type=lambda value: int(value, 0),
        default=[],
        help="Unsigned 32-bit constant to search. Accepts decimal or 0x-prefixed values. Repeatable.",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(".local/reports"),
        help="Directory for the generated JSON report. Default: .local/reports",
    )
    args = parser.parse_args()
    if not args.query and not args.constant:
        parser.error("At least one --query or --constant is required.")
    return args


def main() -> int:
    args = parse_args()
    try:
        report = build_report(args)
        report_path = write_report(report, args.output_dir, args)
    except Exception as error:
        print(f"error: {error}")
        return 1
    print(f"exe ref index written: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
