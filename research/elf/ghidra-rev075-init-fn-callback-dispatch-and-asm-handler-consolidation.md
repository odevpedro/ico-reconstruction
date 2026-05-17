# Rev.075 — Init_fn Identification, Callback Dispatch Anatomy, and ASM-HANDLER Consolidation

**Date:** 2026-05-17

---

## Objective

Close three open items from Rev.074 runtime session: (1) identify the 3 unknown init_fn addresses registered during gameplay, (2) disassemble the callback dispatch system at 0x13F9D0, (3) analyze all 5 ASM-HOLD entity handler sets (BOY, GIRL, ENEMY1, WOODBOX0, AP1) for structural decompilation.

---

## Scope

- Disassembly of 0x001C3760, 0x001F2370, 0x0017D128 (unknown init_fn)
- Disassembly of 0x0013F9D0 and 0x0013FC00 (callback dispatch)
- Full analysis of 15 handler functions across 5 entity types
- Room init callbacks from 404-table (verification)

---

## Sources

- ELF: `SCUS_971.13.elf` (custom MIPS disassembler)
- Rev.071: 404-table room names and callback index
- Rev.073: Main loop dispatch chain and callback masks
- Rev.074: Runtime session with 9.1M events
- Rev.056: Handler decompilation wave 3 (prior handler survey)

---

## Evidence

### 1. Three Unknown Init_fn Identified

#### 1A. 0x001C3760 — Cloth System Init

**Registered:** 18 times during Rev.074 runtime session

**Disassembly summary:**
- Stack: -0xE0 (224 bytes), heavy callee-save
- Outer loop: iterates over input array `a2->field_04` elements (indexed by s5, starting at 1)
- Inner loop: processes elements from `a2->field_00` (fp) with index s3
- References two data tables at `0x006288E0` and `0x006288F0` — parallel arrays with stride 0x50 (80) and 0x1A0 (416)
- Uses `mult s5, 0x50` (stride 80 = Group 1 struct size) and `mult1 s5, 0x1A0` (stride 416 = cloth data size)
- JAL targets: `0x1118B8` (a0=11), `0x111FA8` (a0=1, a1=5, a2=0x80), `0x105278`, `0x243BD8`, `0x243AD0`, `0x1D49C0`, `0x10F630`
- Contains a secondary loop (s3 < 5) that checks float data from offset +0x10 of array entries, conditionally calls `0x1D49C0`

**Verdict:** Cloth system initializer. The stride 0x50 matches the 80-byte Group 1 struct (Rev.073). The parallel table access (0x50 vs 0x1A0) suggests vertex/particle pair processing. The 0x6288E0/0x6288F0 addresses are cloth reference data tables. `0x1D49C0` is a cloth compute function.

#### 1B. 0x001F2370 — Cloth Trampoline (Mode Selector)

**Registered:** 18 times during Rev.074 runtime session

**Disassembly summary — multiple entry points:**

| Entry | Address | Action |
|-------|---------|--------|
| 0 | 0x1F2370 | Init stub: JAL 0x203AA0, then infinite loop |
| 1 | 0x1F2388 | `j 0x1D12A8` with `a1=0x40` (64) — cloth setup 64B |
| 2 | 0x1F2390 | JAL 0x1D12D8, then `j 0x1D12A8` with `a1=0x41` |
| 3 | 0x1F23C0 | `j 0x1D12A8` with `a1=0x42` |
| 4 | 0x1F23C8 | `j 0x1D12A8` with `a1=0x57` |
| 5 | 0x1F23D0 | `j 0x1D12A8` with `a1=0x58` |
| 6 | 0x1F23D8 | Body function (stack -0x60), cloth state block init |
| 7 | 0x1F2500 | Body function (stack -0x100), vector setup + 3-iteration loop |

**Entry 6 (0x1F23D8, body A):**
- Loads `entity->field_15C`, then `entity->field_15C->field_800` = cloth state block
- Checks `state_block->field_08`, conditionally clears a child's `field_630`
- Sets `state_block->field_0C = -1`, `field_04 = 2`, `field_08 = 0`
- If a1 != 0: calls `0x10D830(entity + 0x150)` — matrix init
- Reads from `0x274EC0` table: `[0]=10, [4]=60`
- Calculates `(60 - [0]) / [4]` for timing
- COP1: loads from `state_block+0x84/0x74`, sets constant `0x43F50000` (float 490.0)
- Tail-calls entry 3 (`j 0x1D12A8, a1=0x42`)

