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

ELF_PATH = os.path.expanduser(
    "~/Documentos/repos/ico-reconstruction/.local/extracted/SCUS_971.13.elf"
)
PT_LOAD_VA = 0x100000
PT_LOAD_FILE = 0x1000

COMPILER = "ee-gcc2.9-991111-01"
PLATFORM = "ps2"
FLAGS = "-march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2"


def byteswap32(data):
    result = bytearray()
    for i in range(0, len(data), 4):
        result.extend(
            data[i + 3 : i + 4]
            + data[i + 2 : i + 3]
            + data[i + 1 : i + 2]
            + data[i : i + 1]
        )
    return bytes(result)


def disassemble(va, n_insns=200):
    file_offset = va - PT_LOAD_VA + PT_LOAD_FILE
    with open(ELF_PATH, "rb") as f:
        f.seek(file_offset)
        raw = f.read(n_insns * 4)

    raw_be = byteswap32(raw)
    md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS64 + CS_MODE_BIG_ENDIAN)
    return [(i.address, i.mnemonic, i.op_str) for i in md.disasm(raw_be, va)]


def asm_to_decompme(insns):
    """Convert instruction list to decomp.me assembly format."""
    lines = []
    for addr, mn, ops in insns:
        line = f"{mn} {ops}" if ops else mn
        lines.append(line)
    return "\n".join(lines)


def submit(va, c_source, name=None, description=None):
    insns = disassemble(va)
    if not insns:
        print(f"ERROR: No instructions at 0x{va:X}")
        return None

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
