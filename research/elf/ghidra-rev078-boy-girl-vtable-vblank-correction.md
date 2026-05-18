# Rev.078 — Final Static Corrections: BOY/GIRL Updates, Vtables, VBlank Mystery

**Date:** 2026-05-17

## Objective

Complete the remaining high-priority static analyses: BOY cb_update (main character controller), GIRL cb_update (Yorda AI), vtable analysis, and VBlank/write-to-counter mechanism for 0x274EC0.

---

## Finding 1: BOY cb_update (0x1C1F58) — Main Character Controller

### Structure (stack -0x60, 80 instructions, 12 JALs)

```
Entity offsets: +0x15C (state_table ptr), +0x644 (distance sensor), +0x648 (interaction ID)
State block [+0x15C → +0x800]: +0x10 (active flag), +0x14 (animation time/result)
```

#### Path A — Active state (state_data->field_10 != 0):
1. `sub_104508` — Animation state machine
2. `sub_102850` — Matrix/skeleton update (tail-calls 0x102790/0x10D830)
3. `sub_10E158` — Heading/rotation (signed angle from 16-bit input)
4. `sub_12A060(-0x1A8, stack, state_data->field_14)` — **Collision/physics query** (-0xA0 stack)
5. Store animation time to state_data->field_14
6. If `world_state_main != 0x27` (cutscene/transition): skip animation time storage

#### Path B — Idle state (state_data->field_10 == 0):
1. `sub_10ECD8` — **NOP** (jr $ra; nop — placeholder stub)
2. `sub_10ECB8` — Idle stub (tail-calls sub-idle chain)
3. `sub_1C1EA8` — **Model transform pipeline** (43 insns, 6 JALs): reads animation frame index, selects bone hierarchy, applies 2 matrix transforms, tail-calls world-matrix finalizer at 0x121D90
4. `sub_1C1250` — **Physics step** (reads state+0x18 model chunk)

#### Interaction trigger (after both paths):
- If `world_state_main == 0x27` (gameplay room):
  - Check `state_table->float_644 < 20.0f` (proximity sensor)
  - If true AND `state_table->int_648 != 0` AND entity not busy:
    - Call `sub_10CB48(buffer, interaction_id, entity)` — send interaction
    - Call `sub_10CF88(buffer)` — interaction response

### BOY cb_routine2 (0x1C1DD8) — Movement/Stance Physics

Stack -0x30, 51 instructions, 9 JALs. Tail-calls 0x103D50:
1. Cloth physics hook (0x1D23E0) — in `clothAnimation.c` range
2. Character frame pre/post (0x1E3FC8, 0x1C12F0, 0x1E4868)
3. Animation state set (0x1BB8C0, args: 0x23, 0x2C, 0x198)
4. Movement query: if entity at rest → trigger sound (0x13FF88 with arg 6)
5. Speed check: 0x14A0D8 returns run (30.0f) vs walk (15.0f)
6. Tail-call movement solver with computed speed and damping (0.7f)

### BOY cb_routine3 (0x1C1A98) — Entity Initializer

Stack -0x50, 107 instructions, 14 JALs. Returns allocated state block ptr:
1. Allocates 0x4C-byte state block (tag 0xFE via allocator 0x13A0F8)
2. Loads **5 model chunks** from 0x4C0000 data region (BOY_model_setup at 0x1C8478)
3. Sets up **3 attributes**: type 1=model root, 0xB=hitbox, 0xC=shadow (via 0x19F310)
4. Calls scene setup (0x1E4798), interaction setup (0x1C1380), cloth collision type 2 (0x1D4B40)
5. Initializes state block: +0x2C=0x14, +0x30=0x14, +0x34=300.0f, +0x38=300.0f, +0x48=0x80808080

---

## Finding 2: GIRL cb_update (0x1D1A98) — Yorda Controller

### Structure (stack -0x20, thin wrapper)

Calls: generic pre (0x10ECD8 → NOP), generic post (0x10ECB8 → idle stub), GIRL-specific (0x1D1580). Tail-calls real body at 0x1D19C0 (stack -0x90).

The real body at 0x1D19C0: sets up transforms/vector ops, then dispatches from a **function table at 0x4C4690** indexed by state_block[0].

### GIRL cb_routine2 (0x1D17F8) — Proximity Follower

Stack -0x50, follows Ico logic:
1. Checks state_block[+0x310] for state == 4 (locked/disabled?)
2. Checks flags at +0x7C and +0x3C8
3. Computes distance to Ico (`gp-0x6e0c` = Ico entity pointer?) via 0x243AE8/0x243950
4. If distance > 10.0f: computes ratio `distance/50.0`, clamps to 0.75, calls 0x10A4E0
5. Calls 0x1D14B8 — interaction/constraint solver

#### Entity offsets used:
- +0x15C: entity_data_ptr
- +0x800: state_block (animation state machine)
- +0x310: state_id (control mode)
- +0x7C: flags
- +0x3C8: permissions
- +0x03: animation state (in cb_routine3)

### GIRL cb_routine3 (0x1D1668) — Animation State Init

Stack -0x40, initializes animation state machine:
1. Reads `entity+0x30` (animation state ID)
2. Stores to state_block[0]
3. If state ID == 1 (standing/idle): loads 3 animation pointers from 0x4C2F50/0x4C3780/0x4C3BF0
4. If state ID == 2 (walking/running): loads 4 animation pointers from 0x4C4550/0x4C0F90/0x4C23C0/0x4C2830

