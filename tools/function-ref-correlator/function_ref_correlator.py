#!/usr/bin/env python3
"""Function reference correlator for ICO.

This tool correlates detected function prologues with known code references
to identify which functions contain the references to DATA.DF/DFDATAS.
"""

from __future__ import annotations

import argparse
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


TOOL_NAME = "function-ref-correlator"
TOOL_VERSION = "0.1.0"


def utc_now() -> str:
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat()


def parse_virtual_address(addr_str: str) -> int:
    if addr_str.startswith("0x"):
        return int(addr_str, 16)
    return int(addr_str)


def find_containing_function(
    ref_addr: int,
    prologues: list[dict[str, Any]],
) -> dict[str, Any] | None:
    """Find the function that contains the given reference address."""

    prologue_addrs = []
    for p in prologues:
        p_addr = parse_virtual_address(p["virtual_address"])
        prologue_addrs.append((p_addr, p))

    prologue_addrs.sort(key=lambda x: x[0])

    best_prologue = None
    best_addr = -1

    for p_addr, p in prologue_addrs:
        if p_addr <= ref_addr and p_addr > best_addr:
            best_addr = p_addr
            best_prologue = p

    return best_prologue


def build_report(
    immediate_report_path: Path,
    prologue_report_path: Path,
    args: argparse.Namespace,
) -> dict[str, Any]:
    with open(immediate_report_path) as f:
        immediate_data = json.load(f)

    with open(prologue_report_path) as f:
        prologue_data = json.load(f)

    prologues = prologue_data.get("prologues", [])
    for i, p in enumerate(prologues):
        if i > 0:
            p["_prev_addr"] = parse_virtual_address(prologues[i - 1]["virtual_address"])

    matches = immediate_data.get("matches", [])
    targets = immediate_data.get("targets", [])

    target_refs: dict[str, list[dict[str, Any]]] = {}
    for match in matches:
        target = match.get("target_address", "")
        if target not in target_refs:
            target_refs[target] = []
        target_refs[target].append(match)

    correlated: list[dict[str, Any]] = []
    functions_with_refs: dict[str, dict[str, Any]] = {}

    for match in matches:
        lui_addr = parse_virtual_address(match["lui_virtual_address"])
        addiu_addr = parse_virtual_address(match["matched_virtual_address"])

        lui_func = find_containing_function(lui_addr, prologues)
        addiu_func = find_containing_function(addiu_addr, prologues)

        func_info = {
            "lui_address": match["lui_virtual_address"],
            "lui_function": lui_func["virtual_address"] if lui_func else None,
            "lui_stack_size": lui_func.get("stack_adjustment") if lui_func else None,
            "addiu_address": match["matched_virtual_address"],
            "addiu_function": addiu_func["virtual_address"] if addiu_func else None,
            "addiu_stack_size": addiu_func.get("stack_adjustment") if addiu_func else None,
            "constructed_address": match["constructed_address"],
            "target": match["target_address"],
        }

        correlated.append(func_info)

        if lui_func:
            func_addr = lui_func["virtual_address"]
            if func_addr not in functions_with_refs:
                functions_with_refs[func_addr] = {
                    "virtual_address": func_addr,
                    "stack_size": lui_func.get("stack_adjustment"),
                    "file_offset": lui_func.get("file_offset"),
                    "references": [],
                }
            functions_with_refs[func_addr]["references"].append({
                "type": "lui",
                "address": match["lui_virtual_address"],
                "target": match["target_address"],
            })

        if addiu_func:
            func_addr = addiu_func["virtual_address"]
            if func_addr not in functions_with_refs:
                functions_with_refs[func_addr] = {
                    "virtual_address": func_addr,
                    "stack_size": addiu_func.get("stack_adjustment"),
                    "file_offset": addiu_func.get("file_offset"),
                    "references": [],
                }
            functions_with_refs[func_addr]["references"].append({
                "type": "addiu",
                "address": match["matched_virtual_address"],
                "target": match["target_address"],
            })

    target_summary = {}
    for target in targets:
        addr = target["address"]
        refs = target_refs.get(addr, [])
        unique_functions = set()

        for ref in refs:
            lui_addr = parse_virtual_address(ref["lui_virtual_address"])
            func = find_containing_function(lui_addr, prologues)
            if func:
                unique_functions.add(func["virtual_address"])

        target_summary[addr] = {
            "total_references": len(refs),
            "unique_functions": len(unique_functions),
            "function_list": sorted(unique_functions),
        }

    return {
        "tool": TOOL_NAME,
        "tool_version": TOOL_VERSION,
        "generated_at_utc": utc_now(),
        "legal_boundary": "metadata-only; correlation of existing metadata only",
        "source": {
            "immediate_report": str(immediate_report_path),
            "prologue_report": str(prologue_report_path),
        },
        "summary": {
            "total_immediate_matches": len(matches),
            "unique_functions_with_refs": len(functions_with_refs),
            "targets_analyzed": len(targets),
        },
        "targets": target_summary,
        "functions_with_data_refs": sorted(
            functions_with_refs.values(),
            key=lambda x: parse_virtual_address(x["virtual_address"])
        ),
        "correlation_details": correlated[: args.max_details] if args.max_details > 0 else correlated,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Correlate function prologues with code references."
    )
    parser.add_argument(
        "--immediate-report",
        type=Path,
        required=True,
        help="Path to mips-immediate-scan JSON report.",
    )
    parser.add_argument(
        "--prologue-report",
        type=Path,
        required=True,
        help="Path to mips-prologue-scan JSON report.",
    )
    parser.add_argument(
        "--max-details",
        type=int,
        default=20,
        help="Maximum correlation details to include (default: 20, 0 for all).",
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
        report = build_report(args.immediate_report, args.prologue_report, args)

        output_dir = args.output_dir
        output_dir.mkdir(parents=True, exist_ok=True)

        timestamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        output_path = output_dir / f"{timestamp}-function-ref-correlator.json"
        output_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    except Exception as error:
        print(f"error: {error}")
        return 1
    print(f"function reference correlator report written: {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())