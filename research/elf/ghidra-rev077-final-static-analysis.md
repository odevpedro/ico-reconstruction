# Rev.077 — Final Static Analysis: Step Dispatcher, Descriptor Table, Entry Table, Utility Functions, Debug Table, GP Map

**Date:** 2026-05-17

## Objective

Complete all remaining static analyses that do not require runtime (emulator) access. Cover: 8-step scene loading dispatcher, 7-type env effect table, descriptor table (68 entity types), entry table (512 instances), slot 0 callback, scene loader helper, resource check, wait/yield, VU0 kick substitution, GP data map, HUD pool, and debug visualization table.

## Scope

- All static analyses possible without PCSX2 runtime
- Consolidation of all findings into a unified model
- Correction of prior misconceptions

---

## Finding 1: 8-Step Scene Loading State Machine at 0x1B08E4

**Source file identified:** `src/kanban.c` (string at 0x616CA8)

**GP = 0x27A7A8** (NOT the main GP=0x006388F0 — different compilation unit). This GP is computed as `0x274ED4 + 0x58D4`.

### Jump Table at 0x616CC0 (8 entries)

| Case | Address | Name | Description |
|------|---------|------|-------------|
| 0 | 0x1B0914 | INIT | `mask_set(0, 1)` — enable callback group. Reset step_counter and flag_lock. |
| 1 | 0x1B0938 | POLL | Wait for `flag_ready` (externally set when scene assets are staged). |
| 2 | 0x1B094C | LOAD | Call `check_loading_state(0x1B06D8)`. If ready: mark resource slots (29 slots at 0x706110, stride 0x20), register resource descriptor via `0x1AFB98`. |
| 3 | 0x1B0980 | RESET | Zero all state vars. Counter set to 10. Step resets to 0 (returns to INIT). |
| 4 | 0x1B09C4 | DELAY | Decrement `counter` until it reaches -1, then advance step. |
| 5 | 0x1B09E0 | CHECK | Wait for `flag_lock` to clear (non-zero = loading in progress). |
| 6 | 0x1B09F8 | RELOAD | Call `mark_resource_slots()` again. |
| 7 | 0x1B0A0C | DONE | Wait for resource descriptor to unlock. Set `flag_done = 1`. Exit (step > 7). |

### GP-Relative Variables (local GP = 0x27A7A8)

| Offset | Abs Addr | Name | Role |
|--------|----------|------|------|
| -0x58D4 | 0x274ED4 | step_counter | Current step index |
| -0x58D0 | 0x274ED8 | flag_lock | Loading state lock (advances: 0→1→2→3→4) |
| -0x58CC | 0x274EDC | flag_ready | Scene-ready flag (set by external trigger) |
| -0x58C8 | 0x274EE0 | flag_done | All-done flag |
| -0x4A6C | 0x275D3C | res_ptr | Resource descriptor pointer |
| -0x4A68 | 0x275D40 | counter | Decrementing counter |

### Sub-Functions Used

| Address | Name | What It Does |
|---------|------|-------------|
| 0x13ED40 | mask_set | mask_set(bit, set) — callbacks gate |
| 0x1B06D8 | check_loading_state | State machine on flag_lock (0→1→2→3→4). Manages struct at 0x706500. Calls 0x1381D0/0x137EF0. |
| 0x1B0570 | mark_resource_slots | Iterates 29 slots (stride 0x20) at 0x706110. Sets bit 0 at +0x0C. |
| 0x1AFB98 | register_resource | Computes resource offset (multiply by 0x38). Base at 0x54XXXX. |
| 0x1B0A38 | reset_state | Zeros all state variables. |
| 0x1B0A58 | set_ready | Sets flag_ready=1. |

---

## Finding 2: Descriptor Table at 0x2A31B8 — 68 Entity Types

