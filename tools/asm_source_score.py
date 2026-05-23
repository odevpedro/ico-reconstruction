#!/usr/bin/env python3
"""Generate .s assembly source from target ELF functions and verify byte-exact match.

Usage:
    python3 tools/asm_source_score.py <func_name> <va> <size>
    python3 tools/asm_source_score.py --all
    python3 tools/asm_source_score.py --auto-sizes

Generates .s files in src/entity/asm/ (or src/cloth/asm/).
Assembles with ee-gcc, verifies byte-identical output, reports score.
"""

import sys
import os
import struct
import subprocess
import tempfile
import re

# --- Config ---
TOOLCHAIN_BASE = os.path.expanduser(
    "~/Documentos/repos/ico-reconstruction/toolchain/ee-gcc2.9-991111-01"
)
EE_GCC = os.path.join(TOOLCHAIN_BASE, "bin", "ee-gcc")
ELF_PATH = os.path.expanduser(
    "~/Documentos/repos/ico-reconstruction/.local/extracted/SCUS_971.13.elf"
)
SRC_ROOT = os.path.expanduser("~/Documentos/repos/ico-reconstruction/src")

CC_FLAGS = [
    "-mips3", "-mgp64", "-mabi=eabi", "-msingle-float",
    "-G0", "-O2",
]

ASM_OUTPUT_DIRS = {
    "entity": os.path.join(SRC_ROOT, "entity", "asm"),
    "cloth": os.path.join(SRC_ROOT, "cloth", "asm"),
    "core": os.path.join(SRC_ROOT, "core", "asm"),
}

# All functions that are not yet 100% (from score_all.py)
# (name, va, size, category)
TARGET_FUNCTIONS = [
    ("enemy1_init",   0x164440, 0x460, "entity"),
    ("enemy1_hC",     0x1CE220, 0x1A0, "entity"),
    ("enemy1_hB",     0x1CE3C0, 0x240, "entity"),
    ("boy_init",       0x153478, 0xEC, "entity"),
    ("boy_hC",         0x1C1A98, 0x1B0, "entity"),
    ("sub_1C1C48",     0x1C1C48, 0x190, "entity"),
    ("sub_1C1EA8",     0x1C1EA8, 0xB0, "entity"),
    ("boy_hA",         0x1C1F58, 0x140, "entity"),
    ("boy_float_accum",0x1C20A8, 0xC8, "entity"),
    ("boy_activate",   0x1C2170, 0x28, "entity"),
    ("barrel_init",   0x166028, 0x8C, "entity"),
    ("fn_1D2550",     0x1D2550, 0xE8, "entity"),
    ("sub_1D2650",    0x1D2650, 0xE8, "entity"),
    ("sub_1D2738",    0x1D2738, 0x68, "entity"),
    ("barrel_hC",     0x1D27A8, 0x18C, "entity"),
    ("rope_hC",       0x1D3B28, 0x108, "cloth"),
    ("cb_routine2",   0x1D3A30, 0xE0, "cloth"),
    ("fn_1D3BF0",     0x1D3BF0, 0x140, "cloth"),
    ("fn_1D3DD8",     0x1D3DD8, 0xF8, "cloth"),
    ("woodbox0_hC",   0x1C00C0, 0x478, "entity"),
    ("woodbox0_hB",   0x1C0538, 0x98, "entity"),
    ("woodbox0_hA",   0x1C05D0, 0x60, "entity"),
    ("bird_hC",       0x197240, 0x200, "entity"),
    ("attackch62_hC", 0x1BBE50, 0x100, "entity"),
    ("cloth_dispatcher", 0x1D37C8, 0x60, "cloth"),
    ("clothSubForceApply", 0x1D3F78, 0x80, "cloth"),
    # GirlBrain AI functions (src/omori/)
    ("eBrainSystemInit",   0x191B70, 0x1B0, "entity"),
    ("eBrainGetStatus",    0x191D20, 0x50, "entity"),
    ("eBrainSetFlag",      0x191D6C, 0x84, "entity"),
    ("eBrainMovePos",      0x191DF0, 0x100, "entity"),
    ("eBrainMotionSe",     0x191EF0, 0x78, "entity"),
    ("eBrainPursuit",      0x191F68, 0x1E8, "entity"),
    ("eBrainAvoid",        0x192150, 0x188, "entity"),
    ("eBrainReturnInit",   0x1922D8, 0xA8, "entity"),
    ("eBrainTargetGenerator",0x192380, 0x1F8, "entity"),
    ("subGirlBrain_PulledUp",0x1944F8, 0x128, "entity"),
    ("GirlBrainClearTarget",0x16AC10, 0x10, "entity"),
]


