#!/usr/bin/env python3
"""Generate a metadata-only report for ICO .DVP.* ELF sections."""

from __future__ import annotations

import argparse
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


TOOL_NAME = "dvp-index"
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
    headers: list[dict[str, Any]] = []
    offset = header["program_header_offset"]
    entry_size = header["program_header_entry_size"]
    for index in range(header["program_header_count"]):
        start = offset + index * entry_size
        entry = data[start : start + entry_size]
        if len(entry) < 32:
            raise RuntimeError(f"Program header {index} is truncated.")
        p_type = int.from_bytes(entry[0:4], "little")
        virtual_address = int.from_bytes(entry[8:12], "little")
        memory_size = int.from_bytes(entry[20:24], "little")
        headers.append(
            {
                "index": index,
                "type": p_type,
                "offset": int.from_bytes(entry[4:8], "little"),
                "virtual_address": virtual_address,
                "virtual_address_hex": f"0x{virtual_address:08x}",
                "memory_size": memory_size,
                "memory_end": virtual_address + memory_size,
                "memory_end_hex": f"0x{virtual_address + memory_size:08x}",
            }
        )
    return headers


def load_ranges(program_headers: list[dict[str, Any]]) -> list[dict[str, int]]:
    return [
        {"start": header["virtual_address"], "end": header["memory_end"]}
        for header in program_headers
        if header["type"] == 1
    ]


def parse_sections(data: bytes, header: dict[str, Any]) -> list[dict[str, Any]]:
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


def section_bytes(data: bytes, section: dict[str, Any]) -> bytes:
    start = section["offset"]
    end = start + section["size"]
    return data[start:end]


def dvp_sections(sections: list[dict[str, Any]]) -> list[dict[str, Any]]:
    return [section for section in sections if section.get("name", "").startswith(".DVP.")]


def strtab_stats(data: bytes, section: dict[str, Any] | None) -> dict[str, Any] | None:
    if not section:
        return None
    raw = section_bytes(data, section)
    strings = [part.decode("ascii", errors="replace") for part in raw.split(b"\x00") if part]
    lengths = [len(item) for item in strings]
    return {
        "section_index": section["index"],
        "size": section["size"],
        "string_count": len(strings),
        "min_length": min(lengths) if lengths else 0,
        "max_length": max(lengths) if lengths else 0,
        "total_string_bytes": sum(lengths),
    }


def value_correlation(
    value: int,
    data_df_size: int | None,
    ranges: list[dict[str, int]] | None = None,
) -> dict[str, Any]:
    correlation: dict[str, Any] = {
        "value": value,
        "hex": f"0x{value:08x}",
    }
    if data_df_size is not None:
        correlation["within_data_df_size"] = 0 <= value < data_df_size
        correlation["data_df_2048_aligned"] = value % 2048 == 0
        correlation["data_df_16_aligned"] = value % 16 == 0
    if ranges is not None:
        correlation["within_elf_load_range"] = any(
            item["start"] <= value < item["end"] for item in ranges
        )
    return correlation


def parse_ovlytab(
    data: bytes,
    section: dict[str, Any] | None,
    data_df_size: int | None,
    ranges: list[dict[str, int]],
) -> dict[str, Any] | None:
    if not section:
        return None
    raw = section_bytes(data, section)
    entry_size = section["entry_size"] or 12
    entries: list[dict[str, Any]] = []
    for index, offset in enumerate(range(0, len(raw) - entry_size + 1, entry_size)):
        entry = raw[offset : offset + entry_size]
        words = [int.from_bytes(entry[pos : pos + 4], "little") for pos in range(0, entry_size, 4)]
        entries.append(
            {
                "index": index,
                "entry_offset": offset,
                "words": [value_correlation(value, data_df_size, ranges) for value in words],
            }
        )
    values = [word["value"] for entry in entries for word in entry["words"]]
    within_count = 0
    aligned_2048_count = 0
    elf_load_range_count = sum(
        1 for value in values for item in ranges if item["start"] <= value < item["end"]
    )
    if data_df_size is not None:
        within_count = sum(1 for value in values if 0 <= value < data_df_size)
        aligned_2048_count = sum(1 for value in values if value % 2048 == 0)
    return {
        "section_index": section["index"],
        "offset": section["offset"],
        "size": section["size"],
        "entry_size": entry_size,
        "entry_count": len(entries),
        "value_count": len(values),
        "values_within_data_df_size": within_count,
        "values_2048_aligned": aligned_2048_count,
        "values_within_elf_load_range": elf_load_range_count,
        "entries": entries,
    }


