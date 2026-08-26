#!/usr/bin/env python3
"""
Rev.110 — Behavioral Taxonomy for Runtime Entity Work Areas.

Groups the 100 filtered runtime work-area pointers using:
1. GObj binding patterns (1:N entity→GObj)
2. Handler address ranges (boyAI, GirlBrain, ENEMY1, etc.)
3. World state affinity (room-specific vs persistent)
4. Event frequency patterns
5. Descriptor table statistics for context

The tool does not assign exact descriptor types; that still requires callback or field evidence.

Usage:
    python3 tools/runtime/classify_entities.py [--csv-dir research/runtime/rev108]
"""

import csv
import os
import struct
import sys
from collections import defaultdict, Counter
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent
ELF_PATH = SCRIPT_DIR.parent.parent / ".local" / "extracted" / "SCUS_971.13.elf"


def load_elf():
    with open(ELF_PATH, "rb") as f:
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


def read_csv(path):
    rows = []
    with open(path, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)
    return rows


def hx(s):
    if not s or s == '?':
        return None
    try:
        return int(s, 16)
    except ValueError:
        return None


# Known handler address ranges
HANDLER_RANGES = {
    "boyAI":       (0x142000, 0x164000),
    "GirlBrain":   (0x16A000, 0x175000),
    "ENEMY1":      (0x164000, 0x16A000),
    "cloth/barrel": (0x1D0000, 0x1E0000),
    "eBrain":      (0x190000, 0x194000),
    "QUEEN":       (0x199000, 0x19C000),
    "AP1/lever":   (0x1B8000, 0x1BC000),
    "environ":     (0x1E8000, 0x1F8000),
    "generic_10ECC0": (0x10EC00, 0x10ED00),
    "woodbox0":    (0x1C0000, 0x1C1000),
    "rotobject":   (0x1E9000, 0x1EB000),
    "chain":       (0x18E000, 0x18F700),
    "BIRD":        (0x195000, 0x198000),
    "GENERATOR":   (0x192000, 0x194000),
    "BOSS":        (0x197000, 0x199000),
    "CAGE":        (0x1C2000, 0x1C3000),
    "candle":      (0x1C3000, 0x1C4000),
    "BGA":         (0x203000, 0x204000),
}

