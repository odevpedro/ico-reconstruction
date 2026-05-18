#!/usr/bin/env python3
"""
Submit a decompiled function to decomp.me for ee-gcc 2.9-991111-01 matching.
Usage:
  python3 decompme_submit.py <va> <c_source_file> [-n <name>] [-d <description>]

Extracts assembly from the ICO ELF and submits with the given C source.
"""

import json
import sys
import os
import urllib.request
from capstone import *
import re

ELF_PATH = os.path.expanduser(
    "~/Documentos/repos/ico-reconstruction/.local/extracted/SCUS_971.13.elf"
)
PT_LOAD_VA = 0x100000
PT_LOAD_FILE = 0x1000

COMPILER = "EE GCC 2.9 build 991111-01"
PLATFORM = "ps2"
FLAGS = "-mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2"


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


def abi_to_numeric(line: str) -> str:
    """Convert ABI register names to numeric in a single asm line."""
    for abi, num in ABI_TO_NUM.items():
        line = line.replace(abi, num)
    return line


def disassemble(va, n_insns=200):
    file_offset = va - PT_LOAD_VA + PT_LOAD_FILE
    with open(ELF_PATH, "rb") as f:
        f.seek(file_offset)
        raw = f.read(n_insns * 4)

    # PS2 EE is little-endian
    md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS64 + CS_MODE_LITTLE_ENDIAN)
    return [(i.address, i.mnemonic, abi_to_numeric(i.op_str))
            for i in md.disasm(raw, va)]


def normalize_target_asm(text: str) -> str:
    """Normalize target ASM to match ee-gcc 2.9 output format."""
    # Convert hex immediates ($0xNNN or 0xNNN) to decimal
    text = re.sub(r'\$?0x([0-9a-fA-F]+)',
                  lambda m: str(int(m.group(1), 16)), text)
    # jr $31 → j $31 (GCC emits 'j $31', not 'jr $31')
    text = re.sub(r'\bjr \$31\b', 'j $31', text)
    # Remove extra whitespace, trailing commas
    text = re.sub(r' +', ' ', text)
    text = re.sub(r',\s+', ', ', text)
    return text.strip()


def asm_to_decompme(insns):
    """Convert instruction list to decomp.me assembly format."""
    lines = []
    for addr, mn, ops in insns:
        line = f"{mn} {ops}" if ops else mn
        lines.append(line)
    return normalize_target_asm("\n".join(lines))


def submit(va, c_source, name=None, description=None, n_insns=200):
    insns = disassemble(va, n_insns)
    if not insns:
        print(f"ERROR: No instructions at 0x{va:X}")
        return None

    # Trim trailing nops (padding)
    while insns and insns[-1][1] == "nop":
        insns = insns[:-1]

    print(f"Disassembled {len(insns)} instructions from 0x{va:X}")

    target_asm = asm_to_decompme(insns)
    slug_part = f"ico_{name or f'func_{va:X}'}_{len(insns)}i"

    payload = {
        "compiler": COMPILER,
        "compiler_flags": FLAGS,
        "target_asm": target_asm,
        "source_code": c_source,
        "platform": PLATFORM,
        "name": name or f"ICO 0x{va:X} ({len(insns)} insns)",
        "description": description or f"Auto-submitted from ico-reconstruction",
    }

    req = urllib.request.Request(
        "https://decomp.me/api/scratch",
        data=json.dumps(payload).encode(),
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json",
            "User-Agent": "ico-reconstruction/1.0",
        },
        method="POST",
    )

    with urllib.request.urlopen(req, timeout=60) as resp:
        result = json.loads(resp.read())

    print(f"\nCreated: https://decomp.me/scratch/{result['slug']}")
    print(f"Score:   {result['score']}/{result['max_score']}")
    print(f"Token:   {result.get('claim_token', 'N/A')}")
    print(f"Claim:   https://decomp.me/scratch/{result['slug']}/claim?token={result.get('claim_token', '')}")
    return result


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    va = int(sys.argv[1], 16) if sys.argv[1].startswith("0x") else int(sys.argv[1])
    c_path = sys.argv[2]

    name = None
    description = None
    for i, arg in enumerate(sys.argv[3:]):
        if arg == "-n" and i + 1 < len(sys.argv[3:]):
            name = sys.argv[3 + i + 1]
        if arg == "-d" and i + 1 < len(sys.argv[3:]):
            description = sys.argv[3 + i + 1]

    with open(c_path) as f:
        c_source = f.read()

    result = submit(va, c_source, name, description)
    if result:
        print("\nDONE")
