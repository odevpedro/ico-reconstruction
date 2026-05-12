#!/usr/bin/env python3
"""Generate a metadata-only structural triage report for DFDATAS/DATA.DF."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
from collections import Counter
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


TOOL_NAME = "data-df-index"
TOOL_VERSION = "0.1.0"
PAYLOAD_SIZE = 2048
DEFAULT_SCAN_BYTES = 1024 * 1024
DEFAULT_TARGET_WINDOW_BYTES = 256 * 1024


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def read_image_region(
    image_path: Path,
    lba: int,
    size: int,
    sector_size: int,
    data_offset: int,
    max_bytes: int,
    region_offset: int = 0,
) -> bytes:
    if region_offset >= size:
        return b""
    read_size = min(size - region_offset, max_bytes)
    chunks: list[bytes] = []
    remaining = read_size
    current_lba = lba + region_offset // PAYLOAD_SIZE
    payload_offset = region_offset % PAYLOAD_SIZE
    with image_path.open("rb") as handle:
        while remaining > 0:
            handle.seek(current_lba * sector_size + data_offset + payload_offset)
            chunk = handle.read(min(PAYLOAD_SIZE - payload_offset, remaining))
            if not chunk:
                break
            chunks.append(chunk)
            remaining -= len(chunk)
            current_lba += 1
            payload_offset = 0
    data = b"".join(chunks)
    if len(data) != read_size:
        raise RuntimeError(f"Expected {read_size} bytes from image region, read {len(data)} bytes.")
    return data


def entropy(data: bytes) -> float:
    if not data:
        return 0.0
    counts = Counter(data)
    total = len(data)
    return -sum((count / total) * math.log2(count / total) for count in counts.values())


def byte_profile(data: bytes) -> dict[str, Any]:
    zero_count = data.count(0)
    ff_count = data.count(0xFF)
    ascii_printable = sum(1 for byte in data if 0x20 <= byte <= 0x7E)
    control = sum(1 for byte in data if byte < 0x20 and byte not in (0x09, 0x0A, 0x0D))
    return {
        "sample_size": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
        "entropy_bits_per_byte": round(entropy(data), 6),
        "zero_byte_count": zero_count,
        "zero_byte_ratio": round(zero_count / len(data), 6) if data else 0,
        "ff_byte_count": ff_count,
        "ff_byte_ratio": round(ff_count / len(data), 6) if data else 0,
        "ascii_printable_count": ascii_printable,
        "ascii_printable_ratio": round(ascii_printable / len(data), 6) if data else 0,
        "control_byte_count": control,
        "unique_byte_count": len(set(data)),
    }


def read_u32_values(data: bytes, endian: str, limit: int) -> list[int]:
    usable = min(len(data), limit)
    values: list[int] = []
    for offset in range(0, usable - 3, 4):
        values.append(int.from_bytes(data[offset : offset + 4], endian))
    return values


def longest_monotonic_prefix(values: list[int], archive_size: int, alignment: int) -> int:
    count = 0
    previous = -1
    for value in values:
        if value < previous or value > archive_size:
            break
        if alignment and value % alignment != 0:
            break
        previous = value
        count += 1
    return count


def offset_table_candidates(data: bytes, archive_size: int, scan_limit: int) -> list[dict[str, Any]]:
    candidates: list[dict[str, Any]] = []
    for endian in ("little", "big"):
        for start in range(0, min(4096, len(data) - 16), 4):
            values = read_u32_values(data[start:], endian, min(scan_limit, len(data) - start))
            for alignment in (0, 4, 16, 2048):
                prefix_count = longest_monotonic_prefix(values, archive_size, alignment)
                if prefix_count >= 8:
                    prefix = values[:prefix_count]
                    deltas = [b - a for a, b in zip(prefix, prefix[1:])]
                    nonzero_deltas = [delta for delta in deltas if delta > 0]
                    candidates.append(
                        {
                            "start_offset": start,
                            "endian": endian,
                            "alignment": alignment,
                            "entry_count": prefix_count,
                            "first_value": prefix[0],
                            "last_value": prefix[-1],
                            "min_delta": min(nonzero_deltas) if nonzero_deltas else 0,
                            "max_delta": max(nonzero_deltas) if nonzero_deltas else 0,
                        }
                    )
    candidates.sort(key=lambda item: (-item["entry_count"], item["start_offset"], item["alignment"]))
    return candidates[:20]


def fixed_record_candidates(data: bytes, archive_size: int) -> list[dict[str, Any]]:
    candidates: list[dict[str, Any]] = []
    record_sizes = (8, 12, 16, 20, 24, 32)
    sample_size = min(len(data), 65536)
    for record_size in record_sizes:
        plausible = 0
        total = sample_size // record_size
        for offset in range(0, total * record_size, record_size):
            first = int.from_bytes(data[offset : offset + 4], "little")
            second = int.from_bytes(data[offset + 4 : offset + 8], "little")
            if first <= archive_size and second <= archive_size:
                plausible += 1
        if total and plausible / total >= 0.75:
            candidates.append(
                {
                    "record_size": record_size,
                    "sampled_records": total,
                    "plausible_record_count": plausible,
                    "plausible_ratio": round(plausible / total, 6),
                }
            )
    return candidates


def window_offsets(size: int, scan_bytes: int) -> list[tuple[str, int]]:
    if size <= scan_bytes:
        return [("full", 0)]
    middle = max(0, (size // 2) - (scan_bytes // 2))
    tail = max(0, size - scan_bytes)
    offsets = [("head", 0), ("middle", middle), ("tail", tail)]
    deduped: list[tuple[str, int]] = []
    seen: set[int] = set()
    for label, offset in offsets:
        if offset not in seen:
            deduped.append((label, offset))
            seen.add(offset)
    return deduped


def profile_windows(args: argparse.Namespace, image: Path) -> list[dict[str, Any]]:
    windows: list[dict[str, Any]] = []
    for label, offset in window_offsets(args.size, args.scan_bytes):
        data = read_image_region(
            image,
            args.lba,
            args.size,
            args.sector_size,
            args.data_offset,
            args.scan_bytes,
            region_offset=offset,
        )
        window: dict[str, Any] = {
            "label": label,
            "region_offset": offset,
            "scan_bytes_actual": len(data),
            "byte_profile": byte_profile(data),
        }
        if label == "head":
            window["offset_table_candidates"] = offset_table_candidates(
                data, args.size, args.offset_scan_bytes
            )
            window["fixed_record_candidates"] = fixed_record_candidates(data, args.size)
        windows.append(window)
    return windows


def targeted_window_offset(size: int, target_offset: int, window_bytes: int) -> int:
    if target_offset < 0 or target_offset >= size:
        raise RuntimeError(f"Target offset {target_offset} is outside DATA.DF size {size}.")
    half_window = window_bytes // 2
    start = max(0, target_offset - half_window)
    if start + window_bytes > size:
        start = max(0, size - window_bytes)
    return start


def profile_targeted_windows(args: argparse.Namespace, image: Path) -> list[dict[str, Any]]:
    windows: list[dict[str, Any]] = []
    for target_offset in args.target_offset:
        region_offset = targeted_window_offset(args.size, target_offset, args.target_window_bytes)
        data = read_image_region(
            image,
            args.lba,
            args.size,
            args.sector_size,
            args.data_offset,
            args.target_window_bytes,
            region_offset=region_offset,
        )
        relative_target = target_offset - region_offset
        windows.append(
            {
                "target_offset": target_offset,
                "target_offset_hex": f"0x{target_offset:08x}",
                "region_offset": region_offset,
                "region_offset_hex": f"0x{region_offset:08x}",
                "relative_target_offset": relative_target,
                "scan_bytes_actual": len(data),
                "byte_profile": byte_profile(data),
                "offset_table_candidates": offset_table_candidates(
                    data, args.size, min(args.offset_scan_bytes, len(data))
                ),
                "fixed_record_candidates": fixed_record_candidates(data, args.size),
            }
        )
    return windows


def build_report(args: argparse.Namespace) -> dict[str, Any]:
    image = args.image.expanduser().resolve()
    if not image.exists():
        raise FileNotFoundError(f"Image path does not exist: {args.image}")
    windows = profile_windows(args, image)
    targeted_windows = profile_targeted_windows(args, image)
    head_window = windows[0] if windows else {}
    return {
        "tool": TOOL_NAME,
        "tool_version": TOOL_VERSION,
        "generated_at_utc": utc_now(),
        "legal_boundary": "metadata-only; no archive contents extracted into this report",
        "source": {
            "mode": "disc-image-region",
            "image_path": str(image),
            "source_name": args.source_name,
            "extent_lba": args.lba,
            "size_bytes": args.size,
            "sector_size": args.sector_size,
            "data_offset": args.data_offset,
        },
        "scan": {
            "scan_bytes_requested": args.scan_bytes,
            "window_count": len(windows),
            "total_scan_bytes": sum(window["scan_bytes_actual"] for window in windows),
            "target_window_bytes_requested": args.target_window_bytes,
            "targeted_window_count": len(targeted_windows),
            "targeted_total_scan_bytes": sum(
                window["scan_bytes_actual"] for window in targeted_windows
            ),
            "total_scan_ratio": round(
                (
                    sum(window["scan_bytes_actual"] for window in windows)
                    + sum(window["scan_bytes_actual"] for window in targeted_windows)
                )
                / args.size,
                6,
            )
            if args.size
            else 0,
        },
        "head_byte_profile": head_window.get("byte_profile", {}),
        "head_offset_table_candidates": head_window.get("offset_table_candidates", []),
        "head_fixed_record_candidates": head_window.get("fixed_record_candidates", []),
        "windows": windows,
        "targeted_windows": targeted_windows,
    }


def safe_report_name(args: argparse.Namespace) -> str:
    stem = args.source_name or args.image.name
    safe_stem = "".join(char if char.isalnum() or char in ("-", "_", ".") else "_" for char in stem)
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    return f"{timestamp}-{safe_stem}-data-df-index.json"


def write_report(report: dict[str, Any], output_dir: Path, args: argparse.Namespace) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    report_path = output_dir / safe_report_name(args)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return report_path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate a metadata-only DATA.DF triage report.")
    parser.add_argument("--image", type=Path, required=True, help="Local disc image containing DATA.DF.")
    parser.add_argument("--lba", type=int, required=True, help="Start LBA of DATA.DF.")
    parser.add_argument("--size", type=int, required=True, help="Size in bytes of DATA.DF.")
    parser.add_argument("--sector-size", type=int, default=2352, help="Physical sector size. Default: 2352.")
    parser.add_argument("--data-offset", type=int, default=24, help="Data payload offset. Default: 24.")
    parser.add_argument("--source-name", default="DFDATAS/DATA.DF", help="Display name for the source.")
    parser.add_argument(
        "--scan-bytes",
        type=int,
        default=DEFAULT_SCAN_BYTES,
        help="Initial bytes to scan from DATA.DF. Default: 1048576.",
    )
    parser.add_argument(
        "--offset-scan-bytes",
        type=int,
        default=65536,
        help="Initial bytes to consider for monotonic offset-table candidates. Default: 65536.",
    )
    parser.add_argument(
        "--target-offset",
        action="append",
        type=lambda value: int(value, 0),
        default=[],
        help="Candidate DATA.DF offset to scan. Accepts decimal or 0x-prefixed values. Repeatable.",
    )
    parser.add_argument(
        "--target-window-bytes",
        type=int,
        default=DEFAULT_TARGET_WINDOW_BYTES,
        help="Bytes to scan around each --target-offset. Default: 262144.",
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
    print(f"data df index written: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