### Structure (stride 0x64 = 100 bytes)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| +0x00 | 16 | name | Null-terminated string (max 15 chars) |
| +0x10 | 16 | pad | Always zero |
| +0x20 | 4 | model_id | Model/resource ID (0 for most) |
| +0x24 | 4 | radius | Float. -1.0 = default; 8.0=SOFA; 60.0=BIRD |
| +0x28 | 4 | mass | Float. 0=no physics; 1.0=physics obj; 2.0=BIRD |
| +0x2C | 4 | threshold | Float. 0.01=physics; 0.02=BIRD; 3/10/20=INTEREST* |
| +0x30 | 4 | type_id | Entity subtype (1=physics, 2=SOFA, 3=BIRD, 10=AP1) |
| +0x34 | 12 | pad | Always zero |
| +0x40 | 4 | init_fn | Entity init function (12/68 have one) |
| +0x44 | 4 | cb_flag | 0 or 1 — callback block active |
| +0x48 | 4 | cb_update | Primary update routine (56/68 have one) |
| +0x4C | 4 | pad | Always zero |
| +0x50 | 4 | cb_routine2 | Secondary routine |
| +0x54 | 4 | pad | Always zero |
| +0x58 | 4 | cb_routine3 | Tertiary routine |
| +0x5C | 4 | cb_routine4 | Quaternary routine (GIRL, ENEMY1, DEVIL_GIRL only) |
| +0x60 | 4 | vtable | V-table / class pointer (29/68 have one) |

### Complete Entity List

