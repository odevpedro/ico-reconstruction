#!/usr/bin/env python3
"""Analyze the BOY entity work area structure from verified .s assembly files.

Focuses on memory load/store instructions (lw/sw/lwc1/swc1/ld/sd/lh/sh/lb/sb)
to extract struct field offsets. Excludes addiu (pointer arithmetic).
"""

import sys
import os
import re
from collections import defaultdict, Counter

SRC_ROOT = os.path.expanduser(
    "/home/hoper/Documentos/repos/ico-reconstruction/src/entity/asm"
)

KNOWN_OFFSETS = {
    0x0000: ("type/flags",           373),
    0x0004: ("state_flags",          359),
    0x0008: ("animation_state",      181),
    0x000C: ("timer/counter",        145),
    0x0010: ("status_halfword",       52),
    0x0014: ("events",                49),
    0x0018: ("float_position_angle",  46),
    0x0020: ("float_coordinate_A",    33),
    0x0024: ("float_coordinate_B",    28),
    0x0030: ("table_pointer",        161),
    0x0034: ("parent_data_pointer",   50),
    0x0038: ("float_scale_height",    46),
    0x0048: ("behavior_state",        34),
    0x015C: ("motion_substruct_ptr", 329),
    0x0164: ("ai_data_block_ptr",    738),
    0x0670: ("shared_scene_data_A",  260),
    0x0678: ("shared_scene_data_B",  141),
}

# Memory load/store instructions only
MEM_INSTRUCTIONS = re.compile(
    r'(lw|sw|lhu|lhu|lh|sh|lb|sb|lwc1|swc1|ld|sd|lq|sq)\s+'
    r'(\$\w+),\s*(-?0x[0-9a-fA-F]+|-?\d+)\((\$\w+)\)'
)


def parse_s_files():
    """Parse all .s files in the entity/asm directory."""
    all_lines = []
    files = sorted(f for f in os.listdir(SRC_ROOT) if f.endswith('.s'))
    for fname in files:
        path = os.path.join(SRC_ROOT, fname)
        with open(path) as f:
            for i, line in enumerate(f, 1):
                all_lines.append((fname, i, line.rstrip()))
    return all_lines, files


def extract_memory_refs(lines):
    """Extract memory offset references from assembly lines."""
    refs = []
    for fname, lineno, line in lines:
        code = line.split('#')[0].strip()
        if not code or code.startswith('.') or code.startswith('#'):
            continue

        m = MEM_INSTRUCTIONS.search(code)
        if m:
            mnem = m.group(1)
            dst = m.group(2)
            offset_str = m.group(3)
            base = m.group(4)
            try:
                offset = int(offset_str, 16) if 'x' in offset_str.lower() else int(offset_str)
            except ValueError:
                continue
            if offset >= 0:
                refs.append((fname, lineno, offset, base, mnem, code))
    return refs


def classify_type(mnemonics):
    """Classify field type from instruction patterns."""
    has_float = any(m in mnemonics for m in ["lwc1", "swc1"])
    has_word = any(m in mnemonics for m in ["lw", "sw"])
    has_half = any(m in mnemonics for m in ["lh", "lhu", "sh"])
    has_byte = any(m in mnemonics for m in ["lb", "lbu", "sb"])
    has_double = any(m in mnemonics for m in ["ld", "sd"])

    if has_double and has_word:
        return "ptr/word (8B)"
    elif has_double:
        return "dword (8B)"
    elif has_float and has_word:
        return "float/int (4B)"
    elif has_float:
        return "float (4B)"
    elif has_half and has_byte:
        return "mixed (2-4B)"
    elif has_half:
        return "halfword (2B)"
    elif has_byte:
        return "byte/flag (1B)"
    elif has_word:
        return "word/int (4B)"
    else:
        return "?"


