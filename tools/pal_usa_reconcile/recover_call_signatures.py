#!/usr/bin/env python3
"""Find conservative PAL→USA object anchors from resolved call relationships.

This is deliberately a candidate generator. A call signature never promotes a
symbol by itself: it reports only functions whose resolved PAL callees appear
at the same call ordinals in a USA function. A later byte/structural check and
multiple agreeing object offsets are required before creating a seed.
"""
from __future__ import annotations

import argparse
import csv
import json
import struct
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "research" / "pal-usa"
PAL_ELF = ROOT / ".local" / "extracted" / "pal" / "SCES_507.60"
USA_ELF = ROOT / ".local" / "extracted" / "SCUS_971.13.elf"
RELIABLE_METHODS = {"raw_sha1", "op_seq_hash", "norm_sha1"}


def elf_text(path: Path) -> tuple[int, bytes]:
    from elftools.elf.elffile import ELFFile
    with path.open("rb") as handle:
        elf = ELFFile(handle)
        for section in elf.iter_sections():
            if section.name == ".text":
                return section.header.sh_addr, section.data()
    raise RuntimeError(f"no .text section in {path}")


def calls(data: bytes, text_va: int, va: int, size: int) -> list[int]:
    offset = va - text_va
    result = []
    for local in range(0, size - 3, 4):
        word = struct.unpack_from("<I", data, offset + local)[0]
        if word >> 26 == 3:  # jal
            pc = va + local + 4
            result.append((pc & 0xF0000000) | ((word & 0x03FFFFFF) << 2))
    return result


def load_rows(path: Path) -> list[dict]:
    with path.open() as handle:
        return list(csv.DictReader(handle))


def load_reliable_seeds() -> dict[int, int]:
    seeds: dict[int, int] = {}
    for row in load_rows(ROOT / "docs" / "symbols" / "pal_usa_symbol_map.csv"):
        if row["status"] != "verified" or row["match_method"] not in RELIABLE_METHODS:
            continue
        if not row["usa_va"]:
            continue
        seeds[int(row["pal_va"], 16)] = int(row["usa_va"], 16)
    return seeds


