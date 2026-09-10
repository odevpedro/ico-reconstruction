#!/usr/bin/env python3
"""extract_room_role_tables.py — Rev.159 (Passo 1): verified per-room role plans.

Reads PCSX2 runtime JSONL captures (probes: world_state_load boundaries,
init_scene_gobj a0 = scene being entered, isys_gobj_proc_add a1 = GObj pool
address, a2 = handler address, a3 = primary-flag) and emits a C++ header with
the verified per-room primary-handler repertoire:

    for each world_state_load segment:
        sceneId   = dominant init_scene_gobj a0 in that segment
        roles     = distinct GObj pool addresses per handler (a3==1, a2!=0),
                    named via asm_source_score.TARGET_FUNCTIONS

The count per role is the number of DISTINCT GObj pool addresses that carried
that handler as primary in that room segment. Because the GObj pool is reused
across rooms, these are the strongest reproducible per-room signals available;
they are NOT host GObj handles (host-side pairing is a documented HOST
heuristic, see KanbanSceneLoader).

Output: native/src/game/GeneratedRoomRoleTables.h — AUTO-GENERATED, do not
edit by hand. Re-run the tool after adding captures.

Usage:
    python3 tools/extract_room_role_tables.py [log.jsonl ...]
Default: all .local/pcsx2-logs/session-*.jsonl (only the extracted one is
ranked first; the raw .env-expanded captures are skipped automatically).
"""

import argparse
import json
import os
import re
import sys
from collections import Counter, defaultdict

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def load_target_functions():
    """Resolve ico_ptr handler -> function name from asm_source_score.py."""
    path = os.path.join(PROJECT_ROOT, "tools", "asm_source_score.py")
    name_of = {}
    try:
        with open(path, "r", encoding="utf-8") as fh:
            src = fh.read()
    except OSError as exc:
        print(f"warning: cannot read {path}: {exc}", file=sys.stderr)
        return name_of
    match = re.search(r"^TARGET_FUNCTIONS\s*=\s*\[(.*?)\n\]",
                      src, re.S | re.M)

    if not match:
        print("warning: TARGET_FUNCTIONS not found", file=sys.stderr)
        return name_of
    for entry in re.finditer(r"\(\s*(['\"])([^\"]+)\1\s*,\s*0x([0-9a-fA-F]+)",
                             match.group(1)):
        name_of[int(entry.group(3), 16)] = entry.group(2)
    return name_of


def iter_segments(events):
    """Split the event stream at world_state_load; each segment gets its
    dominant init_scene_gobj a0 as the sceneId of the room being entered."""
    markers = [i for i, e in enumerate(events) if e["label"] == "world_state_load"]
    boundaries = markers + [len(events)]
    for start, end in zip([0] + markers, boundaries):
        chunk = events[start:end]
        enters = [
            int(e["regs"]["a0"], 16)
            for e in chunk if e["label"] == "init_scene_gobj"
        ]
        if not enters:
            continue
        scene_id = max(Counter(enters).items(), key=lambda kv: kv[1])[0]

        # distinct GObj pool addr -> set of primary handlers (a3==1, a2!=0)
        gobj_handlers = defaultdict(set)
        for e in chunk:
            if e["label"] != "isys_gobj_proc_add":
                continue
            if int(e["regs"]["a3"], 16) != 1:
                continue
            gobj = e["regs"].get("a1", "0")
            handler = int(e["regs"].get("a2", "0"), 16)
            if handler == 0:
                continue
            gobj_handlers[gobj].add(handler)

        per_handler = Counter()
        for handlers in gobj_handlers.values():
            for handler in handlers:
                per_handler[handler] += 1
        yield scene_id, per_handler