def main():
    print("Parsing .s files...")
    lines, files = parse_s_files()
    boyai_files = [f for f in files if f.startswith('boy') or f.startswith('boyAI')]

    boyai_lines = [(f, n, l) for f, n, l in lines if f in boyai_files]
    print(f"  {len(boyai_files)} boyAI .s files, {len(boyai_lines)} lines")

    # Extract memory references (load/store only)
    refs = extract_memory_refs(boyai_lines)
    print(f"  {len(refs)} memory offset references")

    # Work area base registers
    WORK_REGS = {'$4', '$16', '$17', '$20', '$21', '$22'}
    work_refs = [(f, n, o, b, m, c) for f, n, o, b, m, c in refs if b in WORK_REGS]
    print(f"  {len(work_refs)} work-area-relative refs")

    # Aggregate
    offset_mnemonics = defaultdict(Counter)
    offset_files = defaultdict(set)
    offset_examples = defaultdict(list)
    offset_regs = defaultdict(Counter)
    offset_count = Counter()

    for fname, lineno, offset, base, mnem, code in work_refs:
        offset_mnemonics[offset][mnem] += 1
        offset_files[offset].add(fname)
        offset_regs[offset][base] += 1
        offset_count[offset] += 1
        if len(offset_examples[offset]) < 4:
            offset_examples[offset].append((fname, lineno, code))

    # Separate: work area direct (base=$4/$16/$17/$20/$21) vs
    # sub-struct (base=$2/$3 after loading from 0x15C or 0x164)
    # For now, keep all direct work area refs

    # Build complete offset set
    all_offsets = set(offset_mnemonics.keys()) | set(KNOWN_OFFSETS.keys())
    sorted_offs = sorted(o for o in all_offsets if o < 0x5000)

    # Group into regions (gap > 0x30 starts new region)
    regions = []
    if sorted_offs:
        cur_start = sorted_offs[0]
        cur_end = sorted_offs[0]
        cur_list = [sorted_offs[0]]
        for off in sorted_offs[1:]:
            if off - cur_end > 0x30:
                regions.append((cur_start, cur_end, cur_list[:]))
                cur_start = off
                cur_end = off
                cur_list = [off]
            else:
                cur_end = off
                cur_list.append(off)
        regions.append((cur_start, cur_end, cur_list[:]))

    # === PRINT STRUCT LAYOUT ===
    print()
    print("=" * 100)
    print("BOY ENTITY WORK AREA — PROPOSED STRUCT LAYOUT")
    print("=" * 100)
    print()
    print(f"Source: {len(boyai_files)} boyAI .s files, {len(work_refs)} memory refs")
    max_off = max(sorted_offs)
    print(f"Max offset: +0x{max_off:04X} ({max_off} bytes)")
    print(f"Est. struct size: ~0x{((max_off + 0x100) & ~0xFF):05X}")
    print()

    for reg_start, reg_end, reg_offsets in regions:
        region_size = reg_end - reg_start + 4
        print("-" * 100)
        print(f"REGION +0x{reg_start:04X}–+0x{reg_end:04X} ({region_size} bytes, {len(reg_offsets)} fields)")
        print("-" * 100)
        print(f"  {'Off':<8} {'Type':<16} {'Name':<32} {'RW':<5} {'N':<5} {'#F':<4} {'Regs':<30}")
        print(f"  {'---':<8} {'----':<16} {'----':<32} {'--':<5} {'--':<5} {'--':<4} {'----':<30}")

        for off in sorted(reg_offsets):
            if off in KNOWN_OFFSETS:
                name = KNOWN_OFFSETS[off][0]
            else:
                name = f"field_0x{off:04X}"

            if off in offset_mnemonics:
                mnem = offset_mnemonics[off]
                total = sum(mnem.values())
                reads = sum(v for k, v in mnem.items() if k.startswith('l'))
                writes = sum(v for k, v in mnem.items() if k.startswith('s'))
                rw = "R" if reads > 0 and writes == 0 else "W" if writes > 0 and reads == 0 else "R/W"
                ftype = classify_type(mnem)
                nfiles = len(offset_files[off])
                top_regs = ", ".join(f"{r}({c})" for r, c in offset_regs[off].most_common(3))
            else:
                total = KNOWN_OFFSETS.get(off, ("", 0))[1]
                rw = "known" if off in KNOWN_OFFSETS else "?"
                ftype = "?"
                nfiles = 0
                top_regs = ""

            print(f"  +0x{off:04X}  {ftype:<16} {name:<32} {rw:<5} {total:<5} {nfiles:<4} {top_regs}")

        print()

    # === DETAILED PATTERNS ===
    print("=" * 100)
    print("TOP 30 MOST-ACCESSED OFFSETS — DETAILED PATTERNS")
    print("=" * 100)
    print()

    ranked = sorted(offset_count.keys(), key=lambda o: offset_count[o], reverse=True)
    for off in ranked[:30]:
        mnem = offset_mnemonics[off]
        total = sum(mnem.values())
        name = KNOWN_OFFSETS.get(off, (f"field_0x{off:04X}", 0))[0]
        print(f"  +0x{off:04X}  {name}  ({total} accesses, {len(offset_files[off])} files)")
        print(f"    ops: {dict(mnem.most_common())}")
        print(f"    base: {dict(offset_regs[off].most_common())}")
        for fn, ln, code in offset_examples[off][:3]:
            print(f"      {fn}:{ln}: {code}")
        print()

    # === SUB-STRUCTURE SUMMARY ===
    print("=" * 100)
    print("SUB-STRUCTURE HIERARCHY")
    print("=" * 100)
    print("""
  BoyWorkArea (base pointer in $a0 on entry, saved to $s0/$s16)
  │
  ├─ +0x0000–0x0048   Core fields (type, state, animation, timer, flags)
  │   +0x0000  lw/sw  type/entity type word
  │   +0x0004  lw/sw  state_flags word
  │   +0x0008  lw/sw  animation_state word
  │   +0x000C  lw/sw  timer/counter word (read-only pattern: lw)
  │   +0x0010  lh/lhu status halfword
  │   +0x0014  lw/sw  events word
  │   +0x0018  ld/sd  8-byte value (position/angle packed)
  │   +0x001C  lw     float word
  │   +0x0020  ld/sd  8-byte value (coordinate pair A)
  │   +0x0024  lw/sw  float word (coordinate B)
  │   +0x0028  ld     pointer (64-bit read)
  │   +0x002C  lw/sw  word
  │   +0x0030  lw/sw  table pointer (dispatch/action table)
  │   +0x0034  lw/sw  parent/data pointer
  │   +0x0036  lh     halfword field
  │   +0x0038  lw/sw  float (scale/height)
  │   +0x003C  lw     word
  │   +0x0040  swc1   float (animation blend?)
  │   +0x0044  swc1   float
  │   +0x0048  lw     behavior state (dispatched)
  │
  ├─ +0x0050–0x0090   Mid section (sparse, possibly padding or sub-structs)
  │   +0x0058  ld/sd  8-byte value (only 3 files)
  │   +0x0090  lw/sw  word (only 2 files)
  │
  ├─ +0x00A0–0x0100   Extended state area
  │   +0x00A0  lw/sw  word
  │   +0x00C8  lw     word (read-only)
  │   +0x00F0  lw/sw  word
  │   +0x0104  lw/sw  word
  │
  ├─ +0x0110–0x0130   Position/velocity floats (XYZ)
  │   +0x0110  swc1   float X (write-only)
  │   +0x0114  swc1   float Y (write-only)
  │   +0x0118  swc1   float Z (write-only)
  │   +0x0120  sw     word
  │   +0x0124  lw/sw  word
  │   +0x012A  lh/sh  halfword
  │   +0x0130  swc1   float
  │
  ├─ +0x015C  ──→ MotionSubStruct* (lw, 113 accesses in 54 files)
  │   │         Pointer to motion/animation sub-structure
  │   │         After loading: lw $reg, 0x800($motion) → StateBlock
  │   │
  │   └─ +0x800  StateBlock (accessed via motion+0x800)
  │       +0x000  lw   state_id (word: 1/2/3 → branch)
  │       +0x008  lw   state_ptr_1 (when state_id==1)
  │       +0x00C  lw   state_ptr_2 (when state_id==2)
  │       +0x014  sw   animation_timer (cleared to 0)
  │       +0x02C  addiu → derived pointer (+0x2C offset)
  │
  ├─ +0x0164  ──→ AIDataBlock* (lw, 131 accesses in 65 files)
  │   │         Pointer to AI/behavior data block
  │   │         Sub-offsets are relative to this pointer
  │   │
  │   ├─ +0x03C   lw   current_state_id (loop variable)
  │   ├─ +0x120   sw   function call result
  │   ├─ +0x3D0   addiu → derived offset (from +0x678 base)
  │   ├─ +0x468   ld   64-bit bitfield (dsrl32+andi for flag bits)
  │   ├─ +0x478   ld   64-bit bitfield (dsrl32+andi for flag bits)
  │   ├─ +0x480   ld/sd 64-bit value (61 accesses, 23 files)
  │   ├─ +0x610   array base (stride access pattern)
  │   └─ +0x678   lw   shared_scene_data_B (duplicate of work area +0x678)
  │
  ├─ +0x016C–0x01FC  Additional fields
  │   +0x016C  lw   word (9 accesses)
  │   +0x0180  lw   word (25 accesses, all via $17)
  │   +0x01B8  lw/sw word
  │   +0x01D0  swc1 float
  │   +0x01FC  lw   word
  │
  ├─ +0x0250–0x038C  Motion/animation parameter area
  │   +0x02D0  lw   word (6 accesses)
  │   +0x02D4  lw   word (13 accesses, $20 dominant)
  │   +0x0328  lw   word (8 accesses)
  │   +0x032C  lw/sw word (7 accesses)
  │   +0x0330  lw   word
  │   +0x033C  lwc1 float (31 accesses, $20/$16)
  │   +0x0340  lw/sw word
  │   +0x0344  swc1 float
  │   +0x0350  lw/sw word
  │
  ├─ +0x03D0–0x04B8  Extended behavior/physics area
  │   +0x0408  lw/ld/sd mixed (7 accesses)
  │   +0x0418  swc1 float
  │   +0x0440  sw   word
  │   +0x0460  lw/sw float
  │   +0x0468  ld   64-bit bitfield (24 accesses, $20 only)
  │   +0x0470  ld   pointer (25 accesses, $20/$16)
  │   +0x0478  ld/sd 64-bit bitfield (41 accesses)
  │   +0x0480  ld/sd 64-bit value (61 accesses, 23 files)
  │   +0x0488  ld/sd 64-bit value
  │   +0x04A0  lw   word
  │   +0x04B0–0x04B8  floats (1 access each)
  │
  ├─ +0x0560–0x0678  Scene/physics data area
  │   +0x0560  lwc1 float
  │   +0x0590–0x0598  floats (animation params)
  │   +0x05D0  lw   word
  │   +0x0604–0x0618  lw/sw words
  │   +0x0628  lw   word
  │   +0x0644  lwc1 float (compared to 20.0f)
  │   +0x0648  lw   pointer (NULL-checked)
  │   +0x0650  lw/sw word
  │   +0x0670  lw   shared_scene_data_A (11 files)
  │   +0x0678  lw   shared_scene_data_B (36 accesses, 14 files)
  │
  └─ +0x067C–0x4EC0  Extended / scene-specific / overlay areas
      +0x067C  (set in boy_init)
      +0x0800  lw/sw word (1 access)
      +0x1928  lw/sw word (1 access)
      +0x22F0–0x2338  words (2-4 files)
      +0x2390  swc1 float (7 accesses, $20 only)
      +0x23C0  lw/sw word
      +0x4EC0  lw   word (14 accesses, $22 dominant — likely GP-relative, NOT work area)
""")

    # === TYPE SUMMARY ===
    print("=" * 100)
    print("FIELD TYPE SUMMARY")
    print("=" * 100)
    print()

    type_counts = Counter()
    total_fields = 0
    for off in sorted_offs:
        if off in offset_mnemonics:
            ftype = classify_type(offset_mnemonics[off])
            type_counts[ftype] += 1
            total_fields += 1

    for ftype, count in type_counts.most_common():
        print(f"  {ftype:<20} {count:>4} fields")

    print(f"  {'TOTAL':<20} {total_fields:>4} fields")
    print()

    # === CONFIRMED vs UNCONFIRMED ===
    confirmed = sum(1 for o in sorted_offs if o in KNOWN_OFFSETS)
    discovered = sum(1 for o in sorted_offs if o not in KNOWN_OFFSETS and o in offset_mnemonics)
    print(f"  Confirmed offsets (user-provided):  {confirmed}")
    print(f"  Discovered from .s analysis:         {discovered}")
    print(f"  Total work-area fields:              {confirmed + discovered}")
    print()

    # === STRUCT SIZE ESTIMATE ===
    # Core fields are dense up to +0x0048
    # Then sparse until +0x015C (motion ptr)
    # Then +0x0164 (AI data ptr)
    # Then +0x0670/0x0678 (scene data)
    print("=" * 100)
    print("STRUCT SIZE ESTIMATE")
    print("=" * 100)
    print()
    print("  Core fields:        +0x0000–0x004C  (76 bytes, tightly packed)")
    print("  Mid section:        +0x0050–0x0100  (176 bytes, sparse)")
    print("  Position/velocity:  +0x0110–0x0130  (32 bytes, XYZ floats)")
    print("  Pointers:           +0x015C–0x0168  (12 bytes, motion+AI)")
    print("  Extended state:     +0x016C–0x01FC  (144 bytes, sparse)")
    print("  Motion params:      +0x0250–0x038C  (316 bytes)")
    print("  Behavior area:      +0x03D0–0x04B8  (232 bytes)")
    print("  Scene data:         +0x0560–0x0678  (280 bytes)")
    print("  ─────────────────────────────────────────────")
    print("  Contiguous core:    ~0x0680 bytes (1664 bytes)")
    print("  With gaps (to +0x0678): ~0x0680 bytes")
    print()
    print("  NOTE: +0x4EC0 accessed via $22 (GP-relative, NOT work area)")
    print("  The work area is likely ~0x0680–0x0700 bytes (1664–1792).")
    print("  The +0x0800, +0x1928, +0x22F0, +0x4EC0 offsets may be from")
    print("  different base pointers (GP, scene data, overlay).")
    print()


if __name__ == "__main__":
    main()
