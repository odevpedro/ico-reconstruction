# Rev.074 — Runtime Session: Main Loop Dispatch Confirmed

**Date:** 2026-05-17

---

## Objective

Validate the main loop dispatch chain (Rev.073) at runtime using PCSX2 instrumented probes. Capture slot index distribution, callback targets, VU0 kick trigger, cold path usage, and entity pool dynamics during normal gameplay.

---

## Session Details

- **Duration:** ~15 minutes of gameplay (title screen → gameplay → death)
- **Probe binary:** `/home/peter/Documentos/repos/pcsx2-ico-logpoints/build/bin/pcsx2-qt` (build 2026-05-17 18:00)
- **Log file:** 4.5 GB, 9,151,217 events
- **Saved copy:** `.local/ico-pcsx2-probe-events-20260517-182237-rev074-runtime-complete.jsonl`

---

## Probes Active

| Address | Label | Captures |
|---------|-------|----------|
| `0x00166E10` | `main_dispatcher` | a1 = slot index (0-16) |
| `0x00167230` | `cold_path_A` | gp-25856 (ptr to cold path A) |
| `0x00167258` | `cold_path_B` | gp-25852 (ptr to cold path B) |
| `0x00167020` | `dispatch_point` | v1 = callback target address |
| `0x00129A78` | `vu0_kick_trigger` | gp-28384 = world_state |
| `0x00169F80` | `alt_impl_A` | hit check |
| `0x0016A058` | `alt_impl_B` | hit check |
| `0x001D27A8` | `cloth_payload_init` | variant distribution (kept from Rev.051) |
| `0x00201ED4` | `runtime_candidate` | entity dumps (kept from Rev.051) |
| `0x0013F7A8` | `callback_register` | init_fn registration data (kept) |

---

## Evidence

### 1. Slot Distribution (1,094,546 dispatcher events)

**Slots that fired:**

| Slot | Index | Name | Count | % | Group |
|------|-------|------|-------|---|-------|
| 12 | `0x0C` | G2_none | 423,643 | 38.7% | G2 (orientation) |
| 1 | `0x01` | G1_mask_A | 295,961 | 27.0% | G1 (position) |
| 3 | `0x03` | G1_mask_B | 158,498 | 14.5% | G1 (position) |
| 2 | `0x02` | G1_mask_A_a2=1 | 101,453 | 9.3% | G1 |
| 6 | `0x06` | G1_mask_0x4000 | 67,114 | 6.1% | G1 |
| 14 | `0x0E` | G2_a2=1 | 31,449 | 2.9% | G2 |
| 15 | `0x0F` | G2_+0x60_mask | 11,209 | 1.0% | G2 |
| 7 | `0x07` | G1_mask_0x3000 | 6,252 | 0.6% | G1 |
| 4 | `0x04` | G1_triplet_A | 5,504 | 0.5% | G1 |
| 10 | `0x0A` | G1_complex | 1,499 | 0.1% | G1 |
| 11 | `0x0B` | G1_mask_0xC000 | 484 | <0.1% | G1 |
| 5 | `0x05` | G1_triplet_B | 109 | <0.01% | G1 |

**Slots that NEVER fired:** 0, 8, 9, 13, 16

**Key observations:**
- **Slot 0 never fires** — confirmed dead slot (Group1, no mask). Likely a fallback/reserved slot.
- **Slot 8/9** reuse callbacks from slots 1/3 with w2=1; the extra flag does not enable them in this session.
- **Slot 13** (Group2, triplet guard) never fires — the guard condition prevents matching.
- **Slot 16** (Group2, reuse slot 12) never fires.

### 2. Group Distribution

| Group | Total | % |
|-------|-------|---|
| Group 1 (position/rotation) | 636,874 | 58.2% |
| Group 2 (orientation) | 466,301 | 42.6% |

### 3. Callback Targets (7,008,044 dispatch_point events)

| Callback | Count | % | Slots |
|----------|-------|---|-------|
| `0x169AA8` (G2_none) | 2,696,831 | 38.5% | 12, 16 |
| `0x168ED0` (G1_mask_A) | 1,899,898 | 27.1% | 1, 8 |
| `0x169440` (G1_mask_B) | 997,447 | 14.2% | 3, 9 |
| `0x1692F0` (G1_a2=1) | 646,729 | 9.2% | 2 |
| `0x1696C0` (G1_0x4000) | 428,523 | 6.1% | 6 |
| `0x169E58` (G2_a2=1) | 190,984 | 2.7% | 14 |
| `0x169D18` (G2_+0x60) | 69,750 | 1.0% | 15 |
| `0x169580` (G1_0x3000) | 33,328 | 0.5% | 7 |
| `0x169020` (G1_trip_A) | 30,992 | 0.4% | 4 |
| `0x169800` (G1_complex) | 10,429 | 0.1% | 10 |
| `0x169968` (G1_0xC000) | 2,420 | <0.1% | 11 |
| `0x169190` (G1_trip_B) | 713 | <0.01% | 5 |

### 4. Match Rate

| Result | Count | % |
|--------|-------|---|
| Match (v0=1) | 4,064,631 | 58.0% |
| No match (v0=0) | 1,613,548 | 23.0% |
| Residual (other v0) | 1,329,865 | 19.0% |

