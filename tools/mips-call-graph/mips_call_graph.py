#!/usr/bin/env python3
"""MIPS call graph analyzer for ICO.

This tool identifies function calls (jal, jalr) and builds a call graph
to find which functions call the known DATA.DF/DFDATAS reference functions.
"""

from __future__ import annotations

import argparse
import json
import struct
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


TOOL_NAME = "mips-call-graph"
TOOL_VERSION = "0.1.0"
PAYLOAD_SIZE = 2048

MIPS_JAL = 0x03
MIPS_JALR = 0x09


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
        "program_header_offset": int.from_bytes(data[28:32], "little"),
        "section_header_offset": int.from_bytes(data[32:36], "little"),
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


def find_containing_function(ref_addr: int, prologues: list[tuple[int, dict[str, Any]]]) -> dict[str, Any] | None:
    best_prologue = None
    best_addr = -1

    for p_addr, p in prologues:
        if p_addr <= ref_addr and p_addr > best_addr:
            best_addr = p_addr
            best_prologue = p

    return best_prologue


def scan_calls(
    data: bytes,
    sections: list[dict[str, Any]],
    prologues: list[dict[str, Any]],
    target_functions: list[int],
) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []

    prologue_data = []
    for p in prologues:
        p_addr = int(p["virtual_address"], 16)
        prologue_data.append((p_addr, p))
    prologue_data.sort(key=lambda x: x[0])

    for section in sections:
        if section.get("name") != ".text" or section["size"] == 0:
            continue

        section_offset = section["offset"]
        section_addr = section["address"]
        section_size = section["size"]

        for i in range(0, section_size - 3, 4):
            file_offset = section_offset + i
            if file_offset + 4 > len(data):
                break

            word = struct.unpack("<I", data[file_offset : file_offset + 4])[0]
            opcode = (word >> 26) & 0x3F

            call_info = None

            if opcode == MIPS_JAL:
                target = (word & 0x3FFFFFF) << 2
                if section_addr < 0x80000000:
                    target = (section_addr & 0xF0000000) | target
                call_info = {
                    "type": "jal",
                    "target": target,
                    "instr_encoding": f"jal 0x{word & 0x3FFFFFF:07x}",
                }

            elif opcode == MIPS_JALR:
                rd = (word >> 11) & 0x1F
                rs = (word >> 21) & 0x1F
                call_info = {
                    "type": "jalr",
                    "rd": rd,
                    "rs": rs,
                    "instr_encoding": f"jalr ${rd}, ${rs}",
                }

            if call_info:
                caller_addr = section_addr + i
                caller_func = find_containing_function(caller_addr, prologue_data)

                call_info["caller_address"] = f"0x{caller_addr:08x}"
                call_info["caller_function"] = caller_func["virtual_address"] if caller_func else None
                call_info["caller_stack_size"] = caller_func.get("stack_adjustment") if caller_func else None
                call_info["file_offset"] = file_offset
                call_info["section"] = ".text"

                if call_info["type"] == "jal":
                    call_info["target_address"] = f"0x{call_info['target']:08x}"

                    for target_func_addr in target_functions:
                        if abs(call_info["target"] - target_func_addr) < 16:
                            call_info["is_target_call"] = True
                            call_info["target_function"] = f"0x{target_func_addr:08x}"
                            results.append(call_info)
                            break
                else:
                    results.append(call_info)

    return results


def build_report(args: argparse.Namespace) -> dict[str, Any]:
    image = args.image.expanduser().resolve()
    if not image.exists():
        raise FileNotFoundError(f"Image path does not exist: {args.image}")

    data = read_image_region(image, args.lba, args.size, args.sector_size, args.data_offset)
    header = parse_elf_header(data)
    sections = parse_sections(data, header)

    target_functions = [int(x, 16) for x in args.target]

    prologue_report_path = args.prologue_report
    with open(prologue_report_path) as f:
        prologue_data = json.load(f)

    prologues = prologue_data.get("prologues", [])

    calls = scan_calls(data, sections, prologues, target_functions)

    target_calls = {}
    for call in calls:
        if call.get("is_target_call"):
            target = call.get("target_function", "")
            if target not in target_calls:
                target_calls[target] = []
            target_calls[target].append(call)

    callers_by_function = {}
    for target in target_functions:
        target_str = f"0x{target:08x}"
        calls_for_target = target_calls.get(target_str, [])
        unique_callers = {}
        for c in calls_for_target:
            caller = c.get("caller_function")
            if caller and caller not in unique_callers:
                unique_callers[caller] = c

        callers_by_function[target_str] = list(unique_callers.values())

    return {
        "tool": TOOL_NAME,
        "tool_version": TOOL_VERSION,
        "generated_at_utc": utc_now(),
        "legal_boundary": "metadata-only; pattern-based call detection only",
        "source": {
            "mode": "disc-image-region",
            "image_path": str(image),
            "source_name": args.source_name,
            "prologue_report": str(prologue_report_path),
        },
        "targets": [{"address": f"0x{x:08x}"} for x in target_functions],
        "summary": {
            "total_calls_found": len(calls),
            "target_calls_found": sum(1 for c in calls if c.get("is_target_call")),
            "unique_callers": sum(len(v) for v in callers_by_function.values()),
        },
        "callers": callers_by_function,
        "all_calls": calls[: args.max_calls] if args.max_calls > 0 else calls,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Analyze MIPS function call graph."
    )
    parser.add_argument("--image", type=Path, required=True, help="Local disc image containing the ELF.")
    parser.add_argument("--lba", type=int, required=True, help="Start LBA of the ELF.")
    parser.add_argument("--size", type=int, required=True, help="Size in bytes of the ELF.")
    parser.add_argument("--sector-size", type=int, default=2352, help="Physical sector size. Default: 2352.")
    parser.add_argument("--data-offset", type=int, default=24, help="Data payload offset. Default: 24.")
    parser.add_argument("--source-name", default="SCUS_971.13", help="Display name for the source.")
    parser.add_argument(
        "--prologue-report",
        type=Path,
        required=True,
        help="Path to mips-prologue-scan JSON report.",
    )
    parser.add_argument(
        "--target",
        action="append",
        required=True,
        help="Target function address (hex). Repeatable.",
    )
    parser.add_argument(
        "--max-calls",
        type=int,
        default=1000,
        help="Maximum calls to include (default: 1000, 0 for all).",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(".local/reports"),
        help="Directory for the generated JSON report. Default: .local/reports",
    )
    return parser.parse_args()


def safe_report_name(args: argparse.Namespace) -> str:
    stem = args.source_name or args.image.name
    safe_stem = "".join(char if char.isalnum() or char in ("-", "_", ".") else "_" for char in stem)
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    return f"{timestamp}-{safe_stem}-mips-call-graph.json"


def write_report(report: dict[str, Any], output_dir: Path, args: argparse.Namespace) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    report_path = output_dir / safe_report_name(args)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return report_path


def main() -> int:
    args = parse_args()
    try:
        report = build_report(args)
        report_path = write_report(report, args.output_dir, args)
    except Exception as error:
        print(f"error: {error}")
        return 1
    print(f"mips call graph report written: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())