KNOWN_HANDLERS = {
    0x153478: "boy_init",
    0x1C1F58: "boy_hA",
    0x1C1DD8: "boy_hB",
    0x1C1A98: "boy_hC",
    0x164440: "enemy1_init",
    0x1CE690: "enemy1_hA",
    0x1CE3C0: "enemy1_hB",
    0x1CE220: "enemy1_hC",
    0x1CE760: "enemy1_hD",
    0x174BA0: "girl_init",
    0x1D1A98: "girl_hA",
    0x1D17F8: "girl_hB",
    0x1D1668: "girl_hC",
    0x1D1AD0: "girl_hD",
    0x17D1D0: "woodbox0_init",
    0x1C05D0: "woodbox0_hA",
    0x1C0538: "woodbox0_hB",
    0x1C00C0: "woodbox0_hC",
    0x1D3B28: "barrel_hA",
    0x1D3A30: "ItemGeo (barrel_hB)",
    0x1D27A8: "barrel_hC",
    0x1E9630: "rope_hA",
    0x1E9810: "rope_hB",
    0x1E8F38: "rope_hC",
    0x18F640: "chain_hA",
    0x18ECC8: "chain_hB",
    0x18E5B0: "chain_hC",
    0x1BC438: "flever_hA",
    0x1BC1A8: "flever_hB",
    0x1C09C8: "lever_hC",
    0x1BC7F0: "wlever_hA",
    0x1BC530: "wlever_hB",
    0x1C0C40: "wlever_hC",
    0x1F1508: "tree_hA",
    0x1F1330: "tree_hB",
    0x1F17B0: "tree_hC",
    0x1F2140: "torch_hA",
    0x1F1CF0: "torch_hB",
    0x1F2048: "torch_hC",
    0x1E8F30: "partlayout_hA",
    0x1E8F28: "partlayout_hB",
    0x1E8EB8: "partlayout_hC",
    0x1F0550: "lightbit_hA",
    0x1F0540: "lightbit_hB",
    0x1F0568: "lightbit_hC",
    0x1F3BD0: "weapon_hA",
    0x1F3A00: "weapon_hB",
    0x1F36F0: "weapon_hC",
    0x1EA5E8: "spider_hB",
    0x1EA278: "spider_hC",
    0x1AEA58: "psobj_hA",
    0x1AEA50: "psobj_hB",
    0x1AEA60: "psobj_hC",
    0x1971C0: "bird_init",
    0x197080: "bird_hA",
    0x197078: "bird_hB",
    0x197240: "bird_hC",
    0x193600: "generator_hA",
    0x1930B0: "generator_hB",
    0x193730: "generator_hC",
    0x1C3130: "candle_hA",
    0x1C3178: "candle_hB",
    0x1C2FF0: "candle_hC",
    0x1C3470: "chandelier_hA",
    0x1C33D8: "chandelier_hB",
    0x1C34B8: "chandelier_hC",
    0x1F69B0: "worm_hA",
    0x1F66F0: "worm_hB",
    0x1F62E8: "worm_hC",
    0x10C5C0: "pool_hA",
    0x10D070: "pool_hB",
    0x10B2D0: "pool_hC",
    0x1CBD78: "darkvolume_hA",
    0x1CBD70: "darkvolume_hB",
    0x1CBD68: "darkvolume_hC",
    0x1E6960: "mcoltest_hA",
    0x1E6788: "mcoltest_hB",
    0x1E6968: "mcoltest_hC",
    0x1E98C8: "ropefix_hA",
    0x1E9888: "ropefix_hB",
    0x1E9910: "ropefix_hC",
    0x1C2DF8: "cage_hA",
    0x1C28D0: "cage_hB",
    0x1C2338: "cage_hC",
    0x1D01E8: "flag_hA",
    0x1D00F8: "flag_hB",
    0x1CFB58: "flag_hC",
    0x19B7F8: "queen_init",
    0x19A9A0: "queen_hA",
    0x19A8F0: "queen_hB",
    0x19A7E8: "queen_hC",
    0x1BB6B0: "ap1_init",
    0x1BA530: "ap1_hB",
    0x1BA330: "ap1_hB2",
    0x1B8720: "ap1_hC",
    0x1BBF78: "attackchk_init",
    0x1BBEC8: "attackchk_hA",
    0x1BBEA0: "attackchk_hB",
    0x1BBE50: "attackchk_hC",
    0x198140: "boss_ctrl_init",
    0x198000: "boss_ctrl_hA",
    0x197FC8: "boss_ctrl_hB",
    0x198218: "boss_ctrl_hC",
    0x191D08: "enemy_control_hC",
    0x1D03A0: "fly_info_hA",
    0x1D03C8: "fly_info_hC",
    0x203EE8: "bga_init",
}

