#!/usr/bin/env python3
"""Local ee-gcc 2.9 compilation + scoring pipeline.

Compiles a C function with ee-gcc 2.9-991111-01 (MIPS-III, EABI, -O2),
disassembles it, and scores against the target ELF function.

Usage:
    python3 tools/ee_gcc_compile.py path/to/file.c --fn func_name --va 0xADDRESS
"""
import argparse
import subprocess
import struct
import sys
import os
import tempfile
import re

import capstone

# === Configuration ===
SRC_ROOT = os.path.expanduser(
    "~/Documentos/repos/ico-reconstruction/src"
)
TOOLCHAIN = os.path.expanduser(
    "~/Documentos/repos/ico-reconstruction/toolchain/ee-gcc2.9-991111-01/bin/ee-gcc"
)
ELF_PATH = os.path.expanduser(
    "/home/peter/Documentos/repos/ico-reconstruction/.local/extracted/SCUS_971.13.elf"
)
CC_FLAGS = [
    "-mips3", "-mgp64", "-mabi=eabi", "-msingle-float",
    "-G0", "-O2",
] + [f"-I{SRC_ROOT}", f"-I{SRC_ROOT}/entity", f"-I{SRC_ROOT}/cloth"]

# === Helpers ===

def disassemble_mips64(data: bytes, start_va: int) -> list[dict]:
    """Disassemble MIPS64 little-endian binary data."""
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


def compile_c_to_asm(c_code: str) -> str | None:
    """Compile C code with ee-gcc 2.9 and return assembly text."""
    fd, infile = tempfile.mkstemp(suffix=".c")
    os.write(fd, c_code.encode())
    os.close(fd)

    sfile = infile.rsplit(".", 1)[0] + ".s"

    try:
        result = subprocess.run(
            [TOOLCHAIN] + CC_FLAGS + ["-S", infile, "-o", sfile],
            capture_output=True, text=True, timeout=30,
        )
        if result.returncode != 0:
            print("COMPILE ERROR:", result.stderr, file=sys.stderr)
            return None

        if os.path.exists(sfile):
            with open(sfile) as f:
                return f.read()
        return result.stdout
    finally:
        os.unlink(infile)
        if os.path.exists(sfile):
            os.unlink(sfile)


def extract_func_asm(asm_text: str, func_name: str) -> str:
    """Extract assembly for a specific function from GCC output."""
    lines = []
    in_func = False
    for line in asm_text.splitlines():
        stripped = line.strip()
        # Detect function entry
        if stripped.startswith(func_name + ":"):
            in_func = True
            continue
        if in_func:
            # Next public label or function ends it
            if (stripped.startswith("\t.globl") and not stripped.endswith(func_name)) \
               or re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*:', stripped):
                break
            # Skip assembler directives
            if stripped.startswith(".") and not stripped.startswith(".set"):
                continue
            lines.append(stripped)
    return "\n".join(lines)


# Register name mapping: ABI names → numbers
ABI_TO_NUM = {
    "$zero": "$0", "$at": "$1", "$v0": "$2", "$v1": "$3",
    "$a0": "$4", "$a1": "$5", "$a2": "$6", "$a3": "$7",
    "$t0": "$8", "$t1": "$9", "$t2": "$10", "$t3": "$11",
    "$t4": "$12", "$t5": "$13", "$t6": "$14", "$t7": "$15",
    "$s0": "$16", "$s1": "$17", "$s2": "$18", "$s3": "$19",
    "$s4": "$20", "$s5": "$21", "$s6": "$22", "$s7": "$23",
    "$t8": "$24", "$t9": "$25", "$k0": "$26", "$k1": "$27",
    "$gp": "$28", "$sp": "$29", "$fp": "$30", "$ra": "$31",
    "$0": "$zero",
}

NUM_TO_ABI = {v: k for k, v in ABI_TO_NUM.items()}

# CP1 register names
FREG_ABI = {f"${i}": f"$f{i}" for i in range(32)}


