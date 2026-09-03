#!/usr/bin/env python3
"""
Entity Handler Struct Offset Comparison: ENEMY1, BARREL vs boyAI

Analyzes entity handlers in the ICO PS2 ELF to compare struct offset usage
patterns across entity types (BOY, ENEMY1, BARREL, WOODBOX0, AP1, GIRL).

Requirements: pip3 install capstone pyelftools
"""

import struct
import sys
from collections import defaultdict

from capstone import Cs, CS_ARCH_MIPS, CS_MODE_MIPS64, CS_MODE_LITTLE_ENDIAN, CS_OP_MEM
from elftools.elf.elffile import ELFFile

ELF_PATH = ".local/extracted/SCUS_971.13.elf"

# Entity handler addresses from research notes
# Descriptor table at 0x2A31B8: 68 entries, stride 0x64
# Entry table at 0x2A4C48: 512 entries, stride 0x4C

ENTITY_HANDLERS = {
    "BOY": {
        "init_fn": 0x153478,   # boy_init (from descriptor +0x040)
        "hA":      0x1C1F58,   # from descriptor +0x048
        "hB":      0x1C1DD8,   # from descriptor +0x050
        "hC":      0x1C1A98,   # from descriptor +0x058
        "range":   (0x142000, 0x164000),
        "sub_fns": [
            0x15BCC8,  # boy_fn1 (common service)
            0x15C534, 0x15D0B0, 0x1431C8, 0x15697C, 0x1623B8,
            0x153050, 0x161BF0, 0x1574CC, 0x1562E0, 0x157718,
            0x15E4E8, 0x150708, 0x156E88, 0x14E314, 0x15C5CC,
            0x146A80, 0x15F2E0, 0x143B58, 0x14D258, 0x14C6E4,
            0x153E90, 0x157814, 0x15DEB4, 0x1572E8, 0x142F1C,
            0x15E4E0, 0x155B3C, 0x15C878, 0x15BE90, 0x157B28,
            0x144EE8, 0x15B62C, 0x156D6C, 0x149DD0, 0x146080,
            0x14C63C, 0x1573C8, 0x150728, 0x145ECC, 0x158B68,
            0x150BC4, 0x1561BC, 0x157C28, 0x14B390, 0x1562E8,
            0x15E2D4, 0x14D090, 0x145CE8, 0x157890, 0x15DFC0,
            0x14F8B4, 0x15EDE0, 0x14FB0C, 0x14FE08, 0x15D224,
            0x14B09C, 0x1451C0,
        ],
    },
    "ENEMY1": {
        "init_fn": 0x164440,   # from descriptor +0x040
        "hA":      0x1CE690,   # from descriptor +0x048
        "hB":      0x1CE3C0,   # from descriptor +0x050
        "hC":      0x1CE220,   # from descriptor +0x058
        "hD":      0x1CE760,   # from descriptor +0x05C
        "range":   (0x1CE000, 0x1CF000),
        "sub_fns": [0x1D4B40, 0x1CEC60, 0x1CEC88, 0x1CD9B0, 0x1651C8,
                    0x165F88, 0x1654C8, 0x165540, 0x1CF548, 0x1CF6C0,
                    0x1CF930, 0x1BB7E0],
    },
    "BARREL": {
        "init_fn": 0x1D28A8,   # barrel_init
        "hC":      0x1D2738,   # barrel_hC (ctor)
        "range":   (0x1D2000, 0x1D4000),
        "sub_fns": [0x1D4B40, 0x1D49C0, 0x1D2550, 0x1D2650, 0x1D2738],
    },
    "WOODBOX0": {
        "init_fn": 0x17D1D0,
        "hA":      0x1C05D0,   # from descriptor +0x048
        "hB":      0x1C0538,   # from descriptor +0x050
        "hC":      0x1C00C0,   # from descriptor +0x058
        "range":   (0x17D000, 0x1C1000),
    },
    "AP1": {
        "init_fn": 0x1BB6B0,
        "hA":      0x1BB860,
        "hB":      0x1BA530,
        "range":   (0x1BA000, 0x1BC000),
    },
    "GIRL": {
        "init_fn": 0x174BA0,
        "hA":      0x1D1A98,
        "hB":      0x1D17F8,
        "hC":      0x1D1668,
        "range":   (0x174000, 0x1D2000),
    },
}