| Idx | Name | InitFn | cb_update | cb_r2 | cb_r3 | cb_r4 | vtable |
|-----|------|--------|-----------|-------|-------|-------|--------|
| 0 | NULL | — | — | — | — | — | — |
| 1 | BOY | 0x153478 | 0x1C1F58 | 0x1C1DD8 | 0x1C1A98 | — | 0x202A60 |
| 2 | GIRL | 0x174BA0 | 0x1D1A98 | 0x1D17F8 | 0x1D1668 | **0x1D1AD0** | 0x202A60 |
| 3 | GIRLDEMOCTRL | — | 0x1D1A98 | 0x1D17F8 | 0x1D1668 | — | — |
| 4 | ENEMY1 | 0x164440 | 0x1CE690 | 0x1CE3C0 | 0x1CE220 | **0x1CE760** | 0x202A60 |
| 5 | ENEMY_TEST | — | 0x1CE5F8 | 0x1E08B8 | 0x1CE220 | 0x1CE760 | — |
| 6 | DEMOMOTCTRL | — | 0x10ECC0 | 0x1CE6F0 | 0x1CEB18 | — | — |
| 7 | SOBJ | — | 0x10ECC0 | — | — | — | 0x23D660 |
| 8 | PSOBJ | — | 0x1AEA58 | 0x1AEA50 | 0x1AEA60 | — | 0x23D660 |
| 9 | TREE | — | 0x1F1508 | 0x1F1330 | 0x1F17B0 | — | — |
| 10 | TORCH | — | 0x1F2140 | 0x1F1CF0 | 0x1F2048 | — | — |
| 11 | PARTICLE | — | — | — | — | — | — |
| 12 | PARTLAYOUT | — | 0x1E8F30 | 0x1E8F28 | 0x1E8EB8 | — | — |
| 13 | LIGHTBIT | — | 0x1F0550 | 0x1F0540 | 0x1F0568 | — | — |
| 14 | WEAPON | — | 0x1F3BD0 | 0x1F3A00 | 0x1F36F0 | — | — |
| 15 | SPIDER_LAYOUT | — | — | 0x1EA5E8 | 0x1EA278 | — | — |
| 16 | SOFA | — | 0x10ECC0 | — | — | — | 0x23D660 |
| 17 | **WOODBOX01** | **0x17D1D0** | **0x1C05D0** | **0x1C0538** | **0x1C00C0** | — | 0x23D660 |
| 18 | ROTOBJECT | — | 0x1EA030 | 0x1E9950 | 0x1E9F08 | — | — |
| 19 | **BARREL** | — | **0x1D3B28** | **0x1D3A30** | **0x1D27A8** | — | — |
| 20 | ROPE | — | 0x1E9630 | 0x1E9810 | 0x1E8F38 | — | — |
| 21 | CHAIN | — | 0x18F640 | 0x18ECC8 | 0x18E5B0 | — | — |
| 22 | FLEVER | — | 0x1BC438 | 0x1BC1A8 | 0x1C09C8 | — | — |
| 23 | FLEVER_TRISTATE | — | 0x1BC438 | 0x1BC130 | 0x1C09C8 | — | — |
| 24 | WLEVER | — | 0x1BC7F0 | 0x1BC530 | 0x1C0C40 | — | — |
| 25 | WLEVER2 | — | 0x1BC7F0 | 0x1BC530 | 0x1C0C40 | — | — |
| 26 | NONE | — | — | — | — | — | — |
| 27 | SV | — | 0x10EC60 | — | — | — | — |
| 28 | CAMERADUMMY | — | — | — | — | — | 0x23D660 |
| 29 | DUMMY | — | — | — | — | — | — |
| 30 | **BGA** | **0x203EE8** | — | — | — | — | 0x23D660 |
| 31 | SEFFECT | — | 0x1EF988 | 0x1EF980 | 0x1EF8E8 | — | — |
| 32 | BIRD | 0x1971C0 | 0x197080 | 0x197078 | 0x197240 | — | 0x1956E8 |
| 33 | GENERATOR | — | 0x193600 | 0x1930B0 | 0x193730 | — | 0x192EB8 |
| 34 | CANDLE | — | 0x1C3130 | 0x1C3178 | 0x1C2FF0 | — | — |
| 35 | MOBJ | — | 0x10ECB0 | — | — | — | 0x23D660 |
| 36 | DEMO_QSWORD | — | 0x10ECB0 | 0x1F44C8 | 0x1F43D0 | — | 0x23D660 |
| 37 | CHANDELIER | — | 0x1C3470 | 0x1C33D8 | 0x1C34B8 | — | 0x23D660 |
| 38 | WORM | — | 0x1F69B0 | 0x1F66F0 | 0x1F62E8 | — | — |
| 39 | POOL | — | 0x10C5C0 | 0x10D070 | 0x10B2D0 | — | — |
| 40 | DARKVOLUME | — | 0x1CBD78 | 0x1CBD70 | 0x1CBD68 | — | — |
| 41 | MCOLTEST | — | 0x1E6960 | 0x1E6788 | 0x1E6968 | — | — |
| 42 | ROPEFIX | — | 0x1E98C8 | 0x1E9888 | 0x1E9910 | — | 0x23D660 |
| 43 | CAGE | — | 0x1C2DF8 | 0x1C28D0 | 0x1C2338 | — | — |
| 44 | DYNAMICMOTIONDATA | — | — | — | — | — | — |
| 45 | FLAG | — | 0x1D01E8 | 0x1D00F8 | 0x1CFB58 | — | — |
| 46 | QUEEN | 0x19B7F8 | 0x19A9A0 | 0x19A8F0 | 0x19A7E8 | — | 0x199A60 |
| 47 | QUEENDEMOCTRL | — | 0x19A9A0 | 0x19A8F0 | 0x19A7E8 | — | — |
| 48 | **DEVIL_GIRL** | **0x174BA0** | **0x1D1A98** | **0x1D17F8** | **0x1D1668** | **0x1D1AD0** | **0x202A60** |
| 49 | SKELTEST | — | 0x1E0860 | 0x1E08B8 | — | — | 0x23D660 |
| 50 | CAGEFIX | — | 0x1C2FA0 | 0x1C2F20 | 0x1C2FE8 | — | 0x23D660 |
| 51 | CLOTHTEST | — | 0x1C9330 | 0x1C9328 | 0x1C92D8 | — | — |
| 52 | QUEEN_BALL | — | 0x19B660 | 0x19AE98 | 0x19BC58 | — | 0x19BB60 |
| 53 | QUEEN_BARRIER | — | 0x19AE50 | 0x19AA20 | 0x19BAA8 | — | 0x19B998 |
| 54 | STAGESETTING | — | — | — | — | — | — |
| 55 | GIRLFORCEFIELD | — | 0x1D1B30 | 0x1D1CF8 | 0x1D1C78 | — | — |
| 56 | INTEREST1 | — | 0x10ECC0 | — | — | — | 0x23D660 |
| 57 | INTEREST3 | — | 0x10ECC0 | — | — | — | 0x23D660 |
| 58 | INTEREST10 | — | 0x10ECC0 | — | — | — | 0x23D660 |
| 59 | INTEREST20 | — | 0x10ECC0 | — | — | — | 0x23D660 |
| 60 | KYOMI | — | — | 0x23D518 | — | — | — |
| 61 | **AP1** | **0x1BB6B0** | **0x1BA530** | **0x1BA330** | **0x1B8720** | — | **0x1BB3E0** |
| 62 | ATTACKCHECKBOUNDARY | 0x1BBF78 | 0x1BBEC8 | 0x1BBEA0 | 0x1BBE50 | — | 0x1BB988 |
| 63 | ATTCKCHKBNDRYMNGR | 0x1BBF78 | 0x1BBE48 | 0x1BBDD8 | 0x1BBB20 | — | — |
| 64 | BOSS_CTRL | 0x198140 | 0x198000 | 0x197FC8 | 0x198218 | — | 0x1978B0 |
| 65 | temp | — | — | — | — | — | — |
| 66 | ENEMY_CONTROL | — | — | — | 0x191D08 | — | 0x191C80 |
| 67 | FLY_INFO | — | 0x1D03A0 | — | 0x1D03C8 | — | — |

