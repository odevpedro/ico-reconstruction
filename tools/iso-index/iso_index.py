#!/usr/bin/env python3
"""Generate a metadata-only ISO9660 index for a local disc image."""

from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


TOOL_NAME = "iso-index"
TOOL_VERSION = "0.1.0"
PVD_SECTOR = 16
PAYLOAD_SIZE = 2048


@dataclass(frozen=True)
class SectorLayout:
    sector_size: int
    data_offset: int
    label: str


LAYOUT_CANDIDATES = (
    SectorLayout(2048, 0, "iso-2048"),
    SectorLayout(2352, 16, "raw-2352-data-offset-16"),
    SectorLayout(2352, 24, "raw-2352-data-offset-24"),
    SectorLayout(2336, 8, "raw-2336-data-offset-8"),
    SectorLayout(2336, 0, "raw-2336-data-offset-0"),
)


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def decode_text(raw: bytes) -> str:
    return raw.decode("ascii", errors="replace").strip()


def read_payload(handle, layout: SectorLayout, lba: int, size: int) -> bytes:
    remaining = size
    current_lba = lba
    chunks: list[bytes] = []
    while remaining > 0:
        handle.seek(current_lba * layout.sector_size + layout.data_offset)
        chunk = handle.read(min(PAYLOAD_SIZE, remaining))
        if not chunk:
            break
        chunks.append(chunk)
        remaining -= len(chunk)
        current_lba += 1
    return b"".join(chunks)


def detect_layout(image_path: Path) -> SectorLayout:
    with image_path.open("rb") as handle:
        for layout in LAYOUT_CANDIDATES:
            handle.seek(PVD_SECTOR * layout.sector_size + layout.data_offset)
            descriptor = handle.read(PAYLOAD_SIZE)
            if len(descriptor) >= 7 and descriptor[0] == 1 and descriptor[1:6] == b"CD001":
                return layout
    raise RuntimeError("Could not detect a supported ISO9660 Primary Volume Descriptor layout.")


def parse_directory_record(record: bytes) -> dict[str, Any] | None:
    if not record or record[0] == 0:
        return None
    length = record[0]
    if length < 34 or len(record) < length:
        return None

    name_len = record[32]
    raw_name = record[33 : 33 + name_len]
    name = raw_name.decode("ascii", errors="replace")
    if name == "\x00":
        name = "."
    elif name == "\x01":
        name = ".."

    return {
        "name": name,
        "extent_lba": int.from_bytes(record[2:6], "little"),
        "size_bytes": int.from_bytes(record[10:14], "little"),
        "flags": record[25],
        "is_directory": bool(record[25] & 0x02),
    }


def normalize_file_name(name: str) -> str:
    if ";" in name:
        name = name.split(";", 1)[0]
    return name