def normalize_insn(line: str) -> str:
    """Normalize MIPS instruction for structural comparison."""
    line = re.sub(r'\s*#.*$', '', line).strip()
    if not line:
        return ""
    parts = line.split(None, 1)
    if len(parts) < 2:
        return parts[0]
    mnem, ops = parts

    # 1. ABI register names → numeric ($a0 → $4, $sp → $29)
    for abi, num in ABI_TO_NUM.items():
        ops = ops.replace(abi, num)

    # 2. All hex immediates ($0x15c or 0x15c) → decimal
    ops = re.sub(r'\$?0x([0-9a-fA-F]+)', lambda m: str(int(m.group(1), 16)), ops)

    # 3. Strip leading $ from bare numbers in immediate positions
    #    (after comma or at start of operand group)
    ops = re.sub(r'(?<=[\s,(])\$(\d+)', r'\1', ops)

    # 4. Normalize zero offset: 0($reg) or $0($reg) → ($reg)
    ops = re.sub(r'(?<=[\s,(])0\(', '(', ops)
    ops = re.sub(r'(?<=[\s,(])\$0\(', '(', ops)

    # 5. Normalize comma spacing: always comma+space
    ops = re.sub(r',\s*', ', ', ops)

    # 6. Collapse spacing
    ops = re.sub(r'\s+', ' ', ops).strip()

    # 6. Normalize mnemonics
    if mnem == 'j' and (ops == '$31' or ops == '$ra'):
        mnem = 'jr'

    return f"{mnem} {ops}"


def parse_asm_lines(asm: str) -> list[str]:
    """Parse assembly into (mnemonic + operands) strings, stripping labels."""
    lines = []
    for line in asm.splitlines():
        line = line.strip()
        if not line or line.endswith(":") or line.startswith("."):
            continue
        norm = normalize_insn(line)
        if norm:
            lines.append(norm)
    return lines


def score_against_target(func_asm: str, target_va: int, func_size: int = 0x100):
    """Score generated assembly against target ELF function."""
    target_bytes = extract_elf_func(target_va, func_size)
    target_insns = disassemble_mips64(target_bytes, target_va)

    gen_lines = parse_asm_lines(func_asm)

    # Normalize target instructions for comparison
    tgt_lines = []
    for insn in target_insns:
        tgt_lines.append(normalize_insn(f"{insn['mnemonic']} {insn['op_str']}"))

    print(f"Target instructions: {len(tgt_lines)}")
    print(f"Generated instructions: {len(gen_lines)}")
    print()

    # Score
    max_score = len(tgt_lines) * 100
    score = 0
    mismatches = []

    for i, tgt in enumerate(tgt_lines):
        if i < len(gen_lines):
            gen = gen_lines[i]
            if gen == tgt:
                score += 100
            else:
                mismatches.append((i, tgt, gen))
                # Partial credit for matching mnemonic
                gen_mnem = gen.split()[0] if gen else ""
                tgt_mnem = tgt.split()[0] if tgt else ""
                if gen_mnem == tgt_mnem:
                    score += 50
        else:
            mismatches.append((i, tgt, "(missing)"))
            score += 0

    # Extra instructions in generated code
    for i in range(len(tgt_lines), len(gen_lines)):
        mismatches.append((i, "(extra)", gen_lines[i]))

    pct = (score / max_score * 100) if max_score > 0 else 0

    print(f"Score: {score}/{max_score} = {pct:.2f}%")
    print()

    if mismatches:
        print("Differences (line | target | generated):")
        print("-" * 60)
        for idx, tgt, gen in mismatches:
            print(f"  +{idx:3d}: | {tgt:30s} | {gen}")
        print()
    else:
        print("PERFECT MATCH!")
        print()

    # Also print full comparison
    print("Full comparison (target vs generated):")
    print("-" * 60)
    for i in range(max(len(tgt_lines), len(gen_lines))):
        tgt = tgt_lines[i] if i < len(tgt_lines) else "(missing)"
        gen = gen_lines[i] if i < len(gen_lines) else "(missing)"
        marker = " " if tgt == gen else "X"
        print(f"  {marker} +{i:3d}: T: {tgt:35s} G: {gen}")
    print()

    return pct