### GIRL cb_routine4 (0x1D1AD8) — Shared Mode Setter [CORRECTION]

**Not a "die" handler.** This is a generic mode-dispatching setter:
```
a1 == 0: state_block[+4]  = a2
a1 == 1: state_block[+12] = a2
a1 == 2: state_block[+24] = a2
otherwise: return immediately
```

Shared by GIRL, ENEMY1, DEVIL_GIRL because they share the same state block layout. ENEMY1 sharing this confirms it's a generic setter, not character-specific.

### DEVIL_GIRL

Index 48, shares ALL routines with GIRL including init_fn=0x174BA0. Confirmed: shadow/dark Yorda uses the exact same behavioral code.

---

## Finding 3: Vtable Correction

**The Rev.077 interpretation was wrong.** The 11 addresses at descriptor table offset +0x60 are NOT vtable pointers — they are **constructor/init functions**. They all begin with a MIPS prologue (`0x27BDFFxx` = `addiu sp, sp, -0xXX`).

| Address | Rev.077 Label | First Word | Verdict |
|---------|--------------|------------|---------|
| 0x202A60 | MainChars | `0x27BDFF10` | Constructor |
| 0x23D660 | PhysicsProps | `0x08080F64` | Jump table / inline switch |
| 0x1956E8 | BIRD | `0x27BDFF60` | Constructor |
| 0x192EB8 | GENERATOR | `0x27BDFF40` | Constructor |
| 0x199A60 | QUEEN | `0x27BDFF40` | Constructor |
| 0x1BB3E0 | AP1 | `0x27BDFF60` | Constructor |
| 0x1BB988 | ATTACKCHECKBOUNDARY | `0x27BDFF60` | Constructor |
| 0x1978B0 | BOSS_CTRL | `0x27BDFE70` | Constructor |
| 0x191C80 | ENEMY_CONTROL | `0x27BDFFE0` | Constructor |

### Real Vtables Found

| Address | Entries | Description |
|---------|---------|-------------|
| 0x2828C0 | 10 | Closest to "main character" vtable — all ptrs in 0x16xxxx range |
| 0x2A3890 | 4 | Adjacent to descriptor table (928 bytes before 0x2A31B8) |
| 0x553760 | 73 | C++ class vtables with many repeated 0x101970 (default/null handler) |
| 0x618FB0 | 5 | **Confirmed cloth dispatcher jump table** (Rev.021 targets) |

The 50+ vtables in 0x55xxxx range are C++ subsystem vtables. The actual BOY/GIRL constructor functions at descriptor+0x60 initialize entity structs with vtable pointers from these regions — identifying which maps to which requires reverse-engineering each constructor.

---

## Finding 4: VBlank Counter Mystery

### Counter at 0x274EC0

The "frame counter" used by `wait_yield(0x203AA0)` is **NOT incremented by any instruction in .text**.

All writers found set it to **only 0, 2, 3, or 4**:
- Function 0x16BA00 writes values from 0x16B3C0 (returns 2 or 4 based on byte at 0x282AC0)
- Three paths write 0 (cleared during init)

There is **no `addiu` or `add` instruction anywhere in .text** that increments this counter by 1 per frame.

### Possible sources (runtime-only investigation):

1. **IOP (I/O Processor)** — writes to EE memory through shared DMA/SIF
2. **EE kernel timer** — timer configured to auto-increment at VSync rate
3. **VBlank interrupt handler** — registered via INTC, runs in kernel context (not visible in .text)
4. **Hardware VSync counter** — the address 0x274EC0 might be special

The `VSyncWait`/`VSyncCallback` functions exist in the EE SDK — the counter is likely updated by a VBlank interrupt service routine registered through the kernel's `AddIntcHandler`. This code would run in kernel mode and is not captured in the ELF's .text section.

### Recommended runtime probe:
```
hwbreak on write to 0x274EC0 → capture EPC (exception PC) + caller chain
```

---

## What Is Confirmed

1. BOY cb_update: two-path controller (active vs idle) at 0x1C1F58 (stack -0x60)
2. BOY model: 5 chunks from 0x4C0000, 3 attributes (root/hitbox/shadow)
3. GIRL cb_update: Yorda follows Ico via proximity constraint (threshold 10.0, clamp 0.75)
4. cb_routine4: **NOT a die handler** — generic mode setter for state block (+4/+12/+24)
5. DEVIL_GIRL: shares ALL routines with GIRL (identical behavior)
6. Descriptor +0x60: **NOT vtables** — constructor/init functions
7. Real vtables: at 0x2828C0 (10 entries), 0x553760 region (C++ subsystem vtables)
8. Counter 0x274EC0: NOT incremented by .text code — likely VBlank ISR or IOP

## What Is Corrected

- Rev.077 "vtable" field → **constructor function pointer**
- Rev.075 "BOY stack -0x160" → **actual stack -0x60**
- "cb_routine4 = die handler" → **generic mode setter**
- "VBlank handler increments 0x274EC0" → **no .text code does this** (runtime question)

## What Still Requires Runtime

1. Who increments 0x274EC0? (VBlank ISR? IOP? Kernel timer?)
2. What are the actual vtable function bodies?
3. Ico entity pointer at gp-0x6e0c?
4. Interaction chain at 0x10CB48/0x10CF88?

---

## Related Files

- `research/elf/ghidra-rev077-final-static-analysis.md`
- `research/elf/ghidra-rev076-post-runtime-consolidation.md`
- `docs/data-model.md`
- `docs/system-feature-flows.md`
- `docs/backlog.md`
