#!/usr/bin/env python3
"""Disassemble a project-local EE address range from the USA ELF, read-only."""

from __future__ import annotations

import argparse
import struct
from pathlib import Path

from capstone import CS_ARCH_MIPS, CS_MODE_LITTLE_ENDIAN, CS_MODE_MIPS64, Cs


ROOT = Path(__file__).resolve().parents[1]
ELF = ROOT / ".local/extracted/SCUS_971.13.elf"


def file_offset(data: bytes, local_va: int) -> int:
    phoff = struct.unpack_from("<I", data, 0x1C)[0]
    entsize = struct.unpack_from("<H", data, 0x2A)[0]
    count = struct.unpack_from("<H", data, 0x2C)[0]
    for index in range(count):
        offset = phoff + index * entsize
        if struct.unpack_from("<I", data, offset)[0] != 1:
            continue
        file_start, va, _physical, file_size = struct.unpack_from("<IIII", data, offset + 4)
        for target in (local_va, local_va + 0x00100000):
            if va <= target < va + file_size:
                return file_start + target - va
    raise ValueError(f"local VA 0x{local_va:08X} is not file-backed")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("va", type=lambda text: int(text, 0))
    parser.add_argument("size", type=lambda text: int(text, 0))
    args = parser.parse_args()
    data = ELF.read_bytes()
    offset = file_offset(data, args.va)
    decoder = Cs(CS_ARCH_MIPS, CS_MODE_MIPS64 | CS_MODE_LITTLE_ENDIAN)
    decoder.skipdata = True
    for instruction in decoder.disasm(data[offset : offset + args.size], args.va):
        print(f"0x{instruction.address:08X}: {instruction.mnemonic:<8} {instruction.op_str}")


if __name__ == "__main__":
    main()
