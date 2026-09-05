# Rev.126 — Finish session: game beaten, 58 world_states, 0x3D max, credits sequence with return to boot (0x01)

- **Date:** 2026-09-05
- **Objective:** Analyze the runtime capture of the full-game playthrough that ended
  with the credits (user reported "game zerado!!!").
- **Scope:** Same instrumented fork capture `ico-runtime-20260825-152452` (14-probe
  binary that was already in memory; the rebuilt 25-probe binary was not yet loaded —
  this session uses the old probe set).
- **Sources used:**
  - `research/elf/ghidra-rev125-extended-session-36-worldstates-yorda-escape-probes.md`
  - `research/elf/rev124-runtime-probe-prep-and-game-loop-scene-bridge.md`
- **Evidence used:** `.local/pcsx2-logs/ico-runtime-20260825-152452.jsonl`
  (2.4 GB, **3,650,258 well-formed events**, 1 malformed line at ~1039188).
  `verify_runtime_probe_log.py` → `status=valid`.
- **Method:** count events per label; extract all `world_state_load` transitions;
  enumerate new world states vs Rev.125 (which had capped at 36 states / 0x33).

## 1. Session totals (full playthrough, ending with credits)

| Label | Count |
|-------|-------|
| `ios_om_main` | 2,553,499 |
| `isys_gobj_proc_add` | 521,805 |
| `halfword_second_caller_entry` | 167,047 |
| `halfword_second_caller_return` | 167,047 |
| `ios_om_create_dl` | 160,309 |
| `isys_gobj_add` | 59,160 |
| `init_scene_gobj` | 20,158 |
| `isys_gobj_init` | 308 |
| `isys_gobj_alloc` | 308 |
| `isys_gobj_dl_init` | 308 |
| `world_state_load` | 298 |
| `elf_entry_sentinel` | 11 |
| **Total** | **3,650,258** |

## 2. World state coverage — 58 distinct values (up from 36 in Rev.125)

All 36 values from Rev.125 are re-observed, plus **22 new** states reached only in the
final hours of the playthrough (including the credits block):

`0x20 0x21 0x22 0x23 0x24 0x25 0x27` and `0x2C 0x2E 0x2F 0x30 0x31 0x32 0x34 0x35 0x36 0x37 0x38 0x3A 0x3B 0x3C 0x3D`.

- **Max observed world state:** `0x3D` (61).
- Full sorted set: `0x01 0x03-0x25 0x27-0x38 0x3A-0x3D`.
  (gaps: `0x02 0x26 0x39` never seen in any ordered stage traversal.)

## 3. The credits / final sequence (from `world_state_load`, last 60 transitions)

After the deep-game traffic (`0x1B↔0x1C↔0x1D↔0x1E` loop, `0x33`), the endgame path is:

```
0x1F → 0x20 → 0x1F → 0x20 → 0x21 → 0x22 → 0x23 → 0x22 → 0x2E → 0x25 → 0x2F → 0x36 → 0x30 → 0x35 → 0x34 → 0x2F → 0x2C → 0x24 → 0x37 → 0x31 → 0x38 → 0x3A → 0x3B → 0x3C → 0x27 → 0x3D → 0x01 → 0x01
```

Key facts:

- `0x20` and `0x21` were visited, took a brief `0x1F` re-visit, then a long `0x22`
  stretch (9 hits, ~4 min of in-game load transitions).
- The **rapid-fire ride** `0x2E→0x25→0x2F→0x36→0x30→0x35→0x34→0x2F→0x2C→0x24→0x37→0x31→0x38→0x3A→0x3B→0x3C→0x27→0x3D`
  is a single short cascade (18 distinct states over ~18 world_state_load events,
  line span ≈ 3.3M→3.64M i.e. the very end of the file) — characteristic of a
  **scripted endgame/credits roll**, not manual room traversal.
- The last two values are **`0x01`** — **return to the boot/title state** after the
  credits complete. This mirrors the boot path (which begins with `0x01 → 0x29 → ...`).
- `0x3D` (61) is the final gameplay/credits state before the loop-back to `0x01`.

## 4. What is confirmed

- The game was beaten in this capture: the world_state sequence ends in a scripted
  cascade then **returns to the boot state `0x01`**, matching a post-credits return
  to title.
- **58 distinct world_state values** total (Rev.105 had ~25, Rev.125 had 36).
- 22 of those 58 states are new to this capture and appear only in the final stretch.
- `world_state == scene_id` 1:1 mapping still holds (all values numeric-sorted match
  the `init_scene_gobj.a0`/`world_state_raw` sets in both captures).
- No world state `0x02`, `0x26`, `0x39` appeared across the entire playthrough —
  three unused slots in the 0x01..0x3D range.
- The credits cascade is dense but each value still hits its own DL slot behavior
  (per Rev.125 `ios_om_create_dl` per-slot mapping is scene-dependent).

## 5. What is probable

- `0x3D` (61) is the last credits/finish state before the title loop-back `0x01`.
- The `0x20–0x25` block is the final dungeon / escape-path area, and `0x2C–0x38`
  (plus `0x3A–0x3D`) are the credits/staff-roll states, because of their position
  between the last manual room traffic and the `0x01` return.

## 6. What is possible / unknown / discarded

- **Possible:** some of the `0x20–0x25` values are the sword-fight / final-bridge /
  "Yorda carried to the boat" moments. **Not asserted** — needs init_fn identity
  from the (not yet validated) `world_state_room_init_fn` probe.
- **Unknown:** exact semantics of each final state; whether `0x01` loop-back is
  "title screen" or "post-credits attract".
- **Discarded:** the earlier "0x33 is the last state" conclusion (Rev.125) — the
  finish capture shows states through `0x3D`. Rev.125 is superseded for the upper
  bound; its slot-mapping table still stands for the states it covered.

## 7. Next minimum test

1. Rebuild already done (25-probe binary). Next session: launch with the new binary,
   validate `world_state_room_init_fn` fires, then replay the final stretch (or use
   the just-finished save) to map init_fn per state `0x20–0x3D`.
2. Confirm the `0x01` loop-back state at credits end is the same code path as the
   boot entry `0x01`.
3. Correlate the 7 GirlBrain probes (`girl_brain_runaway_*` etc.) once a capture that
   includes the escape path is produced with the new binary.

## 8. Conservative verdict

The capture confirms a **completed playthrough**: 58 distinct world states, a scripted
credits cascade ending at `0x3D`, and a documented return to the boot state `0x01`.
No gameplay-naming claims are made for individual final states beyond their position in
the sequence; init_fn identity requires the next (25-probe) capture. The Rev.125
upper-bound claim (max `0x33`) is superseded.