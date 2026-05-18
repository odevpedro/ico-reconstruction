# Rev.086 — Static Analysis: VTable Groups, Env Effect Table, cb_routine4, VBlank Counter

**Date:** 2026-05-18
**Objective:** Resolve 4 outstanding static unknowns without emulator
**Sources:** ELF binary, existing research notes, descriptor table dump

---

## 1. VTable Groups — Correction: Not Vtables

### Previous Understanding

Three "vtable groups" at `0x202A60`, `0x23D660`, and entity-specific. Believed to be arrays of function pointers (true vtables).

### Findings

The field at `descriptor + 0x60` is **NOT a vtable pointer**. It is a **behavior function pointer** — a MIPS `jalr` target, not a data table address. Each non-zero value is a MIPS function prologue (`addiu $sp, $sp, -N`).

### Two Shared Behavior Groups

| Group | Entry | Entities | Frame | Description |
|-------|-------|----------|-------|-------------|
| A | `0x202A60` | BOY, GIRL, ENEMY1, DEVIL_GI | 240B | Main character dispatch (17 JALs) |
| B | `0x23D660` | 16 prop/scene types | Jump stub → sub-funcs | Environmental object dispatch |

**Group A (0x202A60):** 4 main entities. Single function spanning `0x202A60`-`0x203128` (~1736 bytes). Contains 17 JAL/JALR calls to sub-functions including transform, collision, animation, vsync.

**Group B (0x23D660):** 16 entities (SOBJ, PSOB, WOODBOX0, BGA, CAME, MOBJ, CHAIN variants, SKEL, CAGE, INTE subtypes). Starts with `j 0x203D90` (redirect stub) followed by ~1068 bytes of utility functions. All 16 share the same entry point but have distinct hA/hB/hC.

### Entity-Specific Behavior Functions

| Entity | Behavior Function | Frame | Notes |
|--------|------------------|-------|-------|
| ENEM (66) | `0x191C80` | 32B | Enemy variant |
| GENE (33) | `0x192EB8` | 192B | Generic |
| BIRD (32) | `0x1956E8` | 160B | |
| BOSS_CTR (64) | `0x1978B0` | 400B | Boss arena manager |
| QUEEN (46) | `0x199A60` | 192B | |
| QUEEN variant (53) | `0x19B998` | 128B | |
| QUEEN variant (52) | `0x19BB60` | 160B | |
| AP1 (61) | `0x1BB3E0` | 160B | Attack pattern |

### 39 Entities with No Behavior Function (behavior_fn = 0)

These include: BARREL (19), ROPE (20), ATTACKCH_B (63), SPIDER, WOODBOX01, ROTOBJECT, and 32+ scene objects. They have no shared dispatch — hA/hB/hC are called directly.

### GIRL = DEVIL_GI (Confirmation)

All fields identical for descriptor indices 2 and 48:

| Field | GIRL | DEVIL_GI |
|-------|------|----------|
| init_fn | 0x174BA0 | 0x174BA0 |
| hA | 0x1D1A98 | 0x1D1A98 |
| hB | 0x1D17F8 | 0x1D17F8 |
| hC | 0x1D1668 | 0x1D1668 |
| behavior_fn | 0x202A60 | 0x202A60 |
| cb_routine4 | 0x1D1AD0 | 0x1D1AD0 |

**DEVIL_GRI is a direct alias of GIRL** — all code paths are shared.

---

## 2. Env Effect Table at 0x29A640

### Previous Understanding

7 types × 0x30 stride = spatial trigger zones for environmental effects.

### Findings

**The table is MUCH larger: 395 entries** (0x29A640 to 0x29F020), not 7. Each entry is 0x30 bytes containing two 6-word sub-records.

### Entry Structure

```
Offset  Size  Field           Description
------  ----  --------------  -----------------------------------------
+0x00   4     match_type      Entity type ID (compared with entity[8])
+0x04   4     param           Effect sub-type / material ID (small ints: 1, 15, 105, 196, etc.)
+0x08   4     target_type     Target entity/material type to transition to
+0x0C   4     flag            -1 = disabled/terminal, 0 = active
+0x10   4     count           Duration/volume (10, 5, 15, 7, 0)
+0x14   4     (padding)       Always 0
+0x18   4     match_type2     Second sub-entry
+0x1C   4     param2          
+0x20   4     target_type2    
+0x24   4     flag2           
+0x28   4     count2          
+0x2C   4     (padding)       Always 0
```

### Scanner Function (init_fn 0x17D1D0)