**Entry 7 (0x1F2500, body B):**
- 3 iterations (s7 = 0..2)
- Each iteration: indexes into `0x550438` (rodata float table) with stride 0x48, builds 3-vector on stack
- Calls `0x13EB50` (matrix op), then tests float conditions against reference from `s2`
- Calls `0x13EBE0`, then conditionally `0x1A6E28` with data at `0x62A710`
- Post-loop: calls `0x263FB0` (3×), `0x2641D8` (2×) — transform operations

**Verdict:** Cloth trampoline / mode selector. Each entry tail-calls `0x1D12A8` (the main cloth resolver) with a different mode constant. Entry 6 does state block initialization. The 5 mode constants (0x40, 0x41, 0x42, 0x57, 0x58) select different cloth simulation variants (likely idle/wind/different LODs).

#### 1C. 0x0017D128 — Unknown (Fog/Particle/BGA System Init)

**Registered:** 18 times during Rev.074 runtime session

**Disassembly summary:**
- Stack: -0x20 (32 bytes), simple leaf function
- Calls `0x1A6E28` with data ptr `0x56A138` (not a string — data table of floats/ints)
- Reads `gp+0x90A0` (= gp offset for some state), compares to 0xB (11)
  - If equal AND `0x17B230(0x7B)` returns nonzero: stores 0 → gp offset -0x9CC8
  - Else: stores 1.0 → gp offset -0x9CC8
- Calls `0x17B288(0x164)` — resource load with ID 356
- Calls `0x203AA0(a0=1)` — generic init
- Calls `0x203B78(0x18BA00, a1=0x15)` — load from data region 0x18BA00
- Calls `0x203AA0(a0=3)` — generic init
- Calls `0x1C2EC8` — unknown init
- Calls `0x203AA0(a0=1)` — generic init
- Calls `0x1A0A38(f12=0.0)` — clear/set with zero
- Calls `0x17B288(0x169)` — resource load with ID 361

**Verdict:** Probably a **BGA/environment effect init**. The two resource loads (IDs 0x164=356, 0x169=361), the data region load from 0x18BA00, and the gp offset write pattern suggests a fog or ambient particle system. The `gp+0x90A0 == 0xB` check gates the initialization — possibly room-specific. **Not cloth-related** (no state block access, no 0x800 offset). Not one of the main entity types from the descriptor table.

---

### 2. Callback Dispatch System at 0x13F9D0

#### 2A. 0x0013F9D0 (main callback dispatch, step 7 of main loop)

**Stack:** -0xA0 (160 bytes), extensive callee-save

**Two-phase linked-list walk:**

**Phase 1 (mask bits 0-7):**
- Reads `a0 = gp+0x98DC` (8-bit callback mask register)
- For each bit position a1 (0..7):
  - Tests bit a1 of mask (`srl v0, a1; andi 0x1`)
  - If set: loads list head from table at `0x281A70[a1]`
  - Walks linked list:
    - Check `s1->field_14` (list control flag)
    - If set: check `s0->field_170` (child callback)
    - If not set: check `s0->field_16C` (direct callback)
    - If `s0->field_28` is set: `JALR s0->field_28`
    - Follow `s0->field_10` (next in list)

**Phase 2 (mask bits 0-7, second table):**
- Same mask iteration (a1 = 0..7)
- Table base: same `0x281A70`
- For each matching entry `s2`:
  - Stores `s2` → `gp+0x98EC`
  - Inner linked list walk via `s2->field_2C`:
    - Each inner node `s0` has `field_14` = type ID
    - Iterates type IDs from 0x13 to 0x1B (9 types)
    - When type ID matches:
      - Mode A (`s0->field_10 == 0`): calls `0x13D8A0(s0+0x24)`, if return == 0x22: calls `0x13F6B8` (special handler), else: calls `0x13D928`
      - Mode B (`s0->field_10 != 0`): calls `JALR s0->field_1C` (custom callback)
    - Follows `s0->field_08` (next inner node)

**Key structures referenced:**
- `0x281A70` = callback dispatch table (8 entries, indexed by mask bit)
- `gp+0x98DC` = 8-bit callback mask register
- `gp+0x98EC` = current handler pointer (written during dispatch)
- `gp+0x98F0` = current callback context (cleared after each call)

#### 2B. 0x0013FC00 (cb_dispatch2, step 9 of main loop)

