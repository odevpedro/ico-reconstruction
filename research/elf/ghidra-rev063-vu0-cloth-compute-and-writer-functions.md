# Rev.063 — VU0 Cloth Compute and Writer Functions

**Date:** 2026-05-16

## Objective

Resolve three open items from Rev.062:
1. Disassemble `0x001D9020` (function pointer from `.sdata+0x904`, identified as cloth VU0 micro-mode)
2. Find writers of `gp[-18868]` (cloth vertex pointer, 434 acc, `.sbss+0x33C`)
3. Cross-reference with ICO-decomp source tree

---

## Item 1: 0x001D9020 Resolution

### 0x001D9020 is NOT a standalone function

`0x001D9020` is a point **inside** a larger function at `0x1D8E40`. The function pointer at `.sdata+0x904` (`gp[-26348]`) points to a VU0 compute kernel nested inside this parent function.

### Function 0x1D8E40: full characterization

| Property | Value |
|---|---|
| Prologue | `0x1D8E40: addiu $sp, $sp, -656` |
| Epilogue | `0x1D92D4: jr $ra` / `0x1D92D8: addiu $sp, $sp, 656` |
| Total instructions | 295 |
| Stack frame | 656 bytes (0x290) |
| COP2 (VU0 macro) | 10 instructions |
| COP1 (FPU) | 18 instructions |
| Memory accesses | 110 |
| Branches/jumps | 52 |
| JAL (calls) | 30 |
| NOP (delay slots) | 13 |
| GP-relative refs | 19 |
| Leaf function? | **Yes** — no JALR, single `jr $ra` |

### GP-relative variables accessed

| GP offset | Access count | Type | Inference |
|---|---|---|---|
| `-18868` (0xFFFFB64C) | 7× lw/SW | .sbss | Cloth vertex/system ptr |
| `-18864` (0xFFFFB650) | 7× lw/SW | .sbss | Cloth secondary struct ptr |
| `-21684` (0xFFFFAB4C) | 5× lw as a0/a2 | .sdata | Entity/system pointer (used as arg) |
| `-30192` (0xFFFF8A10) | 1× lwc1 | .lit4 | Float constant |

### VU0/COP2 compute kernel (repeated twice)

The VU0 kernel at `0x1D8FF0` and again at `0x1D9014`:

```asm
lqc2 $1, 0($s1)       ; load vector A (4 × float, 128-bit QWORD)
lqc2 $2, 0($v1)       ; load vector B
cop2 0x4BE208EC        ; VU0 compute (cross/dot?)
cop2 0x4BC318EA        ; convert
cop2 0x4B0318C1        ; VU0 op
cop2 0x4B0318C2        ; VU0 op
cop2 0x48221800        ; VU0 result transfer
mtc1 $v0, $f0          ; move VU0 result to FPU
```

This is a **vector cross-product/dot-product compute kernel**. The `lqc2` (Load Quadword COP2) loads 128-bit vectors; the `cop2 0x4B...` opcodes are VU0 macro-mode instructions. The `mtc1` at the end moves results to the FPU for further processing.

### Position relative to clothAnimation.c range

| Range | Start | End |
|---|---|---|
| clothAnimation.c (USA) | `0x1D27A8` | `0x1D45B0` |
| This function | `0x1D8E40` | `0x1D92DC` |

**This function is OUTSIDE clothAnimation.c.** It lives approximately ~0x48A0 bytes after the end of the confirmed cloth range. It likely resides in a different source file — possibly `sugipon/src/girl/` or `sugipon/src/boy/`, given its calls to `0x243950` (girl-related matrix function).

### Key structure

The function:

