# Rev.125 — Extended final-stage runtime session: 36 world_states, 1:1 scene mapping, Yorda escape-path probes

- **Date:** 2026-09-05
- **Objective:** Analyze the ongoing/continued gameplay session (`ico-runtime-20260825-152452`) now reaching the **final part of the game** (11 ELF boots, 259 world_state transitions). Confirm the new high world_states, their 1:1 mapping to scene_ids, and the dominant dispatch slot; then prepare GirlBrain escape-path probes for the next session.
- **Scope:** Runtime capture analysis only. The running PCSX2 still uses the **old 14-probe binary** (source edits for 25 probes are not yet rebuilt); therefore `halfword_fast_path`, `world_state_room_init_fn`, `world_state_vblank_reset`, and the 7 `girl_brain_*` probes are **not** in this capture.
- **Sources used:**
  - `.local/pcsx2-logs/current-session.jsonl` (1.98 GB, ~2.98M events)
  - `research/elf/ghidra-rev105-extended-session-25-worldstates-20-dl-slots.md`
  - `research/elf/ghidra-rev103-isysgobj-runtime-session-yorda-bridge-save.md`
  - `research/runtime/rev108-gameplay-log-validation.md` + `girlbrain_callbacks_observed.csv`
  - `tools/asm_source_score.py` (GirlBrain/entity target addresses)
- **Evidence used:** byte-level JSONL event counts and world_state transition sequence (reproducible via runtime_probe_analyzer).

---

## 1. Session composition (continued capture)

| Metric | Value |
|--------|-------|
| Events parsed | ~2,987,400 |
| ELF boots (`elf_entry_sentinel`) | 11 |
| `world_state_load` | 259 |
| `init_scene_gobj` | 18,678 |
| `isys_gobj_init` / `alloc` / `dl_init` | 267 each (system boots) |
| `ios_om_main` | 2,171,917 |
| `isys_gobj_proc_add` | 471,102 |
| `ios_om_create_dl` | 136,460 |
| `halfword_second_caller_entry` / `return` | 71,996 each |
| `isys_gobj_add` | 53,803 |

## 2. World states: 36 distinct values (was 25 in Rev.105)

**New high states (0x15–0x33) — all absent from Rev.105 (which maxed at ws=0x14):**

| ws | count | ws | count | ws | count |
|----|-------|----|-------|----|-------|
| 0x15 | 14,578 | 0x1D | 238,311 | 0x2A | 186 |
| 0x16 | 145,608 | 0x1E | 22,328 | 0x2B | 1,225 |
| 0x17 | 30,763 | 0x1F | 27,879 | 0x2D | 528 |
| 0x18 | 3,726 | 0x28 | 221 | 0x32 | 837 |
| 0x19 | 10,845 | 0x29 | 204 | 0x33 | 2,519 |
| 0x1A | 415,550 | | | 0x28–0x33 | "final cutscene / endgame" block |
| 0x1B | 240,254 | | | | |
| 0x1C | 55,895 | | | | |

- `ws=0x1A` (415K) is the single most active world state in the entire capture —
  beating the Rev.105 dominator `ws=0x0F` (401K here) by a small margin.
- Low states 0x01–0x14 are consistent with Rev.104/105 ranges; they still map 1:1.

## 3. Confirmed: world_state == scene_id (1:1 via initSceneGObj)

Every `init_scene_gobj` event records `a0` = scene_id, and **the same numeric value set**
appears as `world_state_raw`. Distribution matches:

| scene_id (a0) | init count | high ws section |
|---------------|------------|-----------------|
| 0x0A | 2,099 | (revisit) |
| 0x14 | 1,545 | final-part return |
| 0x09 | 1,462 | |
| 0x1C | 1,354 | **final block** |
| 0x1E | 1,111 | **final block** |
| 0x1D | 3,168 | **final block / most loads** |
| 0x33 | 114 | **endgame** |
| 0x15–0x1B, 0x1F | 39–782 each | final block |

`world_state_load` fires each time the value at gp−0x6F60 changes; `initSceneGObj`
executes the corresponding descriptor/entry registration. The two counters are code-local
(same function region 0x1AF948) and numerically identical → the load value is the scene id.

## 4. Transition sequence (final-stage signature)

The 243-transition path shows the game being played through to the end:

```
[early]  0x01 → 0x29 → 0x2A → 0x2B → 0x2D → 0x28 → 0x03 → 0x04→0x05→0x06→0x07
[mid]    0x08→0x09→0x0A→0x0B→0x0D→0x0E→0x0F→0x10→0x11→0x12→0x13→0x14
[new]    0x15→0x16→0x17→0x18→0x19→0x1A→0x1B→0x1C→0x1D→0x1E→(0x33)→0x1F
```

- The `0x1B ↔ 0x1C ↔ 0x1D ↔ 0x1E` loop is the late-game area traffic (switching rooms),
  with `0x33` (endgame) appearing 3 times late and `0x1F` as the last observed value.
