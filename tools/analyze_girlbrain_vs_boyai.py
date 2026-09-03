#!/usr/bin/env python3
"""
Analyze GirlBrain vs boyAI struct access patterns from the ICO PS2 ELF.

Disassembles the GirlBrain range (0x16A000-0x175000) and all boyAI .s files,
extracts struct field offsets, GP-relative accesses, and JAL targets.
Produces a comparison table.
"""

import os
import re
import sys
from collections import defaultdict
from pathlib import Path

try:
    from capstone import Cs, CS_ARCH_MIPS, CS_MODE_MIPS64, CS_MODE_LITTLE_ENDIAN
except ImportError:
    print("ERROR: capstone not installed. Run: pip3 install capstone")
    sys.exit(1)

try:
    from elftools.elf.elffile import ELFFile
except ImportError:
    print("ERROR: pyelftools not installed. Run: pip3 install pyelftools")
    sys.exit(1)


ELF_PATH = os.path.join(os.path.dirname(__file__), '..', '.local', 'extracted', 'SCUS_971.13.elf')
ASM_DIR = os.path.join(os.path.dirname(__file__), '..', 'src', 'entity', 'asm')

# GirlBrain address range (Rev.102 corrected)
GIRLBRAIN_LO = 0x16A000
GIRLBRAIN_HI = 0x175000

# GP-relative offsets to track (from AGENTS.md known offsets)
KNOWN_GP_OFFSETS = {
    -0x6E08: "gp-0x6E08 (entity_work_a)",
    -0x6E0C: "gp-0x6E0C (entity_work_b)",
    -0x6F60: "gp-0x6F60 (world_state)",
    -0x6724: "gp-0x6724 (mask_bits)",
    -0x6740: "gp-0x6740 (thread_counter)",
    -0x4C4C: "gp-0x4C4C (gobj_count)",
    -0x49B4: "gp-0x49B4 (entity_work_area)",
    -0x6714: "gp-0x6714",
    -0x7B44: "gp-0x7B44",
    -0x7B40: "gp-0x7B40",
    -0x7B88: "gp-0x7B88",
    -0x7B84: "gp-0x7B84",
}


def parse_s_file_offsets(filepath):
    """Parse a .s file and extract struct offsets from lw/sw/lwc1/swc1/ld/sd instructions."""
    offsets = defaultdict(int)
    gp_offsets = defaultdict(int)
    jal_targets = defaultdict(int)

    with open(filepath) as f:
        for line in f:
            line = line.strip()
            # Skip comments, directives, labels
            if not line or line.startswith('.') or line.startswith('#') or line.endswith(':'):
                continue

            # Extract struct offsets: OFFSET(REG) patterns
            # lw/sw/ld/sd/lwc1/swc1/sdc1/ldc1 OFFSET($reg)
            m = re.findall(r'(?:lw|sw|ld|sd|lwc1|swc1|ldc1|sdc1|lh|sh|lhu|shu|lb|sb|lbu|sbu)\s+.*?,\s*(-?0x[0-9a-fA-F]+)\((\$\w+)\)', line)
            for off_str, reg in m:
                off = int(off_str, 16)
                if off_str.startswith('-'):
                    off = -abs(off)
                offsets[(reg, off)] += 1

            # GP-relative: lw/sw offset($28) where $28 = gp
            m_gp = re.findall(r'(?:lw|sw|ld|sd|lwc1|swc1)\s+.*?,\s*(-?0x[0-9a-fA-F]+)\((\$28)\)', line)
            for off_str, _ in m_gp:
                off = int(off_str, 16)
                if off_str.startswith('-'):
                    off = -abs(off)
                gp_offsets[off] += 1

            # JAL targets
            m_jal = re.findall(r'jal\s+(0x[0-9a-fA-F]+)', line)
            for addr in m_jal:
                jal_targets[int(addr, 16)] += 1

            # J targets (unconditional jumps to other functions)
            m_j = re.findall(r'\bj\s+(0x[0-9a-fA-F]+)', line)
            for addr in m_j:
                jal_targets[int(addr, 16)] += 1

    return offsets, gp_offsets, jal_targets


