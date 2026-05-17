#!/usr/bin/env python3
"""Refined analysis of ICO ELF: Entry table, slot 0 callback, scene helper."""
import struct
from capstone import *
from capstone.mips import *

ELF_PATH = "/home/peter/Documentos/repos/ico-reconstruction/.local/extracted/SCUS_971.13.elf"
with open(ELF_PATH, 'rb') as f:
    elf_data = f.read()

VA_BASE = 0x100000
FILE_BASE = 0x1000

def va2off(va):
    return va - VA_BASE + FILE_BASE

def r32(off):
    return struct.unpack_from('<I', elf_data, off)[0]

def r16(off):
    return struct.unpack_from('<H', elf_data, off)[0]

def r8(off):
    return elf_data[off]

def f32(off):
    return struct.unpack_from('<f', elf_data, off)[0]

# ================================================================
# TASK A: Entry Table at 0x2A4C48 (512 x 0x4C)
# ================================================================
print("=" * 72)
print("TASK A: Entry Table at 0x2A4C48")
print("512 entries x 0x4C (76 bytes)")
print("=" * 72)

ENTRY_VA = 0x002A4C48
entry_off = va2off(ENTRY_VA)

# Raw dump of first 10 entries in hex
print("\nRaw hex dump (first 10 entries, 76 bytes each):")
for i in range(10):
    off = entry_off + i * 0x4C
    hex_bytes = ' '.join(f'{b:02x}' for b in elf_data[off:off+0x4C])
    print(f"  [{i:2d}] {hex_bytes}")

# Decode structure - identify fields by their patterns
print("\nDecoded structure analysis:")
# 0x3F800000 = 1.0f (scale)
# Look at fields that vary

# Dump +0x30 word as possible entity ID/class
print("\nKey fields for all 512 entries:")
print(f"{'Idx':>4} {'+0x30(desc)':>12} {'+0x34(sub)':>12} {'+0x38':>12} {'+0x3C':>12} {'+0x40':>12} {'+0x44(pk)':>12} {'+0x46(byte)':>12} {'+0x48':>12} {'+0x4A':>12}")
print("-" * 108)

from collections import Counter
class_counter = Counter()
for i in range(512):
    off = entry_off + i * 0x4C
    w30 = r32(off + 0x30)
    w34 = r32(off + 0x34)
    w38 = r32(off + 0x38)
    w3c = r32(off + 0x3C)
    w40 = r32(off + 0x40)
    w44 = r32(off + 0x44)
    b46 = r8(off + 0x46)
    w48 = r32(off + 0x48)
    w4a = r16(off + 0x4A)
    class_counter[w30] += 1
    if i < 30 or i >= 505:
        print(f"{i:4d} 0x{w30:08X} 0x{w34:08X} 0x{w38:08X} 0x{w3c:08X} 0x{w40:08X} 0x{w44:08X} 0x{b46:02X} 0x{w48:08X} 0x{w4a:04X}")

print(f"\nEntity count by +0x30 class ID: ({len(class_counter)} unique)")
for cid, cnt in sorted(class_counter.items(), key=lambda x: -x[1]):
    print(f"  Class 0x{cid:08X} ({cid:6d}): {cnt:3d} instances")

# Show relationships: +0x34 values distribution
subtype_counter = Counter()
for i in range(512):
    off = entry_off + i * 0x4C
    w34 = r32(off + 0x34)
    subtype_counter[w34] += 1
print(f"\n+0x34 (subtype) distribution:")
for sid, cnt in sorted(subtype_counter.items(), key=lambda x: -x[1]):
    print(f"  0x{sid:08X} ({sid:6d}): {cnt:3d} instances")

# Examine +0x44 packed field
print("\n+0x44 packed field (high16:low16):")
for i in range(20):
    off = entry_off + i * 0x4C
    w44 = r32(off + 0x44)
    high = (w44 >> 16) & 0xFFFF
    low = w44 & 0xFFFF
    print(f"  [{i:2d}] 0x{w44:08X} high=0x{high:04X}({high:5d}) low=0x{low:04X}({low:5d})")

# Float data at +0x0C through +0x20 (position/rotation)
print("\nFloat fields at +0x0C, +0x10, +0x14, +0x18, +0x1C, +0x20:")
for i in range(12):
    off = entry_off + i * 0x4C
    f0c = f32(off + 0x0C)
    f10 = f32(off + 0x10)
    f14 = f32(off + 0x14)
    f18 = f32(off + 0x18)
    f1c = f32(off + 0x1C)
    f20 = f32(off + 0x20)
    print(f"  [{i:2d}] 0x0C={f0c:8.2f} 0x10={f10:8.2f} 0x14={f14:8.2f} 0x18={f18:8.2f} 0x1C={f1c:8.2f} 0x20={f20:8.2f}")

