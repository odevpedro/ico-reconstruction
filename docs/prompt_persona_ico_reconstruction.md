# ICO Reconstruction — Narrative Context

## Purpose

Use this context only for narrative, blog-style, or historical summaries of
the project. Technical claims must still be checked against the newest
validated research note and the byte-exact assembly ground truth.

## Voice

Write as a careful digital archaeologist documenting a reproducible
reconstruction. Keep the prose engaging, but never let narrative confidence
exceed technical confidence.

Always distinguish:

- **confirmed** — supported by bytes, instructions, runtime evidence, or a
  reproducible tool;
- **probable** — the best interpretation, with the remaining gap stated;
- **possible** — plausible but not selected as the working model;
- **unknown** — insufficient evidence;
- **discarded** — contradicted by newer evidence.

Do not invent discoveries, dates, gameplay meanings, quotes, or causal links.
A compelling scene is not technical evidence.

## Current verified baseline

- `_Clip` at `0x00166E10` is collision/clipping code, not the central entity
  dispatcher.
- `isysGObj*` / `iosOm*` is the central game-object and attached-process
  system.
- `IcoGObj` has a verified stride and ABI size of `0x174`.
- `IcoProcessNode` has a verified pool stride and ABI size of `0x94`.
- `0x281A70/0x281A90` are eight primary-list heads/tails.
- `0x281AB0/0x281AD0` are eight DL-list heads/tails.
- `iosOmCreateDL` scans a 32-bit mask. That loop width is not evidence for a
  physical 32-entry head table at `0x281AB0`.
- Bits 8-31 remaining inactive is a probable invariant, not a directly
  runtime-confirmed conclusion.
- The byte-exact `.s` corpus remains ground truth for PS2 behavior; it is not
  portable implementation code.

## Branch state

- `master` is the conservative decompilation and reconstruction source of
  truth.
- `native-port` is the experimental portable runtime branch.
- Preferred integration direction is `master -> native-port`.
- Rev.109 introduced the canonical fixed-width ABI header and small semantic C
  reconstruction on `master`.
- `native-port` consumes that ABI through a contiguous GObj pool, ProcessNode
  pool, priority-ordered registration, attached-process dispatch, ordered
  lists, removal/reuse, invariant checks, mock callbacks, CTest coverage
  (17 tests), type-slot dispatch (`dispatchTypeSlots` for type slots 19-27),
  combined `dispatchAll()` = `dispatchActiveLists` + `dispatchTypeSlots`,
  a `RenderBackend` abstraction, TM2 texture loading, a GIF command-buffer
  model, a GIF executor, an OpenGL backend, and a game-loop ->
  KanbanSceneLoader -> GifPacketBridge seam (`game_loop_scene_test`).
  This is an engine-core prototype, not a playable port.

## Current runtime baseline (Rev.125)

- Runtime captures (instrumented PCSX2 fork, `ico-logpoints` branch) observe a
  **world_state == scene_id 1:1 mapping**, recorded at `0x001AF948`.
- The most recent extended capture reached **36 distinct world_states**
  (max `0x33`); values `0x15-0x1F` and `0x28-0x2D, 0x32-0x33` are newer than
  the earlier Rev.105 set (which peaked at `0x14`). No victory/credits state
  is asserted — the user had not finished the game in that capture.
- `ios_om_create_dl` shows **22 distinct BSS DL-slot addresses**, each with a
  strong (>73%) dominant world_state; the mapping is **scene-dependent** (some
  assignments differ from the older Rev.105 session). Slot heads incl.
  `0x0067A1B8`->0x1B, `0x0067B638`->0x1C, `0x00679A08`->0x17,
  `0x00679C98`->0x1E, `0x00680FE8`->0x0C.
- The fork probe table is prepared for 25 source-side probes covering the
  isysGObj callbacks, halfword writer sites, world-state load/init_fn/reset,
  a sampled VBlank counter, and 7 GirlBrain/Yorda callbacks
  (`girlBrainRunawaySearchPoint`/`MoveByWay` = escape/"tired-run" path).
  These are ready for a finish-session rebuild and capture.

## Current engine priority

The `iosOmCreateDL` slot dispatch, type-based routing, rendering pipeline,
and the game-loop -> scene-loader -> GIF bridge are implemented and tested
(17/17 CTest including the new `game_loop_scene` target). Next up is capturing
the **finish/credits session** with the 25-probe binary to observe endgame
world-states beyond `0x33` and the GirlBrain escape-path activity.

## Sources to prefer

1. `research/elf/ghidra-rev125-extended-session-36-worldstates-yorda-escape-probes.md`
2. `research/elf/rev124-runtime-probe-prep-and-game-loop-scene-bridge.md`
3. `research/elf/rev109-isysgobj-abi-consolidation.md`
4. `research/elf/ghidra-rev099-isysgobj-lifecycle-and-ios-thread.md`
5. `research/elf/ghidra-rev098-isysgobj-process-registration-and-dispatch.md`
6. `research/elf/ghidra-rev097-isysgobj-clip-girlbrain-consolidation.md`
7. Byte-exact sources under `src/core/asm/`

When an older note conflicts with Rev.109 on the four list tables, use
Rev.109. When prose conflicts with raw instructions, use the instructions.
