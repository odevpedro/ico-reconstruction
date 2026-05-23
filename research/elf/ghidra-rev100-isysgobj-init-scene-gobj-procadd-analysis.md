# Rev.100 — initSceneGObj and isysGObjProcAdd_ structural analysis

## Date

2026-05-22

## Objective

Document the internal flow of two key functions:
- `initSceneGObj` (0x1B76F8, 2088B) — bridges scene loader entry table to isysGObj* system
- `isysGObjProcAdd_` (0x13F3F0, 512B) — central TCB/thread registration

## Sources

- ELF binary (SCUS_971.13)
- Byte-exact .s files in src/core/asm/
- Rev.097-098 research notes

## initSceneGObj (0x1B76F8, 2088B)

### Prologue

```
addiu $sp, $sp, -0x130    # stack frame: 304 bytes
addiu $v1, $0, 0x4C       # stride = 0x4C (entry table stride!)
lui   $v0, 0x2A
...
addiu $v0, $v0, 0x4C48    # $v0 = 0x2A4C48 (entry table base)
```

### Body structure

1. **Entry table iteration**: Iterates 512 entries at 0x2A4C48, stride 0x4C.
   - Reads `lbu $a0, 0x46($s4)` = `entry->descriptor_idx`
   - Multiplies by 0x64 (descriptor table stride): `mult $a0, $a2` where $a2 = 0x64
   - Adds to 0x2A31B8 (descriptor table base): `addu $s7, $a2, $a3`
   - Calls 0x1AE5F0 (helper): processes entry

2. **Descriptor lookup**: For each entry, finds the matching descriptor at:
   ```
   descriptor_base = 0x2A31B8
   descriptor = descriptor_base + entry->descriptor_idx * 0x64
   ```

3. **GObj creation**: Calls isysGObj* functions to create GObjs for each scene object.

### Key registers

| Register | Usage |
|----------|-------|
| $s4 | Current entry table pointer |
| $s2 | Descriptor table base (0x2A31B8) |
| $s6 | Entry count/counter |
| $v0 | Temp/calculated address |
| $a2 | Stride 0x64 (descriptor) |
| $v1 | Stride 0x4C (entry) |

### Called functions

- 0x1AE5F0: entry processing helper (~200B)
- 0x1B6A78: GObj allocation/callback registration
- isysGObjAdd: adds GObj to system (type + user data)

## isysGObjProcAdd_ (0x13F3F0, 512B)

### Prologue

```
addiu $sp, $sp, -0x90     # stack frame: 144 bytes
andi  $s6, $a3, 0xFF      # $s6 = type_mask & 0xFF
bne   $s4, $0, ...        # check extra param (t0)
```

### Parameters

| Param | Register | Purpose |
|-------|----------|---------|
| a0 | $s0 | GObj pointer |
| a1 | $s1 | callback_fn |
| a2 | $s2 | callback_type |
| a3 | $s6 | type_id (AND 0xFF) |
| t0 | $s4 | extra_param |
| t1 | $s3 | extra_param |
| t2 | $s5 | extra_param |

### Body structure

1. **Null check**: If `$s4 (t0)` is zero, return 0 immediately.

2. **Slot scan**: Loads thread count from `gp-0x4C44`. If > 0, scans array at `gp-0x4C48` (stride 0x94 = THREAD_STRIDE) for a slot where `node->self == 0` (free).

3. **Allocation**: When free slot found, or all slots full, allocates new TCB.

4. **Init**: Initializes node fields:
   - self = node ptr
   - parent_gobj = a0 (GObj)
   - prev/next = list linkage
   - type_mask = a3 & 0xFF
   - priority = t0
   - active = t1 (?)
   - callback_fn = a1

5. **List insert**: Inserts into GObj's child process linked list at GObj+0x2C, sorted by priority.

### Thread/TCB layout (stride 0x94)

| Offset | Field | Source |
|--------|-------|--------|
| +0x00 | self | node ptr |
| +0x04 | parent_gobj | a0 |
| +0x08 | prev | linked list |
| +0x0C | next | linked list |
| +0x10 | type_mask | a3 & 0xFF |
| +0x14 | priority | t0 |
| +0x18 | active | t1 |
| +0x1C | callback_fn | a1 |
| +0x24-0x93 | stack/init | 0x70 bytes |

## Confirmed

- initSceneGObj bridges 0x2A4C48 (entry) → 0x2A31B8 (descriptor) → isysGObj* system
- isysGObjProcAdd_ = central TCB allocator + registration
- TCB stride 0x94 confirmed from code (addiu $v1, $v1, 0x94)

## Probable

- initSceneGObj processes all 512 entry table entries, filtering by entry flags
- The helper at 0x1AE5F0 handles the actual isysGObjAdd call

## Next steps

- Analyze helper 0x1AE5F0 (entry processing)
- Trace initSceneGObj call chain to understand scene loading order
- Document isysGObjKindTableGetHead (0x13EB50) and GetNext (0x13EBE0)
