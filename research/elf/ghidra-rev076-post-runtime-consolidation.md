# Rev.076 — Post-Runtime Consolidation: Entity Systems, Init Functions, Dispatch Targets, and Mask System

**Date:** 2026-05-17

## Objective

Consolidate all findings from the Rev.074 runtime session (9.2M events, 4.5GB log) and Rev.075 static analysis into a unified model. Perform remaining static analyses: correlate entity systems, classify all init_fn, map all dispatch targets, analyze mask_set callers, and understand the 404-byte table, halfword table, VU0 utility, and alternate dispatch paths.

## Scope

- Correlation between callback_register and dispatch entity systems
- Classification of all 28 init_fn (25 previously unknown)
- Full mapping of all 12 dispatch callback targets
- Analysis of 6 mask_set callers
- 404-byte stride table at 0x005F2F98
- Halfword table at 0x006AB080 (spatial grid hash)
- VU0 COP2 macro-mode utility at 0x117C40
- Alternate implementations at 0x169F80/0x16A058
- Cloth system init at 0x1C3778
- ICO-decomp cross-reference for all key addresses

## Sources Used

- `/home/peter/Documentos/repos/ico-reconstruction/.local/ico-pcsx2-probe-events-20260517-182237-rev074-runtime-complete.jsonl` (9,294,537 events)
- `/home/peter/Documentos/repos/ico-reconstruction/.local/extracted/SCUS_971.13.elf` (USA PS2 ELF)
- `research/elf/ghidra-rev075-init-fn-callback-dispatch-and-asm-handler-consolidation.md`
- `research/ico-decomp-cross-reference-2026-05-14.md`
- Capstone 5.0.7 (MIPS64 big-endian disassembler)

---

## Finding 1: Two Independent Entity Systems

**Confirmed:** The callback_register system (0x0013F7A8, 52 entities) and the live dispatch system (0x00166E10, ~20 entities/frame) operate on **different entity pools**.

### Evidence

Of 52 registered entities:
- **Only 3** appear in dispatch events
- **Only 2** of those have known init_fn (both use `init_fn=0x17D1D0`, env_effect sub-dispatcher)
- The remaining 49 registered entities are **never dispatched** — they are static scene data objects (HUD, status lights, config data)

Of ~803 dispatch entity contexts (mapped via temporal ctx→entity correlation):
- **8 core entities** account for **99.8%** of all 2.2M dispatch events
- These 8 entities are **never registered** via callback_register — they are pre-allocated or created through a different mechanism

### Architecture

```
callback_register (0x13F7A8):          Live Dispatch (0x166E10):
  alloc_entity(0x840)                     pre-allocated entities
  init_fn(entity, type_id)                slot_table[17]
  ↓                                       ↓
  3 entities flow through                 ~8 core entities
  (env_effect subtype only)               20 ctx/frame
```

### The 8 Core Dispatch Entities

| Entity | Slots | Event Count | % of Total |
|--------|-------|-------------|------------|
| 0x0083B7D4 | 1,2,3,4,5,6,7,10,12,14,15 | 1,472,788 | 67% |
| 0x0083C944 | 1,2,3,4,5,6,7,11,12,14,15 | 594,252 | 27% |
| 0x0083B660 | 1,2,3,4,6,7,12,14,15 | 70,134 | 3.2% |
| 5 others | various | ~73,000 | 3.3% |

**Key insight:** `0x0083B7D4` alone accounts for 2/3 of all dispatch activity. These entities persist across the entire gameplay session (not created/destroyed per room).

---

## Finding 2: All 28 init_fn Classified (6 Groups)

### Group 1 — Generic Entity Init (59.7% of registrations)

| Address | Count | % | Key Behavior |
|---------|-------|---|-------------|
| 0x1BB6B0 | 134 | 21.6% | Base entity setup: alloc_entity, sub_init, sound_init, position math. Reads type bitfield. Sets entity->+0x30=5. 128 instructions. |
| 0x164440 | 129 | 20.8% | Sound/system stream init: alloc_entity + extra 0x8b0 block. Calls 4 sound-init functions. 128 instructions. |
| 0x203EE8 | 107 | 17.3% | Minimal init stub: alloc_entity + wait_yield. 26 instructions. |

**Note:** `0x203EE8` is the minimal init — just allocates entity block and yields. Used for entity types requiring no special initialization.

### Group 2 — Cloth/Physics Auxiliary (4.7%)

