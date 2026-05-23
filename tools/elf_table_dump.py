#!/usr/bin/env python3
"""Dump known data tables from the ICO PS2 ELF binary.

Usage:
    python3 tools/elf_table_dump.py <table_name> [--json]

Tables:
    descriptors      — Entity descriptor table (0x2A31B8, 68 entries, stride 0x64)
    entry            — Entry table (0x2A4C48, 512 entries, stride 0x4C)
    dispatch_32      — 32-slot dispatch table at 0x281AB0
    clip_config       — 17-slot _Clip collision config (0x282690)
    thread           — Thread table (0x6A6F30)
    scene_loader_jt  — Scene loader 21-stage jump table (0x616FD0)
    halfword_grid    — 32x32 halfword spatial hash grid (0x006AB080)
    gp_vars          — Known gp-relative variables
"""

import os
import struct
import sys
import json

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ELF_PATH = os.path.join(SCRIPT_DIR, "..", ".local", "extracted", "SCUS_971.13.elf")


def load_elf():
    import os
    path = os.path.expanduser(ELF_PATH.replace("~/.", "."))
    with open(path, "rb") as f:
        return f.read()


def elf_addr_to_offset(elf, va):
    e_phoff = struct.unpack_from("<I", elf, 0x1C)[0]
    e_phentsize = struct.unpack_from("<H", elf, 0x2A)[0]
    e_phnum = struct.unpack_from("<H", elf, 0x2C)[0]
    for i in range(e_phnum):
        poff = e_phoff + i * e_phentsize
        p_type = struct.unpack_from("<I", elf, poff)[0]
        if p_type != 1:
            continue
        p_offset = struct.unpack_from("<I", elf, poff + 4)[0]
        p_vaddr = struct.unpack_from("<I", elf, poff + 8)[0]
        p_filesz = struct.unpack_from("<I", elf, poff + 16)[0]
        if p_vaddr <= va < p_vaddr + p_filesz:
            return p_offset + (va - p_vaddr)
    return None


def read_u32(elf, off):
    return struct.unpack_from("<I", elf, off)[0]


def dump_descriptors(elf, fout):
    """Entity descriptor table: 0x2A31B8, 68 entries, stride 0x64."""
    base_va = 0x2A31B8
    stride = 0x64
    num = 68
    off = elf_addr_to_offset(elf, base_va)
    if off is None:
        fout.write("ERROR: descriptor table address not in ELF\n")
        return
    fout.write(f"=== Entity Descriptor Table (0x{base_va:08X}) ===\n")
    fout.write(f"{'Idx':<5} {'Name':<12} {'init_fn':<18} {'flags':<10} {'type':<10} {'size':<8}\n")
    fout.write(f"{'-'*5} {'-'*12} {'-'*18} {'-'*10} {'-'*10} {'-'*8}\n")
    for i in range(num):
        e_off = off + i * stride
        name = elf[e_off:e_off + 4].rstrip(b"\x00").decode("ascii", errors="replace")
        init_fn = read_u32(elf, e_off + 0x04)
        flags = read_u32(elf, e_off + 0x08)
        size = read_u32(elf, e_off + 0x10)
        type_val = read_u32(elf, e_off + 0x14)
        if name:
            fout.write(f"{i:<5} {name:<12} 0x{init_fn:08X} 0x{flags:08X} 0x{type_val:08X} 0x{size:X}\n")


def dump_entry_table(elf, fout):
    """Entry table: 0x2A4C48, 512 entries, stride 0x4C."""
    base_va = 0x2A4C48
    stride = 0x4C
    num = 512
    off = elf_addr_to_offset(elf, base_va)
    if off is None:
        fout.write("ERROR: entry table address not in ELF\n")
        return
    fout.write(f"=== Entry Table (0x{base_va:08X}) ===\n")
    fout.write(f"{'Idx':<5} {'type':<10} {'offset':<10} {'flags':<10} {'descr_idx':<12}\n")
    fout.write(f"{'-'*5} {'-'*10} {'-'*10} {'-'*10} {'-'*12}\n")
    for i in range(num):
        e_off = off + i * stride
        type_val = read_u32(elf, e_off)
        offset = read_u32(elf, e_off + 0x04)
        flags = read_u32(elf, e_off + 0x08)
        descr_idx = read_u32(elf, e_off + 0x0C)
        if type_val != 0 or descr_idx != 0:
            fout.write(f"{i:<5} 0x{type_val:08X} 0x{offset:08X} 0x{flags:08X} 0x{descr_idx:08X}\n")


def dump_dispatch_32(elf, fout):
    """32-slot dispatch/display list table at 0x281AB0."""
    base_va = 0x281AB0
    stride = 8
    num = 32
    off = elf_addr_to_offset(elf, base_va)
    if off is None:
        fout.write("ERROR: dispatch table not in ELF\n")
        return
    fout.write(f"=== 32-slot Dispatch Table (0x{base_va:08X}) ===\n")
    fout.write(f"{'Slot':<6} {'word0':<12} {'word1':<12} {'notes'}\n")
    fout.write(f"{'-'*6} {'-'*12} {'-'*12} {'-'*40}\n")
    for i in range(num):
        e_off = off + i * stride
        w0 = read_u32(elf, e_off)
        w1 = read_u32(elf, e_off + 4)
        notes = ""
        if w0 == 0 and w1 == 0:
            notes = "(unused)"
        elif w0 > 0x00100000 and w0 < 0x02A00000:
            notes = f"-> 0x{w0:08X}"
        else:
            w0_bytes = struct.pack("<I", w0)
            if 0x20 <= min(w0_bytes[:4]) < 0x7F:
                try:
                    s = w0_bytes.decode("ascii", errors="replace")
                    notes = f"string: '{s}'"
                except Exception:
                    pass
            if not notes:
                notes = f"raw: 0x{w0:08X} 0x{w1:08X}"
        fout.write(f"{i:<6} 0x{w0:08X} 0x{w1:08X} {notes}\n")