def parse_overlay_name_numbers(name: str) -> list[int]:
    numbers: list[int] = []
    for token in name.replace(".", " ").split():
        if token.startswith("0x"):
            try:
                numbers.append(int(token, 16))
            except ValueError:
                pass
        elif token.isdigit():
            numbers.append(int(token))
    return numbers


def overlay_section_summary(
    sections: list[dict[str, Any]],
    data_df_size: int | None,
    ranges: list[dict[str, int]],
) -> list[dict[str, Any]]:
    output: list[dict[str, Any]] = []
    for section in sections:
        name = section.get("name", "")
        if not name.startswith(".DVP.overlay"):
            continue
        numbers = parse_overlay_name_numbers(name)
        output.append(
            {
                "section_index": section["index"],
                "name": name,
                "type_name": section["type_name"],
                "flags": section["flags"],
                "elf_offset": section["offset"],
                "size": section["size"],
                "alignment": section["alignment"],
                "numeric_tokens": [
                    value_correlation(value, data_df_size, ranges) for value in numbers
                ],
            }
        )
    return output


def build_report(args: argparse.Namespace) -> dict[str, Any]:
    image = args.image.expanduser().resolve()
    if not image.exists():
        raise FileNotFoundError(f"Image path does not exist: {args.image}")
    data = read_image_region(image, args.lba, args.size, args.sector_size, args.data_offset)
    header = parse_elf_header(data)
    program_headers = parse_program_headers(data, header)
    ranges = load_ranges(program_headers)
    sections = parse_sections(data, header)
    dvp = dvp_sections(sections)
    ovlytab = next((section for section in dvp if section.get("name") == ".DVP.ovlytab"), None)
    ovlystrtab = next((section for section in dvp if section.get("name") == ".DVP.ovlystrtab"), None)
    overlays = overlay_section_summary(dvp, args.data_df_size, ranges)

    return {
        "tool": TOOL_NAME,
        "tool_version": TOOL_VERSION,
        "generated_at_utc": utc_now(),
        "legal_boundary": "metadata-only; no executable, overlay, or archive contents extracted",
        "source": {
            "mode": "disc-image-region",
            "image_path": str(image),
            "source_name": args.source_name,
            "extent_lba": args.lba,
            "size_bytes": args.size,
            "sector_size": args.sector_size,
            "data_offset": args.data_offset,
        },
        "data_df_context": {
            "size_bytes": args.data_df_size,
        },
        "elf_header": header,
        "load_ranges": [
            {"start": f"0x{item['start']:08x}", "end": f"0x{item['end']:08x}"}
            for item in ranges
        ],
        "summary": {
            "dvp_section_count": len(dvp),
            "dvp_overlay_section_count": len(overlays),
            "has_ovlytab": ovlytab is not None,
            "has_ovlystrtab": ovlystrtab is not None,
        },
        "dvp_sections": [
            {
                "index": section["index"],
                "name": section.get("name", ""),
                "type_name": section["type_name"],
                "offset": section["offset"],
                "size": section["size"],
                "entry_size": section["entry_size"],
                "link": section["link"],
                "alignment": section["alignment"],
            }
            for section in dvp
        ],
        "ovlystrtab": strtab_stats(data, ovlystrtab),
        "ovlytab": parse_ovlytab(data, ovlytab, args.data_df_size, ranges),
        "overlay_sections": overlays,
    }


def safe_report_name(args: argparse.Namespace) -> str:
    stem = args.source_name or args.image.name
    safe_stem = "".join(char if char.isalnum() or char in ("-", "_", ".") else "_" for char in stem)
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    return f"{timestamp}-{safe_stem}-dvp-index.json"


def write_report(report: dict[str, Any], output_dir: Path, args: argparse.Namespace) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    report_path = output_dir / safe_report_name(args)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return report_path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate a metadata-only .DVP.* overlay report.")
    parser.add_argument("--image", type=Path, required=True, help="Local disc image containing the ELF.")
    parser.add_argument("--lba", type=int, required=True, help="Start LBA of the ELF.")
    parser.add_argument("--size", type=int, required=True, help="Size in bytes of the ELF.")
    parser.add_argument("--sector-size", type=int, default=2352, help="Physical sector size. Default: 2352.")
    parser.add_argument("--data-offset", type=int, default=24, help="Data payload offset. Default: 24.")
    parser.add_argument("--source-name", default="SCUS_971.13", help="Display name for the source.")
    parser.add_argument("--data-df-size", type=int, help="Known size of DFDATAS/DATA.DF for correlation.")
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
    print(f"dvp index written: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