def disasm_elf_range(elf_path, va_lo, va_hi):
    """Disassemble a VA range from the ELF and extract struct offsets."""
    offsets = defaultdict(int)
    gp_offsets = defaultdict(int)
    jal_targets = defaultdict(int)
    functions_found = []

    with open(elf_path, 'rb') as f:
        elf = ELFFile(f)
        for section in elf.iter_segments():
            if section['p_type'] != 'PT_LOAD':
                continue
            seg_va = section['p_vaddr']
            seg_size = section['p_memsz']
            seg_file_off = section['p_offset']

            # Check if our range overlaps this segment
            if va_hi <= seg_va or va_lo >= seg_va + seg_size:
                continue

            # Calculate file offset for our range
            start_in_seg = max(va_lo, seg_va) - seg_va
            end_in_seg = min(va_hi, seg_va + seg_size) - seg_va
            file_off = seg_file_off + start_in_seg
            data_size = end_in_seg - start_in_seg

            f.seek(file_off)
            code = f.read(data_size)
            base_va = max(va_lo, seg_va)

            md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS64 | CS_MODE_LITTLE_ENDIAN)
            md.detail = True

            # Track function prologues
            for insn in md.disasm(code, base_va):
                # Detect function prologues: addiu $sp,$sp,-N
                if insn.mnemonic == 'addiu' and len(insn.operands) == 3:
                    if (insn.operands[0].reg == 29 and  # $sp
                        insn.operands[1].reg == 29 and
                        insn.operands[2].type == 2):  # immediate
                        functions_found.append(insn.address)

                # Extract struct offsets from memory ops
                if insn.mnemonic in ('lw', 'sw', 'ld', 'sd', 'lwc1', 'swc1',
                                      'ldc1', 'sdc1', 'lh', 'sh', 'lhu', 'shu',
                                      'lb', 'sb', 'lbu', 'sbu'):
                    if len(insn.operands) >= 2:
                        src = insn.operands[1] if insn.mnemonic.startswith(('lw', 'ld', 'lh', 'lb', 'lwc1', 'ldc1', 'lhu', 'lbu')) else insn.operands[0]
                        if src.type == 3:  # memory operand
                            reg_name = f"${src.mem.base}" if src.mem.base != 0 else "$0"
                            disp = src.mem.disp
                            # Sign-extend 16-bit displacement
                            disp = disp if disp < 0x8000 else disp - 0x10000
                            offsets[(reg_name, disp)] += 1

                            # GP-relative tracking ($28 = $gp)
                            if src.mem.base == 28:  # $gp
                                gp_offsets[disp] += 1

                # JAL targets
                if insn.mnemonic == 'jal' and len(insn.operands) > 0:
                    target = insn.operands[0].imm
                    jal_targets[target] += 1
                elif insn.mnemonic == 'j' and len(insn.operands) > 0:
                    target = insn.operands[0].imm
                    if va_lo <= target < va_hi:  # only internal targets
                        jal_targets[target] += 1

            break  # only need one segment

    return offsets, gp_offsets, jal_targets, functions_found


def collect_boyai_files():
    """Find all boyAI .s files."""
    boyai_files = []
    for f in sorted(Path(ASM_DIR).glob('boyAI_*.s')):
        boyai_files.append(f)
    return boyai_files


def collect_girlbrain_files():
    """Find all girlBrain .s files."""
    gb_files = []
    for f in sorted(Path(ASM_DIR).glob('girlBrain*.s')):
        gb_files.append(f)
    return gb_files


def merge_offsets(target, source):
    for k, v in source.items():
        target[k] = target.get(k, 0) + v