def parse_directory(handle, layout: SectorLayout, lba: int, size: int) -> list[dict[str, Any]]:
    data = read_payload(handle, layout, lba, size)
    entries: list[dict[str, Any]] = []
    offset = 0
    while offset < len(data):
        length = data[offset]
        if length == 0:
            offset = ((offset // PAYLOAD_SIZE) + 1) * PAYLOAD_SIZE
            continue
        record = parse_directory_record(data[offset : offset + length])
        if record:
            entries.append(record)
        offset += length
    return entries


def walk_directory(
    handle,
    layout: SectorLayout,
    lba: int,
    size: int,
    path_prefix: str = "",
    seen: set[tuple[int, int]] | None = None,
) -> list[dict[str, Any]]:
    seen = seen or set()
    key = (lba, size)
    if key in seen:
        return []
    seen.add(key)

    output: list[dict[str, Any]] = []
    for entry in parse_directory(handle, layout, lba, size):
        if entry["name"] in (".", ".."):
            continue
        display_name = normalize_file_name(entry["name"])
        full_path = f"{path_prefix}/{display_name}" if path_prefix else display_name
        item = {
            "path": full_path,
            "name": display_name,
            "raw_name": entry["name"],
            "kind": "directory" if entry["is_directory"] else "file",
            "extent_lba": entry["extent_lba"],
            "size_bytes": entry["size_bytes"],
            "flags": entry["flags"],
        }
        output.append(item)
        if entry["is_directory"]:
            output.extend(
                walk_directory(
                    handle,
                    layout,
                    entry["extent_lba"],
                    entry["size_bytes"],
                    full_path,
                    seen,
                )
            )
    return output


def parse_cue(cue_path: Path) -> dict[str, Any]:
    text = cue_path.read_text(encoding="utf-8", errors="replace")
    lines = [line.strip() for line in text.splitlines() if line.strip()]
    return {"path": str(cue_path.resolve()), "lines": lines}


def candidate_executables(entries: list[dict[str, Any]]) -> list[dict[str, Any]]:
    candidates: list[dict[str, Any]] = []
    for entry in entries:
        if entry["kind"] != "file":
            continue
        name = entry["name"].upper()
        if name.endswith(".ELF") or name.startswith(("SCUS_", "SLUS_", "SCES_", "SLES_", "SLPM_", "SLPS_")):
            candidates.append(entry)
    return candidates


def parse_elf_header(data: bytes) -> dict[str, Any] | None:
    if len(data) < 52 or data[:4] != b"\x7fELF":
        return None
    elf_class = data[4]
    endian = data[5]
    if elf_class != 1 or endian != 1:
        return {
            "is_elf": True,
            "supported_header": False,
            "elf_class": elf_class,
            "endian": endian,
        }

    return {
        "is_elf": True,
        "supported_header": True,
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


def add_executable_metadata(handle, layout: SectorLayout, entries: list[dict[str, Any]]) -> list[dict[str, Any]]:
    enriched: list[dict[str, Any]] = []
    for entry in candidate_executables(entries):
        item = dict(entry)
        header = read_payload(handle, layout, entry["extent_lba"], min(entry["size_bytes"], PAYLOAD_SIZE))
        elf_metadata = parse_elf_header(header)
        if elf_metadata:
            item["elf_metadata"] = elf_metadata
        enriched.append(item)
    return enriched


def build_report(image_path: Path, cue_path: Path | None) -> dict[str, Any]:
    resolved = image_path.expanduser().resolve()
    if not resolved.exists():
        raise FileNotFoundError(f"Image path does not exist: {image_path}")
    if not resolved.is_file():
        raise RuntimeError(f"Image path is not a file: {image_path}")

    layout = detect_layout(resolved)
    with resolved.open("rb") as handle:
        pvd = read_payload(handle, layout, PVD_SECTOR, PAYLOAD_SIZE)
        root_record = parse_directory_record(pvd[156 : 156 + pvd[156]])
        if not root_record:
            raise RuntimeError("Could not parse ISO9660 root directory record.")
        entries = walk_directory(
            handle,
            layout,
            root_record["extent_lba"],
            root_record["size_bytes"],
        )
        executables = add_executable_metadata(handle, layout, entries)

    file_entries = [entry for entry in entries if entry["kind"] == "file"]
    directory_entries = [entry for entry in entries if entry["kind"] == "directory"]
    total_file_bytes = sum(entry["size_bytes"] for entry in file_entries)

    report: dict[str, Any] = {
        "tool": TOOL_NAME,
        "tool_version": TOOL_VERSION,
        "generated_at_utc": utc_now(),
        "legal_boundary": "metadata-only; no file contents extracted into this report",
        "image_path": str(resolved),
        "image_size_bytes": resolved.stat().st_size,
        "sector_layout": {
            "label": layout.label,
            "sector_size": layout.sector_size,
            "data_offset": layout.data_offset,
            "payload_size": PAYLOAD_SIZE,
        },
        "volume": {
            "system_identifier": decode_text(pvd[8:40]),
            "volume_identifier": decode_text(pvd[40:72]),
            "volume_space_size": int.from_bytes(pvd[80:84], "little"),
            "root_extent_lba": root_record["extent_lba"],
            "root_size_bytes": root_record["size_bytes"],
        },
        "summary": {
            "directory_count": len(directory_entries),
            "file_count": len(file_entries),
            "total_file_bytes": total_file_bytes,
        },
        "candidate_executables": executables,
        "entries": entries,
    }
    if cue_path:
        cue_resolved = cue_path.expanduser().resolve()
        if not cue_resolved.exists():
            raise FileNotFoundError(f"CUE path does not exist: {cue_path}")
        report["cue"] = parse_cue(cue_resolved)
    return report


def safe_report_name(image_path: Path) -> str:
    stem = image_path.name or "image"
    safe_stem = "".join(char if char.isalnum() or char in ("-", "_", ".") else "_" for char in stem)
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    return f"{timestamp}-{safe_stem}-iso-index.json"


def write_report(report: dict[str, Any], output_dir: Path, image_path: Path) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    report_path = output_dir / safe_report_name(image_path)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return report_path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate a metadata-only ISO9660 index.")
    parser.add_argument("image", type=Path, help="Local ISO/BIN image to inspect.")
    parser.add_argument("--cue", type=Path, help="Optional local CUE file for context.")
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
        report = build_report(args.image, args.cue)
        report_path = write_report(report, args.output_dir, args.image)
    except Exception as error:
        print(f"error: {error}")
        return 1
    print(f"iso index written: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
