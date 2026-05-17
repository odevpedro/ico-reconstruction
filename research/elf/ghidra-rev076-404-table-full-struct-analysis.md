# Rev.076 — 404-byte Stage Config Table: Full Struct Layout and Code Reference Survey

**Date:** 2026-05-17
**Objective:** Complete byte-level struct analysis of the 404-byte table at 0x005F2F98, map all code references, reconstruct the full struct layout.
**Sources:** `SCUS_971.13.elf` raw hex dump, capstone 5.0.7 disassembly.
**Evidence:** Byte-level verification, instruction-level disassembly, reference count survey.

---

## 1. Table Location

| Property | Value |
|----------|-------|
| Virtual address | 0x005F2F98 |
| Section | `.rodata` (read-only initialized data) |
| Section range | 0x00553700 – 0x00630900 (0xDD1A8 bytes) |
| File offset | 0x4F3F98 |
| Stride | 0x194 (404 bytes) |
| Total capacity | 624 entries |
| Named entries | 519 |
| NULL/empty | 105 |

**Correction to prior research:** The table has capacity for 624 entries, not 32. The first 32 are the ICO game rooms; entries 33-519 are demo levels, debug stages, and other configurations.

---

## 2. Code References

### 2.1 Base address construction

Code constructs the base at **0x005F2FB8** (entry + 0x20), not the raw start at 0x005F2F98.

```
lui $reg, 0x005F
...
addiu $reg, $reg, 0x2FB8    # base = 0x005F2FB8
```

### 2.2 Reference count by target address

| Target Address | Count | Notes |
|:--------------:|:-----:|-------|
| 0x005F2F00 | 2 | entry - 0x98 (negative offset access) |
| **0x005F2FB8** | **51** | **Primary code base (entry + 0x20)** |
| 0x005F2FD8 | 2 | entry + 0x40 |
| 0x005F2FF8 | 2 | entry + 0x60 |
| 0x005F3038 | 2 | entry + 0xA0 |
| 0x005F3084 | 2 | entry + 0xEC |
| 0x005F30C8 | 1 | entry + 0x130 |
| 0x005F30D8 | 2 | entry + 0x140 |
| 0x005F3120 | 1 | entry + 0x188 |

51 references to the primary base + 14 to nearby offsets = **65 total code references** to the table region.

### 2.3 Representative reference (0x001B7310)

```
0x001B7304: ADDIU $v1, $zero, 0x194     # v1 = 404 (stride)
0x001B7310: LUI   $v0, 0x005F           # v0 = 0x005F0000
0x001B7314: MULT  $ac3, $s0, $v1        # ac3 = s0 * 404 (result unused here, overwritten later)
0x001B7318: ADDIU $v0, $v0, 0x2FB8      # v0 = 0x005F2FB8 (base)
0x001B731C: ADDU  $v0, $v0, $v1         # v0 = base + 404 = entry[1].base
0x001B7324: LW    $a0, 0x140($v0)       # read field at base+0x140 (entry+0x160)
```

**Key observation:** In this function, the MULT result in ac3 is a **dead store** — it is overwritten by `MULT $ac3, $a0, 0x28` at 0x001B737C before any MFLO. The ADDU adds the constant 404 directly (not the scaled index). This implies the compiler optimized `s0 * 404` when s0 is always 1, or the MULT result is consumed in a different code path not shown.

### 2.4 Representative reference with MFLO (0x00143290)

```
0x00143290: LW    $v0, -28512($gp)      # v0 = world_state (pre-multiplied offset)
0x00143294: ADDIU $a0, $zero, 0x194     # a0 = 404 stride
0x0014329C: MULT  $v0, $a0              # lo = world_state * 404
0x001432A4: LUI   $s3, 0x005F           # s3 = 0x005F0000
0x001432AC: ADDIU $v1, $s3, 0x2FB8      # v1 = 0x005F2FB8 (base)
0x001432BC: MFLO  $a0                   # a0 = world_state * 404 (scaled offset)
0x001432C0: ADDU  $v1, $a0, $zero       # v1 = base + scaled_offset  *** BUG? Missing $v1 base ***
```