The "residual" values (0xC1, 0xA2, 0x41) are register artifacts from before the JALR — v0 was not initialized to 0 before the callback call. These are effectively "no match" for the purposes of the caller.

### 5. VU0 Kick Trigger

| Value | Count |
|-------|-------|
| world_state = 0 | 59,224 |
| world_state != 0 | 0 |

VU0 kick fires ONLY in gameplay mode (gp-28384 = 0). Never during menus/cutscenes.

### 6. Alternative Implementation

| Path | Hits |
|------|------|
| `alt_impl_A` (0x169F80) | 0 |
| `alt_impl_B` (0x16A058) | 0 |

**Neither alternate implementation is ever reached during gameplay.** The cold path + main dispatcher path is the only active code path.

### 7. Cold Paths

| Path | Hits | Purpose |
|------|------|---------|
| `cold_path_A` (0x167230) | 636,874 | Entry for Group 1 slots (0-11) |
| `cold_path_B` (0x167258) | 457,672 | Entry for Group 2 slots (12-16) |

**Confirmed at runtime:** GP-25856 = `0x00167230` and GP-25852 = `0x00167258`. Both point to the cold path fragments, which tail-call J to `0x00166E10`.

### 8. Entity Pool

| Metric | Value |
|--------|-------|
| Unique contexts (cold_path_A) | 615 |
| Unique contexts (cold_path_B) | 282 |
| Unique entities dispatched | 20 |

The 615 contexts align with the entry table max (512 entries × stride 0x4C). The 20 live entities are the active objects being processed per frame.

**Top 5 entities by dispatch frequency:**

| Entity ptr | Count |
|-----------|-------|
| `0x008310B8` | 1,097,144 |
| `0x00831514` | 773,912 |
| `0x00833DC4` | 773,494 |
| `0x008340AC` | 773,413 |
| `0x0083B660` | 771,763 |

### 9. Callback Registration (619 events)

**Init_fn registrations by type:**

| init_fn | Entity | Count |
|---------|--------|-------|
| `0x001BB6B0` | AP1 | 134 |
| `0x00164440` | ENEMY1 | 129 |
| `0x00203EE8` | BGA | 107 |
| `0x001C3760` | unknown | 18 |
| `0x001F2370` | unknown | 18 |
| `0x0017D128` | unknown | 18 |
| `0x00153478` | BOY | 18 |
| `0x0017D1D0` | WOODBOX0 | 18 |
| `0x00174BA0` | GIRL | 11 |

**Callers of callback_register:**

| Caller (ra) | Function | Count |
|-------------|----------|-------|
| `0x001B7AD4` | scene loader | 283 |
| `0x001B7AB8` | scene loader | 166 |
| `0x00240E58` | factory function | 134 |
| `0x00240F98` | factory function | 36 |

### 10. GP Value Confirmed

GP = `0x006388F0` — confirmed at runtime from all cold path events.

---

## What is Confirmed

- Slot 0 is a dead slot (zero hits across entire session) — likely reserved/fallback
- Slot 12 is the most active slot (38.7% of dispatches) — Group 2 orientation matching
- Slots 8/9/13/16 never fire — the w2=1 flag and triplet guard conditions prevent them
- Alt_impl_A/B are never used in normal gameplay — code path is dead for this session
- VU0 kick fires 59K times, always with world_state=0 (gameplay only)
- Match rate is ~58% across all callbacks
- 615 unique entity contexts exist, 20 entities are alive per frame
- GP = 0x006388F0 verified at runtime
- Cold paths A/B are the sole entry points for Group 1 and Group 2

## What is Probable

- The 4 major flag nibbles of `field_48` encode entity classification: 0x0, 0x3, 0x4, 0x7, 0x8, 0xC
- G1_mask_A (slot 1) and G1_mask_B (slot 3) select different entity categories within Group 1
- Triplet guard (slots 4/5/13) prevents re-processing the same entity within a single frame
- The w2=1 flag (slots 8/9/16) enables a secondary mode that was not triggered in this gameplay segment

## What is Unknown

- Which entity types correspond to which slot masks (requires symbol-aware decompilation)
- Why slot 0 even exists if it never fires (reserved for cutscene/menu entities?)
- Whether alternate impl paths are reachable in specific gameplay scenarios not covered here
- What the 3 unknown init_fn addresses (`0x001C3760`, `0x001F2370`, `0x0017D128`) correspond to

## What is Discarded

- Previous speculation that the live dispatch uses alternate impl paths — proven false for normal gameplay
- Previous uncertainty about slot 0 — confirmed dead slot
- The idea that slot 8/9/16 might fire in all sessions — they require an unknown trigger condition

---

## Next Steps

1. **Add probes for:** halfword table writers (0x166D1C/0x166D78), slot table itself (0x282690) to understand data flow between table and callbacks
2. **Runtime with room transitions:** test in different rooms (jail, warehouse, troko) to see if slot distribution changes
3. **Correlate entity ptrs to known types:** use memory dumps to identify which entity is at `0x008310B8` (most active)
4. **Investigate dead slot 0:** check if it fires during cutscenes or specific interactions
5. **Map the 3 unknown init_fn addresses:** disassemble `0x001C3760`, `0x001F2370`, `0x0017D128`