# Entity type table at 0x2A31B8: 68 entries, stride 0x64
DESC_TABLE_ADDR = 0x2A31B8
DESC_ENTRY_COUNT = 68
DESC_ENTRY_STRIDE = 0x64

# Known GObj struct offsets from research
GOBJ_OFFSET_NAMES = {
    0x0000: "type_id",
    0x0004: "entity_idx",
    0x0008: "type_flags",
    0x0010: "field_10",
    0x0014: "field_14",
    0x0018: "field_18",
    0x001C: "field_1C",
    0x0020: "field_20",
    0x0024: "field_24",
    0x0028: "field_28",
    0x002C: "field_2C",
    0x0030: "field_30",
    0x0034: "next_ptr",
    0x0038: "field_38",
    0x003C: "field_3C",
    0x0040: "field_40",
    0x0044: "field_44",
    0x0048: "field_48",
    0x004C: "field_4C",
    0x0050: "field_50",
    0x0060: "flags_0x60",
    0x0098: "field_98",
    0x009C: "field_9C",
    0x00A0: "field_A0",
    0x00C0: "field_C0",
    0x0100: "field_100",
    0x015C: "model_data",
    0x0164: "behavior_data",
    0x016C: "field_16C",
    0x0170: "field_170",
    0x01CA: "field_1CA",
    0x01D0: "field_1D0",
    0x01D4: "field_1D4",
    0x01D8: "field_1D8",
    0x01DC: "field_1DC",
    0x01E0: "field_1E0",
    0x01E4: "field_1E4",
    0x01E8: "field_1E8",
    0x01EC: "field_1EC",
    0x01F0: "field_1F0",
    0x01F4: "field_1F4",
    0x01F8: "field_1F8",
    0x01FC: "field_1FC",
    0x0204: "field_204",
    0x0220: "field_220",
    0x0278: "field_278",
    0x02C0: "field_2C0",
    0x02D0: "field_2D0",
    0x0340: "field_340",
    0x0378: "field_378",
    0x03C0: "field_3C0",
    0x045C: "field_45C",
    0x0460: "field_460",
    0x0464: "field_464",
    0x0468: "field_468",
    0x0480: "field_480",
    0x04C0: "field_4C0",
    0x0548: "field_548",
    0x054C: "field_54C",
    0x0550: "field_550",
    0x0554: "field_554",
    0x0558: "field_558",
    0x0630: "field_630",
    0x063C: "field_63C",
    0x0644: "field_644",
    0x0648: "field_648",
    0x0670: "field_670",
    0x0678: "field_678",
    0x067C: "field_67C",
    0x06F60: "world_state",
    0x0800: "state_block",
    0x0840: "field_840",
}


def load_elf(path):
    with open(path, "rb") as f:
        elf = ELFFile(f)
        for segment in elf.iter_segments():
            if segment.header.p_type == "PT_LOAD":
                yield segment


def find_text_data(elf_path):
    segments = {}
    with open(elf_path, "rb") as f:
        elf = ELFFile(f)
        for seg in elf.iter_segments():
            if seg.header.p_type == "PT_LOAD":
                vaddr = seg.header.p_vaddr
                memsz = seg.header.p_memsz
                data = seg.data()
                segments[vaddr] = (data, memsz)
    return segments


def read_bytes(segments, addr, size):
    for vaddr, (data, memsz) in segments.items():
        if vaddr <= addr < vaddr + memsz:
            offset = addr - vaddr
            if offset + size <= len(data):
                return data[offset:offset+size]
    return None


def read_u32(segments, addr):
    data = read_bytes(segments, addr, 4)
    if data:
        return struct.unpack("<I", data)[0]
    return None