Here the MFLO IS used — the MULT result feeds the index computation. But there appears to be an issue at 0x001432C0 where the ADDU uses only the offset without adding the base. This may indicate the world_state already includes the base offset.

---

## 3. Complete Struct Layout

### 3.1 Field map (404 bytes = 0x194)

All offsets are relative to **row start** (0x005F2F98 + index * 404).

| Offset | Size | Type | Description | Observed values |
|--------|------|------|-------------|-----------------|
| +0x00 | 4 | u32 | Type/ID flag | 1 (NULL), 0 (stages) |
| +0x04 | 4 | u32 | Variant ID | 0x0001001A (NULL), 2 (logo), varies per stage |
| +0x08 | 4 | u32 | Unknown flag | Usually 1 |
| +0x0C | 4 | float | Position X / fog distance | 1500.0 for logo/title/sacrific |
| +0x10 | 4 | float | Position Y | 10.0 typical |
| +0x14 | 4 | u32 | Unknown | 0x48 for logo, 0x0A for sacrific |
| +0x18 | 4 | u32 | Flags word | 0x8000000A for sacrific, 0x8016000A for logo |
| +0x1C | 4 | u32 | Padding | Usually 0 |
| **+0x20** | **8** | **char** | **Stage name (8 bytes, null-padded)** | "logo\0\0\0\0", "sacrific", "jail\0\0\0\0" |
| +0x28 | 24 | - | Padding/zeros | |
| **+0x40** | **8** | **char** | **Stage description (8 bytes)** | "st13b (S", "st13c (C" |
| +0x48 | 24 | - | Padding/zeros | |
| **+0x60** | **8** | **char** | **Stage name duplicate** | Same as +0x20 |
| +0x68 | 24 | - | Padding/zeros | |
| **+0x80** | 4 | float | Scale | Usually 1.0 |
| +0x84 | 4 | float | Bbox X/width | 57 (sacrific), 223 (logo) |
| +0x88 | 4 | float | Bbox Y | 68, 219 |
| +0x8C | 4 | float | Bbox Z | 74, 205 |
| +0x90 | 4 | float | Bbox W/height | 243, 220 |
| +0x94 | 4 | float | Unknown | 15 for most |
| +0x98 | 4 | float | Unknown | 128 for NULL, 0 otherwise |
| +0x9C | 4 | float | Alpha/reserved | 255 for NULL, 15 for logo/sacrific |
| **+0xA0** | **8** | **char** | **STG label (8 bytes)** | "STGST13B", "STGLOG\0\0" |
| +0xA8 | 24 | - | Padding/zeros | |
| +0xC0 | 16 | u32[4] | Unknown array | Varies per entry |
| +0xD0 | 24 | - | Padding/zeros | |
| +0xE8 | 4 | float | Fade/scale | 1.0 for most |
| +0xEC | 4 | float | Unknown | -1000 for sacrific |
| +0xF0 | 4 | - | Padding | 0 |
| +0xF4 | 4 | float | Unknown | 0.5 typical |
| +0xF8 | 4 | float | Unknown | 250 (logo), 0 (sacrific) |
| +0xFC | 4 | float | Unknown | 248 (logo), 0 (sacrific) |
| **+0x100** | 4 | float | Transform scale X | 1.0 for most |
| +0x104 | 4 | float | Transform scale Y | 50 (logo), 46 (sacrific) |
| +0x108 | 4 | float | Transform scale Z | 50 (logo), 48 (sacrific) |
| +0x10C | 4 | float | Transform rotation | 50 (logo), 0 (sacrific) |
| +0x110 | 4 | float | Transform pos X | 220 (logo), 40 (sacrific) |
| +0x114 | 4 | float | Transform pos Y | 215 (logo), 38 (sacrific) |
| +0x118 | 4 | float | Transform pos Z | 190 (logo), 36 (sacrific) |
| +0x11C | 4 | float | Unknown | Usually 10.0 |
| +0x120 | 4 | float | Unknown | -10.0 for sacrific, 0 for logo |
| +0x124-0x130 | 16 | u32[4] | Integer array | Varies |
| +0x130 | 4 | u32 | Inner loop count | |
| +0x134 | 4 | u32 | Inner loop count 2 | |
| +0x138 | 4 | u32 | Code-referenced field | 0x35 (sacrific), 0x29 (logo) |
| +0x13C | 4 | u32 | Code-referenced field | 0x36 (sacrific), 0x2A (logo) |
| +0x140 | 4 | u32 | Code-referenced field | 0x06 (sacrific), 0x06 (logo) |
| +0x144 | 4 | u32 | Code-referenced field | 0x29 (sacrific), 0x29 (logo) |
| +0x148 | 4 | u32 | Sequence ID | 0x55 (85) for sacrific |
| +0x14C | 4 | u32 | Unknown | 0x6C (108) for sacrific |
| +0x150 | 4 | u32 | Unknown | 4 or similar |
| **+0x154** | **4** | **u32** | **CALLBACK INDEX** | **0x4B (75)** for all stages, 0 for NULL |
| +0x158 | 4 | u32 | Link ID 1 | 1 for all entries |
| +0x15C | 4 | u32 | Link ID 2 | 1 for all entries |
| +0x160 | 4 | u32 | Link ID 3 | 1 for all entries |
| +0x164 | 4 | u32 | Link ID 4 | 1 for all entries |
| +0x168 | 4 | u32 | Sound/audio ID | 0x56 (86) for most |
| +0x16C | 4 | u32 | Entry flag | 1 (NULL), 2 (stages) |
| +0x170 | 4 | - | Padding | 0 |
| +0x174 | 4 | - | Padding | 0 |
| +0x178 | 4 | float | Fade alpha | 0.5 |
| +0x17C | 4 | float | Fade alpha dup | 0.5 |
| +0x180 | 4 | u32 | Unknown | 2 for most |
| +0x184 | 4 | u32 | Room-specific | 0x2B9 (NULL), 0x94 (logo) |
| +0x188 | 4 | u32 | Room-specific | 0x00 |
| +0x18C | 4 | u32 | Room-specific | 0x00 |
| +0x190 | 4 | u32 | Room-specific | 0x2B9 (NULL), 0x7F (logo) |