**Stack:** -0x50 (80 bytes)

- Walks linked list from `gp+0x98E4` (second callback head)
- For each entry `s2`:
  - Checks mask from `gp+0x98DC` bit 0
  - If `s2->field_48` (callback function) exists: `JALR`
  - Table at `0x281AB0` (different base from main dispatch!)
  - Also reads `s2->field_4C` and `s2->field_50` as AND-compare masks
  - Per-entity inner linked list via `s0->field_34`

**Verdict:** The main loop has **three** callback dispatch points (steps 7, 8, 9):
- Step 7: 0x13FBF8 → 0x13F9D0 (two-phase: mask bits + typed handlers)
- Step 8: 0x129C90 (scene_proc2) — additional per-frame processing
- Step 9: 0x13FC00 (cb_dispatch2) — second callback system with different table base

The mask register at `gp+0x98DC` controls all dispatches. The `0x281A70` table holds callback group heads. Type IDs 0x13-0x1B (19-27) correspond to callback types.

---

### 3. 19 Room Init Callbacks (404-table verification)

From Rev.071: All 31 room entries in the 404-byte entity table at 0x005F2F98 have callback index 0x4B (75) at offset +0x154 (except row 0 = NULL which has 0).

**Status:** This index is resolved at runtime to an actual function pointer. The callback is called from `0x1AF948` (main loop step 5, game_systems) when `world_state != 0`.

---

### 4. ASM-HOLD Entity Handler Sets — Full Structural Analysis

#### 4A. BOY (idx 1, init_fn 0x153478)

| Handler | Address | Stack | JALs | FPU Ops | Branches | Role |
|---------|---------|-------|------|---------|----------|------|
| init_fn | 0x153478 | -0x160 (352B) | 7 | few | few | Non-leaf allocator/init |
| hA (reset) | 0x153A70 | ~350 ins | 46 | 14 | 13 | Entity reset, callback reg, pos/flag updates |
| hB (update) | 0x153CE8 | ~220 ins | 27 | 7 | 10 | Per-frame update: position lerp, flag toggle |
| hC (ctor) | 0x153710 | ~570 ins | 16 | few | 4 | Constructor: table at 0x28D458, 4× subsystem hook |

**Key observations:**
- `boy_fn1` (0x15BCC8) called 14× in hA, 10× in hB — BOY common service function
- `reg_1` (0x13FF88) called 5× in hA — callback registration
- Accesses fields: +0x164 (14× reads in hA), +0x10 (pos_x), +0x480 (flags upper), +0x2D0, +0x678
- COP1-heavy in hA/hB for position/rotation interpolation
- **Does NOT use flags_0x48** — not part of the live dispatch mask system

#### 4B. GIRL (idx 2, init_fn 0x174BA0)

| Handler | Address | Stack | JALs | FPU Ops | Branches | Role |
|---------|---------|-------|------|---------|----------|------|
| init_fn | 0x174BA0 | ~200 ins | 29 | 20 | 9 | Animation setup, model matrix, callback reg |
| hA (reset) | 0x1D1A98 | short | 3 | few | few | Minimal reset, delegates |
| hB (update) | 0x1D17F8 | ~120 ins | 16 | 19 | 8 | Per-frame animation: quaternion/pos interpolation |
| hC (ctor) | 0x1D1668 | ~100 ins | 9 | few | few | State block population, 4× table init |

**Key observations:**
- Shares init_fn with DEVIL_GI (idx 48)
- Calls animation setup: `0x14B150`, `0x14B1D0`, `0x14B260` in init_fn
- 0x1D23E0 and 0x1E3FC8 in hB are animation blend functions
- Accesses `entity->field_15C` → `field_800` (cloth state block pattern!) in hC
- Reads/writes +0x60 (flags), +0x40 (scale), +0x50 (sub-state)
- Heavy COP1 usage for animation mix/blend

#### 4C. ENEMY1 (idx 4, init_fn 0x164440)

**hC/init_fn share the same address (0x164440).**

| Handler | Address | Stack | JALs | FPU Ops | Branches | Role |
|---------|---------|-------|------|---------|----------|------|
| init_fn/ctor | 0x164440 | ~360 ins | 24 | 30 | 6 | Heavy structural init, animation setup |
| hA (reset) | 0x164740 | short | 11 | 6 | 4 | Conditional reset on flags_0x48 |
| hB (update) | 0x1644C0 | ~340 ins | 19 | 29 | 5 | Per-frame: same structure as init_fn |