# Descriptor table entries (from analyze_descriptor_table.py output)
DESCRIPTOR_TABLE = [
    {"idx": 0, "name": "NULL", "init_fn": 0, "hA": 0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0},
    {"idx": 1, "name": "BOY", "init_fn": 0x153478, "hA": 0x1C1F58, "hB": 0x1C1DD8, "hC": 0x1C1A98, "hD": 0, "vtable": 0x202A60},
    {"idx": 2, "name": "GIRL", "init_fn": 0x174BA0, "hA": 0x1D1A98, "hB": 0x1D17F8, "hC": 0x1D1668, "hD": 0x1D1AD0, "vtable": 0x202A60},
    {"idx": 3, "name": "GIRLDEMOCTRL", "init_fn": 0, "hA": 0x1D1A98, "hB": 0x1D17F8, "hC": 0x1D1668, "hD": 0, "vtable": 0},
    {"idx": 4, "name": "ENEMY1", "init_fn": 0x164440, "hA": 0x1CE690, "hB": 0x1CE3C0, "hC": 0x1CE220, "hD": 0x1CE760, "vtable": 0x202A60},
    {"idx": 5, "name": "ENEMY_TEST", "init_fn": 0, "hA": 0x1CE5F8, "hB": 0x1E08B8, "hC": 0x1CE220, "hD": 0x1CE760, "vtable": 0},
    {"idx": 6, "name": "DEMOMOTCTRL", "init_fn": 0, "hA": 0x10ECC0, "hB": 0x1CE6F0, "hC": 0x1CEB18, "hD": 0, "vtable": 0},
    {"idx": 7, "name": "SOBJ", "init_fn": 0, "hA": 0x10ECC0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0x23D660},
    {"idx": 8, "name": "PSOBJ", "init_fn": 0, "hA": 0x1AEA58, "hB": 0x1AEA50, "hC": 0x1AEA60, "hD": 0, "vtable": 0x23D660},
    {"idx": 9, "name": "TREE", "init_fn": 0, "hA": 0x1F1508, "hB": 0x1F1330, "hC": 0x1F17B0, "hD": 0, "vtable": 0},
    {"idx": 10, "name": "TORCH", "init_fn": 0, "hA": 0x1F2140, "hB": 0x1F1CF0, "hC": 0x1F2048, "hD": 0, "vtable": 0},
    {"idx": 11, "name": "PARTICLE", "init_fn": 0, "hA": 0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0},
    {"idx": 12, "name": "PARTLAYOUT", "init_fn": 0, "hA": 0x1E8F30, "hB": 0x1E8F28, "hC": 0x1E8EB8, "hD": 0, "vtable": 0},
    {"idx": 13, "name": "LIGHTBIT", "init_fn": 0, "hA": 0x1F0550, "hB": 0x1F0540, "hC": 0x1F0568, "hD": 0, "vtable": 0},
    {"idx": 14, "name": "WEAPON", "init_fn": 0, "hA": 0x1F3BD0, "hB": 0x1F3A00, "hC": 0x1F36F0, "hD": 0, "vtable": 0},
    {"idx": 15, "name": "SPIDER_LAYOUT", "init_fn": 0, "hA": 0, "hB": 0x1EA5E8, "hC": 0x1EA278, "hD": 0, "vtable": 0},
    {"idx": 16, "name": "SOFA", "init_fn": 0, "hA": 0x10ECC0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0x23D660},
    {"idx": 17, "name": "WOODBOX01", "init_fn": 0x17D1D0, "hA": 0x1C05D0, "hB": 0x1C0538, "hC": 0x1C00C0, "hD": 0, "vtable": 0x23D660},
    {"idx": 18, "name": "ROTOBJECT", "init_fn": 0, "hA": 0x1EA030, "hB": 0x1E9950, "hC": 0x1E9F08, "hD": 0, "vtable": 0},
    {"idx": 19, "name": "BARREL", "init_fn": 0, "hA": 0x1D3B28, "hB": 0x1D3A30, "hC": 0x1D27A8, "hD": 0, "vtable": 0},
    {"idx": 20, "name": "ROPE", "init_fn": 0, "hA": 0x1E9630, "hB": 0x1E9810, "hC": 0x1E8F38, "hD": 0, "vtable": 0},
    {"idx": 21, "name": "CHAIN", "init_fn": 0, "hA": 0x18F640, "hB": 0x18ECC8, "hC": 0x18E5B0, "hD": 0, "vtable": 0},
    {"idx": 22, "name": "FLEVER", "init_fn": 0, "hA": 0x1BC438, "hB": 0x1BC1A8, "hC": 0x1C09C8, "hD": 0, "vtable": 0},
    {"idx": 23, "name": "FLEVER_TRISTATE", "init_fn": 0, "hA": 0x1BC438, "hB": 0x1BC130, "hC": 0x1C09C8, "hD": 0, "vtable": 0},
    {"idx": 24, "name": "WLEVER", "init_fn": 0, "hA": 0x1BC7F0, "hB": 0x1BC530, "hC": 0x1C0C40, "hD": 0, "vtable": 0},
    {"idx": 25, "name": "WLEVER2", "init_fn": 0, "hA": 0x1BC7F0, "hB": 0x1BC530, "hC": 0x1C0C40, "hD": 0, "vtable": 0},
    {"idx": 26, "name": "NONE", "init_fn": 0, "hA": 0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0},
    {"idx": 27, "name": "SV", "init_fn": 0, "hA": 0x10EC60, "hB": 0, "hC": 0, "hD": 0, "vtable": 0},
    {"idx": 28, "name": "CAMERADUMMY", "init_fn": 0, "hA": 0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0x23D660},
    {"idx": 29, "name": "DUMMY", "init_fn": 0, "hA": 0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0},
    {"idx": 30, "name": "BGA", "init_fn": 0x203EE8, "hA": 0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0x23D660},
    {"idx": 31, "name": "SEFFECT", "init_fn": 0, "hA": 0x1EF988, "hB": 0x1EF980, "hC": 0x1EF8E8, "hD": 0, "vtable": 0},
    {"idx": 32, "name": "BIRD", "init_fn": 0x1971C0, "hA": 0x197080, "hB": 0x197078, "hC": 0x197240, "hD": 0, "vtable": 0x1956E8},
    {"idx": 33, "name": "GENERATOR", "init_fn": 0, "hA": 0x193600, "hB": 0x1930B0, "hC": 0x193730, "hD": 0, "vtable": 0x192EB8},
    {"idx": 34, "name": "CANDLE", "init_fn": 0, "hA": 0x1C3130, "hB": 0x1C3178, "hC": 0x1C2FF0, "hD": 0, "vtable": 0},
    {"idx": 35, "name": "MOBJ", "init_fn": 0, "hA": 0x10ECB0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0x23D660},
    {"idx": 36, "name": "DEMO_QSWORD", "init_fn": 0, "hA": 0x10ECB0, "hB": 0x1F44C8, "hC": 0x1F43D0, "hD": 0, "vtable": 0x23D660},
    {"idx": 37, "name": "CHANDELIER", "init_fn": 0, "hA": 0x1C3470, "hB": 0x1C33D8, "hC": 0x1C34B8, "hD": 0, "vtable": 0x23D660},
    {"idx": 38, "name": "WORM", "init_fn": 0, "hA": 0x1F69B0, "hB": 0x1F66F0, "hC": 0x1F62E8, "hD": 0, "vtable": 0},
    {"idx": 39, "name": "POOL", "init_fn": 0, "hA": 0x10C5C0, "hB": 0x10D070, "hC": 0x10B2D0, "hD": 0, "vtable": 0},
    {"idx": 40, "name": "DARKVOLUME", "init_fn": 0, "hA": 0x1CBD78, "hB": 0x1CBD70, "hC": 0x1CBD68, "hD": 0, "vtable": 0},
    {"idx": 41, "name": "MCOLTEST", "init_fn": 0, "hA": 0x1E6960, "hB": 0x1E6788, "hC": 0x1E6968, "hD": 0, "vtable": 0},
    {"idx": 42, "name": "ROPEFIX", "init_fn": 0, "hA": 0x1E98C8, "hB": 0x1E9888, "hC": 0x1E9910, "hD": 0, "vtable": 0x23D660},
    {"idx": 43, "name": "CAGE", "init_fn": 0, "hA": 0x1C2DF8, "hB": 0x1C28D0, "hC": 0x1C2338, "hD": 0, "vtable": 0},
    {"idx": 44, "name": "DYNAMICMOTIONDAT", "init_fn": 0, "hA": 0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0},
    {"idx": 45, "name": "FLAG", "init_fn": 0, "hA": 0x1D01E8, "hB": 0x1D00F8, "hC": 0x1CFB58, "hD": 0, "vtable": 0},
    {"idx": 46, "name": "QUEEN", "init_fn": 0x19B7F8, "hA": 0x19A9A0, "hB": 0x19A8F0, "hC": 0x19A7E8, "hD": 0, "vtable": 0x199A60},
    {"idx": 47, "name": "QUEENDEMOCTRL", "init_fn": 0, "hA": 0x19A9A0, "hB": 0x19A8F0, "hC": 0x19A7E8, "hD": 0, "vtable": 0},
    {"idx": 48, "name": "DEVIL_GIRL", "init_fn": 0x174BA0, "hA": 0x1D1A98, "hB": 0x1D17F8, "hC": 0x1D1668, "hD": 0x1D1AD0, "vtable": 0x202A60},
    {"idx": 49, "name": "SKELTEST", "init_fn": 0, "hA": 0x1E0860, "hB": 0x1E08B8, "hC": 0, "hD": 0, "vtable": 0x23D660},
    {"idx": 50, "name": "CAGEFIX", "init_fn": 0, "hA": 0x1C2FA0, "hB": 0x1C2F20, "hC": 0x1C2FE8, "hD": 0, "vtable": 0x23D660},
    {"idx": 51, "name": "CLOTHTEST", "init_fn": 0, "hA": 0x1C9330, "hB": 0x1C9328, "hC": 0x1C92D8, "hD": 0, "vtable": 0},
    {"idx": 52, "name": "QUEEN_BALL", "init_fn": 0, "hA": 0x19B660, "hB": 0x19AE98, "hC": 0x19BC58, "hD": 0, "vtable": 0x19BB60},
    {"idx": 53, "name": "QUEEN_BARRIER", "init_fn": 0, "hA": 0x19AE50, "hB": 0x19AA20, "hC": 0x19BAA8, "hD": 0, "vtable": 0x19B998},
    {"idx": 54, "name": "STAGESETTING", "init_fn": 0, "hA": 0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0},
    {"idx": 55, "name": "GIRLFORCEFIELD", "init_fn": 0, "hA": 0x1D1B30, "hB": 0x1D1CF8, "hC": 0x1D1C78, "hD": 0, "vtable": 0},
    {"idx": 56, "name": "INTEREST1", "init_fn": 0, "hA": 0x10ECC0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0x23D660},
    {"idx": 57, "name": "INTEREST3", "init_fn": 0, "hA": 0x10ECC0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0x23D660},
    {"idx": 58, "name": "INTEREST10", "init_fn": 0, "hA": 0x10ECC0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0x23D660},
    {"idx": 59, "name": "INTEREST20", "init_fn": 0, "hA": 0x10ECC0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0x23D660},
    {"idx": 60, "name": "KYOMI", "init_fn": 0, "hA": 0, "hB": 0x23D518, "hC": 0, "hD": 0, "vtable": 0},
    {"idx": 61, "name": "AP1", "init_fn": 0x1BB6B0, "hA": 0x1BA530, "hB": 0x1BA330, "hC": 0x1B8720, "hD": 0, "vtable": 0x1BB3E0},
    {"idx": 62, "name": "ATTACKCHK1", "init_fn": 0x1BBF78, "hA": 0x1BBEC8, "hB": 0x1BBEA0, "hC": 0x1BBE50, "hD": 0, "vtable": 0x1BB988},
    {"idx": 63, "name": "ATTACKCHK2", "init_fn": 0x1BBF78, "hA": 0x1BBE48, "hB": 0x1BBDD8, "hC": 0x1BBB20, "hD": 0, "vtable": 0},
    {"idx": 64, "name": "BOSS_CTRL", "init_fn": 0x198140, "hA": 0x198000, "hB": 0x197FC8, "hC": 0x198218, "hD": 0, "vtable": 0x1978B0},
    {"idx": 65, "name": "temp", "init_fn": 0, "hA": 0, "hB": 0, "hC": 0, "hD": 0, "vtable": 0},
    {"idx": 66, "name": "ENEMY_CONTROL", "init_fn": 0, "hA": 0, "hB": 0, "hC": 0x191D08, "hD": 0, "vtable": 0x191C80},
    {"idx": 67, "name": "FLY_INFO", "init_fn": 0, "hA": 0x1D03A0, "hB": 0, "hC": 0x1D03C8, "hD": 0, "vtable": 0},
]