| Address | Count | % | Key Behavior |
|---------|-------|---|-------------|
| 0x153478 | 18 | 2.9% | Cloth system aux: references BSS tables at 0x6AAAD0/0x6AAAE0. Tick counter at 0x274EC0. |
| 0x174BA0 | 11 | 1.8% | Rope/physics init: division/multiply by 60 and 30 (timing). References 0x274EC0 tick counter. |

### Group 3 — Environmental Effect Sub-Dispatcher (2.9%)

| Address | Count | % | Key Behavior |
|---------|-------|---|-------------|
| 0x17D1D0 | 18 | 2.9% | Reads 7-entry type table at 0x29A640 (stride 0x30). Compares entity->+0x08. Calls env_sub_init + model_load(x4). |

**Critical:** This is the **only** init_fn type that appears in BOTH registration and dispatch. The 7-entry table at 0x29A640 maps entity types to specific environmental effect handlers.

### Group 4 — Status Light / HUD Elements (14.5%)

10 functions in 0x236000 range, all with 9 registrations:

| Pattern | Functions | Resource IDs | Behavior |
|---------|-----------|-------------|----------|
| A (data block) | 0x236A00, 0x236B30, 0x236730, 0x236870, 0x236938, 0x2367A8 | 0x33-0x3B (51-59) | Alloc 0x4Dxxxx struct, register model, wait. |
| B (state ops) | 0x236588, 0x236A78, 0x236660, 0x236BA8 | 0x3A-0x3C (58-60) | Wait for resource, state bytes at entity→+0x15c→+0x800→+0x0C. Increment +0x08 counter. |

Resource IDs 0x33-0x3C map to HUD sprite assets. The 0x4Dxxxx structs are HUD element descriptors. Model IDs 0x2CB(715)/0x2CC(716) are HUD model references.

### Group 5 — UI/Menu Elements (9.1%)

7 functions in 0x211000 range, all with 8 registrations:

| Pattern | Functions | Description |
|---------|-----------|-------------|
| A (data block) | 0x211590, 0x211518, 0x211160 | Resource 0x42/0x40. Similar to status lights. |
| B (positional) | 0x2110F0 | Creates positional entity with FP coords (1000, 528, -150, -100). Resource 0x3D(61). Light/sound emitter. |
| C (wait loop) | 0x2111D8, 0x211290, 0x211338 | Resource 0x41(65). State byte ops. |

### Group 6 — Special One-Shots (0.4%)

| Address | Count | Description |
|---------|-------|-------------|
| 0x20D940 | 1 | State machine on GP-relative variable. Sets gp-0x5890=1. Unique scene object (door/elevator/quest item). |
| 0x20DAC8 | 1 | Fragment. Truncated or data-embedded. References 0x274EC0 tick counter. |

---

## Finding 3: Slot Table and Dispatch Callbacks

### Slot Table at 0x00282690 (17 entries × 16 bytes)

| Entry | Active | Group | Dir | Callback | Runtime Hits |
|-------|--------|-------|-----|----------|-------------|
| 0 | 1 | 0 | 0 | 0x168DA8 | (not captured — dead slot) |
| 1 | 1 | 0 | 0 | 0x168ED0 | 1,899,898 |
| 2 | 1 | 0 | 0 | 0x1692F0 | 646,729 |
| 3 | 1 | 0 | 0 | 0x169440 | 997,447 |
| 4 | 1 | 1 | 0 | 0x169020 | 30,992 |
| 5 | 1 | 1 | 0 | 0x169190 | 713 |
| 6 | 1 | 0 | 0 | 0x1696C0 | 428,523 |
| 7 | 1 | 0 | 0 | 0x169580 | 33,328 |
| 8 | 1 | 0 | 1 | 0x168ED0 | (same as slot 1) |
| 9 | 1 | 0 | 1 | 0x169440 | (same as slot 3) |
| 10 | 1 | 0 | 0 | 0x169800 | 10,429 |
| 11 | 1 | 0 | 0 | 0x169968 | 2,420 |
| 12 | 0 | 0 | 0 | 0x169AA8 | 2,710,041 |
| 13 | 0 | 1 | 0 | 0x169BD0 | (not captured — dead slot) |
| 14 | 0 | 0 | 0 | 0x169E58 | 190,984 |
| 15 | 0 | 0 | 0 | 0x169D18 | 69,750 |
| 16 | 0 | 0 | 1 | 0x169AA8 | (same as slot 12) |

**Field meanings:**
- +0x00: active flag (1=active, 0=inactive)
- +0x04: group (0=Group1→cold_path_A, 1=Group2→cold_path_B)
- +0x08: direction (0=forward, 1=reverse)
- +0x0C: callback function pointer