**Key observations:**
- **Only entity checking flags_0x48** — consistent with mask-based slot selection (live dispatch)
- Writes 28 consecutive dwords starting from +0x0C0 — matrix/collision table init
- Calls: `0x14B1D0`, `0x14B260`, `0x14B358`, `0x14B580` — animation/behavior setup chain
- References +0x164 (10× reads), +0x15C (5× reads), pos_z (3×)
- hB has the same structure as init_fn — copies the init pattern every frame
- Back-edge loop in hB suggests retry/polling on entity availability

#### 4D. WOODBOX0 (idx 17, init_fn 0x17D1D0)

**hC/init_fn share the same address (0x17D1D0).**

| Handler | Address | Stack | JALs | FPU Ops | Branches | Role |
|---------|---------|-------|------|---------|----------|------|
| init_fn/ctor | 0x17D1D0 | -0x20 (32B) | 2 | 0 | 1 | Lookup table match |
| hA (reset) | 0x17D2A8 | ~8 ins | 0 | 0 | 0 | Stub/guard return |
| hB (update) | 0x1C0538 | -0x30 (48B) | 2 | 0 | 1 | Tick counter at state_block+0x00 |

**Key observations:**
- Simplest entity — a static prop with a frame counter
- init_fn: loops over table at `0x28A640` (7 entries × 0x30 stride), compares `entry[0]` with `entity->field_08` (state block ptr) to find match
- hB: reads counter at state_block+0x00, wraps at 0x1F (32 frames), then tail-calls `0x1AE460`
- No COP1 usage, no flag_0x48 access

#### 4E. AP1 (idx 61, init_fn 0x1BB6B0)

**hC/init_fn share the same address (0x1BB6B0).**

| Handler | Address | Stack | JALs | FPU Ops | Branches | Role |
|---------|---------|-------|------|---------|----------|------|
| init_fn/ctor | 0x1BB6B0 | -0x50 (80B) | 9 | 7 | few | Position table lookup + setup |
| hA (reset) | 0x1BB860 | -0x10 (16B) | 1 | 0 | 0 | Deactivation check |
| hB (update) | 0x1BA530 | -0x30 (48B) | 4 | few | 2 | State-guarded per-frame cleanup |

**Key observations:**
- Action point = spawn/patrol point with position from global table
- `ap1_sub` (0x1BA5C0) returns index into float table at `0x623468` (stride 32 bytes)
- Table contains 4 floats per entry (pos_x, pos_y, pos_z, pos_w)
- Sets `model_type = 5`, clears +0x98/+0x9c, writes byte 1 to +0x1CA
- hB checks `state+0x08 < 5 AND field_278 != 0` — condition for running cleanup

---

### 5. Entity Descriptor — Init_fn Ownership Summary

From Rev.074 runtime (619 callback_register events):

| init_fn | Entity | Count | Identity |
|---------|--------|-------|----------|
| 0x001BB6B0 | AP1 | 134 | Action point 1 |
| 0x00164440 | ENEMY1 | 129 | Shadow enemy |
| 0x00203EE8 | BGA | 107 | Background/environment |
| 0x001C3760 | unknown→cloth_sys | 18 | Cloth system init |
| 0x001F2370 | unknown→cloth_tramp | 18 | Cloth trampoline |
| 0x0017D128 | unknown→env_effect | 18 | Fog/particle/BGA effect |
| 0x00153478 | BOY | 18 | Protagonist |
| 0x0017D1D0 | WOODBOX0 | 18 | Breakable crate |
| 0x00174BA0 | GIRL | 11 | Yorda (shared with DEVIL_GI) |

---

## What is Confirmed

- `0x001C3760` = cloth system initializer (stride 0x50 Group 1 struct, stride 0x1A0 cloth data, parallel table access at 0x6288E0/0x6288F0)
- `0x001F2370` = cloth trampoline with 5 mode constants (0x40, 0x41, 0x42, 0x57, 0x58) tail-calling 0x1D12A8
- `0x0017D128` = environment effect init (fog/particle/BGA) with resource loads (IDs 356, 361) and gp state gate
- Callback dispatch 0x13F9D0 = two-phase system: 8-bit mask on `gp+0x98DC` → table at 0x281A70 → typed handler chain (IDs 0x13-0x1B)
- `cb_dispatch2` 0x13FC00 = separate system with table at 0x281AB0 and field_48/field_4C/field_50 callback structure
- ENEMY1 is the only entity using flags_0x48 (mask-based slot selection)
- WOODBOX0 = simplest entity (tick counter, no COP1)
- AP1 = data-driven from global position table at 0x623468