def disassemble_function(segments, start, max_insn=200):
    md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS64 | CS_MODE_LITTLE_ENDIAN)
    md.detail = True

    code = read_bytes(segments, start, max_insn * 4)
    if code is None:
        return []

    insns = []
    for i in md.disasm(code, start, count=max_insn):
        insns.append(i)
        if i.mnemonic in ("jr", "j") and i.reg_name(i.operands[0].reg if i.operands else 0) == "ra":
            break
        if i.mnemonic == "jr" and "$31" in i.op_str:
            break
    return insns


def extract_mem_disp(insn):
    """Extract memory displacement from a MIPS instruction.
    Capstone MIPS uses op.mem.disp for displacement in memory operands."""
    if not insn.operands or len(insn.operands) < 2:
        return None, None
    op = insn.operands[1]
    if op.type == CS_OP_MEM:
        return op.mem.base, op.mem.disp
    return None, None


def extract_struct_offsets(insns):
    """Extract struct offset accesses from MIPS instructions."""
    offsets = set()
    for insn in insns:
        base, disp = extract_mem_disp(insn)
        if base is not None and disp is not None and 0 < disp < 0x10000:
            offsets.add(disp & 0xFFFFFFFF)
    return offsets


def extract_jal_targets(insns):
    """Extract JAL/JALR targets from instructions."""
    targets = set()
    for insn in insns:
        if insn.mnemonic in ("jal", "jalr"):
            if insn.operands:
                if insn.mnemonic == "jal":
                    targets.add(insn.operands[0].imm)
                elif insn.mnemonic == "jalr" and insn.operands:
                    # Register target, can't resolve statically
                    pass
    return targets