### Key Correction: `0x1D3A30` Domain

**BARREL** at index 19 uses the cloth dispatcher at `0x1D3A30` as `cb_routine2` (+0x50). The "cloth" functions at 0x1D3A30, 0x1D3B28, and 0x1D27A8 are **barrel physics constraint solvers**, not cloth animation. The name "clothAnimation.c" in ICO-decomp may refer to a generalized constraint solver used for both cloth and physics objects.

This resolves the **ROPE callback registration gap** (Rev.033-037): `0x1D3A30` is registered via the **descriptor table** at offset +0x50 (cb_routine2), not through the callback_register system at 0x13F7A8.

### Shared init_fn Across Entity Types

```
0x153478 (1): BOY
0x164440 (1): ENEMY1
0x174BA0 (2): GIRL, DEVIL_GIRL
0x17D1D0 (1): WOODBOX01
0x1971C0 (1): BIRD
0x198140 (1): BOSS_CTRL
0x19B7F8 (1): QUEEN
0x1BB6B0 (1): AP1 (action point)
0x1BBF78 (2): ATTACKCHECKBOUNDARY, ATTCKCHKBNDRYMNGR
0x203EE8 (1): BGA (background)

Total: 12/68 entries with init_fn
```

### V-Table Groups

| vtable | Entities | Description |
|--------|----------|-------------|
| NULL | 39 | No class data or self-contained |
| 0x202A60 | 4 | BOY, GIRL, ENEMY1, DEVIL_GIRL — main characters |
| 0x23D660 | 16 | SOBJ, SOFA, WOODBOX01, CAMERADUMMY, BGA, MOBJ, DEMO_QSWORD, CHANDELIER, ROPEFIX, SKELTEST, CAGEFIX, INTEREST* — physics props + sensors |
| Entity-specific | 9 | BIRD, GENERATOR, QUEEN, QUEEN_BALL, QUEEN_BARRIER, AP1, ATTACKCHECKBOUNDARY, BOSS_CTRL, ENEMY_CONTROL |

---

## Finding 3: 7-Type Env Effect Table at 0x29A640

### Structure (7 entries × 0x30)

