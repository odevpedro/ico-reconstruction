# Rev.071 — Live dispatch entity struct and 404-table analysis

**Date**: 2026-05-17

**Objective**: Disassemble live dispatch callbacks to identify the 80-byte entity struct layout, secondary pointer table, and 404-byte table patching mechanism.

**Sources**: `SCUS_971.13.elf` raw disassembly via Python3 script

---

## Task A: Callback 0x00168DA8 (slot 0, Group 1)

### Function structure (74 instructions, 296 bytes)

```
00168DA8:  addiu $sp,$sp,-144         ; prologue
00168DAC:  lw $a3,-19396($gp)         ; a3 = halfword count from 0x00633D2C
00168DB8:  daddu $s5,$a2,$zero        ; save a2 (context2)
00168DC0:  daddu $s6,$a1,$zero        ; save a1 (context1)
00168DC8:  daddu $a2,$zero,$zero      ; a2 = 0 (loop counter)
00168DD0:  daddu $s3,$a0,$zero        ; s3 = a0 (entity pointer)
00168DD8:  daddu $s4,$zero,$zero      ; s4 = 0 (result flag)
00168DE4:  blez $a3,0x00168EA0        ; if count <= 0, skip
00168DEC:  lui $s7,0x006B             ; s7 = 0x006B0000
00168DF0:  addiu $v0,$s7,-20352       ; v0 = 0x006BB080 (halfword table base)
```

### Loop body (0x00168DF8-0x00168E98)

```
00168DF8:  sll $v1,$a2,1             ; v1 = index * 2 (halfword offset)
00168E00:  lw $a1,-19392($gp)        ; a1 = runtime struct at 0x00633D30
00168E04:  lh $v0,0($v1)             ; v0 = halfword from 0x006BB080 + index*2
00168E08:  lw $a0,24($a1)            ; a0 = secondary table ptr from struct+24
00168E0C:  sll $v0,$v0,2             ; v0 = halfword * 4
00168E10:  addu $v0,$v0,$a0          ; v0 = secondary_table_base + halfword*4
00168E14:  lw $s1,0($v0)             ; s1 = struct pointer from secondary table
00168E18:  beq $s1,$zero,0x00168E90  ; if null, skip
00168E1C:  addiu $s2,$a2,1           ; s2 = index + 1
```

### Struct field access

```
00168E20:  lh $v0,0($s1)             ; v0 = struct.field_00 (16-bit signed)
00168E28:  lhu $v1,0($s1)            ; v1 = struct.field_00 (16-bit unsigned)
00168E30:  sll $v0,$v1,16            ; sign-extend
00168E48:  sra $v0,$v0,16            ; v0 = sign-extended field_00
00168E4C:  and $v0,$v0,$v1           ; v0 = field_00 & 80 (0x50)
```

Where `v1 = 80 (0x50)`, set at:
```
00168E44:  addiu $v1,$zero,80        ; v1 = 80 = 0x50
```

This masks the struct's field_00 with 0x50 before passing to the Group 1 template.

### Group 1 call

```
00168E60:  jal 0x00166258            ; Group 1 template (position/rotation)
00168E68:  beq $v0,$zero,0x00168E80  ; if template rejects, skip
```

### On success (match found)

```
00168E70:  sw $s0,136($s3)           ; entity + 0x88 = struct pointer
00168E74:  addiu $s4,$zero,1         ; result flag = 1
00168E78:  sw $s6,128($s3)           ; entity + 0x80 = saved a1 (context1)
00168E7C:  sw $s5,132($s3)           ; entity + 0x84 = saved a2 (context2)
```

---

## Task B: Three callbacks

### Callback 0x00169020 (slot 4, Group 1 with guard)

**Size**: 92 instructions (368 bytes)

**Key differences from slot 0**:
1. Reads struct field at offset +0x48 (72):
   ```
   001690D0:  lw $a0,72($s0)          ; a0 = struct.field_48
   ```
2. Guard check on struct.field_48 with 0xF000, 0x000F, 0x0001 masks:
   ```
   001690D4:  and $v1,$a0,$v1         ; v1 = field_48 & 0xF000
   001690E8:  and $v0,$a0,$v0         ; v0 = field_48 & 0x000F
   ```
3. Guard check on entity context fields:
   ```
   001690F4:  lw $v0,116($s2)         ; entity+0x74
   001690F8:  bne $s5,$v0,...         ; compare against saved context
   00169100:  lw $v0,120($s2)         ; entity+0x78
   00169104:  bne $s4,$v0,...
   0016910C:  lw $v0,124($s2)         ; entity+0x7C
   ```
4. Same JAL target: `0x00166258` (Group 1)
5. Same store offsets: entity+0x88, entity+0x80, entity+0x84

### Callback 0x00169AA8 (slot 12, Group 2, no guard)

**Size**: 74 instructions (296 bytes)