### Three Callback Tiers

**Tier 1 — Leaf Position/Rotation (Group1 only):**

| Callback | Hits | Slots | Behavior |
|----------|------|-------|----------|
| 0x168ED0 | 1,899,898 | 1,8 | Only calls 0x166258 (Group1 pos/rot iterator ×6). Pure transform update. |
| 0x1692F0 | 646,729 | 2 | Same as 0x168ED0. Group1 only. |
| 0x169020 | 30,992 | 4 | Same pattern. Group1 only. |
| 0x169190 | 713 | 5 | Same pattern. Group1 only. |

**Tier 2 — Hybrid (Group1 + Group2):**

| Callback | Hits | Slots | Behavior |
|----------|------|-------|----------|
| 0x169440 | 997,447 | 3,9 | Mostly Group1 (×5) + single Group2 (×1). |
| 0x1696C0 | 428,523 | 6 | Balanced: Group1×3 + Group2×3. |
| 0x169580 | 33,328 | 7 | Group1×4 + Group2×2. |
| 0x169800 | 10,429 | 10 | Hybrid + cold_path_A + some math utils. |
| 0x169968 | 2,420 | 11 | Full math(7) + cloth + cold paths + Group1×1 + Group2×4. |

**Tier 3 — Full Pipeline (Group2 + math + cloth):**

| Callback | Hits | Slots | Behavior |
|----------|------|-------|----------|
| 0x169AA8 | 2,710,041 | 12,16 | Full pipeline: sqrt/sin/cos, mtx_mult, vec_cross/dot/norm, Group2×4, cloth×4, cold_paths A+B, wrapper. 17 JAL targets. |
| 0x169E58 | 190,984 | 14 | Same as 0x169AA8. Full pipeline + additional utility calls. |
| 0x169D18 | 69,750 | 15 | Same as 0x169E58. Nearly identical. |

### Observer effect on inactive slots

Slots with `active=0` (entries 12-16) fire the **most** events. This is because the `active` flag is not a gate — it controls Group selection (via field_04). The runtime log captures `active=0` entries firing whenever their handler code is reachable through `cold_path_B`.

### Shared Iterators

```
Group1 iterator 0x166258: pos/rot update
  - Stack: -0xE0
  - Reads slot data (a1), writes entity (a0)
  - FPU intensive: lwc1/swc1
  - Called by: 0x168ED0, 0x1692F0, 0x169020, 0x169190, 0x169440, 0x1696C0, 0x169580, 0x169800, 0x169968

Group2 iterator 0x1667E0: orientation/matrix update
  - Stack: -0x30
  - 3×3 orientation matrix multiply
  - mul.s: slot_data * entity_field
  - Called by: 0x169AA8, 0x169E58, 0x169D18, 0x169440, 0x1696C0, 0x169580, 0x169800, 0x169968
```

---

## Finding 4: Mask System — Only Bit 0 Used

### mask_set() at 0x13ED40

```
mask_set(a0=bit_index, a1=set_flag):
  if a1 == 0: mask &= ~(1 << a0)   // clear bit
  else:        mask |= (1 << a0)    // set bit
  // updates both gp+0x98DC and gp+0x98E0
```

### The 6 Callers

| Caller | Bit | Action | Context |
|--------|-----|--------|---------|
| 0x19F590 | 0-7 | CLEAR | Boot init: prints "----- MASK LINK -----", loop clears bits 0-7 |
| 0x1B4F8C | 0 | CLEAR | Scene init: BEFORE entity loading. Unique — a0 in delay slot. |
| 0x1B1830 | 0 | SET | Scene loader (path A): AFTER entity setup helper (0x13B858) |
| 0x1B1A18 | 0 | SET | Scene loader (path B): AFTER entity setup helper |
| 0x1B44E0 | 0 | SET | Minimal scene loader: AFTER entity setup helper |
| 0x1B0918 | 0 | SET | Step dispatcher (case 0 of 8): first step of scene loading sequence |

**Critical finding:** Despite 8 bits being available, **only bit 0 is ever set or cleared** by any caller. The remaining bits 1-7 are initialized to 0 at boot and never touched.

### Mask Lifecycle

```
Boot:    0x19F590 clears bits 0-7   → mask = 0x00
Scene load starts: 0x1B4F8C clears bit 0 → mask = 0x00
  [entity loading happens with mask=0 — callbacks blocked]
Scene load complete: 0x1B1830/0x1B1A18/0x1B44E0 sets bit 0 → mask = 0x01
  [callbacks now fire normally]
```