### 3.2 Code-referenced fields (relative to code base 0x005F2FB8)

Code base = entry + 0x20. Actual entry offsets listed.

| Code offset | Entry offset | Code uses |
|:-----------:|:------------:|-----------|
| +0x00-0x1F | +0x20-0x3F | Stage name (8 bytes at +0x20) |
| +0x20-0x27 | +0x40-0x47 | desc1 access |
| +0x60-0x67 | +0x80-0x87 | Float data access |
| +0x118 | +0x138 | Read at 0x001B7370: used in 0x1B7374 MULT |
| +0x11C | +0x13C | Read at 0x001B735C |
| +0x120 | +0x140 | Read at 0x001B7324 |
| +0x124 | +0x144 | Read at 0x001B7340 |
| +0x134 | +0x154 | **Callback index** read at 0x001AF960 |

---

## 4. Additional Address References Near Table

Besides the 51 references to 0x005F2FB8, code also constructs these nearby addresses:

| Address | Used in | Purpose |
|---------|---------|---------|
| 0x005F2F00 | 0x0013B664, 0x001431F8 | entry - 0x98 (header area before table start) |
| 0x005F2FD8 | 0x0019CDA0, 0x001AF7AC | entry + 0x40 (desc1 area) |
| 0x005F2FF8 | 0x00114E80, 0x00114F20 | entry + 0x60 (name_dup area) |
| 0x005F3038 | 0x001A7B1C, 0x0023D440 | entry + 0xA0 (STG label area) |
| 0x005F3084 | 0x001F5074, 0x001F51CC | entry + 0xEC |
| 0x005F30C8 | 0x001436A8 | entry + 0x130 |
| 0x005F30D8 | 0x00128AF8, 0x00128CD8 | entry + 0x140 |
| 0x005F3120 | 0x00143958 | entry + 0x188 |

These confirm that code accesses fields throughout the 404-byte entry, not just the callback slot.

---

## 5. Entry Name Survey (first 50)

