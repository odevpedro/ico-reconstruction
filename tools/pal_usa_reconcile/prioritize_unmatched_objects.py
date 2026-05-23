#!/usr/bin/env python3
"""Rank residual PAL->USA objects for the next offline recovery pass.

This does not recover new objects by itself. It ranks the current residual set
using three practical signals:

1. Impact: larger objects are more valuable to solve first.
2. Provenance: objects with source-file provenance from SRCFILE.TXT are more
   likely to be recoverable without runtime help.
3. Neighborhood: objects close to already solved objects in MAIN.MAP order are
   easier to propagate via offset reuse.

Outputs:
  - research/pal-usa/unmatched_objects_priority.csv
  - research/pal-usa/unmatched_objects_priority.md
"""

from __future__ import annotations

import csv
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Sequence

PROJECT_ROOT = Path(__file__).resolve().parent.parent.parent
OUT_DIR = PROJECT_ROOT / "research" / "pal-usa"


def load_csv(path: Path) -> list[dict]:
    with open(path, newline="") as f:
        return list(csv.DictReader(f))


def parse_offset(value: str | None) -> int | None:
    if not value:
        return None
    value = value.strip()
    if value.startswith("0x-"):
        return -int(value[3:], 16)
    if value.startswith("0x"):
        return int(value, 16)
    return int(value)


@dataclass(frozen=True)
class RankedObject:
    object_file: str
    function_count: int
    size: int
    source_files: tuple[str, ...]
    source_rows: int
    nearest_known_distance: int | None
    nearest_known_objects: tuple[str, ...]
    priority_score: int
    recoverability_score: int
    impact_score: int
    notes: str


def build_object_order(main_map_objects: Sequence[dict]) -> dict[str, int]:
    return {row["object_file"]: idx for idx, row in enumerate(main_map_objects)}


def build_known_offsets(range_candidates: Sequence[dict]) -> dict[str, int]:
    known: dict[str, int] = {}
    for row in range_candidates:
        if row.get("confidence") == "NONE":
            continue
        offset = parse_offset(row.get("offset_used") or row.get("offset"))
        if offset is not None:
            known[row["object_file"]] = offset
    return known


def build_source_map(source_rows: Sequence[dict]) -> tuple[dict[str, set[str]], dict[str, int]]:
    per_object: dict[str, set[str]] = {}
    counts: dict[str, int] = {}
    for row in source_rows:
        source_file = row.get("source_file") or ""
        if not source_file:
            continue
        obj = row["object_file"]
        per_object.setdefault(obj, set()).add(source_file)
        counts[obj] = counts.get(obj, 0) + 1
    return per_object, counts


def nearest_known_neighbors(
    object_file: str,
    object_index: dict[str, int],
    ordered_objects: list[str],
    known_objects: set[str],
    window: int = 5,
) -> tuple[int | None, tuple[str, ...]]:
    idx = object_index.get(object_file)
    if idx is None:
        return None, ()

    hits: list[tuple[int, str]] = []
    for delta in range(1, window + 1):
        left = idx - delta
        right = idx + delta
        if left >= 0:
            left_obj = ordered_objects[left]
            if left_obj in known_objects:
                hits.append((delta, left_obj))
        if right < len(ordered_objects):
            right_obj = ordered_objects[right]
            if right_obj in known_objects:
                hits.append((delta, right_obj))

    if not hits:
        return None, ()

    hits.sort(key=lambda item: (item[0], item[1]))
    min_distance = hits[0][0]
    nearest = tuple(obj for dist, obj in hits if dist == min_distance)
    return min_distance, nearest