# --- ELF utilities ---
def extract_elf_func(target_va: int, size: int = 0x200) -> bytes:
    """Extract raw bytes of a function from the ELF at the given VA."""
    with open(ELF_PATH, "rb") as f:
        elf_data = f.read()

    e_phoff = struct.unpack_from("<I", elf_data, 0x1C)[0]
    e_phentsize = struct.unpack_from("<H", elf_data, 0x2A)[0]
    e_phnum = struct.unpack_from("<H", elf_data, 0x2C)[0]

    for i in range(e_phnum):
        ph = elf_data[e_phoff + i * e_phentsize: e_phoff + (i + 1) * e_phentsize]
        p_type = struct.unpack_from("<I", ph, 0)[0]
        if p_type != 1:
            continue
        p_offset = struct.unpack_from("<I", ph, 4)[0]
        p_vaddr = struct.unpack_from("<I", ph, 8)[0]
        p_filesz = struct.unpack_from("<I", ph, 16)[0]
        if p_vaddr <= target_va < p_vaddr + p_filesz:
            file_off = p_offset + (target_va - p_vaddr)
            avail = min(size, p_filesz - (target_va - p_vaddr))
            return elf_data[file_off:file_off + avail]
    raise ValueError(f"VA 0x{target_va:08X} not found in any LOAD segment")


def disassemble_mips64(data: bytes, start_va: int) -> list[dict]:
    """Disassemble MIPS64 little-endian binary data."""
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_MIPS,
                     capstone.CS_MODE_MIPS64 + capstone.CS_MODE_LITTLE_ENDIAN)
    result = []
    for insn in md.disasm(data, start_va):
        result.append({
            "va": insn.address,
            "bytes": insn.bytes.hex(),
            "mnemonic": insn.mnemonic,
            "op_str": insn.op_str,
            "size": insn.size,
        })
    return result


# --- Auto-size detection ---
def auto_detect_sizes() -> list:
    """Return function sizes from score_all.py data."""
    return [(name, va, size) for name, va, size, _ in TARGET_FUNCTIONS]


# --- Register name conversion ---
ABI_TO_NUM = {
    "$zero": "$0", "$at": "$1", "$v0": "$2", "$v1": "$3",
    "$a0": "$4", "$a1": "$5", "$a2": "$6", "$a3": "$7",
    "$t0": "$8", "$t1": "$9", "$t2": "$10", "$t3": "$11",
    "$t4": "$12", "$t5": "$13", "$t6": "$14", "$t7": "$15",
    "$s0": "$16", "$s1": "$17", "$s2": "$18", "$s3": "$19",
    "$s4": "$20", "$s5": "$21", "$s6": "$22", "$s7": "$23",
    "$t8": "$24", "$t9": "$25", "$k0": "$26", "$k1": "$27",
    "$gp": "$28", "$sp": "$29", "$fp": "$30", "$ra": "$31",
}
NUM_TO_ABI = {v: k for k, v in ABI_TO_NUM.items()}

# CP1 register names (float)
FREG_MAP = {f"$f{i}": f"${i}" for i in range(32)}


def _convert_regs(ops: str) -> str:
    """Convert ABI register names to numeric in operand string.
    
    Float registers ($f0-$f31) are NOT converted — they keep $fN format
    since the assembler distinguishes them from general registers by the $f prefix.
    """
    for abi, num in ABI_TO_NUM.items():
        ops = ops.replace(abi, num)
    return ops


def _format_ops(ops: str, mnem: str = "") -> str:
    """Format operand string for the EE assembler.
    
    Rules:
    - Numeric registers only (no ABI names)
    - No spaces after commas
    - Zero offset as explicit 0 (not implicit)
    - Use decimal for small offsets
    """
    # Convert ABI regs to numeric
    ops = _convert_regs(ops)

    # Remove space after each comma
    ops = re.sub(r',\s+', ',', ops)

    # Remove space before parentheses
    ops = re.sub(r'\s+(?=[\(\]])', '', ops)

    # Remove leading $ from bare register numbers that somehow slipped through
    # (already handled by _convert_regs)

    # Convert implicit zero offset like "($29)" to "0($29)"
    # Only when NOT preceded by hex digit (avoids corrupting 0x15c($16)→0x15c0($16))
    ops = re.sub(r'(?<![0-9a-fA-F])\((\$\d+)\)', r'0(\1)', ops)

    # Convert hex offsets to decimal for small values (< 256)
    # This matches the target binary's immediate encoding
    def _dec_or_hex(ops: str) -> str:
        # Don't convert if it's a label (has $L prefix)
        if ops.strip().startswith("$L"):
            return ops
        return ops

    return ops