def main():
    print("=" * 90)
    print("GirlBrain vs boyAI — Entity Struct Access Pattern Comparison")
    print("=" * 90)
    print()

    # ── 1. Disassemble GirlBrain range from ELF ──
    print(f"[1] Disassembling ELF range 0x{GIRLBRAIN_LO:06X}–0x{GIRLBRAIN_HI:06X} ...")
    elf_gb_off, elf_gb_gp, elf_gb_jal, elf_gb_funcs = disasm_elf_range(
        ELF_PATH, GIRLBRAIN_LO, GIRLBRAIN_HI
    )
    print(f"    Functions detected (prologues): {len(elf_gb_funcs)}")
    print(f"    Total struct accesses: {sum(elf_gb_off.values())}")
    print(f"    Unique (reg,offset) pairs: {len(elf_gb_off)}")
    print()

    # ── 2. Parse GirlBrain .s files ──
    print("[2] Parsing GirlBrain .s files ...")
    gb_files = collect_girlbrain_files()
    print(f"    Found {len(gb_files)} .s files")
    gb_offsets = defaultdict(int)
    gb_gp = defaultdict(int)
    gb_jal = defaultdict(int)
    for fp in gb_files:
        o, g, j = parse_s_file_offsets(fp)
        merge_offsets(gb_offsets, o)
        merge_offsets(gb_gp, g)
        merge_offsets(gb_jal, j)
    print(f"    Total struct accesses: {sum(gb_offsets.values())}")
    print()

    # ── 3. Parse boyAI .s files ──
    print("[3] Parsing boyAI .s files ...")
    boyai_files = collect_boyai_files()
    print(f"    Found {len(boyai_files)} .s files")
    boyai_offsets = defaultdict(int)
    boyai_gp = defaultdict(int)
    boyai_jal = defaultdict(int)
    for fp in boyai_files:
        o, g, j = parse_s_file_offsets(fp)
        merge_offsets(boyai_offsets, o)
        merge_offsets(boyai_gp, g)
        merge_offsets(boyai_jal, j)
    print(f"    Total struct accesses: {sum(boyai_offsets.values())}")
    print()

    # ── Also run ELF disasm for boyAI range to cross-check ──
    print("[4] Disassembling ELF boyAI range 0x142000–0x164000 ...")
    elf_ba_off, elf_ba_gp, elf_ba_jal, elf_ba_funcs = disasm_elf_range(
        ELF_PATH, 0x142000, 0x164000
    )
    print(f"    Functions detected: {len(elf_ba_funcs)}")
    print()

    # ══════════════════════════════════════════════════════════════════════
    # COMPARISON TABLE: Entity struct offsets (non-GP)
    # ══════════════════════════════════════════════════════════════════════
    print("=" * 90)
    print("TABLE 1: Entity Struct Field Offsets (from .s files)")
    print("=" * 90)
    print()
    print("Offsets accessed as OFFSET($reg) where $reg is NOT $28/$gp.")
    print("These represent direct struct member accesses (entity work area, etc).")
    print()

    # Collect all unique offsets from both
    all_offsets = set()
    for (reg, off) in gb_offsets:
        if reg != '$28':
            all_offsets.add(off)
    for (reg, off) in boyai_offsets:
        if reg != '$28':
            all_offsets.add(off)

    # Build per-offset summary
    # Group by "most likely struct" based on offset ranges
    print(f"{'Offset':>10} {'BoyAI cnt':>10} {'GirlBrain cnt':>14} {'Status':<25} {'Notes'}")
    print("-" * 90)

    shared_offsets = []
    boyai_only = []
    girlbrain_only = []

    for off in sorted(all_offsets):
        ba_count = sum(v for (r, o), v in boyai_offsets.items() if o == off and r != '$28')
        gb_count = sum(v for (r, o), v in gb_offsets.items() if o == off and r != '$28')

        if ba_count > 0 and gb_count > 0:
            status = "SHARED"
            shared_offsets.append(off)
        elif ba_count > 0:
            status = "boyAI only"
            boyai_only.append(off)
        else:
            status = "GirlBrain only"
            girlbrain_only.append(off)

        # Add known annotation
        note = KNOWN_GP_OFFSETS.get(off, "")

        print(f"  0x{off:04X}   {ba_count:>8}   {gb_count:>12}   {status:<25} {note}")

    print()
    print(f"  SHARED: {len(shared_offsets)} offsets")
    print(f"  boyAI only: {len(boyai_only)} offsets")
    print(f"  GirlBrain only: {len(girlbrain_only)} offsets")
    print()

    # ══════════════════════════════════════════════════════════════════════
    # TABLE 2: GP-relative accesses
    # ══════════════════════════════════════════════════════════════════════
    print("=" * 90)
    print("TABLE 2: GP-Relative Accesses (offset($28))")
    print("=" * 90)
    print()
    print(f"{'GP Offset':>12} {'BoyAI cnt':>10} {'GirlBrain cnt':>14} {'Status':<20} {'Known label'}")
    print("-" * 90)

    all_gp = set(boyai_gp.keys()) | set(gb_gp.keys())
    for off in sorted(all_gp):
        ba = boyai_gp.get(off, 0)
        gb = gb_gp.get(off, 0)
        if ba > 0 and gb > 0:
            status = "SHARED"
        elif ba > 0:
            status = "boyAI only"
        else:
            status = "GirlBrain only"
        label = KNOWN_GP_OFFSETS.get(off, "")
        # Sign-extend 16-bit immediate to show real GP offset
        signed_off = off if off < 0x8000 else off - 0x10000
        print(f"  gp{signed_off:+#08x}  {ba:>8}   {gb:>12}   {status:<20} {label}")

    print()

    # Cross-check with ELF disasm
    print("  [ELF disasm cross-check]")
    all_gp_elf = set(elf_ba_gp.keys()) | set(elf_gb_gp.keys())
    for off in sorted(all_gp_elf):
        ba = elf_ba_gp.get(off, 0)
        gb = elf_gb_gp.get(off, 0)
        label = KNOWN_GP_OFFSETS.get(off, "")
        if ba + gb > 0:
            print(f"    GP 0x{off & 0xFFFF:04X}  ELF boyAI={ba:>4}  ELF GirlBrain={gb:>4}  {label}")

    print()

    # ══════════════════════════════════════════════════════════════════════
    # TABLE 3: JAL target comparison
    # ══════════════════════════════════════════════════════════════════════
    print("=" * 90)
    print("TABLE 3: JAL/J Target Comparison (shared utility functions)")
    print("=" * 90)
    print()

    all_jal = set(boyai_jal.keys()) | set(gb_jal.keys())
    shared_jal = []
    for addr in sorted(all_jal):
        ba = boyai_jal.get(addr, 0)
        gb = gb_jal.get(addr, 0)
        if ba > 0 and gb > 0:
            shared_jal.append((addr, ba, gb))

    print(f"  Shared call targets (both boyAI and GirlBrain): {len(shared_jal)}")
    print()
    print(f"  {'Target':>12} {'BoyAI calls':>12} {'GirlBrain calls':>16} {'Region':<20}")
    print("  " + "-" * 65)
    for addr, ba, gb in shared_jal:
        if 0x100000 <= addr < 0x110000:
            region = "ios/engine core"
        elif 0x130000 <= addr < 0x150000:
            region = "isysGObj/entity"
        elif 0x150000 <= addr < 0x16A000:
            region = "boyAI domain"
        elif 0x16A000 <= addr < 0x175000:
            region = "GirlBrain domain"
        elif 0x190000 <= addr < 0x1A0000:
            region = "eBrain/Generator"
        elif 0x1A0000 <= addr < 0x1B0000:
            region = "camera/render"
        elif 0x1B0000 <= addr < 0x1C0000:
            region = "scene/kanban"
        elif 0x194000 <= addr < 0x195000:
            region = "geometry utils"
        elif 0x240000 <= addr < 0x260000:
            region = "math/collision"
        else:
            region = f"0x{addr >> 12:03X}xxx"
        print(f"  0x{addr:08X}  {ba:>10}   {gb:>14}   {region}")

    print()

    # girlBrain-only calls
    gb_only_jal = [(a, gb_jal[a]) for a in sorted(gb_jal) if gb_jal[a] > 0 and boyai_jal.get(a, 0) == 0]
    print(f"  GirlBrain-only call targets: {len(gb_only_jal)}")
    print(f"  {'Target':>12} {'Calls':>8}")
    print("  " + "-" * 25)
    for addr, cnt in gb_only_jal[:25]:
        print(f"  0x{addr:08X}  {cnt:>8}")
    if len(gb_only_jal) > 25:
        print(f"  ... and {len(gb_only_jal) - 25} more")
    print()

    # boyAI-only calls
    ba_only_jal = [(a, boyai_jal[a]) for a in sorted(boyai_jal) if boyai_jal[a] > 0 and gb_jal.get(a, 0) == 0]
    print(f"  boyAI-only call targets: {len(ba_only_jal)}")
    print(f"  {'Target':>12} {'Calls':>8}")
    print("  " + "-" * 25)
    for addr, cnt in ba_only_jal[:25]:
        print(f"  0x{addr:08X}  {cnt:>8}")
    if len(ba_only_jal) > 25:
        print(f"  ... and {len(ba_only_jal) - 25} more")
    print()

    # ══════════════════════════════════════════════════════════════════════
    # TABLE 4: High-frequency struct offsets (top 20 each)
    # ══════════════════════════════════════════════════════════════════════
    print("=" * 90)
    print("TABLE 4: Top 20 Most-Accessed Offsets per system")
    print("=" * 90)
    print()

    # boyAI top offsets
    boyai_top = sorted(
        [(off, cnt) for (reg, off), cnt in boyai_offsets.items() if reg != '$28'],
        key=lambda x: -x[1]
    )[:20]

    print("  boyAI top offsets:")
    print(f"  {'Offset':>10} {'Count':>8} {'Primary regs'}")
    print("  " + "-" * 45)
    for off, cnt in boyai_top:
        regs = set()
        for (r, o), c in boyai_offsets.items():
            if o == off and r != '$28':
                regs.add(r)
        print(f"  0x{off:04X}   {cnt:>6}   {', '.join(sorted(regs)[:4])}")

    print()

    # GirlBrain top offsets
    gb_top = sorted(
        [(off, cnt) for (reg, off), cnt in gb_offsets.items() if reg != '$28'],
        key=lambda x: -x[1]
    )[:20]

    print("  GirlBrain top offsets:")
    print(f"  {'Offset':>10} {'Count':>8} {'Primary regs'}")
    print("  " + "-" * 45)
    for off, cnt in gb_top:
        regs = set()
        for (r, o), c in gb_offsets.items():
            if o == off and r != '$28':
                regs.add(r)
        print(f"  0x{off:04X}   {cnt:>6}   {', '.join(sorted(regs)[:4])}")

    print()

    # ══════════════════════════════════════════════════════════════════════
    # TABLE 5: Distinct struct layout inference
    # ══════════════════════════════════════════════════════════════════════
    print("=" * 90)
    print("TABLE 5: Struct Layout Inference — shared vs distinct fields")
    print("=" * 90)
    print()
    print("  Offset ranges and likely struct roles:")
    print()

    # Group offsets by likely struct region
    regions = {
        "0x000-0x03F": (0x000, 0x03F),
        "0x040-0x0FF": (0x040, 0x0FF),
        "0x100-0x17F": (0x100, 0x17F),
        "0x180-0x1FF": (0x180, 0x1FF),
        "0x200-0x3FF": (0x200, 0x3FF),
        "0x400-0x5FF": (0x400, 0x5FF),
        "0x600-0x7FF": (0x600, 0x7FF),
        "0x800-0xFFF": (0x800, 0xFFF),
    }

    for label, (lo, hi) in regions.items():
        ba_in = sum(v for (r, o), v in boyai_offsets.items() if lo <= o <= hi and r != '$28')
        gb_in = sum(v for (r, o), v in gb_offsets.items() if lo <= o <= hi and r != '$28')
        ba_unique = len(set(o for (r, o) in boyai_offsets if lo <= o <= hi and r != '$28'))
        gb_unique = len(set(o for (r, o) in gb_offsets if lo <= o <= hi and r != '$28'))

        if ba_in + gb_in == 0:
            continue

        shared_count = len(set(
            o for (r, o) in boyai_offsets if lo <= o <= hi and r != '$28'
        ) & set(
            o for (r, o) in gb_offsets if lo <= o <= hi and r != '$28'
        ))

        print(f"  [{label}]  boyAI: {ba_in:>4} accesses/{ba_unique:>2} unique  |  "
              f"GirlBrain: {gb_in:>4} accesses/{gb_unique:>2} unique  |  "
              f"shared offsets: {shared_count}")

    print()

    # ── Summary ──
    print("=" * 90)
    print("ANALYSIS SUMMARY")
    print("=" * 90)
    print()
    total_ba = sum(v for (r, o), v in boyai_offsets.items() if r != '$28')
    total_gb = sum(v for (r, o), v in gb_offsets.items() if r != '$28')
    shared_total = len(shared_offsets)
    print(f"  boyAI: {len(boyai_files)} .s files, {total_ba} total struct accesses, "
          f"{len(set(o for (r,o) in boyai_offsets if r != '$28'))} unique offsets")
    print(f"  GirlBrain: {len(gb_files)} .s files, {total_gb} total struct accesses, "
          f"{len(set(o for (r,o) in gb_offsets if r != '$28'))} unique offsets")
    print(f"  Shared offsets: {shared_total}")
    print(f"  boyAI-only offsets: {len(boyai_only)}")
    print(f"  GirlBrain-only offsets: {len(girlbrain_only)}")
    print()
    print(f"  Shared GP offsets: ", end="")
    shared_gp = [o for o in all_gp if boyai_gp.get(o, 0) > 0 and gb_gp.get(o, 0) > 0]
    print(", ".join(f"0x{o & 0xFFFF:04X}" for o in sorted(shared_gp)))
    print(f"  Shared JAL targets: {len(shared_jal)}")
    print()

    if shared_total > 10:
        print("  CONCLUSION: boyAI and GirlBrain share significant struct overlap.")
        print("  They likely operate on the same entity work area structure,")
        print("  but each has domain-specific offset accesses.")
    elif shared_total > 0:
        print("  CONCLUSION: Limited shared offsets suggest partial struct overlap.")
        print("  The two systems may use different sub-structures or different")
        print("  base registers pointing to different parts of the entity area.")
    else:
        print("  CONCLUSION: No shared struct offsets found.")
        print("  boyAI and GirlBrain likely use different structures.")

    if shared_gp:
        print(f"\n  Both use GP-relative globals: {', '.join(KNOWN_GP_OFFSETS.get(o, f'0x{o & 0xFFFF:04X}') for o in sorted(shared_gp))}")
        print("  This confirms they share the same GP-based context (process/thread state).")


if __name__ == '__main__':
    main()