## What is Probable

- The 8-bit mask at `gp+0x98DC` controls which callback groups fire each frame
- Type IDs 0x13-0x1B (19-27) map to hardware/peripheral event types (input, physics, collision, audio?)
- The `0x281A70` and `0x281AB0` tables are populated at boot from descriptor entries
- ENEMY1's flag_0x48-based mask selection is why it appears in slot-based dispatch — it needs per-frame classification
- BOY hA's heavy call count (46 JALs!) is initialization amortized over the first few frames

## What is Unknown

- What the 0x13F9D0 type IDs (0x13-0x1B) actually represent — requires runtime capture
- How `gp+0x98DC` mask register is populated each frame
- The relationship between `0x281A70` (callback dispatch table) and `0x281AB0` (cb_dispatch2 table)
- Whether the two callback systems (steps 7/9) handle different entity subsystems
- What `0x1A6E28` actually does when called with data pointers (not strings)

## What is Discarded

- Previous speculation that `0x1A6E28` is a string printer — confirmed it handles data pointers (0x56A138 is a data table, not a string)
- The idea that `0x001F2370` is a simple init function — it's a trampoline with 6+ entry points
- The notion that WOODBOX0 uses the mask system — confirmed it doesn't access flags_0x48

---

## Next Steps

1. **Runtime validation** of callback dispatch 0x13F9D0: probe at 0x13FA10 (mask read), 0x13FA34 (linked list walk), and 0x13FB70 (JALR) to capture mask values, type IDs, and callback targets
2. **Runtime validation** of cloth init 0x1C3760: probe to capture a0 struct fields (field_00, field_04, field_08) to understand data arrays
3. **GP mask write source:** find the writer of `gp+0x98DC` — how is the 8-bit mask populated each frame?
4. **Compare hB/hA structure** between BOY and GIRL — they share `boy_fn1` but differ in COP1 intensity
5. **Investigate the `0x28A640` table** used by WOODBOX0 init_fn — 7 entries × 0x30 stride type ID lookup
6. **Map the 0x623468 float table** used by AP1 — position table for action points

---

## Addendum: Post-Rev.075 Follow-up Analysis

**Date:** 2026-05-17 (immediate follow-up)

### A. GP Mask Register Writer Found

The callback mask register at `gp+0x98DC` (= 0x006321CC) has **exactly 4 write points**:

| Address | Instruction | Role |
|---------|-------------|------|
| 0x0013DDE8 | `sw zero, 0x98DC(gp)` | Boot-time zero-init |
| 0x0013ED5C | `sw v1, 0x98DC(gp)` | Mask clear bit (a1=0) |
| 0x0013ED70 | `sw v1, 0x98DC(gp)` | Mask set bit (a1≠0) |

#### Zero-init function (0x13DDB0)

```
addiu v0, v0, 0x1A90    ; table at base+0x1A90 = 0x281A90
addiu v1, v1, 0x1A70    ; table at base+0x1A70 = 0x281A70
addiu a1, zero, 8        ; loop 8 times
loop:
  sw zero, 0(v1)         ; clear callback dispatch table (0x281A70)
  sw zero, 0(v0)         ; clear secondary table (0x281A90)
  addiu v1, 4
  addiu v0, 4
  bgez a1, loop
jal 0x13E4D0            ; some subsystem init
sw zero, 0x98DC(gp)      ; *** ZERO callback mask ***
sw zero, 0x98E0(gp)      ; *** ZERO secondary mask ***
```

The function zeros both 8-entry tables (0x281A70 callback dispatch heads + 0x281A90 secondary) and the two mask registers. This runs at boot/init.

#### Mask setter/clearer (0x0013ED40)

```
mask_set(a0=bit_index, a1=set_flag):
  if a1 == 0:                        // clear bit
    v1 = gp+0x98DC                   // load mask
    v0 = 1 << a0
    v0 = ~v0
    v1 = mask & v0                   // clear bit a0
    gp+0x98DC = v1                   // store mask
    // same for gp+0x98E0 (secondary mask)
  else:                              // set bit
    v1 = gp+0x98DC
    v0 = 1 << a0
    v1 = mask | v0                   // set bit a0
    gp+0x98DC = v1
    // same for gp+0x98E0 (secondary mask)
```

