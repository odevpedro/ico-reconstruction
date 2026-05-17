#!/usr/bin/env python3
"""Final refined analysis: clean output for all 3 tasks."""
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

md = Cs(CS_ARCH_MIPS, CS_MODE_MIPS32 + CS_MODE_LITTLE_ENDIAN)
md.detail = True

def dasm(va, size=512):
    off = va2off(va)
    code = elf_data[off:off+size]
    for insn in md.disasm(code, va):
        # Skip the SD/load pair byte sequences that capstone can't decode
        if insn.mnemonic.startswith('.byt'):
            continue
        raw = insn.bytes.hex()
        yield f"  0x{insn.address:08X}:  {raw:16s}  {insn.mnemonic:10s} {insn.op_str}"

def dasm_raw(va, size=512):
    """Disassemble but show byte sequences for undecoded too."""
    off = va2off(va)
    code = elf_data[off:off+size]
    for insn in md.disasm(code, va):
        raw = insn.bytes.hex()
        yield f"  0x{insn.address:08X}:  {raw:16s}  {insn.mnemonic:10s} {insn.op_str}"

# ================================================================
# TASK A: ENTRY TABLE
# ================================================================
print("=" * 76)
print("TASK A: ENTRY TABLE at 0x2A4C48")
print("512 entries x 0x4C (76 bytes) per entry")
print("=" * 76)

ENTRY_VA = 0x002A4C48
entry_off = va2off(ENTRY_VA)

print("\nFIRST 20 ENTRIES — full 0x4C decode:")
header = f"{'Idx':>4} {'+0x00':>10} {'+0x04':>10} {'+0x08':>10} {'+0x0C':>10} {'+0x10':>10} {'+0x14':>10} {'+0x18':>10} {'+0x1C':>10} {'+0x20':>10} {'+0x24':>10} {'+0x28':>10} {'+0x2C':>10} {'+0x30':>10} {'+0x34':>10} {'+0x38':>10} {'+0x3C':>10} {'+0x40':>10} {'+0x44':>10} {'+0x48':>10}"
print(header)
print("-" * len(header))

for i in range(20):
    off = entry_off + i*0x4C
    vals = [r32(off + j*4) for j in range(19)]
    vals_str = ' '.join(f'0x{v:08X}' if i < 10 else f'{v:10d}' for v in vals)
    print(f"  {i:2d}  " + ' '.join(f'0x{v:08X}' for v in vals))

# Group by class (+0x30)
from collections import Counter
class_counts = Counter()
for i in range(512):
    off = entry_off + i*0x4C
    cls = r32(off + 0x30)
    class_counts[cls] += 1

print(f"\nGrouped by +0x30 (class/type) — {len(class_counts)} unique classes:")
print(f"  {'Class':>12} {'Count':>6}")
print(f"  {'-'*12} {'-'*6}")
for cls, cnt in sorted(class_counts.items(), key=lambda x: -x[1]):
    print(f"  0x{cls:08X} ({cls:>3d}) {cnt:6d}")

# Show +0x30=0x15 entries more detail
print(f"\nSample entries with +0x30=0x15 (most common class, {class_counts.get(0x15, 0)} instances):")
print(f"  {'Idx':>4} {'+0x34(sub)':>12} {'+0x38(inst)':>12} {'+0x3C':>10} {'+0x40(link)':>10} {'+0x44(pack)':>10} {'+0x46(byte)':>6} {'+0x48':>10}")
for i in range(512):
    off = entry_off + i*0x4C
    if r32(off + 0x30) == 0x15:
        w34 = r32(off + 0x34)
        w38 = r32(off + 0x38)
        w3c = r32(off + 0x3C)
        w40 = r32(off + 0x40)
        w44 = r32(off + 0x44)
        b46 = r8(off + 0x46)
        w48 = r32(off + 0x48)
        print(f"  {i:4d} 0x{w34:08X} 0x{w38:08X} 0x{w3c:08X} 0x{w40:08X} 0x{w44:08X} 0x{b46:02X} 0x{w48:08X}")

# Show non-zero +0x34 entries (have real subtype IDs)
print(f"\nEntries with non-zero +0x34 (probably room/model IDs):")
print(f"  {'Idx':>4} {'+0x34':>10} {'+0x38':>10}")
for i in range(512):
    off = entry_off + i*0x4C
    w34 = r32(off + 0x34)
    if w34 != 0:
        w38 = r32(off + 0x38)
        print(f"  {i:4d} 0x{w34:08X} ({w34:5d}) 0x{w38:08X}")