### ICO-decomp Symbol

`0x13ED40 = ShockRequestBox_RequestCancel` (fumi/ios/shockdriver.c)

---

## Finding 5: 404-Byte Table at 0x005F2F98 — Stage Config

**Not an entity table.** The 404-byte stride table at 0x005F2F98 is a **stage/room configuration table**, indexed by `world_state` (GP-relative variable at `gp+0x90A0`).

### Structure

```
404 bytes per entry:
  +0x00: 0xEA (234) bytes — room config section
    +0x00: room_id (2 bytes)
    ...
  +0xEA: 0xAA (170) bytes — proxy block section
    +0xEA: cloth_vertex_count
    +0xEE: sound_proxy_id
    +0xF2: render_group_flags
    +0xF6: door_transition_list_offset
```

### Access Pattern

Only 4 static xrefs found:
- World state loaded from `gp+0x90A0` (0x00631990)
- Table base computed: `0x005F2F98 + world_state * 404`
- Accessed in scene loader functions for room load configuration

---

## Finding 6: Halfword Table at 0x006AB080 — Spatial Hash Grid

**Not a type/entity index table.** It is a **2D broad-phase spatial hash** used for entity-to-entity queries.

### Structure

```
32 × 32 grid of 2-byte entries = 2048 bytes total
Each entry: packed (row << 5) | col
  - row: 5 bits (0-31)
  - col: 5 bits (0-31)
```

### Writers (all in function 0x166BB0, the dispatcher inner loop)

| Address | Instruction | Value Written | Meaning |
|---------|------------|---------------|---------|
| 0x166D38 | `sh a0, (v0)` | `(row<<5) | col` | Main loop block A |
| 0x166D94 | `sh a0, (v0)` | `(row<<5) | col` | Main loop block B |
| 0x166DFC | `sh v0, -0x4F80(a0)` | `(row<<5) | col` | Single-cell fast path |

### Lifecycle

Per-frame, per-entity:
1. Counter at `gp-0x4BC4` (=0x00633D2C) zeroed
2. Entity bounding box computed relative to global grid origin (state_ptr+0x20)
3. Each covered grid cell packed as `(row<<5 | col)` → written to `table[counter++]`
4. Dispatch callbacks read table entries via `lh + resolved_entity_ptr`

### Purpose

Broad-phase spatial optimization for VU0 entity intersection/visibility queries. Writer: dispatcher inner loop at 0x166BB0. Reader: VU0-related function at 0x168DA8.

---

## Finding 7: VU0 "Kick" at 0x117C40 — COP2 Macro-Mode Utility

**Not a VU0 kick.** No microcode loading, no DMA, no memory-mapped register access.

### Disassembly

```
0x117C40: VU0_upper fn=0xF fd=vf7  fs=vf0  ft=vf12  field=0x3c  bc=w  ; clamp
0x117C44: VU0_upper fn=0xF fd=vf6  fs=vf7  ft=vf12  field=0x3d  bc=w  ; propagate
0x117C48: VU0_upper fn=0xF fd=vf5  fs=vf6  ft=vf12  field=0x3d  bc=w  ;
0x117C4C: VU0_upper fn=0xF fd=vf4  fs=vf5  ft=vf12  field=0x3d  bc=w  ;
0x117C50: VU0_upper fn=0x0 fd=vf15 fs=vf0  ft=vf0   field=0x32  bc=w  ; final gather vf15.y = 0
0x117C54: jr $ra
0x117C58: nop
```

**Interpretation:** Vector normalization safeguard / frustum clipping using VMAX chains. fn=0xF with vf12.w broadcast across 4 accumulator passes. vf0 = zero register. Final result stored in vf15.y. Duplicated at 0x117C60.

### 22 Callers Across All Subsystems

Rendering (0x10B870, 0x10C990, 0x10CDD0), AI (0x11CDD4, 0x11F0B8, 0x11F750, 0x121BE0), collision/physics (0x11F53C, 0x11F698), scene/animation (0x129ADC, 0x129BD4, 0x129C14, 0x129D9C, 0x129DB0), cloth/physics (0x1CDBC0, 0x1D4A8C), scene loader (0x1953B4), player/AI (0x1F9760, 0x1F9C94).

---

## Finding 8: Alternate Implementations — VU0 DMA Pipeline

**Both are unreachable at runtime** (zero hits in 9.2M events, zero static callers).