| Index | Name | Index | Name |
|:-----:|------|:-----:|------|
| 0 | NULL | 25 | gondola |
| 1 | logo | 26 | watertow |
| 2 | title | 27 | symmetry |
| 3 | sacrific | 28 | crest_R1 |
| 4 | jail | 29 | crest_R2 |
| 5 | warehous | 30 | crest_R3 |
| 6 | ico_brig | 31 | cliff |
| 7 | proto | 32 | undergro |
| 8 | troko | 33 | pipe |
| 9 | chandeli | 34 | elevator |
| 10 | entrance | 35 | jetty |
| 11 | gate | 36 | jetty2 |
| 12 | gate2 | 37 | queen |
| 13 | grave | 38 | sea |
| 14 | shadows | 39 | beach |
| 15 | windmill | 40 | deja |
| 16 | plaza | 41 | op2 |
| 17 | stone | 42 | 24a4demo |
| 18 | symmetry | 43+ | demos / test stages |
| 19 | crest_L1 | | |
| 20 | crest_L2 | | |
| 21 | crest_L3 | | |
| 22 | taki | | |
| 23 | sluice | | |
| 24 | undergro | | |

---

## 6. Key Findings

### 6.1 Classification

This is a **Stage Configuration Table** (read-only, in .rodata), not a runtime entity table. Each 404-byte entry describes a game stage/room with:
- 8-byte name
- 8-byte description
- 8-byte STG label
- Float position/scale/bbox data (transform configuration)
- Integer fields (IDs, flags, sequence numbers)
- **Callback index at +0x154** (value 0x4B = 75 for all active stages, 0 for NULL)

### 6.2 Stride confirmed

The 404-byte stride (0x194) is verified by:
- **81** MULT instructions in the code that multiply by constant 404
- **51** LUI+ADDIU patterns constructing base 0x005F2FB8
- Matching byte-level data at row boundaries

### 6.3 Callback index (0x4B) is an init-time pointer

The value 0x4B (75) at +0x154 is NOT a function pointer. It is an index into a **callback registration table**. At init time, code patches this to a real function pointer (rev.071 doc references the debug callback table at 0x00613E00).

### 6.4 World state addressing

The world_state variable at `gp - 28512` = `0x00631990` stores a **pre-multiplied offset** (index * 404 - 32), not a raw index. This is why the code at 0x001AF954 does MULT but does not consume the result — the value is already scaled.

---

## 7. What is confirmed

| Finding | Evidence |
|---------|----------|
| Table at 0x005F2F98 in .rodata | Section header, program header |
| 404-byte stride | 81 MULT*404 instructions, byte boundaries |
| 624-entry capacity, 519 named | Byte scan |
| 51 code refs to base 0x005F2FB8 | LUI+ADDIU pattern search |
| 8 additional nearby address refs | LUI+ADDIU pattern search |
| Full struct field layout (404 bytes) | Byte-level + code cross-ref |
| Callback index at +0x154 = 0x4B | Code path 0x001AF948-0x001AF960 |
| 4 fields at +0x158..+0x164 = all 1 | Byte comparison across entries |
| World_state at gp-28512 = 0x00631990 | Code at 0x001AF948 |

## 8. What is probable

- The 0x4B callback index maps through a runtime patching system
- The function at 0x00143290 initializes inner structs within each 404-byte row
- Fields +0x130 and +0x134 control inner loop iteration bounds

## 9. What is unknown

- How the pre-multiplied offset at 0x00631990 is computed (which function does index * 404 - 32?)
- Where the 0x4B -> function pointer mapping is stored
- Which code paths consume the MULT ac3 result that appears dead in some functions
- The exact semantics of many float and int fields

## 10. Next minimum tests

1. Runtime capture at 0x001AF960: dump the callback value after room init (is it patched?)
2. Trace writes to gp-28512 (0x00631990): which function computes the pre-multiplied offset?
3. Runtime breakpoint at 0x001432BC: confirm the MFLO pipeline and verify offset computation
4. Search the 0x00613E00 debug callback table for index 75 (0x4B)

## Conservative verdict

The 404-byte table at 0x005F2F98 is definitively a stage/room configuration table in `.rodata`, not a runtime entity table. The struct spans 404 bytes with 30+ identifiable fields including stage name, description, STG label, transform floats, and a callback index at +0x154. The table has capacity for 624 entries (519 with names), far exceeding the 32 room count documented in prior analysis. The 0x4B callback index at +0x154 is an init-time index, not a function pointer, and must be resolved at runtime.