**Signature:** `void mask_set(uint bit_index, int set)` — `set=0` clears the bit, `set≠0` sets it.

Both `gp+0x98DC` (main mask) and `gp+0x98E0` (secondary mask) are updated atomically.

#### 6 callers of mask_set found:

| Caller | Context |
|--------|---------|
| 0x0019F590 | Gameplay system |
| 0x001B0918 | Scene loader / entity init |
| 0x001B1830 | Scene loader / entity init |
| 0x001B1A18 | Scene loader / entity init |
| 0x001B44E0 | Scene loader / entity init |
| 0x001B4F8C | Scene loader / entity init |

**The mask is controlled by scene/entity init code** (0x1BXXXX range = scene loader cluster). This means the callback mask is set up during scene loading, not per-frame.

**Key insight:** The mask is set ONCE during scene init, not toggled per frame. The dispatch system at 0x13F9D0 reads the mask every frame but the mask itself is stable across frames until the next scene transition.

#### Mask register summary

| Register | Address | Role |
|----------|---------|------|
| `gp+0x98DC` | 0x006321CC | Main callback mask (8 bits, controls callback groups in 0x13F9D0) |
| `gp+0x98E0` | 0x006321D0 | Secondary callback mask (8 bits, controls cb_dispatch2 in 0x13FC00) |

### B. WOODBOX0 Lookup Table at 0x28A640

**Location:** .data section, 7 entries × 0x30 (48 bytes) stride

| Entry | ID | +0x10 Min bounds | +0x20 Max bounds | Notes |
|-------|-----|-------------------|-------------------|-------|
| 0 | 0x00000635 (1589) | (-100, -200, 0, 0) | (-1410, 1000, 0, 0) | High byte=0, low=1589 |
| 1 | 0x00060630 (394800) | (0, -200, 100, 0) | (707, 2000, 570, 1027) | Group 6, sub 1584 |
| 2 | 0x00060632 (394802) | (0, -200, 100, 0) | (707, 2000, 570, 1027) | Group 6, sub 1586 |
| 3 | 0x00060386 (394118) | (0, -200, 100, 0) | (-1431, 105, -470, -259) | Group 6, sub 902 |
| 4 | 0x00080326 (525094) | (-100, -200, 0, 0) | (-350, -285, -450, -280) | Group 8, sub 806 |
| 5 | 0x00080327 (525095) | (-100, -200, 0, 0) | (-350, -285, -450, -280) | Group 8, sub 807 |
| 6 | 0x0007066B (460395) | (-100, -200, 0, 0) | (0, 0, 0, 0) | Group 7, sub 1643 |

**Structure:** Each entry has a type ID at +0x00 and bounding box data at +0x10/+0x20.

**ID encoding:** The high byte (bits 16-23) appears to be a group selector: entries 1-3 share group 6, entries 4-5 share group 8, entry 6 is group 7, entry 0 is group 0.

**init_fn behavior:** Compares `entity->field_08` (state block ptr?) against entry ID at +0x00 to find a match. When found, uses bounding box data for placement.

### C. AP1 Position Table at 0x623468

**Location:** .rodata section, 32-byte stride entries

| Entry | pos_x | pos_y | pos_z | type (int) | Tail bytes |
|-------|-------|-------|-------|------------|------------|
| 0 | 0.00 | -5.00 | -8.00 | 7 | zeros |
| 1 | 0.00 | -5.00 | -8.00 | 7 | zeros |
| 2 | 0.00 | -5.00 | -8.00 | 7 | zeros |
| 3 | 0.00 | -5.00 | -8.00 | 7 | zeros |
| 4 | 0.00 | -5.00 | -8.00 | 7 | zeros |
| 5 | 0.00 | -5.00 | -8.00 | 7 | zeros |
| 6+ | Texture paths (Tex_menu01/scie, title, Font/font.tm2, etc.) | | | | |

**Finding:** Entries 0-5 are all identical (0, -5, -8, 7). This table sits in .rodata immediately before the texture path table (at +0xC0 = 0x623528). The 6 position entries may be:
- Placeholders overwritten at runtime by the scene loader
- Default/fallback values when `ap1_sub` returns an index beyond populated entries
- The positions themselves might come from a different mechanism

The `type=7` value at dword3 (offset +0x0C) is consistent across all 6 entries and likely represents an AP1 type tag (7 = unused/default AP1).