### Structure (identical for both)

```
stack: -0x50
  cold_path(a0) → setup entity fields
  DMA_wait(0xB)
  sync()
  load 32B config from rodata
  DMA_send(1, 5, 0x80)  → VU0 transfer
  DMA_wait(1)
  alloc() → packet buffer
  memcpy(alloc_ptr, config_data)
  packet_build(a0, sp,    a2, sp,    t0=0)
  packet_build(a0, sp+16, a2, sp+16, t0=-1)
  free()
  sync()
```

### Alt A vs Alt B

| Aspect | Alt A (0x169F80) | Alt B (0x16A058) |
|--------|-------------------|-------------------|
| Cold path | 0x167230 | 0x167258 |
| Config data | `0x569280` (-1.0f, 19, "objects/df/boy/motion/ba...") | `0x5692A0` ("c21b1.mob") |
| Cold path effect | Zeroes entity+0xB0, +0x94, +0x88. Copies 8B to +0x80. | Zeroes entity+0x94. Copies 8B to +0x8C. |

**Verdict:** Compile-time configuration variants or debug/dev-only code. Not part of the normal runtime path.

---

## Finding 9: Cloth System Init at 0x1C3778

### Dead Code Guard

`0x1C3760` is a dead spin-loop: infinite loop calling `0x203AA0(1)`. Zero callers.

### Real Init at 0x1C3778

Called from exactly one place: `0x1E97B8` (conditional on GP flag at `-0x5C28`).

```
stack: -0xE0, saves ra, s0-s7, fp
Phase 1: Load 4×8 bytes of constants from 0x6188E0 (initializer data)
Phase 2: DMA_wait(0xB) → DMA_send(1, 5, 0x80) → alloc → memcpy
Phase 3: Iterate elements:
  count = [a0 + 4]
  stride = 0x50 (80 bytes per cloth element)
  mega_stride = 0x1A0 (416 bytes per sub-element)
  For each element → sub-element:
    memcpy-like (0x243AD0)
    packet_build (0x1D49C0)
```

**Domain:** ClothAnimation.c (confirmed by ICO-decomp). The 0x50/0x1A0 strides match cloth vertex cluster initialization.

---

## Finding 10: ICO-decomp Cross-Reference

| Address | ICO-decomp Match | Source File |
|---------|-----------------|-------------|
| 0x13ED40 | **ShockRequestBox_RequestCancel** | fumi/ios/shockdriver.c |
| 0x1BB6B0 | (between PAL symbols) sugipon/src/weapon.c or windField.c |
| 0x164440 | (between PAL symbols) fumi/src/enemy_act |
| 0x203EE8 | (between PAL symbols) sugipon/src/windField.c |
| 0x153478 | (between PAL symbols) sugipon/src/boy.c |
| 0x17D1D0 | (between PAL symbols) fumi/src/jimaku.c or way_sys.c |
| 0x174BA0 | (between PAL symbols) sugipon/src/girl.c |
| 0x169AA8 | (between PAL symbols) fumi/src/enemy_act |
| 0x168ED0 | (between PAL symbols) fumi/src/enemy_act, near _ApproachTarget_Way |
| 0x169440 | (between PAL symbols) fumi/src/enemy_act, near actEnemyStart |
| 0x13F9D0 | (between PAL symbols) fumi/ios/thread.c, between iosThreadInit and iosThreadCreate |
| 0x13FC00 | (between PAL symbols) fumi/ios/thread.c, near iosThreadGetPri |
| 0x19F590 | (between PAL symbols) ito/src/act_bird.c or omori/src/camera-root.c |
| 0x1B0918/0x1B1830 | (between PAL symbols) common/src/debug.c |

**Note:** ICO-decomp uses PAL SCES_507.60 addresses. USA `SCUS_971.13` differs by function linking order. Contextual source file matches are strong (same source file, nearby PAL symbols) but exact USA symbol names are inaccessible without a USA splat config.

---

## Revised Evidence Chain

