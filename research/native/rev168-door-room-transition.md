# Rev.168 — Door-triggered room transition (host mechanics, P2)

Date: 2026-09-10
Branch: native-port
TRILHA: PORT

## Objective

Make the native demo leave room 0x0F through its door: player approaches the
`169_door` geometry, the door *opens*, the current room's GObjs are released,
scene 0x2B is loaded through the verified isysGObj seam and the player is
re-seeded past the door. This is PORT priority item 2
(`instrucoes-agente-prioridade-port-vs-decomp.md`).

## Scope

- Pure **host** mechanics. The door zone, spawn offset and open delay are
  heuristics applied on verified asset geometry, not byte-verified original
  door data.
- No animated door model in this revision (static `169_door`; the door
  "open" is the scene swap + OSD/log).
- Geometry stays st00a (scene 0x2B currently reuses the same host piece
  library — host test data per Rev.159). Real second-room geometry swap is
  the next step and needs a multi-bundle store refactor.

## Evidence used

- `src/entity/asm/kanban.c` family seams already present natively:
  `KanbanSceneLoader::initSceneGObj()` releases the previous scene's GObjs
  (pool reuse across rooms) and `execute()` calls
  `relinkAttachmentsForCurrentScene()` on every transition (Rev.154/159).
- `native/assets/scene/stgst00a.manifest` binds scenes 0x0F and 0x2B; the
  `169_door.p2o` piece is real room geometry (28 pieces / 25 host GObjs /
  28 attachments in scene mode).
- BoyController `spawn(x,z)` walkable probe returns bool.

## Changes

### New component: `native/src/game/RoomTransitions.{h,cpp}`

- `RoomTransitionZone`: `{sceneId, name, x, z, radius, targetSceneId,
  spawnX, spawnZ}`.
- `RoomTransitions`: Idle → Opening → Transitioning → Idle state machine.
  - Entering a zone starts *Opening*; opening takes `setOpenDelay()` seconds.
  - Leaving the zone during Opening cancels (door closes; progress returns
    to 0).
  - Full opening fires the callback **once** (nearest-zone selection when
    several zones overlap); phase returns to Idle and the zone goes on a
    per-zone `setCrossBackCooldown()` so stepping across does not
    instantly re-trigger.
  - `reset()` clears phase + cooldowns (usable on scene re-load).
  - `setBoyPosition()`/`update(dtSeconds)` drive it each frame.

### Wiring: `native/src/main.cpp`

- `rebuildGObjDraws` lambda extracted from the startup per-GObj draw build
  (was inline; now reused by the transition callback).
- Door zone fitted from the real `169_door.p2o` AABB while loading scene
  mode; spawn = door center + 60 units along the (door − room-center) axis.
  If the piece is absent (plain single-p2o demo), transitions are disabled
  with a log line.
- Transition callback: `requestScene(target)` + `execute()` (releases old
  GObjs → loads new, re-links) → `rebuildGObjDraws()` → re-seed player via
  `spawn()` walkable probe.
- Real per-frame `dt` (steady_clock, clamped) created; the loop still sleeps
  a fixed 16 ms for pacing, but the state machine now advances on wall time.

### Test: `native/tests/room_transitions_test.cpp`

Covers: no-zone idle, enter→Opening (progress>0 from the entering frame),
cancel on leaving, single fire after openDelay with correct
zone/scene/spawn, cooldown suppression then re-arm, two-zone nearest
selection, `reset()`, re-initialize replacement. Registered as CTest
`room_transitions`.

## Verification

- `ctest -R room_transitions`: PASS.
- Full `ctest`: **28/29** (only `opengl_backend` headless SEGFAULT — the
  documented pre-existing baseline; delta +1 test over Rev.167's 27/28).
- Headless scene run (`--scene pieces --frames 30 --shot`, exit 0):
  - `scene composition from manifest ... (scene 0x0F via KanbanSceneLoader,
    28 pieces)`
  - `door zone at (0,-9.6585) r=40 -> scene 0x2B, spawn(2.96766,-69.5851)`
  - 25 host GObjs / 28 attachments (unchanged from Rev.155/159).

## Label correction (retroactive, from the PORT-priority instruction)

The subject lines of **Rev.166 (`82a6911`)** and **Rev.167 (`c2d938f`)**
carry `[TRILHA: PORT]` but are actually reconstruction work
(byte-exact `.s` + semantic bridges) with **no native-port change**.
Per `instrucoes-agente-prioridade-port-vs-decomp.md`, both should be read as
**`[TRILHA: DECOMP]`**. No commit history rewrite; this note is the
canonical correction record.

## What is confirmed

- RoomTransitions machine behavior (unit-tested): cancel, single-fire,
  cooldown, reset.
- Door zone fits the real `169_door` piece; scene-mode load still yields
  25 GObjs / 28 attachments after the main.cpp refactor.
- Transition callback path (requestScene/execute → rebuildDraws → spawn)
  compiles and initializes headless; the actual fire is exercised by the
  unit test (callback), not yet by a live keyed walk in the demo.

## What is probable

- Entering the fitted zone on a live walk (W toward the door, camera-facing)
  triggers the swap and re-seeds the boy past the door (cooldown 2 s default
  prevents instant bounce).

## What is unknown / open

- Animated door visuals (v1 = swap only). Real second-room geometry
  (bundle/store refactor). PCSX2 capture of scene 0x0F to replace the
  round-robin GObj↔mesh binding (item 1, waits for the user to play).

## Next minimum test

In the demo, walk the boy to the door and confirm the log
`DOOR OPEN -> load scene 0x2B` + `execute: ok` + boy re-seeded past the
door; then a second crossing triggers only after the cooldown.

## Conservative verdict

A testable host room-transition layer is in place on verified geometry and
the verified loader seam. No original door timing/trigger data is claimed.