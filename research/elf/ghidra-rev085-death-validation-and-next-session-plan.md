# Rev.085 — Death Validation: mask_set Not Triggered (Session Same as Rev.084)

**Date:** 2026-05-18
**Context:** Continuation of Rev.084 session — user died intentionally (jumped off cliff)
**Total events:** 44,178,120 (43.8M from Rev.084 + ~400K death/pause events)
**Data source:** Copied from unlinked inode after gzip completed on Rev.084 log
**Log file:** `.local/runtime-captures/ico-probe/events-rev085-raw.jsonl` (15GB raw / compressing)

---

## Objective

Validate whether `mask_set (0x13ED40)` fires during death/game-over transitions.

---

## Method

User continued the Rev.084 PCSX2 session (same 9 probes active, GP=0x006388F0).
After jumping off a cliff, the game entered death animation → continue screen → user paused.

Tail analysis of 200K events (~3 min of gameplay) covering the death transition.
Detected 3 zones in the tail:

| Zone | Duration | Slots Active | Interpretation |
|------|----------|--------------|---------------|
| 0 | ~1.5 min | 0x01+02+03+04+06+07+0a+0c+0e+0f | Normal gameplay (3rd area) |
| 1 | ~30s | 0x01+02+03+04+07+0c | Gameplay, different room mix |
| 2 | ~60s+ | **0x0c only** (slot 12) | **Death/cutscene/continue screen** |

---

## Results

### Rare Probes During Death Zone (Zone 2)

| Probe | Hits | Interpretation |
|-------|------|---------------|
| mask_set (0x13ED40) | **0** | Does NOT fire during death |
| halfword_store_A (0x166D1C) | 0 | Confirmed N/A |
| halfword_store_B (0x166D78) | 0 | Confirmed N/A |
| slot0_v0_from_gp (0x683CC) | 0 | Confirmed dead |
| alt_selection (0x168650) | 0 | Confirmed dead |

### Death Zone Characteristics

- **100% slot 12 (Group 2 only)** — identical to cutscene behavior
- Group 1 slots (1-11) are completely suspended during death
- Slot 12 continues to run (Group 2: orientation/origin matching)
- This matches the pattern observed in the Rev.084 cutscene (zone 11, also 100% slot 12)
- mask_set never fires — death reload does NOT trigger I/O cancellation

---

## Conclusions

### Confirmed

1. **mask_set (ShockRequestBox_RequestCancel) is NOT a gameplay death/state callback.**
   - 66.9M events across 3 sessions (entrance, windmill, extended, death) — zero hits during gameplay
   - Only 2 hits total (both during loading transitions in Rev.079)
   - Even intentional death sequence produces zero hits
   - **Name matches function correctly:** It cancels I/O request boxes during data loading

2. **Death and cutscene share the same dispatch state:** 100% slot 12 (Group 2 only), Group 1 suspended.

3. **Group 1 = gameplay entities only.** When the game is not in gameplay mode (menu, cutscene, death), Group 1 is entirely inactive.

### What This Means

The `mask_set` function at `0x13ED40` implements `ShockRequestBox_RequestCancel` (confirmed in ICO-decomp as fumi/ios/shockdriver.c). It cancels pending I/O request boxes when a scene transition triggers a data load. It has nothing to do with:
- Entity state management
- Death/game-over transitions
- Menu state
- Gameplay mode switching

The GP mask register (gp+0x98DC) that mask_set writes to only has callers in the scene loader range (6 callers found in Rev.075). This is consistent with I/O request cancellation during scene asset loading.

---

## Next Session: Probe Reconfiguration

Current probes (9 addresses, used for Rev.074/079/084):

| Address | Label | Keep? | Reason |
|---------|-------|-------|--------|
| 0x00166E10 | main_dispatcher | **KEEP** | Core slot distribution data |
| 0x00167020 | dispatch_point | **KEEP** | Callback targets |
| 0x00167230 | cold_path_A | **KEEP** | Entity contexts |
| 0x00167258 | cold_path_B | **KEEP** | Entity contexts |
| 0x0013ED40 | mask_set | **DROP** | Confirmed I/O only, zero gameplay hits |
| 0x00166D38 | halfword_store_A | **DROP** | Zero hits in 4 game areas |
| 0x00166D94 | halfword_store_B | **DROP** | Zero hits, redundant |
| 0x001683CC | slot0_v0_from_gp | **DROP** | Dead slot (confirmed static analysis) |
| 0x00168650 | alt_selection | **DROP** | Cold paths always used |

### Proposed new probe configuration (5 probes, ~30% log reduction)

| Address | Label | What it captures |
|---------|-------|-----------------|
| 0x00166E10 | main_dispatcher | slot_index, a0=context |
| 0x00167020 | dispatch_point | v1=callback, a1=struct |
| 0x00167230 | cold_path_A | a0=context |
| 0x00167258 | cold_path_B | a0=context |
| **0x00631990** | **world_state** | **gp+0x6F60 read — track room transitions** |
| **0x00633F3C** | **gp_0x49B4** | **Most-referenced GP var (434 refs) — purpose unknown** |

### Rationale

- Remove 4 dead/misunderstood probes that produce zero data
- Add world_state probe: captures room transitions to map slot distribution to specific rooms
- Add gp-0x49B4 probe: the most-referenced GP variable (434 refs), purpose still unknown
- Reduces log size from ~3.2KB per dispatch cycle to ~2.2KB (~30% reduction)
- Estimated log size: ~9-10GB for 2h session instead of 14GB
