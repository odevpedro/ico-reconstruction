#!/usr/bin/env python3
"""MIPS function prologue scanner for SCUS_971.13.

This tool identifies potential function prologues in MIPS code by detecting
stack allocation patterns (addiu $sp, $sp, -N) and frame setup sequences.
This helps map code structure without symbol information.
"""

from __future__ import annotations

import argparse
import json
import struct
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


TOOL_NAME = "mips-prologue-scan"
TOOL_VERSION = "0.1.0"
PAYLOAD_SIZE = 2048


MIPS_OPCODES = {
    0x09: "addiu",
    0x08: "addi",
    0x0D: "ori",
    0x0F: "lui",
    0x1A: "sw",
    0x1B: "lw",
    0x20: "lb",
    0x21: "lh",
    0x23: "lw",
    0x28: "sb",
    0x29: "sh",
    0x0C: "andi",
    0x0A: "addi",
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


def parse_elf_header(data: bytes) -> dict[str, Any]:
    if len(data) < 52 or data[:4] != b"\x7fELF":
        raise RuntimeError("Input is not an ELF file.")
    if data[4] != 1 or data[5] != 1:
        raise RuntimeError("Only ELF32 little-endian inputs are supported.")
    return {
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


def decode_mips_instruction(word: int) -> dict[str, Any] | None:
    opcode = (word >> 26) & 0x3F
    if opcode not in MIPS_OPCODES:
        return None

    mnemo = MIPS_OPCODES[opcode]

    if opcode == 0x09 or opcode == 0x08:
        rs = (word >> 21) & 0x1F
        rt = (word >> 16) & 0x1F
        imm = word & 0xFFFF
        signed_imm = imm if imm < 0x8000 else imm - 0x10000
        return {"opcode": opcode, "mnemonic": mnemo, "rs": rs, "rt": rt, "immediate": signed_imm}

    elif opcode == 0x1A:
        rs = (word >> 21) & 0x1F
        rt = (word >> 16) & 0x1F
        offset = word & 0xFFFF
        return {"opcode": opcode, "mnemonic": mnemo, "rs": rs, "rt": rt, "offset": offset}

    return {"opcode": opcode, "mnemonic": mnemo}


def is_function_prologue(instr1: dict, instr2: dict | None, instr3: dict | None) -> tuple[bool, str]:
    if not instr1 or instr1["mnemonic"] != "addiu":
        return False, ""

    if instr1["rs"] != 29:
        return False, ""

    if instr1["immediate"] >= 0:
        return False, ""

    stack_size = -instr1["immediate"]

    if stack_size > 0 and stack_size <= 32768:
        if instr2 and instr3:
            if (instr2["mnemonic"] == "sw" and instr2["rs"] == 29) or (instr3["mnemonic"] == "sw" and instr3["rs"] == 29):
                return True, f"prologue with {stack_size} bytes, register save"

        return True, f"prologue with {stack_size} bytes"

    return False, ""


def scan_prologues(
    data: bytes,
    sections: list[dict[str, Any]],
    min_sp_offset: int = 16,
    max_sp_offset: int = 32768,
    check_sequence: bool = False,
) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []

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
            instr1 = decode_mips_instruction(word)

            if not instr1:
                continue

            is_prologue, description = is_function_prologue(instr1, None, None)

            if is_prologue:
                if min_sp_offset <= -instr1["immediate"] <= max_sp_offset:
                    virt_addr = section_addr + i

                    result = {
                        "file_offset": file_offset,
                        "file_offset_hex": f"0x{file_offset:08x}",
                        "virtual_address": f"0x{virt_addr:08x}",
                        "instruction": f"{instr1['mnemonic']} $sp, $sp, {instr1['immediate']}",
                        "stack_adjustment": -instr1["immediate"],
                        "sequence": description,
                        "section": ".text",
                    }

                    if check_sequence and i + 4 < section_size:
                        word2 = struct.unpack("<I", data[file_offset + 4 : file_offset + 8])[0]
                        word3 = struct.unpack("<I", data[file_offset + 8 : file_offset + 12])[0]
                        instr2 = decode_mips_instruction(word2)
                        instr3 = decode_mips_instruction(word3)
                        is_prol, desc = is_function_prologue(instr1, instr2, instr3)
                        result["sequence"] = desc

                    results.append(result)

    return results


def build_report(args: argparse.Namespace) -> dict[str, Any]:
    image = args.image.expanduser().resolve()
    if not image.exists():
        raise FileNotFoundError(f"Image path does not exist: {args.image}")

    data = read_image_region(image, args.lba, args.size, args.sector_size, args.data_offset)
    header = parse_elf_header(data)
    sections = parse_sections(data, header)

    prologues = scan_prologues(
        data,
        sections,
        min_sp_offset=args.min_offset,
        max_sp_offset=args.max_offset,
        check_sequence=args.check_sequence,
    )

    stack_distribution = {}
    for p in prologues:
        size = p["stack_adjustment"]
        if size not in stack_distribution:
            stack_distribution[size] = 0
        stack_distribution[size] += 1

    return {
        "tool": TOOL_NAME,
        "tool_version": TOOL_VERSION,
        "generated_at_utc": utc_now(),
        "legal_boundary": "metadata-only; pattern-based function detection only",
        "source": {
            "mode": "disc-image-region",
            "image_path": str(image),
            "source_name": args.source_name,
            "extent_lba": args.lba,
            "size_bytes": args.size,
            "sector_size": args.sector_size,
            "data_offset": args.data_offset,
        },
        "parameters": {
            "min_stack_offset": args.min_offset,
            "max_stack_offset": args.max_offset,
            "check_sequence": args.check_sequence,
        },
        "summary": {
            "prologues_found": len(prologues),
            "unique_stack_sizes": len(stack_distribution),
        },
        "stack_distribution": dict(sorted(stack_distribution.items())),
        "prologues": prologues[: args.max_results] if args.max_results > 0 else prologues,
    }


def safe_report_name(args: argparse.Namespace) -> str:
    stem = args.source_name or args.image.name
    safe_stem = "".join(char if char.isalnum() or char in ("-", "_", ".") else "_" for char in stem)
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    return f"{timestamp}-{safe_stem}-mips-prologue-scan.json"


def write_report(report: dict[str, Any], output_dir: Path, args: argparse.Namespace) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    report_path = output_dir / safe_report_name(args)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return report_path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Scan for MIPS function prologues in PS2 executables."
    )
    parser.add_argument("--image", type=Path, required=True, help="Local disc image containing the ELF.")
    parser.add_argument("--lba", type=int, required=True, help="Start LBA of the ELF.")
    parser.add_argument("--size", type=int, required=True, help="Size in bytes of the ELF.")
    parser.add_argument("--sector-size", type=int, default=2352, help="Physical sector size. Default: 2352.")
    parser.add_argument("--data-offset", type=int, default=24, help="Data payload offset. Default: 24.")
    parser.add_argument("--source-name", default="SCUS_971.13", help="Display name for the source.")
    parser.add_argument(
        "--min-offset",
        type=int,
        default=16,
        help="Minimum stack offset to consider (default: 16).",
    )
    parser.add_argument(
        "--max-offset",
        type=int,
        default=32768,
        help="Maximum stack offset to consider (default: 32768).",
    )
    parser.add_argument(
        "--check-sequence",
        action="store_true",
        help="Check for register save sequences (slower).",
    )
    parser.add_argument(
        "--max-results",
        type=int,
        default=500,
        help="Maximum prologues to include in report (default: 500, 0 for all).",
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
    print(f"mips prologue scan report written: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())