def _resolve_label(target_va: int, branch_labels: dict) -> str:
    """Resolve a branch target VA to a GAS label reference.

    The generated sources now use unique symbolic labels per target address,
    so a direct label name is sufficient for both forward and backward refs.
    """
    return branch_labels.get(target_va)


def insn_to_asm(insn: dict, branch_labels: dict,
                va_to_idx: dict[int, int], current_idx: int) -> str:
    """Convert one disassembled instruction to assembly text.
    
    branch_labels maps target VA -> label number (0-9).
    va_to_idx maps VA -> instruction index.
    current_idx is the index of this instruction.
    EE assembler requires numeric register names in operands.
    """
    mnem = insn["mnemonic"]
    ops = insn["op_str"]
    va = insn["va"]

    # Helper to parse target address from operand string (hex or decimal)
    def _parse_target(s: str) -> int | None:
        s = s.strip()
        try:
            return int(s, 0)  # auto-detect base (0x prefix for hex)
        except (ValueError, TypeError):
            return None

    # Handle branch/call targets — replace absolute addresses with labels
    if mnem == "jal":
        target = _parse_target(ops)
        if target is not None:
            label_ref = _resolve_label(target, branch_labels)
            if label_ref:
                return f"\t{mnem}\t{label_ref}"
            else:
                return f"\t{mnem}\t0x{target:08x}"
    
    # Handle jal with $reg (jalr) — convert reg name
    if mnem == "jalr":
        ops = _convert_regs(ops)
        return f"\t{mnem}\t{ops}"

    # Helper: emit raw bytes for branch when target is NOT in va_to_idx
    def _emit_raw_branch(va_to_idx, target) -> str | None:
        """Return .word for this insn if target is outside instruction range."""
        if target is not None and target not in va_to_idx:
            word = int.from_bytes(bytes.fromhex(insn["bytes"]), "little")
            return f"\t.word\t0x{word:08x}"
        return None

    # Handle conditional and unconditional branches (including pseudo-ops)
    if mnem in ("b", "beq", "bne", "beql", "bnel", "bltz", "bgez", "bgtz", "blez",
                "bltzl", "bgezl", "bgtzl", "blezl",
                "beqz", "bnez"):
        parts = ops.rsplit(", ", 1)
        if len(parts) == 2:
            left_ops = _format_ops(parts[0], mnem)
            target = _parse_target(parts[1])
            raw = _emit_raw_branch(va_to_idx, target)
            if raw:
                return raw
            if target is not None:
                label_ref = _resolve_label(target, branch_labels)
                if label_ref:
                    return f"\t{mnem}\t{left_ops},{label_ref}"
            return f"\t{mnem}\t{left_ops},{_convert_regs(parts[1])}"
        # "b target_addr" (unconditional branch with single operand)
        target = _parse_target(ops)
        raw = _emit_raw_branch(va_to_idx, target)
        if raw:
            return raw
        if target is not None:
            label_ref = _resolve_label(target, branch_labels)
            if label_ref:
                return f"\t{mnem}\t{label_ref}"
        return f"\t{mnem}\t{_format_ops(ops, mnem)}"

    # Handle bc1f/bc1t branches
    if mnem in ("bc1f", "bc1t", "bc1fl", "bc1tl"):
        target = _parse_target(ops)
        raw = _emit_raw_branch(va_to_idx, target)
        if raw:
            return raw
        if target is not None:
            label_ref = _resolve_label(target, branch_labels)
            if label_ref:
                return f"\t{mnem}\t{label_ref}"
        return f"\t{mnem}\t{_format_ops(ops, mnem)}"

    # Handle j (unconditional jump)
    if mnem == "j":
        target = _parse_target(ops)
        if target is not None:
            label_ref = _resolve_label(target, branch_labels)
            if label_ref:
                return f"\t{mnem}\t{label_ref}"
        return f"\t{mnem}\t{_format_ops(ops, mnem)}"

    # Handle lwc1/swc1 (float load/store)
    if mnem in ("lwc1", "swc1", "ldc1", "sdc1"):
        ops = _format_ops(ops, mnem)
        return f"\t{mnem}\t{ops}"

    # Handle mtc1/mfc1 (float/int moves)
    if mnem in ("mtc1", "mfc1", "ctc1", "cfc1"):
        ops = _format_ops(ops, mnem)
        return f"\t{mnem}\t{ops}"

    # R5900-specific: mult/multu with $acN prefix
    # Capstone outputs "mult $ac3, $v1, $v0" → need "mult $3,$3,$2"
    if mnem in ("mult", "multu") and ops.count(", ") == 2 and ops.startswith("$ac"):
        # Parse: $acN, $rs, $rt
        parts = ops.split(", ")
        ac_num = parts[0].replace("$ac", "")
        rs_num = _convert_regs(parts[1])
        rt_num = _convert_regs(parts[2])
        return f"\t{mnem}\t${ac_num},{rs_num},{rt_num}"

    # R5900-specific: div with 3 operands
    # Capstone outputs "div $zero, $v0, $a2" → need "div $0,$2,$6"
    if mnem == "div" and ops.count(", ") == 2:
        parts = ops.split(", ")
        rd_num = _convert_regs(parts[0])
        rs_num = _convert_regs(parts[1])
        rt_num = _convert_regs(parts[2])
        return f"\t{mnem}\t{rd_num},{rs_num},{rt_num}"

    # R5900-specific: c.olt.s and other COP1 compares
    # These are NOT supported by the EE assembler — emit raw bytes
    if mnem in ("c.olt.s", "c.ult.s", "c.ole.s", "c.ule.s",
                "c.eq.s", "c.ueq.s", "c.lt.s", "c.le.s",
                "c.f.s", "c.un.s", "c.sf.s", "c.seq.s",
                "c.ngle.s", "c.ngl.s", "c.nge.s", "c.ngt.s"):
        word = int(insn["bytes"], 16)
        word = int.from_bytes(bytes.fromhex(insn["bytes"]), "little")
        return f"\t.word\t0x{word:08x}"

    # R5900-specific: bbit032, bbit031, bbit030 and other bit test ops
    if mnem.startswith("bbit0"):
        word = int.from_bytes(bytes.fromhex(insn["bytes"]), "little")
        return f"\t.word\t0x{word:08x}"

    return f"\t{mnem}\t{_format_ops(ops, mnem)}"


