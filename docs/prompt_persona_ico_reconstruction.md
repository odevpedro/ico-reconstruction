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

## Current runtime baseline (Rev.126)

- Runtime captures (instrumented PCSX2 fork, `ico-logpoints` branch) observe a
  **world_state == scene_id 1:1 mapping**, recorded at `0x001AF948`.
- The finish/credits capture reached **58 distinct world_states** (max `0x3D`)
  and **completed the game**: the credits cascade ends by returning to the boot
  state `0x01`. Values `0x15-0x1F`, `0x28-0x33`, and `0x16-0x3D` were used
  across two extended sessions.
- `ios_om_create_dl` shows **20+ distinct BSS DL-slot addresses**, each with a
  strong (>87%) dominant world_state; the mapping is scene-dependent.
- `world_state_load` (0x001AF948, byte-exact, 0x80 B) dispatches per-room init
  via the table at `0x5F2FB8` (stride 0x194, init_fn at +0x154), then runs
  `MakeCollisionDependGObjList` + scene-apply, clears the room-load flags at
  `0x274ED4/8`, and tail-jumps to the shared epilogue `0x13D3F8`. `DispIcoMisc`
  (0x1AF9C8, 0x1C8 B) is a separate function, also byte-exact.
- The fork probe table is prepared for 25 source-side probes covering the
  isysGObj callbacks, halfword writer sites, world-state load/init_fn/reset,
  a sampled VBlank counter, and 7 GirlBrain/Yorda callbacks
  (`girlBrainRunawaySearchPoint`/`MoveByWay` = escape/"tired-run" path).
  Next step is capturing the per-world_state `jalr` targets at `0x001AF96C` to
  populate the native `WorldStateLoader` dispatch table.

## Current engine priority

The `iosOmCreateDL` slot dispatch, type-based routing, rendering pipeline, the
game-loop -> scene-loader -> GIF bridge, and the `WorldStateLoader` per-room
semantic bridge are implemented and tested (18/18 CTest). Next up is a new
runtime session to observe the per-room `init_fn` targets (the `jalr` at
`0x001AF96C`) so the native dispatch table can be bound to real room setup
functions beyond the currently injected mocks.

## Sources to prefer

1. `research/elf/rev131-worldstate-boundary-dispicomisc-and-native-bridge.md`
2. `research/elf/rev130-hot-gaps-3-4-5-byte-exact.md`
3. `research/elf/ghidra-rev126-finish-session-58-worldstates-and-credits-sequence.md`
4. `research/elf/ghidra-rev125-extended-session-36-worldstates-yorda-escape-probes.md`
5. `research/elf/rev124-runtime-probe-prep-and-game-loop-scene-bridge.md`
6. `research/elf/rev109-isysgobj-abi-consolidation.md`
7. `research/elf/ghidra-rev099-isysgobj-lifecycle-and-ios-thread.md`
8. `research/elf/ghidra-rev098-isysgobj-process-registration-and-dispatch.md`
9. `research/elf/ghidra-rev097-isysgobj-clip-girlbrain-consolidation.md`
10. Byte-exact sources under `src/core/asm/`

When an older note conflicts with Rev.131 on the `world_state_load` boundary
(0x80 vs the earlier 0x248), use Rev.131. When an older note conflicts with
Rev.109 on the four list tables, use Rev.109. When prose conflicts with raw
instructions, use the instructions.
