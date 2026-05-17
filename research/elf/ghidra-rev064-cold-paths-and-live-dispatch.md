# Rev.064 — Live scene init dispatch at 0x00166E10, cold paths, and struct map

**Date:** 2026-05-16

---

## Objective

Disassemble the most-called live function pointer at gp-25856 (0x00167230, 14 JALRs) and resolve the real scene/entity initialization pipeline.

## Scope

- Full disassembly of 0x00166E10 (400B stack, 250+ insns)
- Resolve 0x00167230, 0x00167258, 0x00167270 as cold paths
- Verify 0x00166E10 references (JAL, J, data pointers, GP entries)
- Read gp-25904 default state bytes
- Map all struct offsets used by the function
- Cross-reference against the dead-code model from Rev.059

## Sources used

- ELF SCUS_971.13 USA (`.text` at VA 0x00100000)
- Rev.062 GP-relative data map (GP = 0x006388F0)
- Rev.059 scene loader model (now partially refuted)
- Rev.061 MIPS disassembly framework

## Evidence

### 1. Function split confirmed (EE GCC cold path optimization)

The function pointer at gp-25856 (`0x00167230` with 14 JALRs) is NOT a standalone function. It is a **compiler-split cold entry point**. The actual function layout:

```
0x00166E10: addiu sp, sp, -400   <-- MAIN BODY PROLOGUE
            lui v1, 0x0028
            sd s1, 240(sp)
            ... ~250 instructions ...
0x001671FC: ld ra, 368(sp)        <-- EPILOGUE START
            ld fp, 352(sp)
            ld s7-s0 (restore callee-saves)
            lwc1 f20, 384(sp)
0x00167228: jr ra
0x0016722C: addiu sp, sp, 400     <-- EPILOGUE END (delay slot)

0x00167230: daddu v0, a0, zero    <-- COLD PATH 1 (gp-25856, 14 JALRs)
            sw zero, 176(v0)
            sw zero, 148(v0)
            sw zero, 136(v0)
            ldl v1, -25897(gp)    ; unaligned 64-bit load from gp-25904
            ldr v1, -25904(gp)    ; completes 8-byte load
            sdl v1, 135(v0)       ; store unaligned to [v0+128..135]
            sdr v1, 128(v0)
            j 0x00166E10          ; jump to main body

0x00167258: daddu v0, a0, zero    <-- COLD PATH 2 (gp-25852, 6 JALRs)
            sw zero, 148(v0)      ; clears one field only
            ... same LDL/LDR pair ...
            j 0x00166E10

0x00167270: ...                    <-- COLD PATH 3 (gp-25848?)
            sw ... 
            j 0x00166E10
```

**Key insight:** The three cold paths differ only in which fields they clear:
- Cold path 1 (14 JALRs): clears +176, +148, +136
- Cold path 2 (6 JALRs): clears +148 only
- Cold path 3 (J found): different set

### 2. 0x00166E10 has ZERO static references

| Search method | Result |
|---------------|--------|
| JAL to 0x00166E10 | 0 |
| LUI+ADDIU data pointer | 0 |
| Any 32-bit pointer stored in ELF | 0 |
| J from cold paths | 2 (0x00167250, 0x00167270) |

The main body is **only reachable via J from the cold paths**. External callers load the cold path addresses from `.sdata` function pointer slots and JALR to them. The cold paths prepare state then J (fall-through) to the shared body.

### 3. gp-25904 default state

VA 0x006323C0 (.data):
```
00 00 00 00 FF FF FF FF   = 0xFFFFFFFF00000000
```

| 32-bit half | Signed | Meaning |
|-------------|--------|---------|
| Lower (0x00000000) | 0 | [context+128] = null pointer |
| Upper (0xFFFFFFFF) | -1 | [context+132] = -1 (disabled/no-constraint flag) |

These are written to [a0+128..135] by the unaligned LDL/LDR stores. The cold path initializes the entity's sub-entity pointer to NULL and its index to -1.

Surrounding data:
- gp-25896 (0x006323C8): 0x0000000000000000
- gp-25888 (0x006323D0): 0x0000000000000065 (101 decimal)

### 4. Struct offsets map

#### Context struct (a0 = s1, main entity being initialized)

| Offset | Access | Description |
|--------|--------|-------------|
| +0x000 | lw | From desc array at 0x006AAC00 + a1, slot 0 |
| +0x004 | lw | From desc array, slot 1 |
| +0x008 | lw | From desc array, slot 2 |
| +0x00C | lw | **CALLBACK PTR** — JALR dispatch at 0x00167020 |
| +0x010 | — | s1+16 = secondary stream pointer |
| +0x020 | — | s6 = s1+32 = transform/init stream |
| +0x02C | swc1 | Float (set at 0x00166FE4, 0x00167030) |
| +0x074 | lw | Ptr (compared to s2 entity for matching) |
| +0x078 | lw | Index/type (compared to s0 loop index) |
| +0x07C | lw | Ptr (null-check gate for init) |
| +0x080 | lw | **Sub-entity ptr** (initialized to 0 by cold path) |
| +0x084 | lw | **Index/offset** (initialized to -1 by cold path) |
| +0x088 | lw | Ptr (checked != 0, +76 read for data access) |
| +0x08C | lw | Sub-struct +348 accessed (in alternate path) |
| +0x090 | lw | Index/offset (in alternate path) |
| +0x094 | lw | Ptr (dereferenced to +96) |
| +0x098 | sw | Written value from [+148+96] |
| +0x0A0 | — | Transform matrix start (passed to 0x002438B8) |
| +0x0AC | swc1 | Float at [s0+12] in transform matrix |
| +0x0B0 | sw | Cleared by cold path 1 |