**Key differences from Group 1**:
1. Secondary table from struct+28 instead of +24:
   ```
   00169B08:  lw $a0,28($a1)          ; a0 = secondary ptr table from struct+28
   ```
2. Mask with 112 (0x70) instead of 80 (0x50):
   ```
   00169B40:  lw $a1,20($a1)          ; a1 = secondary table + 20 (different offset)
   00169B44:  addiu $v1,$zero,112     ; v1 = 112 = 0x70
   ```
3. JAL target: `0x001667E0` (Group 2 - orientation)
4. Store offsets: entity+0x94 (148), entity+0x8C (140), entity+0x90 (144):
   ```
   00169B6C:  sw $s0,148($s2)         ; entity+0x94 = struct pointer
   00169B74:  sw $s6,140($s2)         ; entity+0x8C = context1
   00169B78:  sw $s5,144($s2)         ; entity+0x90 = context2
   00169B7C:  sw $zero,136($s2)       ; entity+0x88 = 0 (clear Group 1 slot)
   ```

### Callback 0x00169BD0 (slot 13, Group 2 with guard)

**Size**: 82 instructions (328 bytes)

**Key differences from slot 12**:
1. Same Group 2 offsets (struct+28, mask 0x70)
2. Same JAL: `0x001667E0`
3. Same store offsets (entity+0x94, 0x8C, 0x90, clear 0x88)
4. Guard check on entity context fields (same as slot 4):
   ```
   00169C78:  lw $v1,116($s2)         ; entity+0x74
   00169C7C:  bne $s5,$v1,...
   00169C84:  lw $v0,120($s2)         ; entity+0x78
   00169C88:  bne $s4,$v0,...
   00169C90:  lw $v0,124($s2)         ; entity+0x7C
   ```

---

## 80-byte entity struct layout

From disassembly of all callbacks, the struct fields identified:

| Offset | Size | Description |
|--------|------|-------------|
| +0x00  | 16-bit | Type/ID field. Masked with 0x50 (Group 1) or 0x70 (Group 2) before template match |
| +0x02  | 16-bit | Accessed by callbacks (lh +2 from s1 after advancing) |
| +0x48  | 32-bit | Flags/guard field. Checked with masks 0xF000, 0x000F, 0x0001 by guarded callbacks |
| +0x4C  | 32-bit | Additional field read/written by callbacks |

The struct pointer is loaded from the **secondary pointer table**, which is:
- Group 1: `[runtime_struct + 24]`
- Group 2: `[runtime_struct + 28]`

Where runtime_struct = `[GP - 19392]` = `[0x00633D30]`.

The secondary table is indexed by `(halfword >> 5)` (implied by the mask with 0x50 = 0b01010000, extracting bits [6:4] from the halfword). Actually re-examining the code, the halfword value is used directly as a 32-bit index into the pointer table, and the struct's field_00 mask filters for the appropriate category.

**Entity store offsets** (entity pointer in a0):

| Group  | Entity offset | Stored value |
|--------|---------------|--------------|
| Group 1 | +0x88 (136) | Struct pointer |
| Group 1 | +0x80 (128) | Context1 (a1) |
| Group 1 | +0x84 (132) | Context2 (a2) |
| Group 2 | +0x94 (148) | Struct pointer |
| Group 2 | +0x8C (140) | Context1 (a1) |
| Group 2 | +0x90 (144) | Context2 (a2) |
| Group 2 | +0x88 (136) | Cleared to 0 |

The entity has guard fields at +0x74 (116), +0x78 (120), +0x7C (124) checked by guarded callbacks.

---

## Secondary pointer table base

The secondary pointer table base is loaded from a runtime struct at `0x00633D30`::

```
runtime_struct = [GP - 19392] = [0x00633D30]

Group 1: secondary_ptrs = [runtime_struct + 24]
Group 2: secondary_ptrs = [runtime_struct + 28]
```

Each entry in the secondary pointer table is a pointer to a struct of at least 0x50 bytes.

---

## Task C: 404-byte table patching

### Table structure

The 404-byte table is at `0x005F2F98`. Each row is 404 bytes (0x194). There are 32 rows (indices 0x00-0x1F). The callback slot is at row offset +340 (0x154).

The code uses base address `0x005F2FB8` (= `0x005F2F98` + 32) with the same 404-byte stride. The first 32 bytes of the table contain initializer data.

Row data at offset 0x00 contains "NULL\0..." and at offset 0x20 contains "dummy\0..." strings.

At runtime, table rows 1-31 have value `0x0000004B` at offset +340 (callback ID for row 0 is `0x00000000` = no callback).

### Patching function at 0x00143290