# Float positions for entries
print(f"\nEntries with position data (non-zero +0x10..+0x20):")
for i in range(512):
    off = entry_off + i*0x4C
    f10 = f32(off + 0x10)
    f14 = f32(off + 0x14)
    f18 = f32(off + 0x18)
    if f10 != 0 or f14 != 0 or f18 != 0:
        f1c = f32(off + 0x1C)
        f20 = f32(off + 0x20)
        cls = r32(off + 0x30)
        sub = r32(off + 0x34)
        print(f"  [{i:3d}] cls={cls:3d} sub={sub:4d} pos=({f10:8.2f}, {f14:8.2f}, {f18:8.2f}) extra=({f1c:8.2f}, {f20:8.2f})")

# ================================================================
# DESCRIPTOR TABLE: clean name extraction
# ================================================================
print("\n" + "-" * 76)
print("DESCRIPTOR TABLE at 0x2A31B8 (for reference)")
print("-" * 76)

DESC_VA = 0x002A31B8
desc_off = va2off(DESC_VA)

def extract_name_at(offset, max_words=6):
    """Extract null-terminated ASCII string from LE words starting at offset."""
    raw = elf_data[offset:offset+max_words*4]
    name = ''
    for i in range(0, len(raw), 4):
        chunk = raw[i:i+4]
        # Read as LE and interpret bytes in LE order
        for b in chunk:
            if b == 0:
                return name
            if 0x20 <= b < 0x7f:
                name += chr(b)
            else:
                name += '.'
    return name

print("\nDecoded descriptors (name at +0x00 or +0x14, +0x24 bounding radius):")
dummy = 0
for idx in range(80):
    off = desc_off + idx * 0x28
    if off + 0x28 > len(elf_data):
        break
    # Try to extract name: look for ASCII string pattern at +0x00 or +0x14
    raw0 = elf_data[off:off+4]
    name0 = ''.join(chr(b) if 0x20 <= b < 0x7f else '.' for b in raw0)
    name14 = extract_name_at(off + 0x14) if idx > 0 else ''
    
    w0c = r32(off + 0x0C)  # init_fn candidate
    w14_0 = r32(off + 0x14)
    w24 = f32(off + 0x24)
    
    # Determine which name to use
    if all(0x20 <= b < 0x7f for b in raw0 if b != 0):
        name = ''.join(chr(b) for b in raw0 if 0x20 <= b < 0x7f)
    elif name14:
        name = name14
    else:
        name = f"(no-name)"
    
    # Check if +0x0C is a valid function pointer
    fn0c = f"0x{w0c:08X}" if 0x100000 <= w0c < 0x280000 else "-"
    
    # Only print descriptors with a real name or function pointers
    if name == "(no-name)" and fn0c == "-" and w24 == 0.0:
        continue
    
    print(f"  [{idx:2d}] {name:20s} init={fn0c} radius={w24:8.2f}")

# ================================================================
# TASK B: Slot 0 Callback
# ================================================================
print("\n" + "=" * 76)
print("TASK B: Slot 0 Callback at 0x168DA8")
print("=" * 76)

print("\n--- Slot 0 (0x168DA8, 0 hits in 9M runtime events) ---")
for line in dasm_raw(0x168DA8, 288):
    print(line)

print("\n--- Slot 1 (0x168ED0, 591,922 hits) — full disasm ---")
for line in dasm_raw(0x168ED0, 288):
    print(line)

print("\n--- KEY DIFFERENCES ---")
# Read both functions, find diverging instructions
code0 = elf_data[va2off(0x168DA8):va2off(0x168DA8)+288]
code1 = elf_data[va2off(0x168ED0):va2off(0x168ED0)+288]
print(f"\nByte-level comparison:")
print(f"  Identical prologue length: ", end="")
for i in range(min(len(code0), len(code1))):
    if code0[i] != code1[i]:
        print(f"{i} bytes (to 0x{0x168DA8 + i:X})")
        break
print(f"  First divergence at: 0x{0x168DA8 + i:X}")
print(f"    Slot 0 byte: 0x{code0[i]:02X}")
print(f"    Slot 1 byte: 0x{code1[i]:02X}")

# Show divergence zone
print(f"\n  Divergence zone (slot 0 only additions):")
# slot 0 from 0x168E44 (offset in slot 0 = 0x9C from function start)
# slot 1 at same relative position
off0 = va2off(0x168DA8) + 0x9C
off1 = va2off(0x168ED0) + 0x9C
print(f"  Slot 0+0x9C: {elf_data[off0:off0+16].hex()}")
print(f"  Slot 1+0x9C: {elf_data[off1:off1+16].hex()}")