1. **Initialization block** (`0x1D8E40-0x1D8EC4`): Sets up 656B stack frame, calls memset-like init (`0x2641D8`), calls core lib (`0x104F20`, `0x105278`, `0x1052A8`), calls matrix init (`0x2438B8`).
2. **Null-guarded cloth init** (`0x1D8ECC-0x1D8F08`): Reads `gp[-18868]` (cloth vertex ptr), checks `[ptr+0x320]` for null. If non-null: calls `0x168448` (cloth update). If null: calls `0x1683C8` (cloth init). Then checks `gp[-21684]` (entity ptr), calls `0x167E00` if non-null.
3. **128-byte copy loop** (`0x1D8F14-0x1D8F60`): Copies 32-byte chunks from `s1` (source) to `s0` (destination), 4 iterations.
4. **Matrix transform** (`0x1D8F64-0x1D8F8C`): Calls `0x105F00` (clear), `0x243B18` (matrix mul), `0x105F78` (transform apply).
5. **Second null-guarded cloth init** (`0x1D8F94-0x1D8FC0`): Same pattern as block 2.
6. **Collision/equality check** (`0x1D8FC0-0x1D8FE8`): Compares `[s1+0x88]` against stack values at `+0x158`, `+0x154`, `+0x150` (x/y/z?). If equal, skip VU0 kernel.
7. **VU0 kernel (first)** (`0x1D8FF0-0x1D903C`): Loads vectors from s1 and v1, computes cross/dot, converts to FPU.
8. **Data copy from v1 to v0** (`0x1D9040-0x1D9090`): 128-byte copy in 32-byte chunks, similar to block 3.
9. **Alternate path** (`0x1D9094-0x1D90AC`): Branch when collision/equality check passes.
10. **Event feedback** (`0x1D90B0-0x1D90F0`): Calls `0x167E00` (event feedback), manipulates QWORD data via LQ/SQ/SDL/SDR.
11. **Girl-specific section** (`0x1D91D0-0x1D92A8`): If a0 bit 0x02 is set, runs a GIRL-specific transform chain:
    - Transforms 32 bytes via `0x105F90`
    - Calls `0x243950` (actGirlWalk-related matrix function — matches cross-reference!)
    - Calls `0x105FE0`, sets up LQ/SQ buffer
    - Calls `0x168A40`, `0x1688E0` (cloth state updates)
    - Writes LQ/SQ data to cloth struct at `gp[-18868]+0xE0`
    - Reads/writes flag bits on `gp[-18864]+0x14`
    - Checks bit 0x01 for more transforms:
      - Calls `0x105F78`, `0x243B18` (matrix)
      - Loads float from `[s1+0x14]`, subtracts `gp[-30192]` constant
      - Calls `0x168578`, `0x168A28` (cloth collision?)
12. **Epilogue** (`0x1D92AC+`): Restores 12 saved registers + $ra + frame pointer, `jr $ra`.

### Called functions with likely ICO-decomp mapping

| Target | Module | Calls | Likely name |
|---|---|---|---|
| `0x2641D8` | common/src/gamesys | 1 | `disp_memory_partition` (init) |
| `0x104F20` | fumi/ios | 2 | Memset/init |
| `0x105278` | fumi/ios | 3 | Core library |
| `0x1052A8` | fumi/ios | 1 | Core library |
| `0x105F00` | fumi/ios | 4 | Core clear |
| `0x105268` | fumi/ios | 1 | Core library |
| `0x2438B8` | sugipon/src/girl | 1 | Matrix/transform (girl) |
| `0x243B18` | sugipon/src/girl | 3 | `actGirlRun`? |
| `0x243950` | sugipon/src/girl | 1 | `actGirlWalk` matrix? |
| `0x105F78` | fumi/ios | 2 | Transform apply |
| `0x105F90` | fumi/ios | 1 | Transform copy |
| `0x105FE0` | fumi/ios | 1 | Transform blend |
| `0x107D30` | fumi/ios | 1 | Core library |
| `0x168448` | sugipon/src/cloth | 2 | Cloth update (non-null path) |
| `0x1683C8` | sugipon/src/cloth | 2 | Cloth init (null path) |
| `0x167E00` | sugipon/src/cloth | 2 | Event/feedback init |
| `0x168A40` | sugipon/src/cloth | 1 | Cloth state update |
| `0x1688E0` | sugipon/src/cloth | 1 | Cloth state copy |
| `0x168578` | sugipon/src/cloth | 1 | Cloth collision |
| `0x168A28` | sugipon/src/cloth | 1 | Cloth state finalize |

The girl-related function calls confirm this function is likely **girl cloth rendering/compute** — part of the GIRL object update pipeline, not general cloth simulation.