def classify_handler(addr):
    if addr == 0:
        return "—"
    name = KNOWN_HANDLERS.get(addr)
    if name:
        return name
    for range_name, (lo, hi) in HANDLER_RANGES.items():
        if lo <= addr < hi:
            return f"{range_name}+0x{addr - lo:04X}"
    return f"0x{addr:08X}"


def classify_entity_by_behavior(ew_row, gobj_bindings, ws_entities):
    """Classify an entity by its behavioral patterns."""
    addr = ew_row['entity_ptr']
    events = int(ew_row['event_count'])
    ws_count = int(ew_row['unique_ws'])
    gobj_count = int(ew_row['unique_gobjs'])
    ws_list = [w.strip() for w in ew_row.get('world_states', '').strip('"').split(',') if w.strip()]

    # Get GObj bindings for this entity
    entity_gobs = [g for g in gobj_bindings if g['entity_ptr'] == addr]

    # Classification heuristics
    if ws_count == 1 and events > 100000:
        return "HIGH_ACTIVITY_ROOM_SPECIFIC"
    elif ws_count >= 4:
        return "PERSISTENT_SYSTEM"
    elif ws_count == 1 and events < 5000:
        return "LOW_ACTIVITY_ROOM_SPECIFIC"
    elif gobj_count >= 8:
        return "MULTI_GOBJ_ASSOCIATED"
    elif ws_count == 2 and events > 30000:
        return "ADJACENT_ROOM_SHARED"
    else:
        return "STANDARD"