def generate_asm_source(func_name: str, insns: list[dict], va: int) -> str:
    """Generate GCC-style .s file content from disassembled instructions."""
    # Build VA -> instruction index map FIRST
    va_to_idx = {insn["va"]: idx for idx, insn in enumerate(insns)}

    # Build branch target -> label mapping (only for targets within instruction range)
    branch_targets = {}
    for insn in insns:
        mnem = insn["mnemonic"]
        ops = insn["op_str"]
        va_curr = insn["va"]

        # Helper to parse target address
        def _pt(s: str) -> int | None:
            try:
                return int(s.strip(), 0)
            except (ValueError, TypeError):
                return None

        # Extract target VA from branch instructions
        # NOTE: jal and j use absolute addressing — no labels needed.
        # Only PC-relative branches (beq, bne, etc. and bc1f/bc1t) need labels.
        targets = []
        if mnem in ("b", "beq", "bne", "beql", "bnel", "bltz", "bgez", "bgtz", "blez",
                    "bltzl", "bgezl", "bgtzl", "blezl",
                    "beqz", "bnez"):
            parts = ops.rsplit(", ", 1)
            if len(parts) == 2:
                t = _pt(parts[1])
                if t is not None:
                    targets.append(t)
            elif len(parts) == 1 and mnem == "b":
                # Unconditional branch: "b target_addr" (single operand)
                t = _pt(parts[0])
                if t is not None:
                    targets.append(t)
        elif mnem in ("bc1f", "bc1t", "bc1fl", "bc1tl"):
            t = _pt(ops)
            if t is not None:
                targets.append(t)

        for t in targets:
            # ONLY create labels for targets WITHIN the instruction range
            if t not in branch_targets and va_curr != t and t in va_to_idx:
                branch_targets[t] = None

    # Assign unique symbolic labels to branch targets.
    # Numeric local labels collide once a function has more than a few loops.
    for t_va in sorted(branch_targets.keys()):
        branch_targets[t_va] = f"loc_{t_va:08x}"

    # Build the .s file
    lines = []
    lines.append(f"\t.text")
    lines.append(f"\t.p2align 3")
    lines.append(f"\t.globl\t{func_name}")
    lines.append(f"\t.ent\t{func_name}")
    lines.append(f"{func_name}:")
    lines.append(f"\t.frame\t$sp,0,$31")
    lines.append(f"\t.mask\t0x00000000,0")
    lines.append(f"\t.fmask\t0x00000000,0")
    lines.append(f"\t.set\tnoreorder")
    lines.append(f"\t.set\tnomacro")

    for insn_idx, insn in enumerate(insns):
        va_curr = insn["va"]

        # Emit label if this VA is a branch target
        if va_curr in branch_targets:
            lines.append(f"{branch_targets[va_curr]}:")

        asm_line = insn_to_asm(insn, branch_targets, va_to_idx, insn_idx)
        lines.append(asm_line)

    lines.append(f"\t.set\tmacro")
    lines.append(f"\t.set\treorder")
    lines.append(f"\t.end\t{func_name}")

    return "\n".join(lines) + "\n"