# ================================================================
# DESCRIPTOR TABLE at 0x2A31B8
# ================================================================
print("\n" + "-" * 72)
print("Descriptor Table at 0x2A31B8")
print("-" * 72)

DESC_VA = 0x002A31B8
desc_off = va2off(DESC_VA)

# From entry iteration fn: addiu a3, s2, 0x31b8 -> load table base
# After loading desc_idx from entry, use it to index into this table
# The stride appears to be 0x28 (40 bytes) based on adjacent structures

# Let's examine descriptor 4 in detail (has function pointers visible)
print("\nDescriptors 0-30 decoded (stride=0x28):")
print(f"{'Idx':>4} {'name':>16} {'+0x04':>10} {'+0x08':>10} {'init_fn':>10} {'+0x10':>10} {'unk_fn0':>10} {'+0x18':>10} {'unk_fn1':>10} {'+0x20':>10} {'+0x24(float)':>12}")
print("-" * 130)

def bytes_to_str(data, max_len=16):
    """Try to decode bytes as ASCII string."""
    s = ''
    for b in data:
        if 0x20 <= b < 0x7f:
            s += chr(b)
        else:
            s += '.'
    return s

for idx in range(50):
    off = desc_off + idx * 0x28
    if off + 0x28 > len(elf_data):
        break
    raw = elf_data[off:off+0x28]
    name_raw = raw[:4]
    name_str = bytes_to_str(name_raw)
    
    w04 = r32(off+0x04)
    w08 = r32(off+0x08)
    w0c = r32(off+0x0C)
    w10 = r32(off+0x10)
    w14 = r32(off+0x14)
    w18 = r32(off+0x18)
    w1c = r32(off+0x1C)
    w20 = r32(off+0x20)
    fl24 = f32(off+0x24)
    
    name_full = bytes_to_str(raw[:12])  # up to 12 chars
    print(f"{idx:4d} {name_full:>16s} 0x{w04:08X} 0x{w08:08X} 0x{w0c:08X} 0x{w10:08X} 0x{w14:08X} 0x{w18:08X} 0x{w1c:08X} {fl24:12.4f}")

# Check which desc entries have function pointers (in .text range)
print("\nFunction pointers in descriptor table (in .text 0x100000-0x27FFFF):")
fn_count = 0
for idx in range(200):
    off = desc_off + idx * 0x28
    if off + 0x28 > len(elf_data):
        break
    for j in range(10):
        val = r32(off + j*4)
        if 0x100000 <= val < 0x280000:
            fn_count += 1
            if fn_count <= 30:
                offset_name = f"+0x{j*4:02X}"
                print(f"  Desc[{idx}]{offset_name}: 0x{val:08X}")

# ================================================================
# TASK B: Slot 0 Callback at 0x168DA8
# ================================================================
print("\n" + "=" * 72)
print("TASK B: Slot 0 Callback at 0x168DA8")
print("=" * 72)

md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS32 + CS_MODE_LITTLE_ENDIAN)
md.detail = True

def dasm(va, size=512):
    off = va2off(va)
    code = elf_data[off:off+size]
    for insn in md.disasm(code, va):
        raw = insn.bytes.hex()
        yield f"  0x{insn.address:08X}:  {insn.mnemonic:10s} {insn.op_str}"

# Proper disassembly of slot 0
print("\n--- Slot 0: 0x168DA8 ---")
for line in dasm(0x168DA8, 384):
    print(line)

print("\n--- Slot 1: 0x168ED0 (comparison) ---")
for line in dasm(0x168ED0, 384):
    print(line)

print("\n--- Slot 2: 0x169020 (comparison) ---")
for line in dasm(0x169020, 384):
    print(line)

# ================================================================
# TASK C: Scene Loader Helper at 0x13B858
# ================================================================
print("\n" + "=" * 72)
print("TASK C: Scene Loader Helper at 0x13B858")
print("=" * 72)

# Disassemble the function (and its siblings 0x13B858-0x13B877)
print("\n--- 0x13B858 family ---")
for line in dasm(0x13B858, 384):
    print(line)

# Callers
print("\n--- Callers of 0x13B858 ---")
jal_encoding = struct.pack('<I', 0x0C000000 | (0x13B858 >> 2))
pos = 0
while True:
    pos = elf_data.find(jal_encoding, pos)
    if pos == -1:
        break
    va = pos - FILE_BASE + VA_BASE
    print(f"\nCaller at 0x{va:08X}:")
    # Show 3 instructions before and after
    pre_off = max(pos - 12, 0)
    pre_va = pre_off - FILE_BASE + VA_BASE
    post_off = pos + 4
    post_va = post_off - FILE_BASE + VA_BASE
    pre_code = elf_data[pre_off:pos+12]
    for insn in md.disasm(pre_code, pre_va):
        marker = " >>>" if insn.address == va else "    "
        print(f"  {marker} 0x{insn.address:08X}: {insn.mnemonic:10s} {insn.op_str}")
    pos += 4

print("\nDone.")