| Offset | Size | Field |
|--------|------|-------|
| +0x00 | 2 | type_id (int16) |
| +0x02 | 1 | sub_type (int8) — dispatched via 9-entry jump table at 0x55A170 |
| +0x03 | 1 | padding |
| +0x04 | 4 | model_id_1 |
| +0x08 | 4 | model_id_2 |
| +0x0C | 4 | flags (0 or -1) |
| +0x10 | 4 | count (10 most, 5 for entry 3, 15 for entry 1) |
| +0x14 | 4 | padding |
| +0x18 | 2 | type_id_2 |
| +0x1C | 4 | model_id_3 |
| +0x20 | 4 | model_id_4 (also float bounds) |
| +0x24 | 12 | float[3] bounds (used by handler 0x17BB98) |

### The 7 Entries

| Entry | type_id | sub | model_1 | model_2 | flags | count | type_2 | model_3 | model_4 |
|-------|---------|-----|---------|---------|-------|-------|--------|---------|---------|
| 0 | 0x020E(526) | 0 | 0x00C7(199) | 0x020B(523) | 0 | 10 | 0x020E | 0x00C5(197) | 0x0210(528) |
| 1 | 0x020E(526) | 0 | 0x00BE(190) | 0x0220(544) | 0 | 15 | 0x020E | 0x00C4(196) | 0x0210(528) |
| 2 | 0x020E(526) | 0 | 0x003E(62) | 0x0286(646) | 0 | 10 | 0x0210 | 0x0007(7) | 0x0214(532) |
| 3 | 0x0210(528) | 0 | 0x0001(1) | 0x0202(514) | -1 | 5 | 0x0210 | 0x000C(12) | 0x020D(525) |
| 4 | 0x0210(528) | 0 | 0x0055(85) | 0x022F(559) | 0 | 10 | 0x0210 | 0x0056(86) | 0x0231(561) |
| 5 | 0x0210(528) | 0 | 0x0057(87) | 0x0232(562) | 0 | 10 | 0x0210 | 0x0015(21) | 0x025E(606) |
| 6 | 0x0210(528) | 0 | 0x0070(112) | 0x025F(607) | 0 | 10 | 0x0210 | 0x0071(113) | 0x0260(608) |

### Handler 0x17BB98 Sub-Dispatch

9-entry jump table at 0x55A170 (keyed by sub_type, all 0 here):
- Case 0-3: zone_check variants (spatial comparisons: entity.x < bound.x, etc.)
- Case 4: 3D zone check (full X/Y/Z min/max bounds)
- Cases 5-7: separate handlers
- Case 8: alias to case 4

Two entity types: 0x020E (526) and 0x0210 (528). Model IDs reference environmental effect models. Spatial bounds control effect trigger zones.

---

## Finding 4: Entry Table at 0x2A4C48 — 512 Instance Spawns

### Structure (stride 0x4C = 76 bytes)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| +0x00 | 12 | float[3] scale | Uniform 1.0f for most |
| +0x0C | 4 | rotation | Float (0 for most) |
| +0x10 | 12 | float[3] position | (X, Y=0, Z) — world space |
| +0x1C | 8 | float[2] extents/bounds | |
| +0x24 | 8 | float[2] unknown | |
| +0x2C | 4 | data_ptr | Entity data reference |
| +0x30 | 4 | class_id | Entity class/type ID |
| +0x34 | 4 | subtype | Room model / asset ID |
| +0x38 | 4 | instance_idx | Instance index within group |
| +0x3C | 4 | flags/link | |
| +0x40 | 4 | more_flags | |
| +0x44 | 4 | packed | High 16 = entity group, Low 16 = subflag |
| +0x46 | 1 | op_type | Operation selector byte |
| +0x48 | 4 | packed | Flags/link |

### Class Distribution

| Class ID | Count | Description |
|----------|-------|-------------|
| 0x15 (21) | 498 | "Common" entity instance |
| 0x02 (2) | 5 | |
| 0x00 (0) | 2 | |
| 0x01 (1) | 2 | |
| 0x0D (13) | 2 | |
| 0x0E (14) | 1 | |
| 0x0B (11) | 1 | |
| 0x0F (15) | 1 | |