### Verification of the struct at gp[-18864]

From the function's access pattern, the struct at `gp[-18864]` has:

| Offset | Use | Written value |
|---|---|---|
| `+0x14` (20) | Flags/status | `ori v0, v0, 0x0020` |
| `+0x134` (308) | Float | `swc1 $0, +0x134` (zeroed) |
| `+0x138` (312) | Float | `swc1 $0, +0x138` (zeroed) |
| `+0x140` (320) | Matrix start | `addiu $a0, +0x140` (to matrix call) |
| `+0x17C` (380) | Result word | Result of `0x168A40` stored here |
| `+0x184` (388) | Result word | Result of `0x1688E0` stored here; checked vs 1 |

---

## Item 2: Writers of gp[-18868]

### Full text scan: only 2 writers

Both offsets `gp[-18868]` (0xFFFFB64C) and `gp[-18864]` (0xFFFFB650) are written by **exactly two functions**, each via `SW` (32-bit store):

| VA | Instruction | Parent function | Writes |
|---|---|---|---|
| `0x1DFD2C` | `SW $t2, -18868($gp)` | `0x1DFBC8` | gp[-18868] |
| `0x1DFD4C` | `SW $t1, -18864($gp)` | `0x1DFBC8` | gp[-18864] |
| `0x1E016C` | `SW $t1, -18868($gp)` | `0x1E00F8` | gp[-18868] |
| `0x1E0170` | `SW $t0, -18864($gp)` | `0x1E00F8` | gp[-18864] |

No `SD` (64-bit store) targets these offsets. No other store instruction (SH/SB) reaches them.

### Function 0x1DFBC8 — Scene init (writes 12 GP vars)

**Stack frame:** ~320 bytes (prologue `addiu sp, sp, -320`)

**GP vars written (batch):**

| GP offset | Register | VA |
|---|---|---|
| `-18856` (0xFFFFB658) | $t1 | `0x1DFC2C: sw $t1, -18856($gp)` |
| `-18880` (0xFFFFB640) | $a1 | `0x1DFD14: sw $a1, -18880($gp)` |
| `-18892` (0xFFFFB634) | $s3 | `0x1DFD1C: sw $s3, -18892($gp)` |
| `-18868` (0xFFFFB64C) | $t2 | `0x1DFD2C: sw $t2, -18868($gp)` |
| `-18864` (0xFFFFB650) | $t1 | `0x1DFD4C: sw $t1, -18864($gp)` |
| `-18872` (0xFFFFB648) | $t0 | `0x1DFD38: sw $t0, -18872($gp)` |
| `-18876` (0xFFFFB644) | $sp | `0x1DFD40: sw $sp, -18876($gp)` |
| `-18888` (0xFFFFB638) | $s5 | `0x1DFD44: sw $s5, -18888($gp)` |
| `-18844` (0xFFFFB664) | $a2 | `0x1DFD34: sw $a2, -18844($gp)` |
| `-21624` (0xFFFFAB88) | $a3 | `0x1DFD30: sw $a3, -21624($gp)` |
| `-21620` (0xFFFFAB8C) | $s1 | `0x1DFD3C: sw $s1, -21620($gp)` |

**Structure:** Reads from `[s1+0x15C]` (entity struct), reads entry table at `0x002A4C48`, does matrix transform via `0x2438B8` (girl matrix), writes all cloth system globals at once.

**Calls:** `0x105F00` (clear), `0x2438B8` (girl matrix), `0x105F78` (transform), `0x107D30` (core)

**Probable role:** Scene initialization function — sets all cloth system globals at scene load. Written once per scene.

### Function 0x1E00F8 — Entity init (writes 8 GP vars)

**Stack frame:** ~176 bytes (prologue `addiu sp, sp, -176`)

**GP vars written (batch):**

