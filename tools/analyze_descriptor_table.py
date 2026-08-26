#!/usr/bin/env python3
"""Analyze the entity descriptor table at 0x2A31B8 in the ICO PS2 ELF.

NOTE: 0x1A48A0 is CODE (.text section), NOT a data table (Rev.059 correction).
The real descriptor table is at 0x2A31B8 (68 entries, stride 0x64).

This script:
1. Reads all 68 entries from 0x2A31B8
2. Extracts handler pointers and metadata per entry
3. Cross-references with the entry table at 0x2A4C48
4. Identifies handler ranges (boyAI, GirlBrain, ENEMY1, cloth/barrel, etc.)
5. Identifies entity type names and shared handlers
6. Prints a comprehensive analysis table
"""

import os
import struct
import sys
from collections import defaultdict

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ELF_PATH = os.path.join(SCRIPT_DIR, "..", ".local", "extracted", "SCUS_971.13.elf")


def load_elf():
    path = os.path.expanduser(ELF_PATH)
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


# Known function address ranges from research notes
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
    "vtable_202A60":  (0x202A00, 0x202B00),
    "vtable_23D660":  (0x23D600, 0x23D700),
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
    0x10EC60: "sv_hA (generic)",
    0x10ECC0: "generic_callback (SOBJ/SOFA/INTEREST)",
    0x10ECB0: "generic_hA (MOBJ/DEMO_QSWORD)",
    0x153478: "boy_init",
    0x1C1F58: "boy_hA",
    0x1C1DD8: "boy_hB",
    0x1C1A98: "boy_hC",
    0x164440: "enemy1_init",
    0x1CE690: "enemy1_hA",
    0x1CE3C0: "enemy1_hB",
    0x1CE220: "enemy1_hC",
    0x1CE760: "enemy1_hD",
    0x1CE5F8: "enemy_test_hA",
    0x1E08B8: "enemy_test_hB",
    0x174BA0: "girl_init (also devil_girl)",
    0x1D1A98: "girl_hA",
    0x1D17F8: "girl_hB",
    0x1D1668: "girl_hC",
    0x1D1AD0: "girl_hD (cb_routine4)",
    0x17D1D0: "woodbox0_init",
    0x1C05D0: "woodbox0_hA",
    0x1C0538: "woodbox0_hB",
    0x1C00C0: "woodbox0_hC",
    0x1EA030: "rotobject_hA",
    0x1E9950: "rotobject_hB",
    0x1E9F08: "rotobject_hC",
    0x1D3B28: "barrel_hA",
    0x1D3A30: "ItemGeo (barrel_hB/rope_hB)",
    0x1D27A8: "barrel_hC (cloth_payload_init)",
    0x1E9630: "rope_hA",
    0x1E9810: "rope_hB",
    0x1E8F38: "rope_hC",
    0x18F640: "chain_hA",
    0x18ECC8: "chain_hB",
    0x18E5B0: "chain_hC",
    0x1BC438: "flever_hA",
    0x1BC1A8: "flever_hB",
    0x1BC130: "flever_tristate_hB",
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
    0x1CE6F0: "demomotctrl_hB",
    0x1CEB18: "demomotctrl_hC",
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
    0x1F44C8: "demo_qsword_hB",
    0x1F43D0: "demo_qsword_hC",
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
    0x19A9A0: "queen_hA (also queendemoctrl_hA)",
    0x19A8F0: "queen_hB (also queendemoctrl_hB)",
    0x19A7E8: "queen_hC (also queendemoctrl_hC)",
    0x1E0860: "skeltest_hA",
    0x1C2FA0: "cagefix_hA",
    0x1C2F20: "cagefix_hB",
    0x1C2FE8: "cagefix_hC",
    0x1C9330: "clothtest_hA",
    0x1C9328: "clothtest_hB",
    0x1C92D8: "clothtest_hC",
    0x19B660: "queen_ball_hA",
    0x19AE98: "queen_ball_hB",
    0x19BC58: "queen_ball_hC",
    0x19AE50: "queen_barrier_hA",
    0x19AA20: "queen_barrier_hB",
    0x19BAA8: "queen_barrier_hC",
    0x1D1B30: "girlforcefield_hA",
    0x1D1CF8: "girlforcefield_hB",
    0x1D1C78: "girlforcefield_hC",
    0x1BB6B0: "ap1_init",
    0x1BA530: "ap1_hB",
    0x1BA330: "ap1_hB2",
    0x1B8720: "ap1_hC",
    0x1BBF78: "attackchk_init (also attckchkbnrymng)",
    0x1BBEC8: "attackchk_hA",
    0x1BBEA0: "attackchk_hB",
    0x1BBE50: "attackchk_hC",
    0x1BBE48: "attckchkbnrymng_hA",
    0x1BBDD8: "attckchkbnrymng_hB",
    0x1BBB20: "attckchkbnrymng_hC",
    0x198140: "boss_ctrl_init",
    0x198000: "boss_ctrl_hA",
    0x197FC8: "boss_ctrl_hB",
    0x198218: "boss_ctrl_hC",
    0x191D08: "enemy_control_hC",
    0x1D03A0: "fly_info_hA",
    0x1D03C8: "fly_info_hC",
    0x203EE8: "bga_init",
    0x202A60: "vtable_main_chars",
    0x23D660: "vtable_physics_props",
    0x1956E8: "vtable_bird",
    0x192EB8: "vtable_generator",
    0x199A60: "vtable_queen",
    0x19BB60: "vtable_queen_ball",
    0x19B998: "vtable_queen_barrier",
    0x1BB3E0: "vtable_ap1",
    0x1BB988: "vtable_attackchk",
    0x1978B0: "vtable_boss_ctrl",
    0x191C80: "vtable_enemy_control",
    0x122A838: "room_init troko",
}


