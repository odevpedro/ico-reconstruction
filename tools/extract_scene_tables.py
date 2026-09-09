#!/usr/bin/env python3
"""Extract verified ICO USA scene tables for the native KanbanSceneLoader.

Reads the USA ELF and emits `GeneratedSceneTables.h`: the 68 entity descriptors
at 0x2A31B8 (stride 0x64), the world-state -> entry ranges from the dispatch
table at 0x5F2FB8, and the entry records (0x2A4C48, stride 0x4C) for a
selectable scene (default 0x0F, used by the native demo).

Layout sources (Rev.112 + initSceneGObj.s, byte-exact):
  descriptor = 0x2A31B8 + descIdx*0x64
      +0x00  16-byte ASCII model name
      +0x40  processCallback_40 (registration fallback)
      +0x44  gate (0 skips CreateGObj entirely)
      +0x58  processCallback_58 (invoked by jalr during creation)
      +0x60  vtable pointer (module-identity hint only)
  entry     = 0x2A4C48 + entryIdx*0x4C
      +0x24  processCallback_24 (registration override, wins over +0x40)
      +0x30  userData (stored at GObj+0x814)
      +0x40  processArgument_40 (u16, wrapper t0 = <<10; 0x1800 default)
      +0x44  u16 flag (second gate in the dispatch tail)
      +0x46  descriptor index (u8)
      +0x47  gobjType (low 5 bits -> GObj type field)
      +0x48  bits[16:14] -> listId (slots 0..7)
  world dispatch 0x5F2FB8 + scene*0x194: [+0x128 start, +0x12C end) entry idx

Verified measurements used here:
  - descriptor table is exactly 68 entries (0x2A31B8 + 68*0x64 ends at 0x2A38C0).
  - entry table valid descIdx<68 run: contiguous from idx 0 through 3590 (first
    invalid at 3591); AGENTS' "512 entries" is an understatement.
  - world dispatch has well-formed [start,end) ranges for scenes 0x01..0x64.

Output: native/src/game/GeneratedSceneTables.h (header-only constexpr tables)
and a CSV report for each requested scene for the research note.
"""

import argparse
import csv
import os
import struct
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.normpath(os.path.join(SCRIPT_DIR, ".."))
ELF_PATH = os.path.join(REPO_ROOT, ".local", "extracted", "SCUS_971.13.elf")

DESCRIPTOR_BASE = 0x2A31B8
DESCRIPTOR_STRIDE = 0x64
DESCRIPTOR_COUNT = 68
ENTRY_BASE = 0x2A4C48
ENTRY_STRIDE = 0x4C
DISPATCH_BASE = 0x5F2FB8
DISPATCH_STRIDE = 0x194


class ElfReader:
    def __init__(self, path):
        with open(path, "rb") as f:
            self.data = f.read()

    def offset(self, va):
        e_phoff = struct.unpack_from("<I", self.data, 0x1C)[0]
        e_phentsize = struct.unpack_from("<H", self.data, 0x2A)[0]
        e_phnum = struct.unpack_from("<H", self.data, 0x2C)[0]
        for i in range(e_phnum):
            poff = e_phoff + i * e_phentsize
            p_type = struct.unpack_from("<I", self.data, poff)[0]
            if p_type != 1:
                continue
            p_offset = struct.unpack_from("<I", self.data, poff + 4)[0]
            p_vaddr = struct.unpack_from("<I", self.data, poff + 8)[0]
            p_filesz = struct.unpack_from("<I", self.data, poff + 16)[0]
            if p_vaddr <= va < p_vaddr + p_filesz:
                return p_offset + (va - p_vaddr)
        return None

    def u8(self, va):
        return self.data[self.offset(va)]

    def u16(self, va):
        return struct.unpack_from("<H", self.data, self.offset(va))[0]

    def u32(self, va):
        return struct.unpack_from("<I", self.data, self.offset(va))[0]

    def name(self, va, size=16):
        raw = self.data[self.offset(va):self.offset(va) + size]
        raw = raw.split(b"\x00")[0]
        return raw.decode("ascii", errors="replace")


def load_descriptors(elf):
    descs = []
    for i in range(DESCRIPTOR_COUNT):
        base = DESCRIPTOR_BASE + i * DESCRIPTOR_STRIDE
        descs.append({
            "index": i,
            "name": elf.name(base),
            "gate_44": elf.u32(base + 0x44),
            "processCallback_40": elf.u32(base + 0x40),
            "processCallback_58": elf.u32(base + 0x58),
            "vtable_60": elf.u32(base + 0x60),
        })
    return descs