def compile_and_score(c_code: str, func_name: str, target_va: int, func_size: int = 0x100):
    """Full pipeline: compile → extract → score."""
    print("=== Compiling with ee-gcc 2.9-991111-01 ===")
    print(f"  Flags: {' '.join(CC_FLAGS)}")
    print(f"  Target: {func_name} @ 0x{target_va:08X} ({func_size}B)")
    print()

    asm = compile_c_to_asm(c_code)
    if asm is None:
        print("FAILED: compilation error")
        return

    func_asm = extract_func_asm(asm, func_name)
    if not func_asm.strip():
        print(f"FAILED: function '{func_name}' not found in output")
        print("Available sections:")
        for line in asm.splitlines():
            if line.strip().endswith(":") and not line.strip().startswith("."):
                print(f"  {line.strip()}")
        return

    print("=== Generated ASM ===")
    print(func_asm)
    print()

    score_against_target(func_asm, target_va, func_size)


def extract_function_body(c_code: str, func_name: str) -> str | None:
    """Extract function body + ALL preceding declarations/types."""
    lines = c_code.splitlines()
    pre_lines = []   # everything before the function definition
    func_lines = []  # the function definition itself
    func_line_idx = -1

    # Find the target function DEFINITION line
    for i, line in enumerate(lines):
        stripped = line.strip()
        if stripped.endswith(";"):
            continue  # skip forward decls
        pattern = re.compile(
            r'^(void|int|u32|ico_u32|ico_ptr32|float|static)\s+\**\s*'
            + re.escape(func_name) + r'\s*\('
        )
        if pattern.search(stripped):
            func_line_idx = i
            break

    if func_line_idx < 0:
        return None

    # Include ALL lines before the function (except other function bodies)
    brace_depth = 0
    for i in range(func_line_idx):
        stripped = lines[i].strip()
        # Skip comment-only lines (including multi-line comment continuations)
        if stripped.startswith("//") or stripped.startswith("/*") or stripped.startswith("*"):
            continue
        pre_lines.append(lines[i])

    # Extract function body by brace matching
    in_func = False
    brace_depth = 0
    saw_open = False
    for i in range(func_line_idx, len(lines)):
        stripped = lines[i].strip()
        if not in_func:
            pattern = re.compile(
                r'^(void|int|u32|ico_u32|ico_ptr32|float|static)\s+\**\s*'
                + re.escape(func_name) + r'\s*\('
            )
            if pattern.search(stripped):
                in_func = True
        if in_func:
            func_lines.append(lines[i])
            if "{" in lines[i]:
                saw_open = True
            brace_depth += lines[i].count("{") - lines[i].count("}")
            if brace_depth == 0 and saw_open:
                break

    if not func_lines:
        return None

    result = list(pre_lines)
    result.extend(func_lines)
    return "\n".join(result)


def main():
    parser = argparse.ArgumentParser(
        description="Compile C with ee-gcc 2.9 and score against ELF target")
    parser.add_argument("c_file", help="C source file path")
    parser.add_argument("--fn", required=True, help="Function name to extract")
    parser.add_argument("--va", required=True, help="Target VA (e.g. 0x1CE5F8)")
    parser.add_argument("--size", type=int, default=0x100,
                        help="Function size in bytes (default 0x100)")
    parser.add_argument("--whole-file", action="store_true",
                        help="Compile entire file, don't extract function")
    args = parser.parse_args()

    with open(args.c_file) as f:
        c_code = f.read()

    target_va = int(args.va, 16)

    if not args.whole_file:
        extracted = extract_function_body(c_code, args.fn)
        if extracted is None:
            print(f"FAILED: function '{args.fn}' not found in {args.c_file}")
            return
        compile_and_score(extracted, args.fn, target_va, args.size)
    else:
        compile_and_score(c_code, args.fn, target_va, args.size)


if __name__ == "__main__":
    main()
