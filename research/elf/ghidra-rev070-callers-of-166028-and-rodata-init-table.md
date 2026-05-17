# Rev.070 — Callers of `0x00166028` (build runtime pointer list) and rodata debug init table at `0x613E00`

**Date:** 2026-05-17

## Objective

1. Fully disassemble the 3 direct callers of `0x00166028` to understand the scene init chain context:
   - `JAL 0x166028` at `0x101EE8`
   - `JAL 0x166028` at `0x1AF974`
   - `JAL 0x166028` at `0x1B7B50`
2. Investigate the rodata table at `0x613E04` containing `0x00168650` (slot initializer) and the surrounding data.

## Scope

- Static disassembly only
- No runtime, no PCSX2

## Sources used

| Source | Use |
|---|---|
| `research/elf/ghidra-rev067-consolidated-live-dispatch-model.md` | Callers of 0x166028 originally identified |
| `research/elf/ghidra-rev068-naked-init-caller-and-transform-packetizer.md` | 0x1A3208 context, init chain |
| `.local/extracted/SCUS_971.13.elf` | byte-level source |

---

## Function 1: `0x101C80` — Main loop / scene init engine

### Signature

| Property | Value |
|---|---|
| Range | `0x00101C80` – `0x00101F6C` (end of main body; a second function at 0x101F70) |
| Stack | 0x80 (128 bytes) |
| Callee-saves | `$s0`–`$s6`, `$ra` |

### Flow overview

```
main init:
  1. gp-28512 = 0                  (world state)
  2. gp-28480 = 0                  
  3. gp-28448 = 1                  
  4. gp-28416 = 0                  (global flags)
  5. gp-28176 = 0                  (obj_ptr_0)
  6. gp-28400 = 0                  
  7. JAL 0x1AA4E8 → v0            (world state/feature check)
  8. v0 < 104? gp-28432 = v0 (else gp-28432 = 1)
  9. JAL 0x1A3208                  *** NAKED INIT: inits ~70 GP slots with cold paths (0x168650 via a0=0) ***

main loop (0x101E10 backwards branch):
  loop:
  10. JAL 0x13D9C8 (event system init)
  11. JAL 0x13D3F0 (callback pool flush)
  12. Check gp-24768, gp-24764, gp-26976, gp-26856
  13. If gp-24768 == 0:
        JAL 0x1AA098 (scene/entity init)
        JAL 0x00166028           *** BUILD RUNTIME POINTER LIST ***
        JAL 0x103370 (subsystem)
        JAL 0x104C80 (subsystem)
        JAL 0x1AF190 (entity dispatch)
  14. JAL 0x13FBF8 (callback dispatch)
  15. Check gp-28384
  16. JAL 0x13FC00 (slot callback dispatch)
  17. JAL 0x13D3F0 (callback pool flush)
  18. If gp-28176 != 0: set gp-28640=1, loop back to step 10
```

### Key GP variables written

| GP offset | VA | Value |
|-----------|-----|-------|
| -28512 | 0x101D20 | 0 |
| -28480 | 0x101D28 | 0 |
| -28448 | 0x101D2C | 1 |
| -28416 | 0x101D30 | 0 |
| -28176 | 0x101D34 | 0 |
| -28400 | 0x101D3C | 0 |
| -28432 | 0x101D50/0x101D54 | 103 or 1 |
| -28640 | 0x101F58 | 1 (on loop condition) |

### Remarks

- This function contains the **main game loop** (backwards branch at 0x101E10 to 0x101E10)
- `0x00166028` is called only once during the init path (when `gp-24768 == 0`), then the loop runs
- The second function at `0x101F70` has a separate prologue (addiu $sp, -64) — likely a signal handler or exit handler

---

## Function 2: `0x1AF4A0` — Scene / entity subsystem init

### Signature

| Property | Value |
|---|---|
| Range | `0x001AF4A0` – `0x001AF9C4` |
| Stack | 0xB0 (176 bytes) |
| Callee-saves | `$s0`–`$s7`, `$fp`, `$ra` |
| Arguments | `a0` = struct pointer (reads `s5 = [a0+0]` = entity/context pointer) |