def score_object(
    object_file: str,
    function_count: int,
    size: int,
    source_files: set[str],
    source_row_count: int,
    nearest_known_distance: int | None,
    nearest_known_objects: Sequence[str],
) -> tuple[int, int, int, str]:
    impact_score = function_count * 2 + min(20, size // 2000)

    source_score = 0
    if source_files:
        source_score += 30
        source_score += min(20, source_row_count // 2)

    if source_files and nearest_known_distance is not None:
        source_score += max(0, 20 - (nearest_known_distance - 1) * 4)

    if len(nearest_known_objects) >= 2:
        source_score += 8

    if nearest_known_distance is not None:
        source_score += max(0, 10 - nearest_known_distance)

    priority_score = impact_score + source_score
    notes = []
    if source_files:
        notes.append("SRCFILE")
    if nearest_known_distance is not None:
        notes.append(f"near={nearest_known_distance}")
    if function_count >= 20:
        notes.append("large")

    return priority_score, source_score, impact_score, ", ".join(notes)


def main() -> None:
    unmatched_rows = load_csv(OUT_DIR / "unmatched_objects.csv")
    object_rows = load_csv(OUT_DIR / "main_map_objects.csv")
    range_rows = load_csv(OUT_DIR / "pal_usa_object_range_candidates.csv")
    source_rows = load_csv(OUT_DIR / "main_map_functions_source.csv")

    object_index = build_object_order(object_rows)
    ordered_objects = [row["object_file"] for row in object_rows]
    known_offsets = build_known_offsets(range_rows)
    source_map, source_counts = build_source_map(source_rows)
    known_objects = set(known_offsets)

    ranked: list[RankedObject] = []
    for row in unmatched_rows:
        object_file = row["object_file"]
        function_count = int(row["function_count"])
        obj_meta = next((o for o in object_rows if o["object_file"] == object_file), {})
        size = int(obj_meta.get("size") or 0)
        source_files = tuple(sorted(source_map.get(object_file, set())))
        source_row_count = source_counts.get(object_file, 0)
        nearest_distance, nearest_objects = nearest_known_neighbors(
            object_file,
            object_index,
            ordered_objects,
            known_objects,
        )
        priority_score, recoverability_score, impact_score, notes = score_object(
            object_file,
            function_count,
            size,
            set(source_files),
            source_row_count,
            nearest_distance,
            nearest_objects,
        )
        ranked.append(
            RankedObject(
                object_file=object_file,
                function_count=function_count,
                size=size,
                source_files=source_files,
                source_rows=source_row_count,
                nearest_known_distance=nearest_distance,
                nearest_known_objects=nearest_objects,
                priority_score=priority_score,
                recoverability_score=recoverability_score,
                impact_score=impact_score,
                notes=notes,
            )
        )

    ranked.sort(
        key=lambda item: (
            item.priority_score,
            item.recoverability_score,
            item.function_count,
            item.size,
            item.object_file,
        ),
        reverse=True,
    )

    csv_path = OUT_DIR / "unmatched_objects_priority.csv"
    with open(csv_path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(
            [
                "rank",
                "object_file",
                "function_count",
                "size",
                "source_files",
                "source_rows",
                "nearest_known_distance",
                "nearest_known_objects",
                "priority_score",
                "recoverability_score",
                "impact_score",
                "notes",
            ]
        )
        for idx, item in enumerate(ranked, start=1):
            w.writerow(
                [
                    idx,
                    item.object_file,
                    item.function_count,
                    item.size,
                    ";".join(item.source_files),
                    item.source_rows,
                    "" if item.nearest_known_distance is None else item.nearest_known_distance,
                    ";".join(item.nearest_known_objects),
                    item.priority_score,
                    item.recoverability_score,
                    item.impact_score,
                    item.notes,
                ]
            )

    md_path = OUT_DIR / "unmatched_objects_priority.md"
    with open(md_path, "w") as f:
        f.write("# Unmatched Objects Priority\n\n")
        f.write("Date: 2026-05-23\n\n")
        f.write("## Objective\n\n")
        f.write("Rank the current residual PAL->USA objects so the next offline pass can target the highest-value, highest-probability cases first.\n\n")
        f.write("## Criteria\n\n")
        f.write("- Larger objects get higher impact weight.\n")
        f.write("- Objects with SRCFILE.TXT provenance get a recoverability bonus.\n")
        f.write("- Objects close to already solved MAIN.MAP neighbors get a neighborhood bonus.\n\n")
        f.write("## Top Candidates\n\n")
        f.write("| Rank | Object | Funcs | Priority | Recoverability | Impact | Source files | Nearest known |\n")
        f.write("|------|--------|-------|----------|----------------|--------|--------------|---------------|\n")
        for idx, item in enumerate(ranked[:20], start=1):
            f.write(
                f"| {idx} | {item.object_file} | {item.function_count} | {item.priority_score} | "
                f"{item.recoverability_score} | {item.impact_score} | "
                f"{';'.join(item.source_files) or '-'} | "
                f"{';'.join(item.nearest_known_objects) or '-'} |\n"
            )
        f.write("\n## Notes\n\n")
        f.write(f"- Residual objects ranked: {len(ranked)}\n")
        f.write(f"- Residual functions represented: {sum(item.function_count for item in ranked)}\n")
        f.write(f"- Objects with source provenance: {sum(1 for item in ranked if item.source_files)}\n")
        f.write(f"- Objects with a known neighbor within 5 positions: {sum(1 for item in ranked if item.nearest_known_distance is not None)}\n")
        f.write("\n## Next Step\n\n")
        f.write("Attack the top-ranked objects in order, but only with offline methods first: source-provenance joins, neighbor offset propagation, and multi-anchor validation.\n")

    print(f"[OK] Priority CSV -> {csv_path}")
    print(f"[OK] Priority report -> {md_path}")
    print(f"[INFO] Residual objects ranked: {len(ranked)}")
    print("[INFO] Top 10:")
    for idx, item in enumerate(ranked[:10], start=1):
        print(
            f"  {idx:02d}. {item.object_file:24s} "
            f"score={item.priority_score:3d} "
            f"recover={item.recoverability_score:3d} "
            f"impact={item.impact_score:3d} "
            f"funcs={item.function_count:3d} "
            f"src={','.join(item.source_files) or '-'}"
        )


if __name__ == "__main__":
    main()