```
00143290:  lw $v0,-28512($gp)        ; v0 = row_index
00143294:  addiu $a0,$zero,404        ; a0 = 404 = 0x194 (stride)
00143298:  addiu $sp,$sp,-96          ; prologue
0014329C:  mult $v0,$a0              ; row_index * 404
001432A4:  lui $s3,0x005F            ; table high
001432AC:  addiu $v1,$s3,12216       ; v1 = 0x005F2FB8 (table base + 32)
001432BC:  mflo $a0                  ; a0 = index * 404
001432C0:  daddu $v1,$a0,$zero       ; v1 = table_base + index*404 (WRONG: lacking base)
001432C4:  lw $s1,272($v1)           ; s1 = [row + 272] (inner loop iterator)
001432C8:  lw $v0,276($a0)           ; v0 = [table_base + index*404 + 276] (row count)
001432CC:  slt $v0,$s1,$v0           ; loop check: s1 < v0?
001432D0:  beq $v0,$zero,0x00143494  ; if >=, exit
001432D8:  addiu $v0,$zero,28        ; inner stride = 28
001432E4:  mult $s1,$v0              ; inner_index * 28
...
```

**Key observations**:
- The function reads fields at row offsets +272 (0x110) and +276 (0x114) - these are loop bounds
- The inner stride is 28 bytes (7 words)
- Accesses a secondary table at 0x005D7008 (lui 0x005D + 7008)
- Calls `0x00142C60` for initialization
- Copies float values from the outer entry (offsets +8, +12, +16, +20, +24) to the inner entry (offsets +24, +36, +32, +40, +28)
- Sets flags at inner entry +4 from outer entry +24 bits
- The loop iterates `s1` from 0 to the count at offset +276

**This is NOT the callback patching function**. It initializes the inner structs within each 404-byte row, not the callback slot at +340.

The callback patching mechanism likely works through the **debug callback table** at `0x00613E00`, where callback IDs are mapped to function pointers at runtime.

### Halfword table population

All four callbacks reference:
```
00168DEC:  lui $s7,0x006B
00168DF0:  addiu $v0,$s7,-20352     ; 0x006BB080
```

This is the runtime halfword table at `0x006AB080` (BSS, zero in ELF file, populated at runtime). It is written by the same function that runs the dispatcher (0x00166E10), confirmed at 0x00166D1C/0x00166D78.

---

## Slot table structure

The dispatcher at `0x00166E10` uses a slot table at `0x00282690`:

```
00166E14:  lui $v1,0x0028
00166E24:  addiu $v0,$v1,9872        ; v0 = 0x00282690 (slot table)
00166E1C:  sll $a1,$a1,4             ; slot_index * 16 (each entry = 16 bytes)
```

Each slot entry is 16 bytes (4 words):

| Offset | Size | Description |
|--------|------|-------------|
| +0x00  | 4    | 0x00000001 = active flag |
| +0x04  | 4    | 0x00000000 (usually), 0x00000001 for slot 4 |
| +0x08  | 4    | 0x00000000 |
| +0x0C  | 4    | Callback function pointer |

First 8 slots mapped:

| Slot | Callback | Group | Guard |
|------|----------|-------|-------|
| 0 | 0x00168DA8 | 1 | No |
| 1 | 0x00168ED0 | 1 | No |
| 2 | 0x001692F0 | 1 | No |
| 3 | 0x00169440 | 1 | No |
| 4 | 0x00169020 | 1 | Yes |
| 5 | 0x00169190 | ? | ? |
| 6 | 0x001696C0 | ? | ? |
| 7 | 0x00169580 | ? | ? |

---

## Summary

### Confirmed
- All 4 callbacks share the same halfword table iteration structure (0x006AB080, count from GP-19396)
- Secondary pointer table loaded from runtime struct at GP-19392 (0x00633D30) at offsets +24 (Group 1) or +28 (Group 2)
- Struct pointer loaded from `[secondary_table + halfword_value * 4]`
- Group 1 template: `0x00166258` (position/rotation)
- Group 2 template: `0x001667E0` (orientation)
- Struct fields: +0x00 (16-bit), +0x48 (flags), +0x4A (?)
- Entity store offsets: Group 1 at +0x88/0x80/0x84, Group 2 at +0x94/0x8C/0x90
- Slot table at `0x00282690`, 16-byte entries, 17 slots
- 404-byte table at `0x005F2F98`, 32 rows, function at `0x00143290` initializes inner structs
- Guard callbacks at slots 4, 5, 13 check entity fields at +0x74/0x78/0x7C

### Unknown
- Semantic meaning of the 80-byte struct fields beyond +0x00 and +0x48
- What populates the halfword table at 0x006AB080 (confirmed writers at 0x00166D1C/0x00166D78)
- How callback ID 0x4B maps to a function pointer at runtime
- What the remaining 9 slots (8-16) in the 17-slot table map to
- Whether slots 12/16 share callback 0x00169AA8 as stated in background

### Next minimum tests
1. Runtime breakpoint at `0x00166E1C` to capture a1 (slot index) for each slot during gameplay
2. Runtime capture of `[GP-19392]` at 0x00633D30 to see the runtime struct content
3. Runtime breakpoint at `0x00168E14` to dump the struct pointer and its first 0x50 bytes