### Flow overview

```
start:
  1. s5 = [a0+0]                  (entity/context pointer)
  2. gp-28464 = 1
  3. gp-28528 = 0
  4. JAL 0x1EF9D8                 (subsystem init)
  5. JAL 0x1F10F8                 (subsystem init)
  6. JAL 0x1EB9F0                 (subsystem init)
  7. JAL 0x1D03E0                 (cloth/subsystem init)
  8. Check [s0+12], [s0+16]:
       If both non-null: JAL 0x1AE948 (setup events/callbacks from 0x4B3B28, 0x4B6A90)
  9. Check gp-28432, gp-22852:
       If == 1: JAL 0x144B58(s5, 0) + sb zero, gp-22852
  10. JAL 0x1A6E28 (print at 0x613E18, 0x613E30)
  11. JAL 0x17750
  12. JAL 0x1B7408 (a0=s5)        (entity iteration)
  13. JAL 0x1FBC48                (render/subsystem init)
  14. JAL 0x1A6E28 (print)
  15. gp-19080 = gp-28560
  16. If gp-22848 == 0: JAL 0x1A1030, else JAL 0x1A10B0
  17. gp-22848 = 1
  18. If gp-22844 == 0: 
        JAL 0x1AA5C0(0)
        JAL 0x23D420(-1, s1)
        JAL 0x1333C8(s1, v0, 0)
        JAL 0x1AFE50(0)
        JAL 0x1B0A38()
        gp-22844 = 1
  19. If gp-28512 == 1: JAL 0x1B0A58()
  20. Process entity via table at 0x5F2F98 + entity_id * 404:
        Check field at offset 340 — if non-null, JALR v0 (entity callback)
        *** THIS IS WHERE ENTITY-SPECIFIC INIT CALLBACKS FIRE ***
  21. JAL 0x00166028               *** BUILD RUNTIME POINTER LIST ***
  22. JAL 0x1AE3E8                 (cleanup)
  23. sw zero, [s0+24]
  24. sw zero, [s0+20]
  25. J 0x13D3F8                   (tail call: callback pool flush)
```

### The 404-byte stride entity table at 0x005F2F98

This is a different table from the descriptor table (0x002A31B8, stride 0x64). The code:

```asm
0x1AF94C: addiu $a0, $zero, 404     ; a0 = 404 (0x194)
0x1AF950: lui   $a1, 0x005F
0x1AF954: mult  $v1, $a0            ; v1 = gp-28512 (world state)
0x1AF958: addiu $v0, $a1, 12216     ; v0 = 0x005F2F98
0x1AF95C: addu  $v0, $v0, $v1       ; v0 = 0x005F2F98 + world_state * 404
0x1AF960: lw    $v0, 340($v0)       ; v0 = [0x005F2F98 + world_state * 404 + 340]
0x1AF964: beq   $v0, $zero, +0x10   ; if null → skip to JAL 0x166028
0x1AF968: nop
0x1AF96C: jalr  $v0                 ; call entity callback
0x1AF970: nop
; 0x1AF974: JAL 0x166028             ; fall-through to build ptr list
```

This is a **world-state indexed entity callback table** with stride 404. The entity callback at offset +340 is called if non-null, then `0x166028` builds the runtime pointer list.

### Remarks

- This function is a **scene/subsystem initialization** — it's per-scene init, called from the main loop
- The function ends with a tail call to `0x13D3F8` (callback pool flush)
- Its final act after returning from the tail call is `jr $ra` from `0x13D3F8`

---

## Function 3: `0x1B76F8` — Entry iteration (object creation)

### Signature

| Property | Value |
|---|---|
| Range | `0x001B76F8` – `0x001B7B80` |
| Stack | 0x130 (304 bytes) |
| Callee-saves | `$s0`–`$s7`, `$fp`, `$ra` |
| Arguments | `a0` = buffer/context, `a1` = type_id |

### Flow overview

This is the **entry iteration function** originally documented in Rev.050. Key operations:

```
entry_iter(type_id):
  1. entry = 0x002A4C48 + type_id * 0x4C        (entry table stride 0x4C)
  2. descriptor = 0x002A31B8 + type_idx * 0x64  (descriptor table stride 0x64)
  3. s5 = JAL 0x1AE5F0(descriptor[70], descriptor+100, ...)   (alloc object)
  4. If s5 == NULL → JAL 0x1A6E28(print) + cleanup
  5. s0 = [s5+4] u16                              (entity type)
  6. If s0 != a1 → goto epilogue at 0x1B7B5C     (skip if wrong type)
  
  7. JAL 0x1AE808()                                (init/check)
  8. If type check fails → goto epilogue
  
  9. Process transform data:
     - Load floats from entry[+0..+32]
     - Build stack transform struct (FPU operations)
     - Convert angles, compute rotation
     
  10. If s5 != NULL:
        v0 = [s7+56]   (descriptor+0x38)
        If non-null: JALR v0(a0=sp, a1=s5)       (callback +0x38 in init)
        
  11. Load transform data from s5
  12. JAL 0x240D40(entry+100, type_id, subtype, ...)  (object factory)
  13. s3 = v0
  14. JAL 0x19F310(a0=[sp+128], a1=sp)            (alloc)
  15. [s3+348] = v0 (context)
  16. [v0+2048] = entry[+0x30] (param)
  17. JAL 0x115108(s3, subtype, 1)
  18. v0 = [s0+88] (descriptor+0x58 = hC constructor)
        If non-null: JALR v0(a0=s3, a1=sp)        *** cloth_payload_init etc. ***
        [context+2048] = v0 (payload pointer)
  19. If entry[+0x24] != NULL:
        JAL 0x13F7A8(s3, entry[+0x24], 0, 0x13)  (callback reg with data)
      elif descriptor[+0x40] != NULL:
        JAL 0x13F7A8(s3, descriptor[+0x40], 0, 0x13)
  20. Store globals: if type==1: gp-28172=s3 (boy), if type==2: gp-28168=s3
  21. If s5 != NULL and descriptor[+0x34] != NULL:
        JALR descriptor[+0x34](s3, s5)            (callback +0x34 in post-init)
  22. Store type-based global
  23. JAL 0x182000
  24. JAL 0x1918F0
  25. JAL 0x203CB8
  
  26. JAL 0x00166028               *** BUILD RUNTIME POINTER LIST ***
  
  epilogue:
  27. Restore callee-saves, jr $ra
```

### Confirmation of Rev.050 model

This disassembly confirms the model from Rev.050:
- `JALR [s0+88]` at 0x1B7A80 = `descriptor[+0x58]` = hC constructor
- `JAL 0x13F7A8` at 0x1B7AB0 = callback registration with a3=0x13 (cloth type)
- `JAL 0x1AE6F8` = cloth_event_clear
- `JAL 0x240D40` = object factory
- `JAL 0x166028` is called at the **end of every successful entry iteration**, right before the epilogue

### Call chain

```
0x1B76F8(a0=ctx, a1=type_id)
  ├─ JAL 0x1AE5F0  — alloc object
  ├─ JAL 0x1AE808  — init/check
  ├─ JAL 0x240D40  — object factory
  ├─ JAL 0x19F310  — entity alloc
  ├─ JALR [s0+88]  — hC constructor (cloth_payload_init at 0x1D27A8)
  ├─ JAL 0x13F7A8  — callback registration (a3=0x13)
  ├─ JAL 0x1AE6F8  — event clear
  ├─ JAL 0x182000  — model/entity setup
  ├─ JAL 0x1918F0  — more init
  ├─ JAL 0x203CB8  — more init
  ├─ JAL 0x166028  — build runtime pointer list
  └─ epilogue
```

---

## When is `0x00166028` called?

| Caller | Context | When |
|--------|---------|------|
| `0x101C80` (main loop) | After scene/entity init (`JAL 0x1AA098`) when `gp-24768 == 0` | Once per scene load |
| `0x1AF4A0` (scene init) | After entity callback from 404-stride table at `0x5F2F98 + world_state*404 + 340` | Once per scene init |
| `0x1B76F8` (entry iter) | After creating each entity/object (right before epilogue) | Every entry iteration |