def load_events(paths):
    events = []
    for path in paths:
        with open(path, "r", encoding="utf-8", errors="replace") as fh:
            for lineno, line in enumerate(fh, 1):
                line = line.strip()
                if not line:
                    continue
                try:
                    events.append(json.loads(line))
                except json.JSONDecodeError:
                    print(f"warning: {path}:{lineno}: skipped bad JSON line",
                          file=sys.stderr)
    return events


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "logs", nargs="*",
        default=None,
        help="JSONL capture paths; default: all extracted session-*.jsonl")
    args = parser.parse_args()

    logs = args.logs
    if logs is None:
        log_dir = os.path.join(PROJECT_ROOT, ".local", "pcsx2-logs")
        logs = sorted(os.path.join(log_dir, p) for p in os.listdir(log_dir)
                      if p.startswith("session-") and p.endswith(".jsonl"))
    if not logs:
        print("no log files found", file=sys.stderr)
        return 2

    events = load_events(logs)
    if not events:
        print("no events read", file=sys.stderr)
        return 2
    print(f"loaded {len(events)} events from {len(logs)} log(s)",
          file=sys.stderr)

    name_of = load_target_functions()

    plans_by_scene = {}
    for scene_id, per_handler in iter_segments(events):
        if scene_id in plans_by_scene:
            plans_by_scene[scene_id].update(per_handler)
        else:
            plans_by_scene[scene_id] = per_handler

    if not plans_by_scene:
        print("no scene segments found", file=sys.stderr)
        return 2

    out_lines = []
    out_lines.append("// GeneratedRoomRoleTables.h - verified ICO USA per-room primary-handler")
    out_lines.append("// repertoires collected from PCSX2 runtime captures (Rev.159).")
    out_lines.append("//")
    out_lines.append("// AUTO-GENERATED by tools/extract_room_role_tables.py - do not edit by hand.")
    out_lines.append("// Re-run the tool after adding a runtime capture.")
    out_lines.append("//")
    out_lines.append("// Per sceneId (the room being entered, init_scene_gobj a0):")
    out_lines.append("//   roleCount = number of DISTINCT GObj pool addresses that carried this")
    out_lines.append("//   handler as primary (isys_gobj_proc_add a3==1) during that room segment.")
    out_lines.append("// The GObj pool is reused across rooms; these are pool-address counts, not")
    out_lines.append("// host GObj handles. Host pairing is a documented HOST heuristic.")
    out_lines.append("#pragma once")
    out_lines.append("")
    out_lines.append("#include \"core/gobj_abi.h\"")
    out_lines.append("")
    out_lines.append("namespace ico::engine {")
    out_lines.append("")
    out_lines.append("struct VerifiedRoomRole {")
    out_lines.append("    ico_ptr32 handlerAddr;")
    out_lines.append("    const char* handlerName;")
    out_lines.append("    u16 roleCount;")
    out_lines.append("};")
    out_lines.append("")
    out_lines.append("struct VerifiedRoomRolePlan {")
    out_lines.append("    u16 sceneId;")
    out_lines.append("    const VerifiedRoomRole* roles;")
    out_lines.append("    u16 roleCount;")
    out_lines.append("};")
    out_lines.append("")

    counts = 0
    arrays = []
    plan_rows = []
    total_roles = 0
    total_valid_scenes = 0
    for scene_id in sorted(plans_by_scene):
        per_handler = plans_by_scene[scene_id]
        # drop unnamed/zero producers only; keep the full verified repertoire
        roles = [(addr, c) for addr, c in per_handler.items() if c > 0]
        if not roles:
            continue
        roles.sort(key=lambda ac: (-ac[1], ac[0]))
        arr = f"kVerifiedRoomRolesScene{scene_id:02X}"
        arrays.append(f"constexpr VerifiedRoomRole {arr}[] = {{")
        for addr, count in roles:
            name = name_of.get(addr, f"runtime_{addr & 0xFFFFFF:06X}")
            arrays.append(f"    {{0x{addr:08X}u, \"{name}\", {count}u}},")
            total_roles += count
        arrays.append("};")
        arrays.append("")
        plan_rows.append(
            f"    {{0x{scene_id:02X}u, {arr}, "
            f"{len(roles)}u}},")
        total_valid_scenes += 1
        counts += 1

    out_lines.extend(arrays)
    out_lines.append(f"constexpr std::size_t kVerifiedRoomRolePlanCount = {counts};")
    out_lines.append("")
    out_lines.append("constexpr VerifiedRoomRolePlan kVerifiedRoomRolePlans[] = {")
    out_lines.extend(plan_rows)
    out_lines.append("};")
    out_lines.append("")
    out_lines.append("}  // namespace ico::engine")
    out_lines.append("")

    out_path = os.path.join(PROJECT_ROOT, "native", "src", "game",
                            "GeneratedRoomRoleTables.h")
    with open(out_path, "w", encoding="utf-8") as fh:
        fh.write("\n".join(out_lines))
    print(f"wrote {out_path}")
    print(f"  {counts} scenes, {total_valid_scenes} with roles, "
          f"{total_roles} total distinct-GObj role slots")
    for scene_id in sorted(plans_by_scene):
        total = sum(c for _, c in plans_by_scene[scene_id].items() if c > 0)
        print(f"  scene 0x{scene_id:02X}: {total} role slots across "
              f"{len(plans_by_scene[scene_id])} handlers")
    return 0


if __name__ == "__main__":
    sys.exit(main())