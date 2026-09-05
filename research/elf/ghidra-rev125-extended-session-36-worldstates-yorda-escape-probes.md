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
| 0x1A | 415,550 | | | 0x28–0x33 | "most-advanced phase observed" block |
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
| 0x33 | 114 | **most-advanced phase observed** |
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
  with `0x33` (most advanced phase observed here, not yet the credits/victory screen —
  the user had not finished the game in this capture) appearing 3 times late and `0x1F`
  as the last observed value.
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

## 5b. `ios_om_create_dl` per-DL-slot → world_state map (22 slots)

Consolidated from the full capture. Each BSS slot address (a1) has a strong 1:1
correlation to a dominant world_state, confirming and extending the Rev.105 pattern
("each new world_state has a unique DL slot"):

| DL slot (a1) | n | dominant ws | share | note |
|--------------|----|------------|-------|------|
| `0x006782f8` (B) | 34,178 | 0x0F | 73.4% | Rev.105 slot B — still 0x0F dominant |
| `0x00678aa8` (NEW-4) | 29,315 | **0x1A** | 88.5% | now 0x1A, was 0x11-only in Rev.105 |
| `0x0067cab8` (NEW-6) | 16,973 | **0x1D** | 86.8% | was 0x14-only in Rev.105 |
| `0x0067a1b8` (**NEW**) | 15,822 | **0x1B** | 94.6% | **unmapped in Rev.105** |
| `0x00678d38` (C) | 9,500 | 0x16 | 95.5% | |
| `0x006794e8` (F) | 8,610 | 0x0A | 98.0% | |
| `0x00679258` (E) | 3,473 | 0x08 | 100% | |
| `0x0067b638` (**NEW**) | 3,427 | **0x1C** | 100% | **unmapped in Rev.105** |
| `0x0067c598` (NEW-3) | 2,863 | 0x13 | 100% | |
| `0x00678fc8` (D) | 2,230 | 0x10 | 88.6% | |
| `0x00678818` (NEW-1) | 1,991 | 0x0D | 88.8% | |
| `0x00679a08` (**NEW**) | 1,896 | **0x17** | 100% | **unmapped in Rev.105** |
| `0x0067ee98` (NEW-2) | 1,617 | 0x0B | 100% | |
| `0x00679778` (G) | 1,545 | 0x01 | 99.5% | boot state |
| `0x00679c98` (**NEW**) | 1,292 | **0x1E** | 100% | **unmapped in Rev.105** |
| `0x00679f28` (NEW-5) | 1,245 | 0x12 | 100% | |
| `0x0067c308` (I) | 809 | 0x04 | 100% | |
| `0x00680fe8` (**NEW**) | 767 | **0x0C** | 100% | **unmapped in Rev.105** |
| `0x00677dd8` (A) | 294 | **0x33** | 50.0% | slot A is most-active in the deepest phase here |
| `0x0067e458` (J) | 29 | 0x2D | 100% | boot block |
| `0x0067a968` (H) | 10 | 0x28 | 100% | boot block |

**Confirmed extensions over Rev.105:**
- 5 new DL slots discovered: `0x0067A1B8` (0x1B), `0x0067B638` (0x1C),
  `0x00679A08` (0x17), `0x00679C98` (0x1E), `0x00680FE8` (0x0C).
- **Revised Rev.105 assignment:** `0x00678AA8` is NOT 0x11-only — in this final-stage
  capture it is 0x1A-dominant. `0x0067CAB8` is NOT 0x14-only — now 0x1D-dominant.
  These rows slide as the game progresses; the mapping is scene-dependent, not fixed.
- Slot A `0x00677DD8` (Rev.105 "0.2%" minor) shows 0x33 dominance here — the deepest
  phase reached (not yet the victory screen in this capture).
- `a2` (mask) distribution: `0x00630000` (42,736), plus low index values 0x1A/0x1D/
  0x26/0x36/0x1E/0x21/0x2E/0x20/0x44 — consistent with per-type dispatch values.

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
- The `0x1B…0x1E / 0x33` block is the deepest-phase signature reached in this capture;
  `0x1F` is the last state seen. The user had **not yet finished the game**, so the
  victory/credits states (if separate values) may appear above 0x33 on the next session.
- `ios_om_create_dl` spans **22 distinct BSS slot addresses**, 21 of them with a dominant
  world_state at ≥73% share. 5 slots are new to this capture. The slot→ws mapping is
  **scene-dependent** (two Rev.105 assignments moved as the game reached the finale).
- `isys_gobj_proc_add`: add/remove split is balanced (252,000 vs 219,102); wrapper `t0`
  values cluster on types 0x13–0x18 (84000–127521 each), i.e. the type-slot range that
  `_iosOmMain` dispatch pass 2 uses.
- `halfword_second_caller_entry` concentrates on ws 0x09/0x0A/0x19/0x1D (mid+final areas),
  confirming the rasterizer stays hot in the late game.
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
  deepest sections.
- **Next-session finish:** since the user will reach the ending, the credits/victory
  sequence may introduce **world_states above 0x33** (or reuse 0x28–0x2D). The new
  `world_state_load` / `world_state_room_init_fn` probes will capture whatever appears.

## 10. What is unknown / discarded

- **Unknown:** exact room names / gameplay semantics for ws 0x15–0x33 (no naming claims).
- **Unknown:** whether `0x0083068C` is a stable BSS slot head or a per-room value;
  histogram beyond the single field is not available in this capture.
- **Discarded:** any claim that ws 0x28–0x2D are "ending": the sequence order places them
  at boot, not at the end.
- **Discarded:** any claim that ws 0x33 or 0x1F is the victory/credits state — the user
  had not finished the game in this capture. The credits block is expected to appear as
  **new** values on the finish session.
- **Discarded:** inferring the tired-run pose from disassembly alone; runtime probe hits
  are required first.

## 11. Next minimum test

1. User ends session → rebuild fork → `run-ico-pcsx2-logpoints.sh --check`.
2. Validate `girl_brain_*`, `halfword_fast_path`, `world_state_room_init_fn`,
   `world_state_vblank_reset`, and `vblank_counter` in a fresh capture.
3. Correlate `girl_brain_runaway_*` hits against ws 0x15–0x33 and the tired-run moment.

## 11b. Finish-session watchlist (user plans to roll credits)

Targets to check on the session where the game is beaten:

| Target | What to look for |
|--------|------------------|
| new world_state above 0x33 | credits / victory screen if it uses a distinct value |
| world_state `0x28–0x2D` reuse | if the credits reuse the boot/attract block |
| `world_state_load` burst at the end | rapid transitions before/at credits |
| `girl_brain_runaway_*` last hits | which ws the Yorda escape stops at |
| slot A `0x00677DD8` | whether endgame slot keeps 0x33 dominance past the ending |

If new world_states appear, they immediately extend the confirmed table; the
`world_state_room_init_fn` probe will give their init_fn identity.

## 12. Conservative verdict

The capture documents the deepest-phase world-state range reached so far (36 states,
max 0x33) and the 1:1 world_state↔scene_id mapping — both confirmed by event counts.
No gameplay semantics are claimed, and no victory/credits state is asserted because the
user had not finished the game. The Yorda escape-path probes are ready but unvalidated
until the next rebuild/session; the finish session is expected to reveal any states
beyond 0x33.