All 3 callers use `0x166028` to **rebuild the runtime pointer list** after entities are created or scene state changes.

---

## The 404-byte stride entity table at `0x005F2F98`

A previously undocumented table with stride **0x194** (404 bytes). It is indexed by `gp-28512` (world state).

| Field | Offset | Use |
|-------|--------|-----|
| entity_callback | +340 (0x154) | Called from `0x1AF4A0` if non-null, then `0x166028` runs |

This is a separate table from:
- Descriptor table at `0x002A31B8` (stride 0x64)
- Entry table at `0x002A4C48` (stride 0x4C)
- Runtime pointer list at `0x006AAC80`

---

## Investigation: rodata init table at `0x613E00`

### Structure

The table at `0x613E00` contains the slot initializer function pointer `0x00168650` plus debug/profiling string names. No LUI+ADDIU code reference to `0x613E00` exists in `.text`. The table is likely part of a **debug build feature** that is compiled into the binary but not actively used, or accessed via an indirect mechanism.

### Strings found

| VA | String |
|:---:|:------|
| `0x00614780` | `CollisionOldProc` |
| `0x00614768` | `Skelton` |
| `0x00614750` | `ClothInfo` |
| `0x00614738` | `FaceWInterpRatio` |
| `0x00614720` | `CharaTarget` |
| `0x00614708` | `MotionActNode` |
| `0x006146F0` | `MotionInterporate` |
| `0x006146D8` | `MotionDebugWin` |
| `0x006146C0` | `MotionDebugTgt` |

These are subsystem/procedure names for a debug overlay or profiling display. The string `ClothInfo` aligns with the cloth physics focus of the project.

### Key observation

The only two static references to `0x00168650` in the entire binary are:
1. `J 0x168650` at `0x1A3334` (inside naked function `0x1A3208`, always passes `a0=0`)
2. Data entry at `0x613E04` in the debug table (no code reference found)

The function pointer is **never loaded from the table** by any known code path. This table is dead except for debug/display purposes.

### Related data table at `0x004B3000`

Contains pointers to `.sdata`/`.sbss` variables (0x0063xxxx range). Also has zero code references found via LUI search. Likely part of the same debug system.

---

## What is confirmed

1. All 3 callers of `0x00166028` are fully disassembled and their contexts understood.
2. `0x101C80` (128B stack) is the **main game loop** — calls `0x166028` once during scene init.
3. `0x1AF4A0` (176B stack) is a **scene/subsystem init** — calls `0x166028` after entity callback from a 404-stride table.
4. `0x1B76F8` (304B stack) is the **entry iteration / object creation** function — calls `0x166028` at the end of every successful entry.
5. A previously undocumented **404-byte stride entity table** exists at `0x005F2F98`, indexed by world state.
6. The rodata table at `0x613E00` contains debug/profiling string names (`ClothInfo`, `CollisionOldProc`, etc.) and the slot initializer `0x00168650`, but has **zero code references** — it's a debug build artifact.
7. The only code path constructing `0x00168650` is the `J` at `0x1A3334` — no LUI+ADDIU pattern exists.

## What is probable

1. The 404-byte stride table at `0x005F2F98` is a per-world-state entity callback table, separate from the descriptor/entry table system.
2. The rodata debug table is a compile-time feature for development/debug builds, not used in the release game but left in the binary.

## What is unknown

1. Who populates the 404-byte stride table at `0x005F2F98`? (Overlay? Another init function?)
2. Whether the rodata debug table at `0x613E00` is ever accessed via indirect runtime-only code.

## Conservative verdict

The scene init chain context for `0x00166028` is now fully mapped:
- It's called whenever the runtime pointer list needs rebuilding (after entity creation, scene init, or state change)
- The 3 callers cover all static paths identified in Rev.067
- No alternate implementation selection path (`a0 != 0`) was found in any of these callers
- The rodata table at `0x613E00` with `0x00168650` is a dead debug table — not relevant for understanding the live dispatch mechanism
- The 404-byte entity table at `0x005F2F98` is a new discovery worth noting but not critical for the current dispatch model

Next step remains **runtime validation** of the dispatch system.