#### Entity descriptor array (at 0x006AAC00, indexed by a1 parameter)

| Offset | Access | Description |
|--------|--------|-------------|
| +0x000 | lw | First field |
| +0x008 | lw | **Count** — loop bound (fp = [s2+8]) |
| +0x00C | lw | **Callback ptr** (loaded at 0x00166E64, used at 0x00167020) |
| +0x070 | sw | Written to gp-19392 |
| +0x074 | lw | Ptr check (gate for null) |
| +0x078 | lw | Ptr (checked in loop body) |
| +0x080 | lw | Ptr (if non-null, fp=1 overriding +8) |
| +0x15C | lw | **Sub-struct** (heavily accessed) |

#### Sub-struct at [entity + 0x15C]

| Offset | Access | Description |
|--------|--------|-------------|
| +0x008 | lw | Count/num (loop bound fallback) |
| +0x00C | lw | Data array base (vertex/animation data) |
| +0x070 | lw | Written to gp-19392 (init data) |
| +0x074 | lw | Gate check |
| +0x078 | lw | Ptr (checked in multiple code paths) |
| +0x080 | lw | Ptr override (forces fp=1) |

### 5. Internal JAL targets

| Target | Use in function | Total ELF calls |
|--------|-----------------|-----------------|
| 0x00243B60 | Init stream (sp, s1+16, s1+32) | ??? |
| 0x00105F00 | Init function (5 calls in body) | **665 JALs** |
| 0x00105E70 | Another init function | ??? |
| 0x00118648 | Transform/init | ??? |
| 0x00166BB0 | Unknown | ??? |
| 0x00105F20 | Init alt function | ??? |
| 0x00105FA8 | Init alt function | ??? |
| 0x002438B8 | Matrix 4x4 transform? | ??? |
| 0x00243950 | Matrix/quaternion? | ??? |

**0x00105F00 note:** First instruction is `LL a2, 0(a1)` (Load Linked — atomic op). No standard prologue (leaf function, ~10 insns?). Called 665 times from across the entire .text — general utility, not scene-init-specific.

### 6. CORRECTION: Rev.059 scene loader model

The Rev.059 model claimed:
- `0x1B76F8` is the **real descriptor iteration function** (object creation loop)
- 0x1B7D00 is the **scene loader** with 4-phase init

**This is WRONG.** Both 0x1B76F8 and 0x1B7D00 are **DEAD CODE** (zero static references anywhere in .text — confirmed Rev.063).

The **REAL** iteration/dispatch is at:
- **0x00166E10** (main body, 400B stack) — iterates entity descriptor array at `0x006AAC00`
- **0x00167230** (cold path 1, gp-25856) — primary entry point, 14 callers
- **0x00167258** (cold path 2, gp-25852) — secondary entry point, 6 callers

The dispatch pattern:
```
[gp-25856 → 0x00167230] (or gp-25852 → 0x00167258)
  → clears fields, loads default state from gp-25904
  → J 0x00166E10 (main body)
    → iterates array at 0x006AAC00 (entity descriptors)
    → for each entry:
      → calls [context+0x0C] JALR callback (per-entity handler)
      → calls 0x00105F00 and other init functions
```

## What is confirmed

- 0x00166E10 = live scene init main body (stack 400B, 20+ callee-saves, entity array iteration)
- 0x00167230/0x00167258 = compiler-split cold entry points (NOT standalone functions)
- gp-25904 = default state `0xFFFFFFFF00000000` (null ptr + -1 flag)
- Entity descriptor array at 0x006AAC00 (NOT 0x1B76F8)
- [context+0x0C] = callback dispatched per entity per slot
- 0x00105F00 = general utility (665 JALs, LL atomic op, not scene-specific)

## What is probable

- Cold path 3 at 0x00167270 exists (J to 0x00166E10 found) — may correspond to another GP slot
- 0x00243B60/0x002438B8/0x00243950 are matrix/quaternion stream operations
- The 0x006AAC00 array is the entity descriptor table (similar to descriptor table at 0x002A31B8 but for scene loading)

## What is unknown

- Who calls the 5th GP function pointer (gp-20896 → 0x001F4C00, 1 JALR)
- Array at 0x006AAC00 size and relationship to descriptor table at 0x002A31B8
- Concrete semantics of +0x00/+0x04/+0x08 descriptor slots (loaded at function entry)
- Whether 0x00166E10 processes array entries from descriptor table or a different table

## What is discarded

- ~~0x001B76F8 is the descriptor iteration function~~ (DEAD CODE, refuted Rev.063-064)
- ~~0x001B7D00 is the scene loader~~ (DEAD CODE)
- ~~0x00167230 is a standalone function~~ (is cold path split)
- ~~0x00105F00 is scene-init-specific~~ (is 665-call general utility)

## Next minimum test

1. Determine the exact relationship between 0x006AAC00 and the descriptor table at 0x002A31B8
2. Add runtime breakpoint at 0x00167230 to capture a0 (context ptr) and verify which entities are dispatched
3. Disassemble 0x00243B60 to understand the "stream init" pattern
4. Verify the true identity of gp-25848 (cold path 3 address)
5. Search for the descriptor array size and layout at 0x006AAC00

## Conservative verdict

**Confirmed:** The live scene initialization pipeline runs through 0x00166E10 (400B stack, entity array iteration at 0x006AAC00, callback dispatch via [context+0x0C]). The entry points are compiler-split cold paths at 0x00167230/0x00167258 (function pointers in `.sdata`). The Rev.059 model (0x1B76F8 as descriptor iteration) was based on dead code and is now refuted. The gp-25904 default state (0/0xFFFFFFFF) is the initial configuration for entity sub-pointers. 0x00105F00 is a general atomic utility function (665 callers) not specific to scene init.