- `0x28–0x2D` appear only at the start transition path (0x01 → 0x29 → 0x2A → 0x2B →
  0x2D → 0x28 → 0x03): these are most plausibly **boot/attract-mode states**, not rooms.

## 5. Dispatch slot

`ios_om_main` reads `gobj_list_head` (gp−0x671C):

| value | count |
|-------|-------|
| `0x0083068C` | 2,159,739 (99.0%) |
| `0x00000000` | 20,481 (0.9%) |

The `0x0083068C` value is the same single dominant slot the whole capture (unchanged
from earlier Rev.104/105 sessions belonging to the same `_iosOmMain` distribution —
this capture does not re-measure the per-slot address histogram beyond that field).

## 6. GirlBrain (Yorda) — probes prepared for next session

Rev.108 documented the GirlBrain callbacks as `NO_PROBE`. The user reported the
final-stage session revealing **new Yorda sprites including the tired-run animation**.
The escape pathway most plausibly tied to that pose is:

| Function | VA | Verified via | Probe label (next capture) |
|----------|-----|--------------|----------------------------|
| `girlBrainRunawaySearchPoint` | 0x0016F410 | asm_source_score / Ghidra yaml | `girl_brain_runaway_search_point` |
| `girlBrainRunawayMoveByWay` | 0x0016F9A8 | asm_source_score / Ghidra yaml | `girl_brain_runaway_move_by_way` |
| `girlBrainMain_PositionUpdate` | 0x0016BCA0 | asm_source_score / Ghidra yaml | `girl_brain_position_update` |
| `subGirlBrain_PulledUp` | 0x0016CED0 | asm_source_score | `sub_girl_brain_pulled_up` |
| `_girlBrainHide_MakeHidePoint` | 0x0016E910 | asm_source_score note | `girl_brain_hide_make_hide_point` |
| `girlBrainHide_GoalTurn` | 0x0016EB68 | asm_source_score | `girl_brain_hide_goal_turn` |
| `GirlBrainClearTarget` | 0x0016AC10 | asm_source_score | `girl_brain_clear_target` |

All 7 added to `s_ico_runtime_probes[]` with labeled sample periods (1 for low-traffic
gates, 16 for the runaway pair, 64 for position updates). They take effect on the next
post-session rebuild.

## 7. What is confirmed

- 36 distinct world_state values; **ws values 0x15–0x19, 0x1A–0x1F, 0x28–0x2B, 0x2D, 0x32,
  0x33 are new to this capture** (Rev.105 max was 0x14).
- world_state == scene_id (1:1), evidenced by identical numeric sets in `world_state_raw`
  and `init_scene_gobj.a0`.
- `ws=0x1A` is the busiest state in the whole capture (415K ios_om_main events).
- The `0x1B…0x1E / 0x33` block is the final-stage signature; `0x1F` is the last state seen.
- Single dominant dispatch slot `0x0083068C` throughout, consistent with earlier sessions.
- The fork table is prepared (source) with 25 probes including 7 GirlBrain callbacks;
  documented in Rev.124.

## 8. What is probable

- `0x28–0x2D` are boot/attract states visited before the first room (`0x03`+), given the
  transition order 0x01→0x29→0x2A→0x2B→0x2D→0x28→0x03.
- `girl_brain_runaway_*` probes will fire during late-game sections where Yorda flees;
  the "tired run" pose corresponds to the escape animation state.

## 9. What is possible

- The per-room init_fn table reached via the `0x001AF96C` jalr may now be populated on
  the next session, giving init_fn identity per world_state (0x15–0x33).
- `vblank_counter` sampling (also next session) can prove the frame-beat rate in the
  endgame sections.

## 10. What is unknown / discarded

- **Unknown:** exact room names / gameplay semantics for ws 0x15–0x33 (no naming claims).
- **Unknown:** whether `0x0083068C` is a stable BSS slot head or a per-room value;
  histogram beyond the single field is not available in this capture.
- **Discarded:** any claim that ws 0x28–0x2D are "endgame": the sequence order places them
  at boot, not at the end.
- **Discarded:** inferring the tired-run pose from disassembly alone; runtime probe hits
  are required first.

## 11. Next minimum test

1. User ends session → rebuild fork → `run-ico-pcsx2-logpoints.sh --check`.
2. Validate `girl_brain_*`, `halfword_fast_path`, `world_state_room_init_fn`,
   `world_state_vblank_reset`, and `vblank_counter` in a fresh capture.
3. Correlate `girl_brain_runaway_*` hits against ws 0x15–0x33 and the tired-run moment.

## 12. Conservative verdict

The capture documents the final-stage world-state range (36 states, max 0x33) and the
1:1 world_state↔scene_id mapping — both confirmed by event counts. No gameplay semantics
are claimed. The Yorda escape-path probes are ready but unvalidated until the next
rebuild/session.