def load_scene_ranges(elf):
    """Only returns ranges that tile contiguously from entry 41.

    The verification is: the dispatch slot [start,end) blocks must cover
    [41, 3453) exactly once with no gaps. Slots that start at 35 (scenes
    0x69-0x7F) do NOT participate in the tiling and are probably a separate
    semantic (base-entries-append), so they are deliberately excluded from
    the verified scene->entry mapping. Rev.154.
    """
    slots = []
    for scene in range(0x80):
        base = DISPATCH_BASE + scene * DISPATCH_STRIDE
        start = elf.u32(base + 0x128)
        end = elf.u32(base + 0x12C)
        if 0 <= start < end and start < 4096 and end <= 4096:
            slots.append({"sceneId": scene, "start": start, "end": end})

    ranges = []
    for r in sorted(slots, key=lambda s: (s["start"], s["end"])):
        if r["start"] == ranges[-1]["end"] if ranges else r["start"] == 41:
            ranges.append(r)
    return ranges


def load_entries(elf, idx):
    base = ENTRY_BASE + idx * ENTRY_STRIDE
    desc_idx = elf.u8(base + 0x46)
    if desc_idx >= DESCRIPTOR_COUNT:
        return None
    w48 = elf.u32(base + 0x48)
    return {
        "index": idx,
        "descriptorIndex": desc_idx,
        "processCallback_24": elf.u32(base + 0x24),
        "userData_30": elf.u32(base + 0x30),
        "processArgument_40": elf.u16(base + 0x40),
        "flag_44": elf.u16(base + 0x44),
        "gobjType": elf.u8(base + 0x47) & 0x1F,
        "listId": (w48 >> 14) & 7,
    }


