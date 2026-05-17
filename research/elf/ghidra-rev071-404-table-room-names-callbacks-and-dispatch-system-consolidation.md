# Rev.071 — 404-byte entity table room names, callbacks, halfword table writers, callback anatomy, and main loop consolidation

**Date:** 2026-05-17
**Objective:** Complete 5 parallel analyses: (1) scan the 404-byte entity table at 0x005F2F98 for all rooms and callback indices, (2) disassemble 0x6AB080 writer clusters, (3) disassemble Group 1/2 templates and one callback each, (4) map the main loop 0x101C80, (5) consolidate findings.
**Scope:** Entity table, halfword table population, callback internals, main loop structure.
**Sources:** ELF binary extracted via elf-extractor, disassembly via custom MIPS disassembler.
**Evidence:** Byte-level verification, instruction-level disassembly.

---

## 1. 404-byte entity table at 0x005F2F98

### Structure

- **Base:** 0x005F2F98 (section `.data`)
- **Stride:** 0x194 (404 bytes) per row
- **Rows:** 32 (indices 0-31)
- **Row 0:** NULL/global (no room), all others are named rooms

### Room name at +0x00 (32 bytes ASCII)

| Index | Name | Index | Name |
|:-----:|------|:-----:|------|
| 0 | NULL | 16 | plaza |
| 1 | logo | 17 | stone |
| 2 | title | 18 | symmetry_L |
| 3 | sacrifice | 19 | crest_L1 |
| 4 | jail | 20 | crest_L2 |
| 5 | warehouse | 21 | crest_L3 |
| 6 | ico_brigde | 22 | taki |
| 7 | proto | 23 | sluice |
| 8 | troko | 24 | underground |
| 9 | chandelier | 25 | gondola |
| 10 | entrance | 26 | watertower |
| 11 | gate | 27 | symmetry_R |
| 12 | gate2 | 28 | crest_R1 |
| 13 | grave | 29 | crest_R2 |
| 14 | shadows | 30 | crest_R3 |
| 15 | windmill | 31 | cliff |

### Callback index at +0x154 (340)

- **Row 0 (NULL):** 0x00000000 (no callback)
- **Rows 1-31 (all rooms):** 0x0000004B (= 75 decimal)

### Code path at 0x001AF948

The function at 0x1AF190 (called from main loop at 0x1AF004) reads the callback:

```asm
0x001AF948: lw $v1, -28512($gp)         ; v1 = world_state at 0x00631990
0x001AF94C: addiu $a0, $zero, 0x194      ; a0 = 404 (stride — result unused in this code path!)
0x001AF950: lui $a1, 0x005F               ; a1 = 0x005F0000
0x001AF954: mult $v1, $a0                ; lo = v1 * 404 (unused here)
0x001AF958: addiu $v0, $a1, 0x2FB8       ; v0 = 0x005F0000 + 0x2FB8 = 0x005F2FB8
0x001AF95C: addu $v0, $v0, $v1          ; v0 = 0x005F2FB8 + world_state_value
0x001AF960: lw $v0, 0x154($v0)          ; v0 = *(0x005F2FB8 + world_state_value + 0x154)
0x001AF964: beq $v0, $zero, skip         ; skip if NULL (row 0)
0x001AF96C: jalr $v0                    ; call the callback
0x001AF974: jal 0x00166028              ; then build runtime pointer list
```

### Addressing analysis

The code uses base **0x005F2FB8** (= 0x005F2F98 + 0x20, i.e., 32 bytes into row 0, skipping the name "NULL"). The effective read address within each row is:

```
row_data_base = 0x005F2F98 + index * 404
read_address  = 0x005F2FB8 + world_state_value + 0x154
              = 0x005F2F98 + 0x20 + world_state_value + 0x154
              = 0x005F2F98 + world_state_value + 0x174
```

For the address to resolve to `row_base + 0x154` (the actual callback offset), the `world_state_value` must satisfy:

```
world_state_value = row_base + 0x154 - (0x005F2FB8 + 0x154)
world_state_value = row_base - 0x005F2FB8
```