### Position Data

~180 entries have real positions (e.g., [-10, 0, -20], [90, 0, -1685], [0, 0, -800]). Sequential subtype groups suggest room-specific asset loading. The iteration function at 0x1B76F8 reads op_type (+0x46) and processes transforms via div.s/neg.s.

---

## Finding 5: Slot 0 Callback 0x168DA8

### Comparison with Slot 1 (0x168ED0)

Both have a 60-byte identical prologue. Both iterate the halfword table at 0x6AB080 → resolve 80-byte entity structs → call the Group1 iterator at 0x166258.

**Critical difference:** Slot 0 has **no filtering logic**. Slot 1 checks entity flags at +0x48:
- Skip if `flags & 0xF0000000` (hi nibble set)
- Skip if `(flags & 0xF) == 1` (flag value 1)

Slot 0 processes ALL entries unconditionally. This suggests slot 0 is a **debug/fallback path** that is never selected during normal gameplay because the dispatch table's slot index selection never produces index 0 for any entity.

---

## Finding 6: Scene Loader Helper at 0x13B858

**Not one function — four related sub-functions:**

| Address | Name | Operation |
|---------|------|-----------|
| 0x13B858 | scene_load_set | Write 1 to `gp[-0x4c60]` |
| 0x13B868 | scene_load_clear | Write 0 to `gp[-0x4c60]` |
| 0x13B874 | scene_load_get | Return `gp[-0x4c60]` |
| 0x13B878 | scene_load_init | Full init: memset(0x6A6DB0, 0, 0x180), scene_load_prep(), scene_block_init(0), loop scene_load_step(1), scene_config_setup(config) with stride 0x200 |

**7 callers** of `scene_load_set`: 0x132B24, 0x1AD2B4, 0x1B1824, 0x1B1A0C, 0x1B44D4, 0x1B45F4, 0x1B4F80.

GP state modified: ONLY `gp[-0x4c60]` (the loading flag).

---

## Finding 7: Resource Check at 0x17B230

Bitmap-based resource readiness check:

```c
bool check_resource_ready(int id) {
    byte_addr = 0x28A520 + (id >> 3);
    bit_mask = 1 << (id & 7);
    return (byte_at[byte_addr] >> (id & 7)) & 1;
}
```

Also has entry points for set/clear at +0x28/+0x58.

---

## Finding 8: Wait/Yield at 0x203AA0

Cooperative yield via EE kernel syscall 50:

```c
void wait_yield(int frames) {
    counter = *(0x274EC0);   // VBlank-incremented frame counter
    divisor = *(0x274EC4);   // timing divisor (default 2)
    iterations = max(1, ((60 - counter) / divisor) / 60);
    for (i = 0; i < iterations; i++)
        yield();              // syscall 50 → thread_yield
}
```

Edge case: if frames==0 and iterations==0, enters infinite yield loop.

---

## Finding 9: VU0 Kick at 0x117768 — NOT a VU0 Kick

Zero COP2/VU0 instructions. This is a **linked-list deferred processing loop**:

```c
void process_deferred_queue(void) {
    head = *(gp - 0x4CC0);
    while (node = head) {
        if (node->state >= 2 && node->state < 4)
            unlink_node(node);   // 0x114FC8: doubly-linked unlink
        node = node->next;
    }
    *(gp - 0x4CB8) = 0;    // clear busy flag
}
```

Node structure: state at +0x44 (int16), prev at +0x48, next at +0x4C. Total node size ≥ 0x50. Helper 0x114FC8 allocates nodes of size 0x1A8 from pool.

Second queue at GP-0x4CBC with state at +0x90.

The "VU0" name is from a string printed when allocating a new node. This is a **per-frame resource cleanup queue**, not a VU0 kick.

---

## Finding 10: GP-Relative Data Map