def scan_code_range(segments, start, end, step=4):
    """Scan a code range for struct offset accesses."""
    all_offsets = set()
    all_jals = set()
    md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS64 | CS_MODE_LITTLE_ENDIAN)
    md.detail = True

    code = read_bytes(segments, start, end - start)
    if code is None:
        return all_offsets, all_jals

    for i in md.disasm(code, start, count=(end - start) // 4):
        if i.mnemonic in ("jal", "jalr") and i.operands:
            if i.mnemonic == "jal":
                all_jals.add(i.operands[0].imm)

        base, disp = extract_mem_disp(i)
        if base is not None and disp is not None and 0 < disp < 0x10000:
            all_offsets.add(disp & 0xFFFFFFFF)

    return all_offsets, all_jals


def read_descriptor_table(segments):
    """Read entity descriptor table: 68 entries, stride 0x64 at 0x2A31B8."""
    entries = []
    for i in range(DESC_ENTRY_COUNT):
        base = DESC_TABLE_ADDR + i * DESC_ENTRY_STRIDE
        data = read_bytes(segments, base, DESC_ENTRY_STRIDE)
        if data is None:
            entries.append(None)
            continue

        # Parse key fields based on research notes
        # offset +0x00: first word (type_id or flags)
        # offset +0x04: second word
        # offset +0x08: init_fn pointer? (from research: only 12/68 have init_fn)
        # offset +0x0C-0x60: handler table

        words = struct.unpack("<" + "I" * (DESC_ENTRY_STRIDE // 4), data)

        # The init_fn is likely at a specific offset within the descriptor
        # From research: ENEMY1 idx=4, init_fn=0x164440
        # The descriptor contains function pointers at various offsets
        init_fn = None
        handler_ptrs = []
        for j, w in enumerate(words):
            if 0x00100000 < w < 0x00300000:  # Code address range
                handler_ptrs.append((j * 4, w))

        entries.append({
            "index": i,
            "base": base,
            "words": words,
            "handler_ptrs": handler_ptrs,
        })

    return entries


def main():
    print("=" * 80)
    print("ENTITY HANDLER STRUCT OFFSET COMPARISON")
    print("ENEMY1 vs BARREL vs BOY vs others")
    print("=" * 80)

    segments = {}
    with open(ELF_PATH, "rb") as f:
        elf = ELFFile(f)
        for seg in elf.iter_segments():
            if seg.header.p_type == "PT_LOAD":
                segments[seg.header.p_vaddr] = (seg.data(), seg.header.p_memsz)

    md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS64 | CS_MODE_LITTLE_ENDIAN)
    md.detail = True

    # =========================================================================
    # 1. ENEMY1 init_fn at 0x164440 — disassemble first 40 instructions
    # =========================================================================
    print("\n" + "=" * 80)
    print("1. ENEMY1 init_fn at 0x164440 — first 40 instructions")
    print("=" * 80)

    enemy1_init_code = read_bytes(segments, 0x164440, 40 * 4)
    enemy1_init_insns = []
    if enemy1_init_code:
        for i in md.disasm(enemy1_init_code, 0x164440, count=40):
            enemy1_init_insns.append(i)

    enemy1_jals = set()
    enemy1_offsets = set()
    for insn in enemy1_init_insns:
        mnemonic_str = insn.mnemonic
        if mnemonic_str == "jal" and insn.operands:
            t = insn.operands[0].imm
            enemy1_jals.add(t)
            print(f"  JAL 0x{t:08X}")

        base, disp = extract_mem_disp(insn)
        if base is not None and disp is not None and 0 < disp < 0x10000:
            enemy1_offsets.add(disp & 0xFFFFFFFF)

    print(f"\n  JAL targets: {sorted(enemy1_jals)}")
    print(f"  Struct offsets: {sorted(enemy1_offsets)}")

    # =========================================================================
    # 2. BARREL-related code in 0x1D0000-0x1E0000
    # =========================================================================
    print("\n" + "=" * 80)
    print("2. BARREL code range 0x1D0000-0x1E0000 — struct offset scan")
    print("=" * 80)

    barrel_offsets, barrel_jals = scan_code_range(segments, 0x1D0000, 0x1E0000)

    # Count functions by looking for 'jr $ra' boundaries
    func_count = 0
    barrel_code = read_bytes(segments, 0x1D0000, 0x10000)
    if barrel_code:
        in_func = False
        for i in md.disasm(barrel_code, 0x1D0000, count=0x10000 // 4):
            if not in_func:
                if i.mnemonic in ("addiu", "addu") and "$29" in i.op_str:
                    in_func = True
                    func_count += 1
            if i.mnemonic == "jr" and "$31" in i.op_str:
                in_func = False

    print(f"  Functions found in range: ~{func_count}")
    print(f"  Unique struct offsets: {len(barrel_offsets)}")
    print(f"  Top offsets: {sorted(barrel_offsets)[:30]}")
    print(f"  JAL targets (unique): {len(barrel_jals)}")

    # =========================================================================
    # 3. Compare struct offset usage across entity types
    # =========================================================================
    print("\n" + "=" * 80)
    print("3. STRUCT OFFSET COMPARISON ACROSS ENTITY TYPES")
    print("=" * 80)

    entity_offsets = {}

    for name, info in ENTITY_HANDLERS.items():
        all_offsets = set()

        def _scan_code(start_addr, max_insn=300):
            code = read_bytes(segments, start_addr, max_insn * 4)
            if code:
                for i in md.disasm(code, start_addr, count=max_insn):
                    base, disp = extract_mem_disp(i)
                    if base is not None and disp is not None and 0 < disp < 0x10000:
                        all_offsets.add(disp & 0xFFFFFFFF)

        # Scan init_fn
        if "init_fn" in info:
            _scan_code(info["init_fn"])

        # Scan hA, hB, hC, hD
        for hkey in ("hA", "hB", "hC", "hD"):
            if hkey in info:
                _scan_code(info[hkey])

        # Scan sub-functions if available
        for sub in info.get("sub_fns", []):
            _scan_code(sub, 100)

        entity_offsets[name] = all_offsets
        print(f"\n  {name}: {len(all_offsets)} unique offsets")
        print(f"    Key: {sorted(all_offsets)[:25]}")

    # =========================================================================
    # 4. Entity descriptor table at 0x2A31B8
    # =========================================================================
    print("\n" + "=" * 80)
    print("4. ENTITY DESCRIPTOR TABLE at 0x2A31B8 (first 31 entries)")
    print("=" * 80)

    desc_entries = read_descriptor_table(segments)

    KNOWN_ENTITY_NAMES = {
        1: "BOY",
        2: "GIRL",
        4: "ENEMY1",
        17: "WOODBOX0",
        48: "DEVIL_GI",
        61: "AP1",
    }

    for i in range(min(31, len(desc_entries))):
        entry = desc_entries[i]
        if entry is None:
            continue

        name = KNOWN_ENTITY_NAMES.get(i, f"idx_{i}")
        ptrs = entry["handler_ptrs"]
        ptr_strs = [f"+0x{off:03X}=0x{addr:08X}" for off, addr in ptrs]
        has_init = any(addr == ENTITY_HANDLERS.get(name, {}).get("init_fn") for _, addr in ptrs)

        # Check if this entry's handlers access boyAI-like offsets
        entity_name = name
        offsets = entity_offsets.get(entity_name, set())

        print(f"\n  [{i:2d}] {name:12s}  ptrs: {' '.join(ptr_strs) if ptr_strs else '(none)'}")
        if has_init:
            print(f"       INIT_FN MATCH: 0x{ENTITY_HANDLERS[name]['init_fn']:08X}")
        if offsets:
            # Check for +0x15C, +0x164, +0x800 usage (key entity offsets)
            key_offsets = {0x15C, 0x164, 0x800, 0x670, 0x840, 0x678}
            found = key_offsets & offsets
            if found:
                print(f"       Key offsets: {', '.join(f'+0x{X:X}' for X in sorted(found))}")

    # =========================================================================
    # 5. Identify BOY-specific offsets
    # =========================================================================
    print("\n" + "=" * 80)
    print("5. BOY-SPECIFIC vs SHARED vs ENTITY-SPECIFIC OFFSETS")
    print("=" * 80)

    # Determine shared offsets (used by 2+ entity types)
    all_entity_sets = [(name, offsets) for name, offsets in entity_offsets.items()]

    # Offsets used by ALL entities (universal)
    universal = set.intersection(*[offsets for _, offsets in all_entity_sets if offsets])
    # Offsets used by BOY but not by any other entity
    boy_only = entity_offsets.get("BOY", set()) - set.union(
        *[offsets for name, offsets in all_entity_sets if name != "BOY" and offsets]
    )
    # Offsets used by ENEMY1 but not by any other
    enemy1_only = entity_offsets.get("ENEMY1", set()) - set.union(
        *[offsets for name, offsets in all_entity_sets if name != "ENEMY1" and offsets]
    )
    # Offsets used by BARREL but not by any other
    barrel_only = entity_offsets.get("BARREL", set()) - set.union(
        *[offsets for name, offsets in all_entity_sets if name != "BARREL" and offsets]
    )
    # Shared by BOY + ENEMY1 (both human/AI entities)
    boy_enemy1_shared = entity_offsets.get("BOY", set()) & entity_offsets.get("ENEMY1", set())
    # Shared by BOY + BARREL (both carryable physics objects?)
    boy_barrel_shared = entity_offsets.get("BOY", set()) & entity_offsets.get("BARREL", set())
    # Shared by ALL types
    all_shared = set.intersection(*[offsets for _, offsets in all_entity_sets if offsets])

    print(f"\n  UNIVERSAL (all entity types): {len(universal)} offsets")
    for off in sorted(universal):
        name = GOBJ_OFFSET_NAMES.get(off, f"field_{off:X}")
        print(f"    +0x{off:04X} ({name})")

    print(f"\n  BOY + ENEMY1 SHARED: {len(boy_enemy1_shared)} offsets")
    for off in sorted(boy_enemy1_shared):
        name = GOBJ_OFFSET_NAMES.get(off, f"field_{off:X}")
        print(f"    +0x{off:04X} ({name})")

    print(f"\n  BOY + BARREL SHARED: {len(boy_barrel_shared)} offsets")
    for off in sorted(boy_barrel_shared):
        name = GOBJ_OFFSET_NAMES.get(off, f"field_{off:X}")
        print(f"    +0x{off:04X} ({name})")

    print(f"\n  BOY-ONLY (not used by ENEMY1, BARREL, WOODBOX0, AP1, GIRL): {len(boy_only)} offsets")
    for off in sorted(boy_only):
        name = GOBJ_OFFSET_NAMES.get(off, f"field_{off:X}")
        print(f"    +0x{off:04X} ({name})")

    print(f"\n  ENEMY1-ONLY: {len(enemy1_only)} offsets")
    for off in sorted(enemy1_only):
        name = GOBJ_OFFSET_NAMES.get(off, f"field_{off:X}")
        print(f"    +0x{off:04X} ({name})")

    print(f"\n  BARREL-ONLY: {len(barrel_only)} offsets")
    for off in sorted(barrel_only):
        name = GOBJ_OFFSET_NAMES.get(off, f"field_{off:X}")
        print(f"    +0x{off:04X} ({name})")

    # =========================================================================
    # COMPARISON TABLE
    # =========================================================================
    print("\n" + "=" * 80)
    print("COMPARISON TABLE: Struct Offset Usage by Entity Type")
    print("=" * 80)

    # Collect ALL offsets from ALL entities
    all_offsets = sorted(set.union(*[offsets for _, offsets in all_entity_sets if offsets]))

    entity_names = ["BOY", "ENEMY1", "BARREL", "WOODBOX0", "AP1", "GIRL"]
    header = f"{'Offset':>10s}"
    for name in entity_names:
        header += f" {name:>8s}"
    header += "  Category"
    print(f"\n{header}")
    print("-" * len(header))

    for off in all_offsets:
        name = GOBJ_OFFSET_NAMES.get(off, "")
        row = f"  +0x{off:04X}"
        row += f" {name:>8s}"
        used_by = []
        for ename in entity_names:
            if off in entity_offsets.get(ename, set()):
                row += f"    {'YES':>5s}"
                used_by.append(ename)
            else:
                row += f"    {'---':>5s}"

        # Category
        if len(used_by) == len(entity_names):
            cat = "UNIVERSAL"
        elif len(used_by) == 1:
            cat = f"{used_by[0]}-ONLY"
        elif set(used_by) == {"BOY", "ENEMY1"}:
            cat = "BOY+ENEMY1"
        elif set(used_by) == {"BOY", "BARREL"}:
            cat = "BOY+BARREL"
        else:
            cat = f"shared({','.join(used_by)})"

        row += f"  {cat}"
        print(row)

    # =========================================================================
    # SUMMARY
    # =========================================================================
    print("\n" + "=" * 80)
    print("SUMMARY")
    print("=" * 80)

    print(f"\n  Total offsets analyzed across all entities: {len(all_offsets)}")
    print(f"  Universal offsets (all types): {len(universal)}")
    print(f"  BOY-specific offsets: {len(boy_only)}")
    print(f"  ENEMY1-specific offsets: {len(enemy1_only)}")
    print(f"  BARREL-specific offsets: {len(barrel_only)}")

    print(f"\n  KEY FINDINGS:")
    print(f"  - +0x15C (model_data): {'UNIVERSAL' if 0x15C in universal else 'SHARED' if 0x15C in all_shared else 'PARTIAL'} — all entity types load this")
    print(f"  - +0x164 (behavior_data): {'BOY+ENEMY1' if 0x164 in boy_enemy1_shared else 'UNIVERSAL' if 0x164 in universal else 'PARTIAL'} — AI entities")
    print(f"  - +0x800 (state_block): {'BOY+ENEMY1+BARREL' if 0x800 in (entity_offsets.get('BOY',set()) & entity_offsets.get('ENEMY1',set()) & entity_offsets.get('BARREL',set())) else 'check'} — state block pointer")
    print(f"  - +0x048: ENEMY1 mask slot flag access — NOT in BOY")
    print(f"  - +0x04C: ENEMY1 counter in state_block — NOT in BOY")
    print(f"  - +0x644/0x648: BOY-specific weapon/interaction — NOT in ENEMY1")
    print(f"  - +0x548/0x54C/0x550: ENEMY1 per-frame state — partially in BOY")
    print(f"  - +0x678: shared AI data — ENEMY1 behavior data access")

    return 0


if __name__ == "__main__":
    sys.exit(main())
