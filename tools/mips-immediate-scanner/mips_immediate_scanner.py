#!/usr/bin/env python3
"""Metadata-only MIPS split-immediate scanner for SCUS_971.13.

This tool scans for MIPS instructions that use split immediates (lui + ori/addiu)
to construct 32-bit addresses or constants, focusing on virtual addresses of
known DATA.DF and DFDATAS strings without performing full disassembly.
"""

from __future__ import annotations

import argparse
import json
import struct
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


TOOL_NAME = "mips-immediate-scanner"
TOOL_VERSION = "0.1.0"
PAYLOAD_SIZE = 2048


MIPS_OPCODES = {
    0x0F: "lui",
    0x0D: "ori",
    0x09: "addiu",
    0x08: "addi",
    0x0C: "andi",
    0x0A: "subu",
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
        "entry_point": int.from_bytes(data[24:28], "little"),
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


def file_offset_to_virtual(offset: int, program_headers: list[dict[str, Any]]) -> int | None:
    for header in program_headers:
        if header["type"] != 1:
            continue
        start = header["file_offset"]
        end = start + header["file_size"]
        if start <= offset < end:
            return header["virtual_address"] + (offset - start)
    return None


def decode_mips_instruction(word: int) -> dict[str, Any] | None:
    opcode = (word >> 26) & 0x3F
    if opcode not in MIPS_OPCODES:
        return None
    mnemo = MIPS_OPCODES[opcode]
    if opcode == 0x0F:
        rt = (word >> 16) & 0x1F
        imm = word & 0xFFFF
        return {"opcode": opcode, "mnemonic": mnemo, "rt": rt, "immediate": imm}
    else:
        rs = (word >> 21) & 0x1F
        rt = (word >> 16) & 0x1F
        imm = word & 0xFFFF
        return {"opcode": opcode, "mnemonic": mnemo, "rs": rs, "rt": rt, "immediate": imm}


def scan_text_sections(
    data: bytes,
    sections: list[dict[str, Any]],
    program_headers: list[dict[str, Any]],
    target_addresses: list[int],
) -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    target_high_bits: dict[int, list[int]] = {}
    for addr in target_addresses:
        high = (addr >> 16) & 0xFFFF
        low = addr & 0xFFFF
        if high not in target_high_bits:
            target_high_bits[high] = []
        target_high_bits[high].append((addr, low))

    for section in sections:
        if section.get("name") != ".text" or section["size"] == 0:
            continue

        section_offset = section["offset"]
        section_addr = section["address"]
        section_size = section["size"]

        lui_map: dict[int, tuple[int, int, int]] = {}

        for i in range(0, section_size - 3, 4):
            file_offset = section_offset + i
            if file_offset + 4 > len(data):
                break
            word = struct.unpack("<I", data[file_offset : file_offset + 4])[0]
            instr = decode_mips_instruction(word)
            if not instr:
                continue

            virt_addr = section_addr + i

            if instr["mnemonic"] == "lui":
                high = instr["immediate"]
                if high in target_high_bits:
                    lui_map[instr["rt"]] = (file_offset, virt_addr, high)

            elif instr["mnemonic"] in ("ori", "addiu"):
                rt = instr["rt"]
                rs = instr["rs"]
                low = instr["immediate"]
                if rs in lui_map:
                    lui_file_offset, lui_virt_addr, high = lui_map[rs]
                    constructed = (high << 16) | low
                    for target_addr, target_low in target_high_bits.get(high, []):
                        if low == target_low or low == target_low:
                            results.append(
                                {
                                    "file_offset": lui_file_offset,
                                    "file_offset_hex": f"0x{lui_file_offset:08x}",
                                    "lui_virtual_address": f"0x{lui_virt_addr:08x}",
                                    "lui_instruction": f"lui $rt{instr['rt']}, 0x{high:04x}",
                                    "high_16": f"0x{high:04x}",
                                    "matched_file_offset": file_offset,
                                    "matched_virtual_address": f"0x{virt_addr:08x}",
                                    "instruction": f"{instr['mnemonic']} $rt{rt}, $rs{rs}, 0x{low:04x}",
                                    "low_16": f"0x{low:04x}",
                                    "constructed_address": f"0x{constructed:08x}",
                                    "target_address": f"0x{target_addr:08x}",
                                    "section": ".text",
                                }
                            )

    return results


def build_report(args: argparse.Namespace) -> dict[str, Any]:
    image = args.image.expanduser().resolve()
    if not image.exists():
        raise FileNotFoundError(f"Image path does not exist: {args.image}")

    data = read_image_region(image, args.lba, args.size, args.sector_size, args.data_offset)
    header = parse_elf_header(data)
    program_headers = parse_program_headers(data, header)
    sections = parse_sections(data, header)

    target_addresses = [int(x, 0) for x in args.target]
    matches = scan_text_sections(data, sections, program_headers, target_addresses)

    return {
        "tool": TOOL_NAME,
        "tool_version": TOOL_VERSION,
        "generated_at_utc": utc_now(),
        "legal_boundary": "metadata-only; pattern-based immediate scan only",
        "source": {
            "mode": "disc-image-region",
            "image_path": str(image),
            "source_name": args.source_name,
            "extent_lba": args.lba,
            "size_bytes": args.size,
            "sector_size": args.sector_size,
            "data_offset": args.data_offset,
        },
        "targets": [{"address": f"0x{a:08x}", "hex": f"0x{a:08x}"} for a in target_addresses],
        "summary": {
            "target_count": len(target_addresses),
            "pattern_matches": len(matches),
        },
        "matches": matches,
    }


def safe_report_name(args: argparse.Namespace) -> str:
    stem = args.source_name or args.image.name
    safe_stem = "".join(char if char.isalnum() or char in ("-", "_", ".") else "_" for char in stem)
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    return f"{timestamp}-{safe_stem}-mips-immediate-scan.json"


def write_report(report: dict[str, Any], output_dir: Path, args: argparse.Namespace) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    report_path = output_dir / safe_report_name(args)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return report_path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Scan for MIPS split-immediate patterns referencing known virtual addresses."
    )
    parser.add_argument("--image", type=Path, required=True, help="Local disc image containing the ELF.")
    parser.add_argument("--lba", type=int, required=True, help="Start LBA of the ELF.")
    parser.add_argument("--size", type=int, required=True, help="Size in bytes of the ELF.")
    parser.add_argument("--sector-size", type=int, default=2352, help="Physical sector size. Default: 2352.")
    parser.add_argument("--data-offset", type=int, default=24, help="Data payload offset. Default: 24.")
    parser.add_argument("--source-name", default="SCUS_971.13", help="Display name for the source.")
    parser.add_argument(
        "--target",
        action="append",
        required=True,
        help="Target virtual address to scan for (hex with 0x prefix). Repeatable.",
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
    print(f"mips immediate scanner report written: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())