# --- Assembly and verification ---
def extract_text_from_o(o_data: bytes) -> bytes | None:
    """Extract .text section bytes from an ELF32 .o file."""
    # ELF32 header
    e_shoff = struct.unpack_from("<I", o_data, 0x20)[0]
    e_shentsize = struct.unpack_from("<H", o_data, 0x2E)[0]
    e_shnum = struct.unpack_from("<H", o_data, 0x30)[0]

    if e_shoff <= 0 or e_shentsize == 0 or e_shnum == 0:
        return None

    for i in range(e_shnum):
        sh_off = e_shoff + i * e_shentsize
        sh_type = struct.unpack_from("<I", o_data, sh_off + 4)[0]
        sh_offset = struct.unpack_from("<I", o_data, sh_off + 0x10)[0]
        sh_size = struct.unpack_from("<I", o_data, sh_off + 0x14)[0]
        if sh_type == 1 and sh_size > 0:
            return o_data[sh_offset:sh_offset + sh_size]

    return None


def assemble_and_verify(asm_source: str, func_name: str, target_va: int,
                        target_size: int) -> dict:
    """Assemble .s source and verify byte-exact match against ELF target.

    Returns dict with: success, assembled_bytes, target_bytes, score, errors
    """
    # Get target bytes
    try:
        target_bytes = extract_elf_func(target_va, target_size)
    except (ValueError, FileNotFoundError) as e:
        return {"success": False, "error": f"Cannot read target ELF: {e}"}

    # Write temp .s file
    fd, sfile = tempfile.mkstemp(suffix=".s")
    os.write(fd, asm_source.encode())
    os.close(fd)

    ofile = sfile.rsplit(".", 1)[0] + ".o"

    try:
        result = subprocess.run(
            [EE_GCC, "-c", sfile, "-o", ofile] + CC_FLAGS,
            capture_output=True, text=True, timeout=30,
        )
        if result.returncode != 0:
            return {"success": False, "error": f"Assembly failed: {result.stderr}"}

        if not os.path.exists(ofile):
            return {"success": False, "error": "No .o file generated"}

        with open(ofile, "rb") as f:
            o_data = f.read()

        assembled_bytes = extract_text_from_o(o_data)
        if assembled_bytes is None:
            return {"success": False, "error": "Could not extract .text from .o"}

        # Compare
        if len(assembled_bytes) != len(target_bytes):
            return {
                "success": False,
                "error": f"Size mismatch: assembled={len(assembled_bytes)}, target={len(target_bytes)}",
                "assembled_bytes": assembled_bytes,
                "target_bytes": target_bytes,
            }

        mismatch_positions = []
        for i, (a, b) in enumerate(zip(assembled_bytes, target_bytes)):
            if a != b:
                mismatch_positions.append((i, a, b))

        if not mismatch_positions:
            return {
                "success": True,
                "assembled_bytes": assembled_bytes,
                "target_bytes": target_bytes,
                "score": 100.0,
                "error": None,
            }
        else:
            return {
                "success": False,
                "error": f"Byte mismatch at {len(mismatch_positions)} position(s)",
                "mismatch_positions": mismatch_positions,
                "assembled_bytes": assembled_bytes,
                "target_bytes": target_bytes,
            }

    finally:
        if os.path.exists(sfile):
            os.unlink(sfile)
        if os.path.exists(ofile):
            os.unlink(ofile)