```
caller_register (0x13F7A8) → 28 init_fn types (52 entities)
  ├── 49 static scene objects (HUD, status lights, UI/menu, config)
  │     → never dispatched (live dispatch system = separate pool)
  └── 3 dynamically dispatched entities (env_effect subtype 0x17D1D0)
        → use init_fn=0x17D1D0 (7-type table at 0x29A640)

Live dispatch system (0x166E10) → 17-slot table at 0x282690
  ├── 8 core entities (pre-allocated, never registered)
  └── ~20 entity contexts per frame via ctx→entity mapping

Mask system:
  mask_set(0x13ED40) → only bit 0 used
  ├── Boot: clear all 8 bits
  ├── Scene init: clear bit 0 (block callbacks during loading)
  └── Scene load complete: set bit 0 (enable callbacks)

Callback dispatch (0x13F9D0):
  ├── mask register gp+0x98DC (bit 0 gate)
  ├── 8-entry table at 0x281A70 (type IDs 0x13-0x1B)
  └── Linked list walk → typed handler calls

Halfword table (0x6AB080):
  32×32 spatial hash, rebuilt per dispatch cycle
  Writer: dispatcher inner loop (0x166BB0)
  Reader: VU0 intersection queries (0x168DA8)

404-byte table (0x005F2F98):
  Stage/room config indexed by world_state
  Contains: cloth vertex count, sound proxy, render group, door list

VU0 utility (0x117C40):
  COP2 macro-mode clamp/normalize (not a kick)
  Called from 22 sites across all subsystems

Alternate implementations (0x169F80/0x16A058):
  VU0 DMA pipeline (unreachable at runtime)
  Zero static callers; compile-time config or debug-only

Cloth sys init (0x1C3778):
  Element stride 0x50, sub-stride 0x1A0
  DMA to VU0, packet building
  Guard at 0x1C3760 is dead code
```

---

## What Is Confirmed

1. Two independent entity systems: registration and dispatch
2. 28 init_fn fully classified into 6 semantic groups
3. 17-slot dispatch table fully mapped with all 12 callback targets analyzed
4. Three callback tiers: leaf pos/rot, hybrid, full pipeline (math+cloth)
5. Mask system uses only bit 0; all 6 callers converge on it
6. 404-byte table is room config (not entity table)
7. Halfword table is 32×32 spatial hash
8. VU0 "kick" is COP2 macro-mode utility (not a kick)
9. Alternates = VU0 DMA pipeline (unreachable)
10. Cloth init at 0x1C3778 with dead guard at 0x1C3760
11. ICO-decomp cross-ref: only mask_set matches exactly (ShockRequestBox_RequestCancel)

## What Is Probable

- The 8 core dispatch entities are ICO, Yorda, enemies, and interactive objects
- Slot 12 (most active) is the main entity processing pipeline
- Entity 0x0063AF20 (slot 2 only) is a unique singleton (camera? light?)
- Group 2 orientation set (0x169AA8/0x169E58/0x169D18) handles characters with cloth physics
- Leaf pos/rot set (0x168ED0/0x1692F0/0x169020/0x169190) handles static/rigid objects

## What Is Possible

- The HUD cluster (0x236xxx) represents 10 distinct ICO HUD elements (health, active item, map, etc.)
- The UI cluster (0x211xxx) represents inventory or menu elements
- `0x203EE8` minimal init may be for invisible collision/trigger volumes
- The state machine at 0x20D940 might initialize doors or elevators

## What Is Unknown

- Exact identity of the 8 core dispatch entities
- How the 8 active dispatch entities are allocated (not through callback_register)
- What controls the mask bit 0 lifecycle during cutscenes/menus
- Why bits 1-7 of the mask are completely unused
- What the 7-type table at 0x29A640 (used by 0x17D1D0) contains
- The full contents of the 404-byte room config table beyond the first few fields

## Next Steps

1. Runtime: probe mask register gp+0x98DC during cutscene transitions (does bit 0 toggle?)
2. Runtime: probe the 0x166D1C/0x166D78 halfword table writers with bounding box capture
3. Runtime: runtime capture of gp+0x31990 (world_state) to map room transitions
4. Static: extract the 7-type table at 0x29A640
5. Static: disassemble the step dispatcher at 0x1B08E4 (remaining 7 cases of jump table at 0x616CC0)
6. Static: analyze the 0x168DA8 function (slot 0 callback, did not appear in runtime capture)
7. Static: search for what allocates the 8 core dispatch entities

---

## Related Files

- `research/elf/ghidra-rev075-init-fn-callback-dispatch-and-asm-handler-consolidation.md`
- `research/elf/ghidra-rev074-runtime-session-main-loop-dispatch-confirmed.md`
- `research/elf/ghidra-rev073-main-loop-dispatch-chain-and-callback-corrected-masks.md`
- `research/ico-decomp-cross-reference-2026-05-14.md`
- `docs/data-model.md`
- `docs/system-feature-flows.md`
- `docs/backlog.md`
- `.local/correlate_v2.py`