def classify_handler(addr):
    if addr == 0:
        return "—"
    name = KNOWN_HANDLERS.get(addr)
    if name:
        return name
    for range_name, (lo, hi) in HANDLER_RANGES.items():
        if lo <= addr < hi:
            return f"{range_name}+0x{addr - lo:04X}"
    if addr < 0x100000:
        return f"low_addr"
    return f"0x{addr:08X}"


def dump_descriptor_table(elf, fout):
    base_va = 0x2A31B8
    stride = 0x64
    num = 68
    off = elf_addr_to_offset(elf, base_va)
    if off is None:
        fout.write("ERROR: descriptor table address not in ELF\n")
        return

    fout.write(f"{'='*120}\n")
    fout.write(f"Entity Descriptor Table: 0x{base_va:08X} | {num} entries | stride 0x{stride:X}\n")
    fout.write(f"{'='*120}\n\n")

    # Read all entries
    entries = []
    for i in range(num):
        e_off = off + i * stride
        name_raw = elf[e_off:e_off + 16]
        name = name_raw.rstrip(b"\x00").decode("ascii", errors="replace")
        init_fn = read_u32(elf, e_off + 0x40)
        flags = read_u32(elf, e_off + 0x44)
        hA = read_u32(elf, e_off + 0x48)
        hB = read_u32(elf, e_off + 0x50)
        hC = read_u32(elf, e_off + 0x58)
        hD = read_u32(elf, e_off + 0x5C)
        vtable = read_u32(elf, e_off + 0x60)
        entries.append({
            "idx": i, "name": name, "init_fn": init_fn, "flags": flags,
            "hA": hA, "hB": hB, "hC": hC, "hD": hD, "vtable": vtable,
        })

    # Part 1: Full table with all fields
    fout.write(f"{'Idx':<5} {'Name':<14} {'init_fn':<14} {'Fl':<4} {'hA':<14} {'hB':<14} {'hC':<14} {'hD':<14} {'vtable':<14}\n")
    fout.write(f"{'-'*5} {'-'*14} {'-'*14} {'-'*4} {'-'*14} {'-'*14} {'-'*14} {'-'*14} {'-'*14}\n")
    for e in entries:
        def fmt(addr):
            if addr == 0:
                return "—"
            return f"0x{addr:08X}"
        fout.write(f"{e['idx']:<5} {e['name']:<14} {fmt(e['init_fn']):<14} {e['flags']:<4} "
                    f"{fmt(e['hA']):<14} {fmt(e['hB']):<14} {fmt(e['hC']):<14} {fmt(e['hD']):<14} "
                    f"{fmt(e['vtable']):<14}\n")

    # Part 2: Handler classification
    fout.write(f"\n{'='*120}\n")
    fout.write(f"HANDLER CLASSIFICATION\n")
    fout.write(f"{'='*120}\n\n")

    fout.write(f"{'Idx':<5} {'Name':<14} {'init_fn':<24} {'hA':<24} {'hB':<24} {'hC':<24}\n")
    fout.write(f"{'-'*5} {'-'*14} {'-'*24} {'-'*24} {'-'*24} {'-'*24}\n")
    for e in entries:
        fout.write(f"{e['idx']:<5} {e['name']:<14} {classify_handler(e['init_fn']):<24} "
                    f"{classify_handler(e['hA']):<24} {classify_handler(e['hB']):<24} "
                    f"{classify_handler(e['hC']):<24}\n")

    # Part 3: Shared handler analysis
    fout.write(f"\n{'='*120}\n")
    fout.write(f"SHARED HANDLER ANALYSIS\n")
    fout.write(f"{'='*120}\n\n")

    handler_map = defaultdict(list)
    for e in entries:
        for field in ["init_fn", "hA", "hB", "hC", "hD"]:
            addr = e[field]
            if addr != 0:
                handler_map[addr].append((e["idx"], e["name"], field))

    fout.write(f"{'Handler':<14} {'Count':<6} {'Used by'}\n")
    fout.write(f"{'-'*14} {'-'*6} {'-'*60}\n")
    for addr in sorted(handler_map.keys()):
        users = handler_map[addr]
        if len(users) > 1:
            user_str = ", ".join(f"{name}.{field}" for idx, name, field in users)
            fout.write(f"0x{addr:08X}   {len(users):<6} {user_str}\n")

    # Part 4: Vtable groups
    fout.write(f"\n{'='*120}\n")
    fout.write(f"VTABLE GROUPS\n")
    fout.write(f"{'='*120}\n\n")

    vtable_map = defaultdict(list)
    for e in entries:
        vtable_map[e["vtable"]].append(e["name"] or f"[{e['idx']}]")

    for vtable, names in sorted(vtable_map.items(), key=lambda x: -len(x[1])):
        label = KNOWN_HANDLERS.get(vtable, "unknown")
        fout.write(f"vtable=0x{vtable:08X} ({label}): {len(names)} entities\n")
        for n in names:
            fout.write(f"  - {n}\n")
        fout.write("\n")

    # Part 5: Init_fn groups
    fout.write(f"{'='*120}\n")
    fout.write(f"INIT_FN GROUPS\n")
    fout.write(f"{'='*120}\n\n")

    init_map = defaultdict(list)
    for e in entries:
        if e["init_fn"] != 0:
            init_map[e["init_fn"]].append(e["name"] or f"[{e['idx']}]")

    fout.write(f"{'init_fn':<14} {'Count':<6} {'Entities'}\n")
    fout.write(f"{'-'*14} {'-'*6} {'-'*50}\n")
    for addr in sorted(init_map.keys()):
        names = init_map[addr]
        label = KNOWN_HANDLERS.get(addr, "unknown")
        fout.write(f"0x{addr:08X}   {len(names):<6} {', '.join(names)}  ({label})\n")

    # Part 6: Cross-reference with entry table
    fout.write(f"\n{'='*120}\n")
    fout.write(f"CROSS-REFERENCE: Entry Table (0x2A4C48) → Descriptor Table\n")
    fout.write(f"{'='*120}\n\n")

    entry_base = 0x2A4C48
    entry_stride = 0x4C
    entry_off = elf_addr_to_offset(elf, entry_base)
    if entry_off:
        # Count descriptor references
        desc_ref_count = defaultdict(int)
        for i in range(512):
            e_off = entry_off + i * entry_stride
            type_id = read_u32(elf, e_off + 0x30)
            if 0 <= type_id < 68:
                desc_ref_count[type_id] += 1

        fout.write(f"{'DescIdx':<9} {'Name':<14} {'EntryRefs':<10} {'Has init_fn':<12} {'Has handlers':<14}\n")
        fout.write(f"{'-'*9} {'-'*14} {'-'*10} {'-'*12} {'-'*14}\n")
        for i in range(68):
            name = entries[i]["name"]
            ref_count = desc_ref_count.get(i, 0)
            has_init = "YES" if entries[i]["init_fn"] else "—"
            has_h = "YES" if (entries[i]["hA"] or entries[i]["hB"] or entries[i]["hC"]) else "—"
            fout.write(f"{i:<9} {name:<14} {ref_count:<10} {has_init:<12} {has_h:<14}\n")

    # Part 7: Summary statistics
    fout.write(f"\n{'='*120}\n")
    fout.write(f"SUMMARY STATISTICS\n")
    fout.write(f"{'='*120}\n\n")

    total_with_init = sum(1 for e in entries if e["init_fn"])
    total_with_hA = sum(1 for e in entries if e["hA"])
    total_with_hB = sum(1 for e in entries if e["hB"])
    total_with_hC = sum(1 for e in entries if e["hC"])
    total_with_hD = sum(1 for e in entries if e["hD"])
    total_with_vtable = sum(1 for e in entries if e["vtable"])
    unique_handlers = len(handler_map)
    shared_handlers = sum(1 for addr, users in handler_map.items() if len(users) > 1)

    fout.write(f"Total entries:           {num}\n")
    fout.write(f"Named entries:           {sum(1 for e in entries if e['name'])}\n")
    fout.write(f"With init_fn:            {total_with_init}\n")
    fout.write(f"With hA:                 {total_with_hA}\n")
    fout.write(f"With hB:                 {total_with_hB}\n")
    fout.write(f"With hC:                 {total_with_hC}\n")
    fout.write(f"With hD (cb_routine4):   {total_with_hD}\n")
    fout.write(f"With vtable:             {total_with_vtable}\n")
    fout.write(f"Unique handler addrs:    {unique_handlers}\n")
    fout.write(f"Shared handler addrs:    {shared_handlers}\n")
    fout.write(f"Total handler refs:      {sum(len(v) for v in handler_map.values())}\n")


def main():
    elf = load_elf()
    if len(sys.argv) > 1 and sys.argv[1] == "--check":
        base_va = 0x2A31B8
        off = elf_addr_to_offset(elf, base_va)
        if off is None:
            print("ERROR: descriptor table not found")
            sys.exit(1)
        name = elf[off:off+4].rstrip(b"\x00").decode("ascii", errors="replace")
        print(f"Descriptor table at 0x{base_va:08X}: OK (first entry='{name}')")
        sys.exit(0)

    dump_descriptor_table(elf, sys.stdout)


if __name__ == "__main__":
    main()