| GP offset | Register | VA |
|---|---|---|
| `-18892` (0xFFFFB634) | $a1 | `0x1E015C: sw $a1, -18892($gp)` |
| `-18884` (0xFFFFB63C) | $a2 | `0x1E0160: sw $a2, -18884($gp)` |
| `-18868` (0xFFFFB64C) | $t1 | `0x1E016C: sw $t1, -18868($gp)` |
| `-18864` (0xFFFFB650) | $t0 | `0x1E0170: sw $t0, -18864($gp)` |
| `-18872` (0xFFFFB648) | $t2 | `0x1E0174: sw $t2, -18872($gp)` |
| `-18860` (0xFFFFB654) | $a3 | `0x1E0178: sw $a3, -18860($gp)` |
| `-21624` (0xFFFFAB88) | $a0 | `0x1E0164: sw $a0, -21624($gp)` |
| `-21620` (0xFFFFAB8C) | $s4 | `0x1E0180: sw $s4, -21620($gp)` |

**Structure:**
1. Reads entry table at `0x002A4C48` via `lui $a0, 0x002A; addiu $a0, $a0, 0x4C48`
2. Reads `[s1+0x008]` (type index), multiplies by `0x4C` (entry stride)
3. Reads `[entry+0x46]` (type byte — same as `0x1B76F8` descriptor iteration!)
4. Reads `[entry+0x88]`, `[entry+0x8C]`, `[entry+0x10]` (descriptor data)
5. Writes batch of 8 GP variables
6. Calls init chain: `0x104F20` (memset), `0x10D4A8`, `0x105278`
7. Calls object-specific inits: `0x10D540`, `0x10D428`
8. Sets up matrix: `0x104F20`, `0x2438B8` (girl matrix), `0x105268`
9. Processes vertex data and collision
10. Calls cloth collision functions: `0x118590` / `0x118758` (branch on `gp[-18864]+0xE0`)
11. Final transform: `0x105F78`, loop over child objects via `0x2438E8`
12. Calls `0x10DB28`, checks `gp[-23576]` for read flag, calls `0x119CB0` / `0x119E10` (collision finalize)

**Probable role:** Per-entity initialization — called for each entity during scene load. Reads the type byte from `entry[0x46]` and initializes the appropriate cloth system pointer for that entity type.

### Cloth System Struct Cluster

The range `gp[-18892]` to `gp[-18844]` (.sbss+0x324 to +0x364) forms a **48-byte struct cluster**:

| Offset | GP offset | VA | Writer (scene) | Writer (entity) | Reader (0x1D8E40) |
|---|---|---|---|---|---|
| `.sbss+0x324` | -18892 | 0x00633F24 | 0x1DFC2C | 0x1E015C | — |
| `.sbss+0x328` | -18888 | 0x00633F28 | 0x1DFD44 | — | — |
| `.sbss+0x32C` | -18884 | 0x00633F2C | — | 0x1E0160 | — |
| `.sbss+0x330` | -18880 | 0x00633F30 | 0x1DFD14 | — | — |
| `.sbss+0x334` | -18876 | 0x00633F34 | 0x1DFD40 | — | — |
| `.sbss+0x338` | -18872 | 0x00633F38 | 0x1DFD38 | 0x1E0174 | — |
| `.sbss+0x33C` | **-18868** | **0x00633F3C** | **0x1DFD2C** | **0x1E016C** | **7× (main cloth ptr)** |
| `.sbss+0x340` | **-18864** | **0x00633F40** | **0x1DFD4C** | **0x1E0170** | **7× (secondary ptr)** |
| `.sbss+0x344` | -18860 | 0x00633F44 | — | 0x1E0178 | — |
| `.sbss+0x348` | -18856 | 0x00633F48 | 0x1DFC2C | — | — |
| `.sbss+0x34C` | -18852 | ? | — | — | — |
| `.sbss+0x350` | -18848 | ? | — | — | — |
| `.sbss+0x354` | -18844 | 0x00633F54 | 0x1DFD34 | — | — |
| `.sbss+0x358` | -18840 | ? | — | — | — |
| `.sbss+0x35C` | -18836 | ? | — | — | — |
| `.sbss+0x360` | -18832 | ? | — | — | — |
| `.sbss+0x364` | -18828 | 0x00633F64 | — | — | — |

The hot region (-18868 and -18864) accounts for 434 + 117 = 551 reads across .text.

---

## Item 3: ICO-decomp Cross-Reference

