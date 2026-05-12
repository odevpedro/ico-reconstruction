#!/usr/bin/env python3
"""Generate a metadata-only report for a local user-owned ICO input."""

from __future__ import annotations

import argparse
import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


CHUNK_SIZE = 1024 * 1024 * 8
TOOL_NAME = "verify-local-copy"
TOOL_VERSION = "0.1.0"


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def format_timestamp(timestamp: float) -> str:
    return datetime.fromtimestamp(timestamp, timezone.utc).replace(microsecond=0).isoformat()


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(CHUNK_SIZE), b""):
            digest.update(chunk)
    return digest.hexdigest()


def stat_metadata(path: Path, root: Path | None = None, include_hash: bool = True) -> dict[str, Any]:
    stat = path.stat()
    item: dict[str, Any] = {
        "name": path.name,
        "relative_path": path.relative_to(root).as_posix() if root else path.name,
        "kind": "directory" if path.is_dir() else "file",
        "size_bytes": stat.st_size,
        "modified_utc": format_timestamp(stat.st_mtime),
        "mode_octal": oct(stat.st_mode & 0o777),
    }
    if path.is_file() and include_hash:
        item["sha256"] = sha256_file(path)
    return item


def scan_directory(path: Path, include_hash: bool, max_files: int) -> dict[str, Any]:
    files = sorted(item for item in path.rglob("*") if item.is_file())
    if len(files) > max_files:
        raise RuntimeError(
            f"Refusing to scan {len(files)} files; raise --max-files if this is intentional."
        )

    entries = [stat_metadata(item, root=path, include_hash=include_hash) for item in files]
    total_bytes = sum(entry["size_bytes"] for entry in entries)
    return {
        "directory": stat_metadata(path, include_hash=False),
        "total_files": len(entries),
        "total_bytes": total_bytes,
        "files": entries,
    }


def build_report(input_path: Path, include_hash: bool, max_files: int) -> dict[str, Any]:
    resolved = input_path.expanduser().resolve()
    if not resolved.exists():
        raise FileNotFoundError(f"Input path does not exist: {input_path}")

    report: dict[str, Any] = {
        "tool": TOOL_NAME,
        "tool_version": TOOL_VERSION,
        "generated_at_utc": utc_now(),
        "legal_boundary": "metadata-only; no file contents copied into this report",
        "input_path": str(resolved),
        "input_kind": "directory" if resolved.is_dir() else "file",
    }

    if resolved.is_dir():
        report["scan"] = scan_directory(resolved, include_hash=include_hash, max_files=max_files)
    else:
        report["file"] = stat_metadata(resolved, include_hash=include_hash)

    return report


def safe_report_name(input_path: Path) -> str:
    stem = input_path.name or "input"
    safe_stem = "".join(char if char.isalnum() or char in ("-", "_", ".") else "_" for char in stem)
    timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    return f"{timestamp}-{safe_stem}-metadata.json"


def write_report(report: dict[str, Any], output_dir: Path, input_path: Path) -> Path:
    output_dir.mkdir(parents=True, exist_ok=True)
    report_path = output_dir / safe_report_name(input_path)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return report_path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate a metadata-only report for a local user-owned ICO input."
    )
    parser.add_argument("input", type=Path, help="Local file or directory to inspect.")
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path(".local/reports"),
        help="Directory for the generated JSON report. Default: .local/reports",
    )
    parser.add_argument(
        "--no-hash",
        action="store_true",
        help="Skip SHA-256 hashing and record only stat metadata.",
    )
    parser.add_argument(
        "--max-files",
        type=int,
        default=10000,
        help="Maximum files to scan when input is a directory. Default: 10000",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    try:
        report = build_report(args.input, include_hash=not args.no_hash, max_files=args.max_files)
        report_path = write_report(report, args.output_dir, args.input)
    except Exception as error:
        print(f"error: {error}")
        return 1

    print(f"metadata report written: {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