def main():
    csv_dir = sys.argv[1] if len(sys.argv) > 1 else "research/runtime/rev108"

    print("=" * 100)
    print("Rev.110 — Behavioral Taxonomy for Runtime Entity Work Areas")
    print("=" * 100)

    # Load runtime data
    ew = read_csv(os.path.join(csv_dir, "entity_workarea_pointers.csv"))
    gobj = read_csv(os.path.join(csv_dir, "entity_gobj_binding.csv"))
    hot = read_csv(os.path.join(csv_dir, "hot_paths_by_world_state.csv"))

    # Load ELF for descriptor table verification
    elf = load_elf()

    # Build entity→world_state map
    ws_entities = defaultdict(list)
    for r in ew:
        ws_list = [w.strip() for w in r.get('world_states', '').strip('"').split(',') if w.strip()]
        for ws in ws_list:
            ws_entities[ws].append(r)

    # Build entity→GObj map
    entity_gobjs = defaultdict(list)
    for r in gobj:
        entity_gobjs[r['entity_ptr']].append(r)

    # Classify each entity
    print("\n--- ENTITY CLASSIFICATION ---\n")
    print(f"{'Address':<12} {'Events':>10} {'WS':>3} {'GObjs':>5} {'Classification':<30} {'Primary WS'}")
    print(f"{'-'*12} {'-'*10} {'-'*3} {'-'*5} {'-'*30} {'-'*20}")

    classifications = Counter()
    for r in sorted(ew, key=lambda x: int(x['event_count']), reverse=True):
        cls = classify_entity_by_behavior(r, gobj, ws_entities)
        classifications[cls] += 1
        ws_list = r.get('world_states', '').strip('"')
        print(f"{r['entity_ptr']:<12} {int(r['event_count']):>10,} {r['unique_ws']:>3} {r['unique_gobjs']:>5} {cls:<30} {ws_list}")

    print(f"\n--- CLASSIFICATION SUMMARY ---")
    for cls, count in classifications.most_common():
        print(f"  {cls:<30}: {count:3d}")

    # Cross-reference with descriptor table
    print("\n\n--- DESCRIPTOR TABLE CONTEXT (NO EXACT INSTANCE MAPPING) ---\n")
    print("Since entity work areas are heap-allocated instances, not descriptor table entries,")
    print("the categories below are hypotheses from behavioral patterns, not type assignments:\n")

    # High-activity entities → likely BOY/GIRL/ENEMY1
    print("1. HIGH-ACTIVITY MULTI-ROOM CANDIDATES:")
    for r in sorted(ew, key=lambda x: int(x['event_count']), reverse=True)[:5]:
        ws_count = int(r['unique_ws'])
        if ws_count >= 2:
            print(f"   {r['entity_ptr']}: {int(r['event_count']):>10,} events, {ws_count} WS")
            print(f"     → Possible main-character/system entity; exact type unknown")

    # Room-specific entities → likely props/enemies
    print("\n2. ROOM-SPECIFIC HIGH-ACTIVITY ENTITIES (likely enemies/props):")
    for r in sorted(ew, key=lambda x: int(x['event_count']), reverse=True):
        ws_count = int(r['unique_ws'])
        if ws_count == 1 and int(r['event_count']) > 10000:
            print(f"   {r['entity_ptr']}: {int(r['event_count']):>10,} events, ws={r['world_states']}")

    # Multi-instance entities → spawners
    print("\n3. MULTI-GOBJ-ASSOCIATED ENTITIES (spawner is only a hypothesis):")
    for r in sorted(ew, key=lambda x: int(x['unique_gobjs']), reverse=True)[:10]:
        gobj_count = int(r['unique_gobjs'])
        if gobj_count >= 5:
            print(f"   {r['entity_ptr']}: {gobj_count} GObjs, {int(r['event_count']):>8,} events")

    # World state → entity inventory
    print("\n\n--- WORLD STATE → ENTITY INVENTORY ---\n")
    for ws in sorted(ws_entities.keys(), key=lambda x: hx(x) if hx(x) else 0):
        entities = ws_entities[ws]
        total_events = sum(int(e['event_count']) for e in entities)
        print(f"World State {ws}: {len(entities)} entities, {total_events:,} events")
        for e in sorted(entities, key=lambda x: int(x['event_count']), reverse=True)[:3]:
            print(f"  {e['entity_ptr']}: {int(e['event_count']):>8,} events")

    # GObj allocation pattern analysis
    print("\n\n--- GOBJ ALLOCATION PATTERNS ---\n")
    all_gobj_addrs = sorted([hx(r['gobj_ptr']) for r in gobj if hx(r['gobj_ptr']) is not None])

    if all_gobj_addrs:
        # Analyze address gaps
        gaps = []
        for i in range(1, len(all_gobj_addrs)):
            gap = all_gobj_addrs[i] - all_gobj_addrs[i-1]
            if gap > 0x200:  # Significant gap
                gaps.append((all_gobj_addrs[i-1], all_gobj_addrs[i], gap))

        print(f"GObj address range: 0x{all_gobj_addrs[0]:08X} - 0x{all_gobj_addrs[-1]:08X}")
        print(f"Total GObj addresses: {len(all_gobj_addrs)}")
        print(f"Observed address span (not proven pool size): {all_gobj_addrs[-1] - all_gobj_addrs[0] + 0x174:#x} bytes")

        # GObj stride analysis
        strides = Counter()
        for i in range(1, len(all_gobj_addrs)):
            stride = all_gobj_addrs[i] - all_gobj_addrs[i-1]
            strides[stride] += 1

        print(f"\nGObj stride distribution (top 10):")
        for stride, count in strides.most_common(10):
            print(f"  0x{stride:04X} ({stride:5d} bytes): {count:3d} times")

        if gaps:
            print(f"\nSignificant gaps in GObj pool:")
            for lo, hi, gap in gaps[:10]:
                print(f"  0x{lo:08X} → 0x{hi:08X} (gap: {gap:#x} bytes)")

    # Memory region analysis
    print("\n\n--- ENTITY MEMORY REGION ANALYSIS ---\n")
    regions = Counter()
    for r in ew:
        addr = hx(r['entity_ptr'])
        if addr is not None:
            if addr < 0x100000:
                regions["ELF"] += 1
            elif addr < 0x280000:
                regions[".data"] += 1
            elif addr < 0x6A0000:
                regions["BSS"] += 1
            elif addr < 0x800000:
                regions["BSS-ext"] += 1
            elif addr < 0x2000000:
                regions["Heap-early"] += 1
            else:
                regions["Heap"] += 1

    for reg, count in regions.most_common():
        print(f"  {reg}: {count} entities")

    # Descriptor table statistics
    print("\n\n--- DESCRIPTOR TABLE STATISTICS ---\n")
    total_with_init = sum(1 for d in DESCRIPTOR_TABLE if d['init_fn'])
    total_with_hA = sum(1 for d in DESCRIPTOR_TABLE if d['hA'])
    total_with_hB = sum(1 for d in DESCRIPTOR_TABLE if d['hB'])
    total_with_hC = sum(1 for d in DESCRIPTOR_TABLE if d['hC'])
    total_with_hD = sum(1 for d in DESCRIPTOR_TABLE if d['hD'])
    total_with_vtable = sum(1 for d in DESCRIPTOR_TABLE if d['vtable'])

    print(f"Total descriptor entries: {len(DESCRIPTOR_TABLE)}")
    print(f"With init_fn: {total_with_init}")
    print(f"With hA: {total_with_hA}")
    print(f"With hB: {total_with_hB}")
    print(f"With hC: {total_with_hC}")
    print(f"With hD: {total_with_hD}")
    print(f"With behavior_fn: {total_with_vtable}")

    # Vtable groups
    print("\n--- BEHAVIOR_FN GROUPS ---\n")
    vtable_map = defaultdict(list)
    for d in DESCRIPTOR_TABLE:
        vtable_map[d['vtable']].append(d['name'])

    for vtable, names in sorted(vtable_map.items(), key=lambda x: -len(x[1])):
        label = "no behavior_fn" if vtable == 0 else f"0x{vtable:08X}"
        print(f"  {label}: {len(names)} entities")
        for n in names[:5]:
            print(f"    - {n}")
        if len(names) > 5:
            print(f"    ... and {len(names)-5} more")

    print("\n" + "=" * 100)
    print("ANALYSIS COMPLETE")
    print("=" * 100)


if __name__ == '__main__':
    main()
