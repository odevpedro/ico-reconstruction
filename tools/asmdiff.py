#!/usr/bin/env python3
"""Compile C with ee-gcc 2.9 and diff against target ASM from ELF.

Usage:
    python3 tools/asmdiff.py src/entity/enemy1.c --fn fn_1CE5F8 --va 0x1CE5F8
"""
import argparse
import subprocess
import struct
import sys
import os
import tempfile
import re

import capstone

TOOLCHAIN = os.path.expanduser(
    "~/Documentos/repos/ico-reconstruction/toolchain/ee-gcc2.9-991111-01/bin/ee-gcc"
)
ELF_PATH = os.path.expanduser(
    "~/.local/extracted/SCUS_971.13.elf"
)

CC_FLAGS = [
    "-mips3",
    "-mgp64",
    "-mabi=eabi",
    "-msingle-float",
    "-G0",
    "-O2",
    "-S",   # assembly output
]


def compile_c(c_code: str) -> str:
    """Compile C code with ee-gcc 2.9 and return assembly text."""
    with tempfile.NamedTemporaryFile(suffix=".c", mode="w", delete=False) as f:
        f.write(c_code)
        f.flush()
        infile = f.name

    try:
        result = subprocess.run(
            [TOOLCHAIN] + CC_FLAGS + [infile],
            capture_output=True,
            text=True,
            timeout=30,
        )
        if result.returncode != 0:
            print("STDERR:", result.stderr, file=sys.stderr)
            return None
        
        # Read the generated .s file (GCC outputs .s next to input)
        sfile = infile + ".s" if not infile.endswith(".s") else infile
        # Actually with -S it outputs to <input>.s
        outf = infile.rsplit(".", 1)[0] + ".s"
        if not os.path.exists(outf):
            # Maybe it's at stdout?
            return result.stdout
        
        with open(outf) as f:
            asm = f.read()
        return asm
    finally:
        os.unlink(infile)
        outf = infile.rsplit(".", 1)[0] + ".s"
        if os.path.exists(outf):
            os.unlink(outf)


def extract_function_asm(asm_text: str, func_name: str) -> str:
    """Extract assembly for a specific function from GCC output."""
    lines = []
    in_func = False
    for line in asm_text.splitlines():
        if line.startswith(func_name + ":"):
            in_func = True
            continue
        if in_func:
            # Next .globl or label with colon starts a new function
            if line.startswith("\t.globl") or re.match(r'^[a-zA-Z_][a-zA-Z0-9_]*:', line):
                break
            # Skip directives, keep instructions
            if line.startswith("\t.") or line.strip() == "":
                continue
            lines.append(line.strip())
    return "\n".join(lines)


def disassemble_elf(target_va: int, size: int = 0x100) -> str:
    """Disassemble function from ELF at given VA."""
    with open(ELF_PATH, "rb") as f:
        elf_data = f.read()
    
    # 32-bit ELF program header
    e_phoff = struct.unpack_from("<I", elf_data, 0x1C)[0]
    e_phentsize = struct.unpack_from("<H", elf_data, 0x2A)[0]
    e_phnum = struct.unpack_from("<H", elf_data, 0x2C)[0]
    
    file_offset = None
    for i in range(e_phnum):
        ph = elf_data[e_phoff + i * e_phentsize : e_phoff + (i + 1) * e_phentsize]
        p_type = struct.unpack_from("<I", ph, 0)[0]
        if p_type != 1:  # PT_LOAD
            continue
        p_offset = struct.unpack_from("<I", ph, 4)[0]
        p_vaddr = struct.unpack_from("<I", ph, 8)[0]
        p_filesz = struct.unpack_from("<I", ph, 16)[0]
        if p_vaddr <= target_va < p_vaddr + p_filesz:
            file_offset = p_offset + (target_va - p_vaddr)
            break
    
    if file_offset is None:
        return f"ERROR: VA 0x{target_va:08X} not found in any LOAD segment"
    
    func_data = elf_data[file_offset:file_offset + size]
    
    md = capstone.Cs(capstone.CS_ARCH_MIPS, 
                     capstone.CS_MODE_MIPS64 + capstone.CS_MODE_LITTLE_ENDIAN)
    
    result = []
    for insn in md.disasm(func_data, target_va):
        result.append(f"  {insn.mnemonic:10s} {insn.op_str}")
    
    return "\n".join(result)


def compile_and_diff(c_code: str, func_name: str, target_va: int):
    """Compile C and diff against ELF target."""
    print("=== Compiling with ee-gcc 2.9-991111-01 ===")
    print(f"  Flags: {' '.join(CC_FLAGS)}\n")
    
    # Write C to temp file
    with tempfile.NamedTemporaryFile(suffix=".c", mode="w", delete=False) as f:
        f.write(c_code)
        f.flush()
        infile = f.name
    
    try:
        result = subprocess.run(
            [TOOLCHAIN] + CC_FLAGS + [infile],
            capture_output=True,
            text=True,
            timeout=30,
        )
        if result.returncode != 0:
            print("COMPILE ERROR:", result.stderr)
            return
        
        # Read .s output
        sfile = infile.rsplit(".", 1)[0] + ".s"
        if os.path.exists(sfile):
            with open(sfile) as f:
                asm_text = f.read()
            
            func_asm = extract_function_asm(asm_text, func_name)
            target_asm = disassemble_elf(target_va)
            
            print(f"=== Generated ASM for {func_name} === ")
            print(func_asm)
            print()
            print(f"=== Target ASM at 0x{target_va:08X} ===")
            print(target_asm)
            print()
            
            # Simple diff
            print("=== DIFF ===")
            gen_lines = func_asm.splitlines()
            tgt_lines = target_asm.splitlines()
            
            for i, (g, t) in enumerate(zip(gen_lines, tgt_lines)):
                if g != t:
                    print(f"  -{i:3d}: {t}")
                    print(f"  +{i:3d}: {g}")
            
            if len(gen_lines) != len(tgt_lines):
                print(f"\n  SIZE MISMATCH: generated {len(gen_lines)} insns, target {len(tgt_lines)} insns")
        else:
            print("STDOUT:", result.stdout)
            print("STDERR:", result.stderr)
    
    finally:
        os.unlink(infile)
        sfile = infile.rsplit(".", 1)[0] + ".s"
        if os.path.exists(sfile):
            os.unlink(sfile)


def main():
    parser = argparse.ArgumentParser(description="Compile C with ee-gcc and diff against ELF target")
    parser.add_argument("c_file", help="C source file")
    parser.add_argument("--fn", required=True, help="Function name to extract")
    parser.add_argument("--va", required=True, help="Target VA (e.g. 0x1CE5F8)")
    args = parser.parse_args()
    
    with open(args.c_file) as f:
        c_code = f.read()
    
    target_va = int(args.va, 16)
    compile_and_diff(c_code, args.fn, target_va)


if __name__ == "__main__":
    main()
