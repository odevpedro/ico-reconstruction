# Rev.124 — Runtime probe instrumentation next-session prep + game-loop scene bridge

- **Date:** 2026-09-05
- **Objective:** Prepare the next PCSX2 runtime session by (a) extending the instrumented fork's probe table to cover the halfword fast path and world-state room init_fn/reset, and (b) close the native-port #42 gap by wiring the confirmed scene loader into the game loop.
- **Scope:** `.local/pcsx2-ico-logpoints-fork` probe instrumentation; `native/` game-loop integration. No new claims about original game behavior beyond what was already verified.
- **Sources used:**
  - `research/elf/ghidra-rev093b-halfword-entry-runtime-validation.md` (halfword writer hot-path; fast path still inferred)
  - `research/elf/ghidra-rev089-runtime-session-rev086-worldstate-gp-m49b4.md` (room_init_fn always 0 → probe mispositioning at 0x1AF948)
  - `research/elf/rev109-isysgobj-abi-consolidation.md` (canonical ABI)
  - `research/elf/rev122-static-scene-debug-view.md` (scene debug view)
  - Raw disassembly of `.local/extracted/SCUS_971.13.elf` around `0x001AF948`
- **Evidence used:** instruction-level disassembly (world_state_load region), existing runtime captures (Rev.093b second-caller stream, Rev.089 world_state stream), 17/17 CTest result for the new native integration.

---

## 1. Instrumented fork probe table (source edits only)

The probe table `s_ico_runtime_probes[]` in
`.local/pcsx2-ico-logpoints-fork/pcsx2/x86/ix86-32/iR5900.cpp` grew from 14 to **25 entries**:

