#!/usr/bin/env python3
"""Join SRCFILE.TXT source provenance into PAL→USA CSV artifacts.

This script uses the PAL extract's SRCFILE.TXT listing to map USA VAs back to
their originating source file and line number. It then enriches:

- docs/symbols/pal_usa_symbol_map.csv
- docs/symbols/raw_symbols_pal.csv
- research/pal-usa/main_map_functions_source.csv

The goal is to make source provenance explicit without using the emulator.
"""

import csv
import re
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent.parent
SRCFILE_TXT = PROJECT_ROOT / ".local" / "extracted" / "pal" / "SRCFILE.TXT"
PAL_USA_SYMBOL_MAP = PROJECT_ROOT / "docs" / "symbols" / "pal_usa_symbol_map.csv"
RAW_SYMBOLS_PAL = PROJECT_ROOT / "docs" / "symbols" / "raw_symbols_pal.csv"
MAIN_MAP_FUNCS = PROJECT_ROOT / "research" / "pal-usa" / "main_map_functions.csv"
MAIN_MAP_FUNCS_SRC = PROJECT_ROOT / "research" / "pal-usa" / "main_map_functions_source.csv"

FUNC_RE = re.compile(r"^0+([0-9a-fA-F]+) <([^>]+)>:$")
SRC_RE = re.compile(r"^(.*\.(?:c|s)):(\d+)$")


def load_csv(path: Path) -> list[dict]:
    with open(path) as f:
        return list(csv.DictReader(f))


def write_csv(path: Path, rows: list[dict], fieldnames: list[str]) -> None:
    with open(path, "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames)
        w.writeheader()
        w.writerows(rows)


def parse_srcfile(path: Path) -> dict[int, dict]:
    """Return {usa_va: {source_file, source_path, source_line, function_name}}."""
    lines = path.read_text(errors="ignore").splitlines()
    mapping: dict[int, dict] = {}

    i = 0
    while i < len(lines):
        m = FUNC_RE.match(lines[i])
        if not m:
            i += 1
            continue

        va = int(m.group(1), 16)
        func_name = m.group(2)
        source_path = ""
        source_file = ""
        source_line = ""

        j = i + 1
        while j < len(lines):
            next_func = FUNC_RE.match(lines[j])
            if next_func:
                break
            src = SRC_RE.match(lines[j].strip())
            if src:
                source_path = src.group(1)
                source_file = Path(source_path).name
                source_line = src.group(2)
                break
            j += 1

        if source_file:
            mapping[va] = {
                "source_file": source_file,
                "source_path": source_path,
                "source_line": source_line,
                "function_name": func_name,
            }

        i = j if j > i else i + 1

    return mapping


def enrich_pal_usa_symbol_map(src_map: dict[int, dict]) -> tuple[list[dict], int]:
    rows = load_csv(PAL_USA_SYMBOL_MAP)
    hits = 0
    for row in rows:
        row.setdefault("source_file", "")
        row.setdefault("source_path", "")
        row.setdefault("source_line", "")

        usa_va = row.get("usa_va", "")
        if not usa_va:
            continue
        try:
            va = int(usa_va, 16)
        except ValueError:
            continue
        info = src_map.get(va)
        if not info:
            continue
        row["source_file"] = info["source_file"]
        row["source_path"] = info["source_path"]
        row["source_line"] = info["source_line"]
        hits += 1
    return rows, hits


def enrich_raw_symbols_pal(symbol_map: list[dict]) -> tuple[list[dict], int]:
    raw_rows = load_csv(RAW_SYMBOLS_PAL)
    by_pal_va = {}
    for row in symbol_map:
        pal_va = row.get("pal_va", "")
        if pal_va:
            by_pal_va[pal_va] = row

    hits = 0
    for row in raw_rows:
        row.setdefault("source_file", "")
        row.setdefault("source_path", "")
        row.setdefault("source_line", "")

        pal_va = row.get("pal_va", "")
        sym = by_pal_va.get(pal_va)
        if not sym:
            continue
        if sym.get("source_file"):
            row["source_file"] = sym["source_file"]
            row["source_path"] = sym.get("source_path", "")
            row["source_line"] = sym.get("source_line", "")
            hits += 1
    return raw_rows, hits


def enrich_main_map_functions(src_map: dict[int, dict]) -> tuple[list[dict], int]:
    rows = load_csv(MAIN_MAP_FUNCS)
    symbol_rows = load_csv(PAL_USA_SYMBOL_MAP)
    by_pal_va = {}
    for row in symbol_rows:
        pal_va = row.get("pal_va", "")
        if pal_va:
            by_pal_va[pal_va] = row

    hits = 0
    for row in rows:
        row.setdefault("source_file", "")
        row.setdefault("source_path", "")
        row.setdefault("source_line", "")

        pal_va = row.get("pal_va", "")
        sym = by_pal_va.get(pal_va)
        if not sym:
            continue
        usa_va = sym.get("usa_va", "")
        if not usa_va:
            continue
        try:
            va = int(usa_va, 16)
        except ValueError:
            continue
        info = src_map.get(va)
        if not info:
            continue
        row["source_file"] = info["source_file"]
        row["source_path"] = info["source_path"]
        row["source_line"] = info["source_line"]
        hits += 1
    return rows, hits


def main() -> int:
    if not SRCFILE_TXT.exists():
        print(f"[ERR] SRCFILE.TXT not found at {SRCFILE_TXT}")
        return 1

    src_map = parse_srcfile(SRCFILE_TXT)
    print(f"[SRC] parsed {len(src_map)} USA VAs with source provenance")

    symbol_rows, symbol_hits = enrich_pal_usa_symbol_map(src_map)
    write_csv(
        PAL_USA_SYMBOL_MAP,
        symbol_rows,
        ["symbol", "pal_va", "usa_va", "type", "source_file", "match_method", "confidence", "status", "notes", "source_path", "source_line"],
    )
    print(f"[OK] pal_usa_symbol_map.csv -> {symbol_hits} rows enriched")

    raw_rows, raw_hits = enrich_raw_symbols_pal(symbol_rows)
    write_csv(
        RAW_SYMBOLS_PAL,
        raw_rows,
        ["symbol", "pal_va", "type", "source_file", "source_path", "source_line"],
    )
    print(f"[OK] raw_symbols_pal.csv -> {raw_hits} rows enriched")

    main_rows, main_hits = enrich_main_map_functions(src_map)
    write_csv(
        MAIN_MAP_FUNCS_SRC,
        main_rows,
        ["pal_va", "name", "object_file", "source_file", "source_path", "source_line"],
    )
    print(f"[OK] main_map_functions_source.csv -> {main_hits} rows enriched")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