**GP = 0x006388F0** (confirmed: LUI $a0, 0x0064 → ADDIU $a0, -0x7710 → DADDU $gp, $a0, $zero at 0x0010005C)

**Exception:** The step dispatcher at 0x1B08E4 uses GP=0x27A7A8 (different compilation unit).

**1032 unique GP offsets** referenced by 9971 instructions across .text. All known GP variables in range -0x6F60 to -0x4A4C.

### Consolidated GP Variable Map

| AbsAddr | Offset | Name | Section | Ref Count |
|---------|--------|------|---------|-----------|
| 0x00633F3C | -0x49B4 | (heaviest) | .sbss | 434 |
| 0x00633D2C | -0x4BC4 | halfword_tbl_counter | .sbss | |
| 0x00633E9C | -0x4A54 | scene_load_store_C | .sbss | |
| 0x00633EA0 | -0x4A50 | scene_load_store_A | .sbss | |
| 0x00633EA4 | -0x4A4C | scene_load_store_B | .sbss | |
| 0x00633950 | -0x4FA0 | scene_state_flag | .sdata | |
| 0x00633970 | -0x4F80 | halfword_tbl_base | .sdata | |
| 0x0063301C | -0x58D4 | step_counter | .sdata | |
| 0x00633054 | -0x589C | scene_load_flag_B | .sdata | |
| 0x00633060 | -0x5890 | scene_load_flag | .sdata | |
| 0x006332CC8 | -0x5C28 | cloth_init_gate | .sdata | |
| 0x00631990 | -0x6F60 | world_state_main | .sdata | |
| 0x00631A00 | -0x6EF0 | entity_count_src | .sdata | |
| 0x00631A20 | -0x6ED0 | entity_count_var | .sdata | |
| 0x00631A80 | -0x6E70 | world_state_var | .sdata | |
| 0x006321CC | -0x6724 | callback_mask_reg | .sdata | |
| 0x006321D0 | -0x6720 | sec_mask_reg | .sdata | |

---

## Finding 11: HUD Struct Pool at 0x4Dxxxx

**Zero-initialized runtime allocation pool:** 0x004CAEC0–0x004D06AB (22,508 bytes). All zeros in ELF — runtime-allocated.

**Static template data** at 0x4D06AC+: 8-byte records `{float value; char *label_ptr}` pointing to a display-string table in .rodata (0x61B4XX–0x61B5XX). Strings: `" 0 ( -)"` through `" 32 ( -)"`. Likely a **debug status display** panel.

---

## Finding 12: Debug Callback Table at 0x613E00

### Structure (47 entries × 28 bytes = 1316 bytes)

```c
struct debug_entry {
    uint32_t field_00;    // +0x00: base address (0x4B3000-ish)
    void   (*callback)(); // +0x04: debug rendering callback (0x168650 or NULL)
    char   *name;         // +0x08: descriptive string (in .rodata 0x6144C8+)
    uint32_t rgba;        // +0x0C: visualization color
    uint32_t extra_ptr;   // +0x10: tracked variable ptr (.sdata/.sbss)
    uint32_t pad;         // +0x14: zero
    uint32_t id;          // +0x18: entry type ID
};
```

### Entry Categories