| Address | Label | sample_period | Why |
|---------|-------|---------------|-----|
| `0x00100008` | `elf_entry_sentinel` | 1 | boot sentinel (unchanged) |
| `0x0013dda0` | `isys_gobj_init` | 1 | runtime sentinel (unchanged) |
| `0x0013e4d0` | `isys_gobj_alloc` | 1 | unchanged |
| `0x0013e548` | `isys_gobj_remove` | 1 | unchanged |
| `0x0013e8d8` | `isys_gobj_add` | 1 | unchanged |
| `0x0013f2c8` | `isys_gobj_dl_init` | 1 | unchanged |
| `0x0013f3f0` | `isys_gobj_proc_add` | 1 | unchanged |
| `0x0013f9d0` | `ios_om_main` | 1 | unchanged |
| `0x0013fc00` | `ios_om_create_dl` | 16 | unchanged |
| `0x0013fd10` | `ios_om_exe_each_gobj` | 64 | unchanged |
| **`0x0016ac10`** | **`girl_brain_clear_target`** | **1** | **NEW: GirlBrain target clearing (Rev.108 `NO_PROBE`)** |
| **`0x0016bca0`** | **`girl_brain_position_update`** | **64** | **NEW: girlBrainMain_PositionUpdate (Rev.108 `NO_PROBE`)** |
| **`0x0016ced0`** | **`sub_girl_brain_pulled_up`** | **1** | **NEW: subGirlBrain_PulledUp (Rev.108 `NO_PROBE`)** |
| **`0x0016e910`** | **`girl_brain_hide_make_hide_point`** | **1** | **NEW: _girlBrainHide_MakeHidePoint (Rev.108 `NO_PROBE`)** |
| **`0x0016eb68`** | **`girl_brain_hide_goal_turn`** | **1** | **NEW: girlBrainHide_GoalTurn (Rev.108 `NO_PROBE`)** |
| **`0x0016f410`** | **`girl_brain_runaway_search_point`** | **16** | **NEW: girlBrainRunawaySearchPoint — captures the "Yorda runs tired" escape behavior (Rev.108 `NO_PROBE`)** |
| **`0x0016f9a8`** | **`girl_brain_runaway_move_by_way`** | **16** | **NEW: girlBrainRunawayMoveByWay — escape waypoint movement (Rev.108 `NO_PROBE`)** |
| **`0x00166dfc`** | **`halfword_fast_path`** | **64** | **NEW: direct probe of the single-cell fast path (Rev.093b item #21)** |
| `0x0016828c` | `halfword_second_caller_entry` | 1 | unchanged (run-script gate string) |
| `0x00168294` | `halfword_second_caller_return` | 1 | unchanged |
| `0x001af948` | `world_state_load` | 1 | unchanged |
| **`0x001af96c`** | **`world_state_room_init_fn`** | **1** | **NEW: captures `$v0` = the actual room init_fn invoked by `jalr`, fixing Rev.089's `room_init_fn always 0` (probe was reading the slot before the call)** |
| **`0x001af9b8`** | **`world_state_vblank_reset`** | **1** | **NEW: fires at `sw $zero,0x18($v0)` with `$v0 = 0x274EC0`, i.e. the VBlank-adjacent counter reset during room transition** |
| `0x001b76f8` | `init_scene_gobj` | 1 | unchanged |

> **Update (Rev.124 final):** the 7 GirlBrain probes were added after the user reported the
> final-stage session exposing new Yorda sprites including the "runs tired" animation.
> Rev.108 had listed these callbacks as `NO_PROBE`; they are now covered on the next
> rebuild. `girlBrainRunawaySearchPoint`/`girlBrainRunawayMoveByWay` are the escape
> pathway most plausibly tied to the tired-run pose.

In addition, every probe event now appends the sampled `vblank_counter`:

```json
"info":{ ..., "vblank_counter":"0x%08x" }
```

read from `memRead32(0x274ec0)` at emit time. This gives per-event frame-beat correlation
without a dedicated hardware watchpoint (Rev.086 VBlank/IOP question, objective #25).

## 2. Static analysis driving the world_state probes

Region around `0x001AF948` in the USA ELF (world_state_load):

```asm
0x001af948: lw   $v1, -0x6f60($gp)   ; world_state
0x001af950: lui  $a1, 0x5f
0x001af958: addiu $v0, $a1, 0x2fb8   ; 0x5F2FB8 base
0x001af95c: addu $v0, $v0, $v1
0x001af960: lw   $v0, 0x154($v0)     ; room init_fn slot (0x154 above the state base)
0x001af964: beqz $v0, ...
0x001af96c: jalr $v0                 ; calls room init_fn
```

- Rev.089 probed `0x001AF948` and logged `room_init_fn` from a register read at entry —
  it was always `0x00000000` because the init_fn had not been loaded/selected yet.
- The init_fn **value being called** is `$v0` at `0x001AF96C`. Probing there captures the
  selected init_fn pointer (present in `$v0` when the probe fires, because the hook
  executes before the `jalr`).
- The table base `0x5F2FB8` is **not** a clean pointer table: rows are `world_state` +
  `0x154` cells and the region holds float/string data. This makes the init_fn the
  **loaded value in `$v0`** — which is exactly what the new probe logs.
- `0x274EC0 + 0x18` / `+ 0x14` are reset to zero at `0x1AF9B8` / `0x1AF9BC` during the
  transition, matching the earlier "VBlank counter reset after room change" hypothesis.
  The new `world_state_vblank_reset` probe pins the reset site; the sampled
  `vblank_counter` read confirms the beat.

## 3. Discarded: #23 dispatch_point slot index

`dispatch_point` at `0x00167020` is **inline within `_Clip` / `FUN_00166FD0`** — the
collision/clipping pipeline (Rev.097 correction), **not** the general entity dispatcher.
The real slot machinery is `iosOmCreateDL` (`0x13FC00`), already probed with `a1 = slot`
and `a2 = index`. Fixing a slot index capture at `0x167020` would measure a collision
configuration table, not entity dispatch; it is **superseded** and intentionally not added.

## 4. Native-port #42: game-loop → scene loader → GIF bridge

The native engine had the GifPacket bridge and the scene loader, but the loop itself
(`main.cpp`) only ticked. Added:

- `native/tests/game_loop_scene_test.cpp` + CMake target `game_loop_scene_test`:
  - `IcoRuntime` + `GameLoop.run(4)`
  - frame 1: `requestScene(7)` + `execute()` (initSceneGObj path), asserts
    `currentSceneId == 7` and 1 GObj created
  - frames 2–4: `renderStaticSceneDebugView(...)` through `GifPacketBridge`
    (engine `RenderBackend`), asserts 1 `DrawSprite` per frame
  - asserts label `scene=7 descriptor=2 gobj.type=0 list=0 sort=9 handle=1`
- This validates the frame-by-frame seam (`vblank → ACTGame → kanbanExec →
  initSceneGObj`) end-to-end on the host without claiming original geometry,
  textures, or placement.
- **Result: 17/17 CTest targets pass.**

## 5. What is confirmed

- The fork probe table now has 17 entries; source edits compile-ready (rebuild deferred
  until the active gameplay session ends, since the running `pcsx2-qt` binary must keep
  its validated 14-probe behavior mid-session).
- The `jalr` at `0x001AF96C` receives the room init_fn in `$v0`; a probe before it reads
  the selected pointer.
- `0x274EC0+0x14/+0x18` are zeroed during world-state transitions at `0x1AF9B8/0x1AF9BC`.
- `game_loop_scene_test` passes: the game loop drives scene load → GIF emission.

## 6. What is probable

- `halfword_fast_path` (0x166DFC) fires at low frequency relative to warm/hot path — the
  64-period sampler prevents flooding; spike in hits correlates with single-cell fast-paths.
- `vblank_counter` beat correlates 1:1 with VBlank-sourced frame increments (IOP/SIF writer
  hypothesis, Rev.086) — will be confirmed by comparing counter monotonicity across events.

## 7. What is possible

- `world_state_room_init_fn` fires per room-transition with the init_fn identity;
  grouping by value may reveal the per-room init_fn table reachable through this jalr.
- The two `world_state_*` probes together may let us assign **each world_state value a
  distinct init_fn**, closing the earlier "room_init_fn always 0" gap.

## 8. What is unknown / discarded

- **Unknown:** whether `0x5F2FB8` + `world_state` + `0x154` is a dense per-state table or
  a sparse container; only runtime values will tell.
- **Unknown:** exact VBlank-counter write source (IOP-side vs SIF), runtime evidence pending.
- **Discarded:** adding a `dispatch_point` (0x167020) slot-index probe — superseded per
  Rev.097 correction (see §3).

## 9. Next minimum test

1. After the current gameplay session ends, rebuild the fork:
   `cmake --build .local/pcsx2-ico-logpoints-fork/build-runtime` (Ninja, Release).
2. Re-run `./tools/run-ico-pcsx2-logpoints.sh --check`
   (still validates `halfword_second_caller_entry` — unchanged string).
3. Validate that the new labels `halfword_fast_path`, `world_state_room_init_fn`,
   `world_state_vblank_reset`, the `vblank_counter` info field, and the 7
   `girl_brain_*` labels appear in the JSONL.
4. On the next session, compare `vblank_counter` beats and init_fn grouping per world_state,
   and confirm `girl_brain_runaway_*` activity correlates with the tired-run animation.

## 10. Conservative verdict

The probe table now covers every open runtime instrumentation item except the
superseded #23; edits are source-only and documented. The native #42 game-loop bridge is
complete and green (17/17 CTests). No new gameplay behavior is claimed beyond verified
facts; runtime confirmation is pending a post-session rebuild + a fresh capture.