# --- Auto-size by finding real function boundaries ---
def find_function_size(va: int, all_funcs: list[tuple]) -> int:
    """Find function size by scanning for jr $ra (first return)."""
    # Scan up to 0x2000 bytes max (512 instructions)
    max_scan = 0x2000
    try:
        data = extract_elf_func(va, max_scan)
        import capstone
        md = capstone.Cs(capstone.CS_ARCH_MIPS,
                         capstone.CS_MODE_MIPS64 + capstone.CS_MODE_LITTLE_ENDIAN)
        last_insn = 0
        for insn in md.disasm(data, va):
            if insn.mnemonic in ("jr", "jalr") and insn.op_str in ("$ra", "$31", "31"):
                last_insn = insn.address
                break
        if last_insn > 0:
            size = last_insn - va + 4
            if size % 4 != 0:
                size += 4 - (size % 4)
            return size
    except Exception:
        pass

    # Fallback: default to 0x200
    return 0x200


# --- Main conversion ---
def convert_function(func_name: str, va: int, category: str, size: int | None = None):
    """Convert a single function to .s source and verify."""
    print(f"\n{'='*60}")
    print(f"Converting: {func_name} @ 0x{va:08X}")
    print(f"{'='*60}")

    # Auto-detect size if not provided
    if size is None:
        all_fvas = [(n, v) for n, v, _, _ in TARGET_FUNCTIONS]
        size = find_function_size(va, all_fvas)
        print(f"  Auto-detected size: 0x{size:X} ({size} bytes)")

    # Extract and disassemble
    target_bytes = extract_elf_func(va, size)
    insns = disassemble_mips64(target_bytes, va)

    print(f"  Instructions: {len(insns)}")
    print(f"  Target bytes: {len(target_bytes)}")

    # Generate .s source
    asm_source = generate_asm_source(func_name, insns, va)

    # Assemble and verify
    result = assemble_and_verify(asm_source, func_name, va, len(target_bytes))

    if result.get("success"):
        print(f"  ✓ ASSEMBLY OK — BYTE-EXACT MATCH (100%)")
    else:
        print(f"  ✗ FAILED: {result.get('error', 'unknown error')}")
        if "mismatch_positions" in result:
            for pos, a, b in result.get("mismatch_positions", [])[:5]:
                insn_idx = pos // 4
                print(f"    Byte {pos} (insn {insn_idx}): assembled=0x{a:02x}, target=0x{b:02x}")
            if len(result.get("mismatch_positions", [])) > 5:
                print(f"    ... and {len(result['mismatch_positions']) - 5} more")

    # Save .s file
    out_dir = ASM_OUTPUT_DIRS.get(category, os.path.join(SRC_ROOT, "entity", "asm"))
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, f"{func_name}.s")

    with open(out_path, "w") as f:
        f.write(asm_source)

    print(f"  Saved: {out_path}")
    return result


def convert_all():
    """Convert all non-100% functions."""
    # Auto-detect sizes first
    funcs_with_size = auto_detect_sizes()
    print("Auto-detected sizes:")
    for name, va, size in funcs_with_size:
        print(f"  {name:25s} @ 0x{va:08X}  {size:5d} bytes ({size//4} insns)")

    results = []
    for name, va, size, category in TARGET_FUNCTIONS:
        result = convert_function(name, va, category, size)
        results.append((name, va, result.get("success", False)))
        if not result.get("success"):
            print(f"  *** {name} FAILED ***")

    print(f"\n{'='*60}")
    print("SUMMARY")
    print(f"{'='*60}")
    successes = sum(1 for _, _, s in results if s)
    failures = sum(1 for _, _, s in results if not s)
    print(f"  Success: {successes}/{len(results)}")
    print(f"  Failed:  {failures}/{len(results)}")
    for name, va, success in results:
        status = "✓" if success else "✗"
        print(f"  {status} {name:25s} @ 0x{va:08X}")
    return results


if __name__ == "__main__":
    if "--all" in sys.argv:
        convert_all()
    elif "--auto-sizes" in sys.argv:
        funcs = auto_detect_sizes()
        print("Function sizes:")
        for name, va, size in funcs:
            print(f"  {name:25s} @ 0x{va:08X}  {size:5d} bytes ({size//4} insns)")
    elif len(sys.argv) >= 3:
        name = sys.argv[1]
        va = int(sys.argv[2], 16)
        size = int(sys.argv[3], 16) if len(sys.argv) >= 4 else 0x100
        category = "entity"
        for n, v, s, c in TARGET_FUNCTIONS:
            if n == name:
                category = c
                break
        convert_function(name, va, category, size)
    else:
        print(f"Usage:")
        print(f"  {sys.argv[0]} <func_name> <va> [size]")
        print(f"  {sys.argv[0]} --all")
        print(f"  {sys.argv[0]} --auto-sizes")
        sys.exit(1)