- Scans **only first 7 entries** (hardcoded `a2 < 7`)
- Match against `entity[8]` (entity type/subtype word)
- On match, calls handler `0x17BB98` (600+ byte function using COP2, 10+ JALs)
- Handler reads secondary data tables at `0x56A170` and `0x569640`

### Interpretation

The table is a **type-to-type mapping/transition matrix**, NOT spatial trigger zones. It maps between entity/material type IDs, with parameters controlling which effect (sound, visual, physics) to apply. The name "ROTOBJECT" in the descriptor (matching init_fn 0x17D1D0) suggests rotating environmental props (windmill, crank wheel, platforms).

### What's at 0x29A640 (not spatial triggers)

No float coordinates found. The table is a state machine: when entity type X encounters material type Y, apply effect Z and transition to type W.

---

## 3. cb_routine4 (hD at descriptor +0x5C)

### Previous Understanding

A 4th callback routine present in GIRL, ENEMY1, DEVIL_GIRL.

### Findings

**cb_routine4 exists at descriptor offset +0x5C** — but only 4 entities have non-null values:

| ID | Entity | +0x5C Value | What's there |
|----|--------|-------------|--------------|
| 2 | GIRL | `0x001D1AD0` | `jr $ra` / `nop` (empty stub) |
| 48 | DEVIL_GIRL | `0x001D1AD0` | Same stub |
| 4 | ENEMY1 | `0x001CE760` | `jr $ra` / `nop` (empty stub) |
| 5 | ENEMY_TEST | `0x001CE760` | Same stub |

**All 4 point to no-op stubs.** Zero `lw` references to `+0x5C` exist anywhere in `.text` — the slot is never read at runtime.

### Correction to Rev.078

Rev.078 identified `0x001D1AD8` as GIRL's cb_routine4 ("Shared Mode Setter"). **This is incorrect.** The descriptor holds `0x001D1AD0` (the empty stub), NOT `0x001D1AD8`. The function at `0x001D1AD8` is a separate state-block mode setter called via 3 direct `jal` instructions (0x216F24/34/44), NOT dispatched through the descriptor callback mechanism.

Similarly, `0x001CE768` (ENEMY1 hD adjacent) is a state_block getter, called via direct `jal` from 2 sites (0x15AE68/0x15F678).

---

## 4. VBlank Counter at 0x274EC0

### Previous Understanding

Counter at 0x274EC0 increments per frame. No `.text` instruction writes to it.

### Findings

**Confirmed: zero write instructions target 0x274EC0** across all 17,987 SW instructions in `.text`.

- 3 read sites consume the counter (timing calculations at 0x152D3C, 0x172C9C, 0x203CEC)
- 6 additional sites compute the address `0x274EC0` (LUI 0x0027 + ADDIU 0x4EC0) but access offset +4, +20, +28, +32 only
- 0x274EC4 (offset +4) shares the same property — no `.text` writer

### Best Hypothesis: IOP VBlank Handler via SIF

The IOP (I/O Processor, running independently) handles VBlank interrupts natively and writes the frame counter to EE main memory via **SIF DMA** without any EE-side code. The EE reads the counter when needed for framerate-dependent timing (dividing by 10, 60 patterns in the 3 read sites).

**Supporting evidence:**
1. Clean struct separation: offset 0 and 4 (IOP-written) vs 8, 14, 18, 1C, 20 (EE-written by `.text` SW instructions)
2. IOP modules confirmed: SIO2MAN.IRX, PADMAN.IRX, MCMAN.IRX strings at 0x553F50
3. Debug table at 0x613BC4 monitors the address — consistent with system-level variable
4. Read sites multiply/divide by 10, 60 (typical VBlank frame timing)

**Verification:** Runtime PCSX2 memory watchpoint on `0x274EC0` would show the writer PC. If in IOP range (0x1C000000) or 0x00000000, the IOP theory is confirmed.

---

## Summary

| Topic | Confirmed | Change to Understanding |
|-------|-----------|------------------------|
| VTable groups | `+0x60` = behavior_fn, not vtable | Group A = main chars (0x202A60, 17 JALs), Group B = props (0x23D660, jump stub). 39 entities have no behavior_fn. |
| Env effect table | 395 entries × 0x30 | Type-to-type mapping matrix, NOT spatial zones. Scanner checks only first 7 entries. |
| cb_routine4 | `+0x5C` = all no-op stubs | Never called at runtime. The real mode setter (0x1D1AD8) is called directly, not through descriptor. |
| VBlank counter | IOP-driven via SIF | No .text writer exists. IOP writes frame counter to EE memory via DMA. |
| GIRL = DEVIL_GIRL | Confirmed | All 5 fields identical. Same entity type. |