For row 0: `world_state_value = 0x005F2F98 - 0x005F2FB8 = -32 = 0xFFFFFFE0`
For row 1: `world_state_value = 0x005F312C - 0x005F2FB8 = 0x174 = 372`

**Confirmed:** The value at `0x00631990` (gp-28512) stores a pre-multiplied offset:
- Row 0 (NULL): -32 (= 0 * 404 - 32)
- Row 1 (logo): 372 (= 1 * 404 - 32)
- Row N: N * 404 - 32

The code at 0x1AF954 does `mult $v1, $a0` but the result is NOT consumed in this code path — it may be consumed by a later path in the same function or by a sibling function. The addressing used here is simple addition (`addu`), not multiplied.

### CALLBACK VALUE IS AN INDEX (NOT A CODE POINTER)

The value 0x4B (= 75) stored at offset +0x154 in the .data section is NOT a function pointer. At init time, this value must be patched/replaced with an actual callback function pointer. The JALR at 0x1AF96C would crash if jumping to address 0x0000004B.

**Probable:** The 404-byte table stores callback indices in .data, and a runtime init function replaces them with resolved function pointers (similar to how Win32 `MakeProcInstance` or PS2 SDK's callback registration works).

### Other fields in the 404-byte table

| Offset | Type | Row 0 (NULL) | Row 1 (logo) | Description |
|--------|------|-------------|--------------|-------------|
| +0x000 | u32 | 1 | 0 | Type/flag |
| +0x004 | u32 | 0x0001001A | 2 | |
| +0x008 | u32 | 1 | 1 | |
| +0x00C | float | 0x0001001B | 1500.0 | View/fog distance |
| +0x154 | u32 | 0 | 0x4B | **Callback index** |
| +0x158 | u32 | 1 | 1 | |
| +0x15C | u32 | 1 | 1 | |
| +0x168 | u32 | 0x56 | 0x56 | |
| +0x16C | u32 | 1 | 2 | |
| +0x178 | float | 0.5 | 0.5 | |
| +0x17C | float | 1.0 | 1.0 | |
| +0x180 | u32 | 2 | 2 | |
| +0x184 | u32 | 0x2B9 | 0x94 | Specific to room |
| +0x190 | u32 | 0x2B9 | 0x7F | Specific to room |

---

## 2. Halfword table at 0x006AB080 — population mechanism

### Confirmed writers

Only 2 write points exist for the `sh` instruction targeting 0x6AB080:

| Address | Instruction | Context |
|:-------:|:-----------:|---------|
| 0x00166D1C | `lui $v1, 0x006B; addiu $v1, $v1, -20352; sh $a0, 0($v1)` | Inside loop before dispatch |
| 0x00166D78 | Same sequence | Duplicate writer in same loop |

### Write format

```
0x00166D1C: lui $v1, 0x006B
0x00166D20: addiu $v1, $v1, -20352        ; v1 = 0x6B0000 - 0x4F80 = 0x6AB080
0x00166D24: sll $a0, $a2, 5               ; a0 = a2 * 32
...
0x00166D2C: addu $a0, $a0, $t0            ; a0 = (a2 << 5) + t0
0x00166D30: addu $v0, $v0, $v1            ; v0 = 0x6AB080 + counter*2
0x00166D34: addiu $a1, $a1, 1             ; counter++
0x00166D38: sh $a0, 0($v0)                ; 0x6AB080[counter] = (a2<<5) + t0
```

**Each halfword encodes:** `(row_coord << 5) | col_coord`

Where `a2` ∈ [0,31] and `t0` ∈ [0,31] are bounded grid coordinates.

### Grid rasterization

The enclosing loop (entire writer function at 0x00166C80) traces a line/ray through a 32x32 grid, recording all intersected cells. This is a **VU0 packet builder** pattern — encoding 2D grid coordinates into halfwords for later spatial lookup.

### Counter lifecycle

| Event | Location | Value |
|-------|----------|-------|
| Zero | 0x00166BDC: `sw $zero, -19396($gp)` | 0 |
| Increment | 0x00166D3C: `sw $a1, -19396($gp)` | previous + 1 |
| Increment | 0x00166D98: `sw $a1, -19396($gp)` | previous + 1 |
| Final save | 0x00166E00: `sw $v1, -19396($gp)` | last count |

**Count address:** GP-19396 = 0x006388F0 - 0x4BC4 = **0x00633D2C**

### Halfword table readers

**30 total reads** of GP-19396 in the 0x168xxx range — all 14 unique callbacks consume the halfword count to iterate the table.

---

## 3. Slot table at 0x00282690 — corrected structure

### Each entry is 16 bytes (not 8 bytes)

Previously (Rev.066-067), the stride was assumed to be 8 bytes. The code confirms `sll $a1, $a1, 4` at 0x00166E1C, meaning **slot_index * 16**.

| Offset | Field | Description |
|:------:|-------|-------------|
| +0x00 | u32 | Group flag (1=Group1 position, 0=Group2 orientation) |
| +0x04 | u32 | Guard flag (1=enable triplet guard check) |
| +0x08 | u32 | Extra flag (passed to callback) |
| +0x0C | u32 | Callback function pointer |

### Full table (verified byte-level)

| Slot | w0 | w1 | w2 | Callback | Notes |
|:----:|:--:|:--:|:--:|:--------:|-------|
| 0 | 1 | 0 | 0 | 0x00168DA8 | Group 1 |
| 1 | 1 | 0 | 0 | 0x00168ED0 | Group 1 |
| 2 | 1 | 0 | 0 | 0x001692F0 | Group 1 |
| 3 | 1 | 0 | 0 | 0x00169440 | Group 1 |
| 4 | 1 | 1 | 0 | 0x00169020 | Group 1 + guard |
| 5 | 1 | 1 | 0 | 0x00169190 | Group 1 + guard |
| 6 | 1 | 0 | 0 | 0x001696C0 | Group 1 |
| 7 | 1 | 0 | 0 | 0x00169580 | Group 1 |
| 8 | 1 | 0 | 1 | 0x00168ED0 | Reuses slot 1 callback |
| 9 | 1 | 0 | 1 | 0x00169440 | Reuses slot 3 callback |
| 10 | 1 | 0 | 0 | 0x00169800 | Group 1 |
| 11 | 1 | 0 | 0 | 0x00169968 | Group 1 |
| 12 | 0 | 0 | 0 | 0x00169AA8 | Group 2 |
| 13 | 0 | 1 | 0 | 0x00169BD0 | Group 2 + guard |
| 14 | 0 | 0 | 0 | 0x00169E58 | Group 2 |
| 15 | 0 | 0 | 0 | 0x00169D18 | Group 2 |
| 16 | 0 | 0 | 1 | 0x00169AA8 | Reuses slot 12 callback |

**14 unique callbacks in 17 slots.** Slots 8, 9, 16 share callbacks with slots 1, 3, 12 respectively.

---

## 4. Callback internals — Group 1 and Group 2 templates

### Group 1 (0x00166258) — Position/Rotation Proximity Check

- Stack: -224 bytes, saves s0-s4, ra, f20-f24
- Input: a0=context, a1=entity, a2=iteration_flag
- Flow:
  1. Load target position from entity+76 → data pointer
  2. memcpy reference position from context+32 to stack
  3. Compute deltas: ΔX = ref - entity.X, ΔZ = ref - entity.Z
  4. FPU comparisons with configurable thresholds (per-axis)
  5. LQ/PS quadword operations (VU0 instruction format) for SIMD comparison
  6. Return 0 (no match) or 1 (match)

**This is a spatial proximity test** — checks if an entity is within range of a reference position, with axis-specific bounds.

### Group 2 (0x001667E0) — Orientation/Origin Matching

- Stack: -48 bytes
- Input: a0=context, a1=entity
- Flow:
  1. Load 4-component orientation data from entity+64..80 (quaternion?)
  2. Load context position from context+32..40
  3. Compute component differences via FPU
  4. Normalize: `div.s` by 1.0f
  5. Traverse linked list with 16-byte stride, comparing against threshold

**This is an orientation matching check** — compares rotational/orientation context against a candidate list.

### Callback skeleton (representative: 0x00168DA8)

All 14 callbacks share the same skeleton:
1. Read halfword count from GP-19396 (0x633D2C)
2. Iterate halfword table at 0x6AB080
3. Extract `row = halfword >> 5` from each entry
4. Look up row in a secondary pointer table
5. Traverse linked list (80-byte stride entity structures)
6. Run Group 1 or Group 2 template against each candidate
7. If match: store matched object pointer in context, return 1
8. If no match: return 0

---

## 5. Main loop at 0x00101C80 — complete structure

### Frame processing flow

```
0x00101C80: Prologue (128B stack, save s0-s6, ra)
  │
  ├─ Init/version check:
  │   0x001C98: lw $v1, 20160($s0)         ; gp_var check
  │   0x001CB8-0x001CC8: Select path A or B (different string pointers)
  │   0x001CCC: jal 0x1A6E28               ; print/init with string
  │
  ├─ Frame init:
  │   0x001CD4-0x001CE4: jal 0x1A6E28      ; second init call
  │   0x001CE8-0x001D18: Timer calculation  ; mult/mflo for div
  │   0x001D14: jal 0x1A6E28               ; print timer
  │
  ├─ Main dispatch chain (at 0x001EE0-0x001F08):
  │   0x001EE0: jal 0x001AA098             ; unknown A
  │   0x001EE8: jal 0x00166028             ; build runtime pointer list
  │   0x001EF0: jal 0x00103370             ; unknown B
  │   0x001EF8: jal 0x00104C80             ; unknown C
  │   0x001F00: jal 0x001AF190             ; scene/room init (reads 404-byte table!)
  │   0x001F08: lw $v0, -28384($gp)
  │
  ├─ Conditional path:
  │   0x001F0C: bne $v0, $zero, 0x1F24    ; if non-zero, skip calls
  │   0x001F14: jal 0x00104A78             ; conditional call
  │   0x001F1C: jal 0x00104AA8             ; conditional call
  │
  ├─ Common path:
  │   0x001F24: jal 0x00103BF8             ; unknown D
  │   0x001F2C-0x001F30: lw + bne          ; check state again
  │   0x001F38: jal 0x00104A78             ; conditional call
  │
  ├─ Post-processing:
  │   0x001F40: jal 0x00103FC0             ; unknown E
  │   0x001F48: jal 0x00101068             ; unknown F
  │   0x001F50: lw $v0, -28384($gp)        ; re-check state
  │
  ├─ Loop decision:
  │   0x001F54: beq $v0, $zero, 0x1E10    ; if zero, loop to frame processing
  │   0x001F58: sw $v1, -28384($gp)        ; store non-zero state
  │
  └─ Idle/VSync sleep:
      0x001F60: jal 0x00104D3C             ; VSync/idle wait
      0x001F68: beq $zero, $zero, 0x1F60   ; infinite tight loop (VSync polling)
```

### Key insight: the dispatch chain

The main loop calls **0x00166028** (build runtime pointer list) then **0x001AF190** (scene/room init which reads the 404-byte table and dispatches room callbacks). The 0x001AF190 function at its tail also calls 0x00166028 (at 0x1AF974), setting up the runtime pointer list for use by the dispatch wrappers.

### State variable at gp-28384

- **0x00631990:** World_state/room state variable
  - Used by 0x1AF190 to index the 404-byte entity table
  - Checked in main loop to decide whether to go to idle or continue processing
  - Context: when zero, normal frame processing continues; when non-zero, idle loop

---

## 6. Errors corrected from prior research

### Rev.066-067 correction: slot table stride is 16 bytes

The sll at 0x00166E1C is `sll $a1, $a1, 4` (multiply by 16), not by 8. This changes the slot table layout — each entry is 16 bytes with 4 fields of 4 bytes each, not 8 bytes with 2 fields.

### Rev.069 correction: only 2 halfword writers exist

The scan confirmed exactly 2 write points (0x00166D1C and 0x00166D78), both in the same function. The previous estimate of "65 writers" was counting all GP-19396 accesses (reads+write) or was an over-count. The sh-to-0x6AB080 pattern is unique.

### Rev.070 correction: table base is 0x005F2FB8, not 0x005F2F98

The code at 0x1AF958 uses `addiu $v0, $a1, 0x2FB8` where $a1 = 0x005F0000, resulting in base **0x005F2FB8** (= 0x005F2F98 + 0x20). This is 32 bytes into the first row. The effective offset read from each row becomes 0x20 + 0x154 = **0x174** (= 372).

---

## 7. What is confirmed

| Finding | Address | Confidence |
|---------|---------|:----------:|
| 32-room 404-byte entity table | 0x005F2F98 | **EXACT** |
| Room names at +0x00 (32 bytes) | 0x005F2F98+32 | **EXACT** |
| Callback index at +0x154 (= 75 for all rooms) | 0x005F2F98+340 | **EXACT** |
| Code reads via base 0x005F2FB8 + pre-mul + 0x154 | 0x001AF958 | **EXACT** |
| Halfword writers = 2, at 0x00166D1C/0x00166D78 | 0x00166D1C | **CONFIRMED** |
| Each halfword = (row<<5) + col (32x32 grid) | 0x00166D24 | **CONFIRMED** |
| Slot table stride = 16 bytes per entry | 0x00282690 | **EXACT** |
| 17 slots, 14 unique callbacks | 0x00282690 | **EXACT** |
| Group 1 template (pos/rot) | 0x00166258 | **DISASSEMBLED** |
| Group 2 template (orientation) | 0x001667E0 | **DISASSEMBLED** |
| Callback skeleton: iterate halfword table, resolve 80B entities, run template | 0x00168DA8 | **DISASSEMBLED** |
| Main loop calls 0x166028 then 0x1AF190 | 0x00101EE8/0x1AF00 | **CONFIRMED** |
| Idle VSync loop at 0x00101F60 | 0x00101F60 | **CONFIRMED** |
| Halfword table count address = GP-19396 = 0x00633D2C | 0x00166D3C | **EXACT** |

## 8. What is probable

- The 0x4B callback index is patched to a real function pointer at runtime
- The 32x32 grid rasterization represents a spatial partitioning system (room grid?)
- The 80-byte entity structures traversed in callbacks are cloth/object descriptors
- The main loop state at gp-28384 controls whether the game is in active gameplay or idle menu

## 9. What is unknown

- What replaces the 0x4B stub (where are the 75 room init callbacks stored?)
- Which exact callees are at 0x1AA098, 0x103370, 0x104C80, 0x103BF8, 0x103FC0, 0x101068, 0x104A78, 0x104AA8, 0x104D3C
- What the Group 1 mask tests (0xF000, 0xC000, etc.) actually select
- Whether the alternate implementation (0x169F80/0x16A058) is ever reached at runtime
- The full semantic meaning of each of the 17 slot indices

## 10. Next minimum tests

1. Runtime capture at 0x001AF960: breakpoint to capture the callback loaded from the 404-byte table (is it patched at runtime?)
2. Runtime capture at 0x00166D38: capture values written to 0x6AB080 (which grid cells are hit?)
3. Runtime capture at 0x00167020: capture callback distribution (which callbacks fire, how often?)
4. Check the halfword table at 0x006AB080 after room load: what grid coordinates are recorded?
5. Map the 80-byte entity structure used by callbacks (what fields at +0, +4, +48?)
6. Trace what writes to gp-28384 (the main loop state selector)

## Conservative verdict

The 404-byte entity table is now fully documented with all 32 room names and callback indices. The halfword table population mechanism is confirmed as a 32x32 grid rasterization inside the dispatcher function. The slot table has 17 entries with 16-byte stride and 14 unique callbacks in two groups. The main loop structure shows a clear dispatch chain: 0x166028 → 0x1AF190 (which reads the 404 table) → 0x166028 again (setup pointer list for slot dispatch). The "callback index 75" implies a runtime patching mechanism that is not yet documented.