def dump_clip_config(elf, fout):
    """17-slot _Clip collision config at 0x282690."""
    base_va = 0x282690
    stride = 0x10
    num = 17
    off = elf_addr_to_offset(elf, base_va)
    if off is None:
        fout.write("ERROR: clip config table not in ELF\n")
        return
    fout.write(f"=== _Clip Configuration Table (0x{base_va:08X}) ===\n")
    fout.write(f"{'Slot':<6} {'w0':<12} {'w1':<12} {'w2':<12} {'w3':<12}\n")
    fout.write(f"{'-'*6} {'-'*12} {'-'*12} {'-'*12} {'-'*12}\n")
    for i in range(num):
        e_off = off + i * stride
        w0 = read_u32(elf, e_off)
        w1 = read_u32(elf, e_off + 4)
        w2 = read_u32(elf, e_off + 8)
        w3 = read_u32(elf, e_off + 0x0C)
        fout.write(f"{i:<6} 0x{w0:08X} 0x{w1:08X} 0x{w2:08X} 0x{w3:08X}\n")


def dump_scene_loader_jt(elf, fout):
    """Scene loader 21-stage jump table at 0x616FD0."""
    base_va = 0x616FD0
    num = 21
    off = elf_addr_to_offset(elf, base_va)
    if off is None:
        fout.write("ERROR: scene loader jump table not in ELF\n")
        return
    fout.write(f"=== Scene Loader Jump Table (0x{base_va:08X}, {num} entries) ===\n")
    fout.write(f"{'Stage':<7} {'Target':<18} {'Function name'}\n")
    fout.write(f"{'-'*7} {'-'*18} {'-'*40}\n")
    known_handlers = {
        0x1B27A8: "kanbanReqAdd",
        0x1B27A0: "kanbanInit",
        0x1B2810: "kanbanReqAllDel",
        0x1B05A8: "kanbanExec",
        0x1B2A30: "la_load_processing",
        0x1B1A80: "la_switching_stage",
        0x1B76F8: "initSceneGObj",
        0x1B7D40: "HotInitSceneObjects",
        0x1B80B8: "MoveNextStage_Clear",
    }
    for i in range(num):
        e_off = off + i * 4
        target = read_u32(elf, e_off)
        name = known_handlers.get(target, "")
        fout.write(f"{i:<7} 0x{target:08X}  {name}\n")


GP_VARS = {
    (-0x4C4C): ("g_obj_count", "GObj count (isysGObjAlloc)"),
    (-0x49B4): ("gp_m49B4", "Current entity work area ptr (most ref'd GP var)"),
    (-0x4B38): ("gp_m4B38", "Current request/target entity ID (eBrainGetStatus)"),
    (-0x4B3C): ("gp_m4B3C", "Request queue index/pointer"),
    (-0x6F60): ("world_state", "World state (0=title, 1=load, 2=stage init, 3=play)"),
    (-0x6724): ("mask_bitfield", "Mask bitfield for process dispatch (8 bits)"),
    (-0x6F5C): ("sleep_counter", "Sleep counter for thread/process"),
    (-0x6F48): ("in_game", "In-game flag"),
    (-0x6F40): ("fade_state", "Fade state"),
    (-0x4BE4): ("vblank_count", "VBlank frame counter (0x274EC0)"),
}


def dump_gp_vars(elf, fout):
    fout.write("=== GP-Relative Variables ===\n")
    fout.write(f"{'Offset':<10} {'Name':<22} {'Description'}\n")
    fout.write(f"{'-'*10} {'-'*22} {'-'*50}\n")
    for off, (name, desc) in sorted(GP_VARS.items()):
        fout.write(f"{off:<+9d} {name:<22} {desc}\n")
    fout.write("\nGP = 0x27A7A8 (kanban.c)\n")


# Nota: tabelas em BSS nao estao no ELF e nao podem ser dumpadas:
#   0x6A93D0 — GObj type handler table (67 entries x 4B, BSS)
#   0x6A6F30 — Thread table (indexada por thread_id, stride 4, BSS)
#   0x281A70 — GObj type display list heads (8 entries x 4B, BSS)
#   0x281AB0 — Dispatch node table (32 entries x 4B, BSS)
#   0x281A90 — GObj type display list tails (8 entries x 4B, BSS)

TABLES = {
    "descriptors": dump_descriptors,
    "entry": dump_entry_table,
    "dispatch_32": dump_dispatch_32,
    "clip_config": dump_clip_config,
    "scene_loader_jt": dump_scene_loader_jt,
    "gp_vars": dump_gp_vars,
}


def main():
    if len(sys.argv) < 2 or sys.argv[1] in ("-h", "--help"):
        print(__doc__)
        print(f"\nAvailable tables: {', '.join(sorted(TABLES.keys()))}")
        sys.exit(1)

    table = sys.argv[1]
    use_json = "--json" in sys.argv

    elf = load_elf()
    fout = sys.stdout

    if table in TABLES:
        TABLES[table](elf, fout)
    elif table == "all":
        for name in sorted(TABLES.keys()):
            TABLES[name](elf, fout)
            fout.write("\n")
    else:
        print(f"Unknown table: {table}")
        print(f"Available: {', '.join(sorted(TABLES.keys()))}")
        sys.exit(1)


if __name__ == "__main__":
    main()