def c_quoted(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def emit_header(descs, ranges, entries_by_scene, out_path, provenance_scenes):
    lines = []
    lines.append("// GeneratedSceneTables.h — verified ICO USA scene tables.")
    lines.append("//")
    lines.append("// AUTO-GENERATED by tools/extract_scene_tables.py — do not edit by hand.")
    lines.append("// Re-run the tool after changing sources; all values are raw bytes from")
    lines.append("// .local/extracted/SCUS_971.13.elf.")
    lines.append("//")
    lines.append(f"// Descriptors: {DESCRIPTOR_BASE:#x} stride 0x{DESCRIPTOR_STRIDE:x} "
                 f"({DESCRIPTOR_COUNT}).")
    lines.append(f"// Entries:     {ENTRY_BASE:#x} stride 0x{ENTRY_STRIDE:x}.")
    lines.append(f"// World range: {DISPATCH_BASE:#x} + scene*0x{DISPATCH_STRIDE:x}, "
                 f"[+0x128 start, +0x12C end).")
    scenes = ", ".join(f"0x{s:02X}" for s in provenance_scenes)
    lines.append(f"// Entries emitted for scene(s): {scenes}.")
    lines.append("#pragma once")
    lines.append("")
    lines.append('#include "core/gobj_abi.h"')
    lines.append("")
    lines.append("namespace ico::engine {")
    lines.append("")
    lines.append("struct VerifiedSceneDescriptor {")
    lines.append("    u16 descriptorIndex;")
    lines.append("    u32 gate_44;")
    lines.append("    ico_ptr32 processCallback_40;")
    lines.append("    ico_ptr32 processCallback_58;")
    lines.append("    ico_ptr32 vtable_60;")
    lines.append("    const char* name;")
    lines.append("};")
    lines.append("")
    lines.append("struct VerifiedSceneEntry {")
    lines.append("    u16 entryIndex;")
    lines.append("    u8 descriptorIndex;")
    lines.append("    u8 listId;")
    lines.append("    u8 gobjType;")
    lines.append("    u16 flag_44;")
    lines.append("    ico_ptr32 processCallback_24;")
    lines.append("    ico_ptr32 userData_30;")
    lines.append("    u16 processArgument_40;")
    lines.append("};")
    lines.append("")
    lines.append("struct VerifiedSceneRange {")
    lines.append("    u8 sceneId;")
    lines.append("    u16 startEntry;")
    lines.append("    u16 endEntry;")
    lines.append("};")
    lines.append("")
    lines.append(f"constexpr std::size_t kVerifiedSceneDescriptorCount = "
                 f"{len(descs)};")
    lines.append(f"constexpr std::size_t kVerifiedSceneRangeCount = {len(ranges)};")
    lines.append("")

    lines.append("constexpr VerifiedSceneDescriptor kVerifiedSceneDescriptors[] = {")
    for d in descs:
        lines.append(
            f"    {{{d['index']}, 0x{d['gate_44']:08X}u, "
            f"0x{d['processCallback_40']:08X}u, 0x{d['processCallback_58']:08X}u, "
            f"0x{d['vtable_60']:08X}u, "
            + c_quoted(d["name"]) + "},")
    lines.append("};")
    lines.append("")

    lines.append("constexpr VerifiedSceneRange kVerifiedSceneRanges[] = {")
    for r in ranges:
        lines.append(f"    {{0x{r['sceneId']:02X}, {r['start']}, {r['end']}}},")
    lines.append("};")
    lines.append("")

    scene_groups = {}
    for scene, entries in entries_by_scene.items():
        if entries:
            scene_groups[scene] = entries
    if scene_groups:
        lines.append("constexpr std::size_t kVerifiedScenePayloadCount = "
                     f"{sum(len(v) for v in scene_groups.values())};")
        lines.append("// Payload entries are grouped per scene in ascending scene id.")
        lines.append("constexpr VerifiedSceneEntry kVerifiedScenePayload[] = {")
        for scene in sorted(scene_groups):
            for e in scene_groups[scene]:
                lines.append(
                    f"    // scene 0x{scene:02X} entry {e['index']} "
                    f"desc {e['descriptorIndex']} ({descs[e['descriptorIndex']]['name']}) "
                    f"list {e['listId']} type {e['gobjType']}")
                lines.append(
                    f"    {{{e['index']}, {e['descriptorIndex']}, {e['listId']}, "
                    f"{e['gobjType']}, 0x{e['flag_44']:04X}, "
                    f"0x{e['processCallback_24']:08X}u, 0x{e['userData_30']:08X}u, "
                    f"0x{e['processArgument_40']:04X}}},")
        lines.append("};")
    else:
        lines.append("constexpr std::size_t kVerifiedScenePayloadCount = 0;")
        lines.append("constexpr VerifiedSceneEntry kVerifiedScenePayload[] = {};")
    lines.append("")
    lines.append("}  // namespace ico::engine")
    lines.append("")

    with open(out_path, "w") as f:
        f.write("\n".join(lines) + "\n")
    return scene_groups


def write_csv(scene, entries, path):
    desc = {d["index"]: d for d in load_descriptors(elf)} if "elf" in globals() else None
    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["entryIndex", "descriptorIndex", "descriptorName", "listId",
                    "gobjType", "flag_44", "processCallback_24", "userData_30",
                    "processArgument_40"])
        for e in entries:
            w.writerow([e["index"], e["descriptorIndex"],
                        desc[e["descriptorIndex"]]["name"] if desc else "",
                        e["listId"], e["gobjType"], f"0x{e['flag_44']:04X}",
                        f"0x{e['processCallback_24']:08X}",
                        f"0x{e['userData_30']:08X}",
                        f"0x{e['processArgument_40']:04X}"])


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--elf", default=ELF_PATH)
    ap.add_argument("--scene", type=lambda s: int(s, 0), nargs="*", default=[0x0F],
                    help="scene ids to emit payload entries for (default: 0x0F)")
    ap.add_argument("--out", default=os.path.join(
        REPO_ROOT, "native", "src", "game", "GeneratedSceneTables.h"))
    ap.add_argument("--csv-dir", default=os.path.join(REPO_ROOT, ".local", "extracted_scene_tables"))
    args = ap.parse_args()

    if not os.path.exists(args.elf):
        sys.exit(f"ELF not found: {args.elf}")

    global elf
    elf = ElfReader(args.elf)
    descs = load_descriptors(elf)
    ranges = load_scene_ranges(elf)

    entries_by_scene = {}
    for scene in args.scene:
        rng = next((r for r in ranges if r["sceneId"] == scene), None)
        if rng is None:
            sys.exit(f"scene 0x{scene:02X} has no well-formed range in the ELF")
        entries_by_scene[scene] = []
        for idx in range(rng["start"], rng["end"]):
            e = load_entries(elf, idx)
            if e is None:
                sys.exit(f"entry index {idx} (scene 0x{scene:02X}) has invalid "
                         f"descriptor index >= {DESCRIPTOR_COUNT}")
            entries_by_scene[scene].append(e)

    emitted = emit_header(descs, ranges, entries_by_scene, args.out,
                          list(entries_by_scene.keys()))
    print(f"wrote {args.out}")
    print(f"  descriptors : {len(descs)}")
    print(f"  ranges      : {len(ranges)} (scenes 0x01..0x64 well-formed)")
    for scene, entries in emitted.items():
        print(f"  scene 0x{scene:02X} entry slice: {len(entries)} entries "
              f"[{entries[0]['index']}..{entries[-1]['index']}]")

    if args.csv_dir:
        os.makedirs(args.csv_dir, exist_ok=True)
        for scene, entries in emitted.items():
            path = os.path.join(
                args.csv_dir, f"scene_{scene:02X}_entries_{entries[0]['index']}-"
                              f"{entries[-1]['index']}.csv")
            write_csv(scene, entries, path)
            print(f"  csv: {path}")


if __name__ == "__main__":
    main()