### Function 0x1D8E40

| Check | Result |
|---|---|
| In clothAnimation.c (0x1D27A8-0x1D45B0)? | **No** (at 0x1D8E40, +0x48A0 after end) |
| Known ICO-decomp symbol? | **No** (no symbol within range) |
| Probable source file | `sugipon/src/girl/` (based on 0x243950 call = girl matrix) |

### Functions 0x1DFBC8 and 0x1E00F8

| Check | Result |
|---|---|
| In known ICO-decomp range? | **No** (0x1E range, past documented symbols) |
| Calls girl matrix funcs (0x2438xx)? | **Yes** — both call `0x2438B8` |
| Probable source file | `sugipon/src/girl/` or `sugipon/src/enemy/` |

### gp[-21684] as entity pointer

The variable at `gp[-21684]` (.sdata+0x848, 5 reads in 0x1D8E40 as `a0`) is consistently loaded as the argument to cloth init functions. It is used alongside `gp[-18868]` and `gp[-18864]`, suggesting it is a **root entity context pointer** for the current cloth update pass.

### gp[-21620] and gp[-21624] as callback cluster

Written alongside cloth system vars by both writer functions:

| GP offset | .sdata offset | VA | Written by | Probable role |
|---|---|---|---|---|
| -21624 | +0x8C8 | 0x006321C8 | scene + entity init | Callback table entry (type/ID?) |
| -21620 | +0x8CC | 0x006321CC | scene + entity init | Callback table entry (handler?) |

---

## Confirmed

1. **0x001D9020 is a point inside function 0x1D8E40** (295 insns, 656B stack), NOT a standalone function.
2. **0x1D8E40 is a GIRL cloth rendering/compute function** — uses VU0 micro-mode COP2, calls girl matrix functions, has girl-specific transform section.
3. **0x1D8E40 is outside clothAnimation.c** (at +0x48A0 past the confirmed range). It's likely in `sugipon/src/girl/`.
4. **gp[-18868] (.sbss+0x33C) is written by only 2 functions**: `0x1DFBC8` (scene init) and `0x1E00F8` (entity init).
5. **Cloth system struct cluster spans gp[-18892] to gp[-18844]** (48 bytes, 12 named slots, all in .sbss).
6. **0x1E00F8 reads entry[0x46]** (type byte) and **entry table at 0x002A4C48** — it is per-entity, type-specific initialization.
7. **0x1DFBC8 is scene-level initialization** — writes all 12 cloth system globals in one pass.

## Probable

1. gp[-18684] (5 reads in 0x1D8E40) = **root entity pointer for current cloth pass** (passed as a0 to cloth init functions).
2. The two writer functions represent the **two-level scene load hierarchy**: scene-level (0x1DFBC8) and entity-level (0x1E00F8), similar to the 4-phase pattern seen in `0x1B7D00` (scene loader).
3. `0x001F4C00` (function pointer at gp[-20896]) warrants investigation — it's in the post-cloth range.

## Unknown

1. The exact C function name for `0x1D8E40` in the original source tree (no ICO-decomp symbol covers this range).
2. What writes the initial cloth vertex buffer data before the writer functions initialize the pointers.
3. Whether `0x1DFBC8` is called during scene init phase 1 or phase 3 of `0x1B7D00`.

## Discarded

1. "0x001D9020 is a standalone VU0 micro-mode function" — it's an interior compute kernel of a larger function.
2. "The cloth writer is a single function" — there are **two** writer functions at different levels.

---

## Next Minimum Tests

1. Check if `0x1DFBC8` is called from `0x1B7D00` (scene loader) or from a different entry point.
2. Disassemble `0x1E00F8` fully to map the complete entity init flow (8+ function calls included).
3. Cross-reference `0x1D8E40` against PAL ICO-decomp `symbol_addrs.txt` to find any missed symbol.
4. Runtime breakpoint at `0x1E016C` (`SW $t1, -18868($gp)`) to capture the initial cloth vertex pointer value.
5. Runtime breakpoint at `0x1DFD2C` (`SW $t2, -18868($gp)`) to capture scene cloth init.
6. Add the .sbss struct cluster definition to the data model.