def load_high_range_anchors(existing: dict[int, int]) -> dict[int, int]:
    """Load derived anchors for candidate generation, never seed promotion.

    These rows have an op-sequence-and-size match inside a HIGH-confidence
    object range. They are weaker than direct fingerprints because the object
    offset is derived, so callers must treat every result as candidate evidence.
    """
    high_objects = {
        row["object_file"]
        for row in load_rows(OUT / "pal_usa_object_range_candidates.csv")
        if row["confidence"] == "HIGH" and int(row["functions_mapped"]) >= 2
    }
    derived: dict[int, int] = {}
    for row in load_rows(OUT / "pal_usa_function_map_candidates.csv"):
        if row["object_file"] not in high_objects or row["status"] != "MATCH":
            continue
        pal_va = int(row["pal_va"], 16)
        if pal_va not in existing:
            derived[pal_va] = int(row["usa_va"], 16)
    return derived


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--object", action="append", dest="objects",
                        help="MAIN.MAP object to inspect; repeatable")
    parser.add_argument("--include-high-range-anchors", action="store_true",
                        help="use derived HIGH-range matches for candidate generation only")
    args = parser.parse_args()
    targets = set(args.objects or ["girl_act.o", "end.o", "GifPacket.o"])

    pal_text_va, pal_text = elf_text(PAL_ELF)
    usa_text_va, usa_text = elf_text(USA_ELF)
    seeds = load_reliable_seeds()
    direct_seed_count = len(seeds)
    derived_anchor_count = 0
    if args.include_high_range_anchors:
        derived = load_high_range_anchors(seeds)
        derived_anchor_count = len(derived)
        seeds.update(derived)
    functions = load_rows(OUT / "main_map_functions.csv")
    objects = {row["object_file"]: row for row in load_rows(OUT / "main_map_objects.csv")}
    with (ROOT / "docs" / "symbols" / "usa_fingerprints.json").open() as handle:
        usa_functions = json.load(handle)

    by_object: dict[str, list[dict]] = defaultdict(list)
    for row in functions:
        by_object[row["object_file"]].append(row)
    for rows in by_object.values():
        rows.sort(key=lambda row: int(row["pal_va"], 16))

    usa_call_cache = {
        item["va"]: calls(usa_text, usa_text_va, item["va"], item["size"])
        for item in usa_functions
    }
    usa_by_signature: dict[tuple[tuple[int, int], ...], list[int]] = defaultdict(list)

    # Index every non-empty subsequence of resolved calls at its original ordinal.
    # The target side requires at least two entries, so single-call noise cannot
    # create a report candidate.
    for usa_va, targets_called in usa_call_cache.items():
        for start in range(len(targets_called)):
            for end in range(start + 2, len(targets_called) + 1):
                signature = tuple((i, targets_called[i]) for i in range(start, end))
                usa_by_signature[signature].append(usa_va)

    rows_out = []
    object_offsets: dict[str, dict[int, list[dict]]] = defaultdict(lambda: defaultdict(list))
    for obj_name in sorted(targets):
        rows = by_object.get(obj_name, [])
        obj_end = int(objects[obj_name]["pal_end"], 16) if obj_name in objects else 0
        for index, row in enumerate(rows):
            pal_va = int(row["pal_va"], 16)
            next_va = int(rows[index + 1]["pal_va"], 16) if index + 1 < len(rows) else obj_end
            pal_calls = calls(pal_text, pal_text_va, pal_va, max(next_va - pal_va, 0))
            signature = tuple((ordinal, seeds[target]) for ordinal, target in enumerate(pal_calls)
                              if target in seeds)
            candidates = usa_by_signature.get(signature, []) if len(signature) >= 2 else []
            for usa_va in candidates:
                object_offsets[obj_name][usa_va - pal_va].append({
                    "name": row["name"], "pal_va": pal_va, "usa_va": usa_va,
                    "resolved_calls": len(signature),
                })
            rows_out.append({
                "object_file": obj_name,
                "name": row["name"],
                "pal_va": f"0x{pal_va:08X}",
                "resolved_calls": len(signature),
                "usa_candidates": len(candidates),
            })

    suffix = "_with_high_range_anchors" if args.include_high_range_anchors else ""
    csv_path = OUT / f"call_signature_candidates{suffix}.csv"
    with csv_path.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows_out[0]) if rows_out else
                                ["object_file", "name", "pal_va", "resolved_calls", "usa_candidates"])
        writer.writeheader()
        writer.writerows(rows_out)

    report_path = OUT / f"call_signature_recovery{suffix}.md"
    with report_path.open("w") as handle:
        handle.write("# Call-signature recovery candidates\n\n")
        handle.write("This report is candidate evidence only. No row is a PAL→USA seed.\n\n")
        handle.write(f"Direct reliable PAL→USA callee seeds available: {direct_seed_count}\n")
        if args.include_high_range_anchors:
            handle.write(f"Derived HIGH-range anchors for candidate generation: {derived_anchor_count}\n")
        handle.write(f"Call targets available to this run: {len(seeds)}\n\n")
        for obj_name in sorted(targets):
            object_rows = [row for row in rows_out if row["object_file"] == obj_name]
            supported = [row for row in object_rows if row["resolved_calls"] >= 2]
            handle.write(f"## {obj_name}\n\n")
            handle.write(f"Functions: {len(object_rows)}; functions with ≥2 resolved callees: {len(supported)}.\n\n")
            groups = object_offsets[obj_name]
            multi = [(offset, values) for offset, values in groups.items() if len(values) >= 2]
            if not multi:
                handle.write("No shared object offset with two independent call-signature matches.\n\n")
                continue
            handle.write("| Offset | Functions | Minimum resolved calls |\n|---|---:|---:|\n")
            for offset, values in sorted(multi, key=lambda value: (-len(value[1]), value[0])):
                handle.write(f"| 0x{offset:08X} | {len(values)} | {min(v['resolved_calls'] for v in values)} |\n")
            handle.write("\n")

    print(f"[OK] {csv_path}")
    print(f"[OK] {report_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