# Find the CALL to 0x166258 in both
print("\n  JAL 0x166258 locations and arg setup:")
for fn_va, fn_name in [(0x168DA8, "Slot 0"), (0x168ED0, "Slot 1")]:
    off = va2off(fn_va)
    code = elf_data[off:off+288]
    for insn in md.disasm(code, fn_va):
        if insn.mnemonic == 'jal' and '0x166258' in insn.op_str:
            print(f"  {fn_name}: JAL 0x166258 at 0x{insn.address:08X}")
            # Show preceding instructions (arg setup)
            pre_off = va2off(insn.address - 20)
            pre_code = elf_data[pre_off:pre_off+24]
            pre_va = insn.address - 20
            for p in md.disasm(pre_code, pre_va):
                print(f"         0x{p.address:08X}: {p.mnemonic:10s} {p.op_str}")

# ================================================================
# TASK C: Scene Loader Helper at 0x13B858
# ================================================================
print("\n" + "=" * 76)
print("TASK C: Scene Loader Helper at 0x13B858")
print("=" * 76)

# Split into 4 sub-functions
print("\n--- 0x13B858: scene_load_set (write 1 to gp-0x4c60) ---")
for line in dasm(0x13B858, 8):
    print(line)

print("\n--- 0x13B864: scene_load_clear (write 0 to gp-0x4c60) ---")
for line in dasm(0x13B864, 8):
    print(line)

print("\n--- 0x13B870: scene_load_get (read gp-0x4c60) ---")
for line in dasm(0x13B870, 8):
    print(line)

print("\n--- 0x13B878: scene_load_init (main body) ---")
for line in dasm(0x13B878, 0x78):
    print(line)

# Callers of the set function
print("\n--- Callers of scene_load_set (0x13B858) ---")
jal_set = struct.pack('<I', 0x0C000000 | (0x13B858 >> 2))
pos = 0
callers = []
while True:
    pos = elf_data.find(jal_set, pos)
    if pos == -1:
        break
    va = pos - FILE_BASE + VA_BASE
    callers.append(va)
    pos += 4

print(f"Found {len(callers)} callers:")
for va in sorted(callers):
    pre_va = max(va - 16, va2off(va) - 16 - FILE_BASE + VA_BASE)
    # Just print the caller context
    off = va2off(va)
    pre_off = max(off - 16, 0)
    pre_va2 = pre_off - FILE_BASE + VA_BASE
    pre = elf_data[pre_off:off+8]
    for insn in md.disasm(pre, pre_va2):
        m = " >>>" if insn.address == va else "    "
        print(f"  {m} 0x{insn.address:08X}: {insn.mnemonic:10s} {insn.op_str}")

# Callers of scene_load_init (0x13B878)
print(f"\n--- Callers of scene_load_init (0x13B878) ---")
jal_init = struct.pack('<I', 0x0C000000 | (0x13B878 >> 2))
pos = 0
init_callers = []
while True:
    pos = elf_data.find(jal_init, pos)
    if pos == -1:
        break
    va = pos - FILE_BASE + VA_BASE
    init_callers.append(va)
    pos += 4

print(f"Found {len(init_callers)} callers:")
for va in sorted(init_callers):
    off = va2off(va)
    pre_off = max(off - 12, 0)
    pre_va = pre_off - FILE_BASE + VA_BASE
    pre = elf_data[pre_off:off+8]
    for insn in md.disasm(pre, pre_va):
        m = " >>>" if insn.address == va else "    "
        print(f"  {m} 0x{insn.address:08X}: {insn.mnemonic:10s} {insn.op_str}")

# Show the called functions
print(f"\n--- Functions called by scene_load_init ---")
for target, desc in [(0x2641D8, "memset(0x6A6DB0, 0, 0x180)"),
                      (0x13CE48, "scene_load_prep"),
                      (0x13CF08, "scene_block_init(0)"),
                      (0x13CF70, "scene_load_step(s1)"),
                      (0x13CF38, "scene_config_setup(s0-0x14)")]:
    print(f"  JAL 0x{target:08X}: {desc}")

# What gp-relative state is used?
print(f"\n--- GP-relative state at 0x13B858 family ---")
for insn in md.disasm(elf_data[va2off(0x13B858):va2off(0x13B858)+0x98], 0x13B858):
    if 'gp' in insn.op_str:
        print(f"  0x{insn.address:08X}: {insn.mnemonic:10s} {insn.op_str}")

print("\nDone.")