| Category | Color | Count | Examples |
|----------|-------|-------|---------|
| Collision/Cloth | 0x90C0FF80 | 4 | CollisionOldProc, Skelton, ClothInfo, FaceWInterpRatio |
| Motion/Animation | 0x80FFC080 | 7 | MotionActNode, MotionInterporate, MotionDebugWin, MotionDebugTgt, MotionSlopeInterp, CharaTarget, FlyLimitInfo |
| Brain/AI | 0xFF80FF80 | 4 | BrainBar, BrainOnOff, GBrainInfo, WayTool |
| Snap/System | 0xC0C0C080 | 5 | SnapShotNum, SnapSize, SnapForm, JimakuTest, Camera |
| Rendering | 0xFFFF8080 | 10 | RippleRoughness, Bounding, Specular, LightVolume, ShadowOff, Scissoring, FullScreenEffect, DispClusterModel, DispNormalModel, DispLwsModel, DispParticle, DispMesh |
| Input/Debug | 0xC0C0C080 | 10 | DISPLAY BRIGHTNESS, STICK INPUT, STICK SIMULATE, DEBUG SUB THREAD, NEW QUEEN BATTLE, CHAIN CYCLE SPEED, CHAIN SLOW SPEED, DISP ENEMY STATE, ENEMY BATTLE TYPE, ENEMY FLY WITH GIRL |
| Gameplay | 0xC0C0C080 | 5 | DISP ESCORT BALL, GIRL DETOUR, LWSKYOMI LOOKONLY, ONE HIT ONLY, IGNORE DODGE |

### Key Finding: `0x00168650` Debug Dump Callback

Only entry 0 (CollisionOldProc) has a non-null callback: **0x00168650** — the debug dump function referenced throughout the project. This is a **debug visualization rendering callback**, not a production code path.

The string table occupies .rodata 0x6144C8–0x61479F. This is a **runtime debug overlay system** showing tracked variables on screen during development.

---

## What Is Confirmed

1. **8-step scene loading state machine** at 0x1B08E4 in `kanban.c` with GP=0x27A7A8
2. **Descriptor table** at 0x2A31B8: 68 entity types with init_fn, cb_routines, and vtable pointers
3. **BARREL uses `0x1D3A30`** as physics constraint solver — resolves the ROPE callback registration gap
4. **Entry table** at 0x2A4C48: 512 instance spawn positions/classes
5. **Slot 0 callback** 0x168DA8: unfiltered iterater (vs slot 1's filtering) — never selected at runtime
6. **Scene loader helper** 0x13B858: simple flag set/clear at gp-0x4C60
7. **Resource check** 0x17B230: bitmap at 0x28A520 with set/clear sub-entries
8. **Wait/yield** 0x203AA0: cooperative yield via syscall 50 with VBlank counter at 0x274EC0
9. **VU0 "kick" 0x117768**: linked-list deferred processing queue (NOT a VU0 operation)
10. **1032 GP offsets** used across 9971 instructions; the most active is -0x49B4 (434 refs)
11. **Debug table 0x613E00**: 47 entries for runtime debug overlay; only 0x168650 has a callback

## What Is Resolved

- **ROPE callback registration gap** (Rev.033-037): `0x1D3A30` is registered through the **descriptor table** at +0x50 (cb_routine2 for BARREL entity type), not through callback_register at 0x13F7A8
- **VU0 "kick" naming**: The function at 0x117768 is NOT a VU0 operation — it processes a deferred linked-list queue
- **Slot 0 silence**: The slot 0 callback has no filtering logic but slot index 0 is never selected by the dispatch system

## What Is Unknown

- The semantics of the 8 core dispatch entities (inferred: ICO, Yorda, enemies, interactive objects)
- What specific dispatch selection produces slot index 0 (never observed at runtime)
- Full contents of the 404-byte stage config beyond known fields
- The meaning of most GP-relative variables (-0x49B4 with 434 refs is a prominent unknown)

## Next Steps (all require runtime)

1. Probe world_state (gp-0x6F60) during room transitions
2. Probe mask register toggle during cutscenes
3. Capture bounding box at halfword table writers to verify spatial hash
4. Investigate slot 0 selection mechanism (when does it fire?)
5. Capture GP-0x49B4 to understand what the most-referenced variable is

---

## Related Files

- `research/elf/ghidra-rev076-post-runtime-consolidation.md`
- `research/elf/ghidra-rev075-init-fn-callback-dispatch-and-asm-handler-consolidation.md`
- `research/elf/ghidra-rev074-runtime-session-main-loop-dispatch-confirmed.md`
- `docs/data-model.md`
- `docs/system-feature-flows.md`
- `docs/backlog.md`
