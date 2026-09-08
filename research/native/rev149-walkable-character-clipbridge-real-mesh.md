# Rev.149 — ClipBridge real-mesh floor/wall model + GObj-driven PlayerController (first visible walking character)

**Date:** 2026-09-07
**Trilha:** native-port (`native-port` branch) — `[TRILHA: PORT]`
**State:** validates and closes the Fase A+B milestone of `instrucoes-agente-input-colisao-boy.md`.

---

## Objective

Land the first *visibly moving, collision-constrained character* on a decoded real room
mesh (`170_st00a_p1.p2o`): the player entity spawns on the actual floor of room p1 and
walks with WASD through the GObj process-dispatch seam, without losing floor support or
tunneling through walls. Fase C (real `boy.c` logic) remains future work.

## Scope

- `native/src/engine/ClipBridge.{h,cpp}` — grid heightfield + wall model able to resolve the real p1 mesh.
- `native/src/game/PlayerController.{h,cpp}` — GObj-backed player entity (new).
- `native/tests/clip_bridge_test.cpp`, `native/tests/player_controller_test.cpp` — new.
- `native/src/main.cpp`, `native/CMakeLists.txt` — demo integration (WASD → player → ClipBridge).
- `native/tests/gif_packet_test.cpp`, `native/tests/gif_executor_test.cpp` — pre-existing Rev.147 `drawStrips` ABI breakage fixed (test backends lacked the new pure virtual).

## Sources used

- USA ELF byte-exact evidence for `_Clip` (0x166E10) catalogued in `research/elf/ghidra-rev097-*.md` (collision/clip domain, 4 active `_clipW*` callbacks). ClipBridge is a **semantic bridge**, not a byte-exact port.
- `research/elf/rev142-p2o-vertex-layout-and-face-record-structure.md` — p2o decode: 16 B/vertex `(x,y,z,1)` from +0x20; strip faces. p1 → 13,877 verts, 15,161 tris, 3 submeshes.
- `research/native/rev148-playable-gap-analysis.md` — gap analysis that motivated this revision.
- Prior native ABI contracts: `IcoGObj`/`IcoProcessNode` (`src/core/gobj_abi.h`), `IsysGObjRuntime`, `ProcessNodePool` (Rev.109/Rev.134).

## Evidence used

### Real p1 mesh floor/wall statistics (new, reproduced from the decoded mesh)

| Metric | Value | Meaning |
|--------|-------|---------|
| Total triangles | 15,161 | room p1 |
| Up-facing (`ny>0.5`) | 2,722 | potential walk surfaces |
| Down-facing (`ny<-0.5`) | 3,543 | ceiling/underside (ignored) |
| Near-vertical (`|ny|<=0.25`, new threshold) | 8,025 | wall candidates |
| Up-floor Y centroid | mean −611, min −1671, max +200 | multi-level room; main hall is tiered |
| Wall bbox | X [−900,800], Z [0,2000] | walls span the whole room footprint |

Key finding: with the original threshold `|ny|<=0.5`, wall candidates were 8,833 —
including sloped band geometry — and successful floor samples were only ~7–51 cells.
The main blockers were (a) cell-center-only floor probes missing the real floor
triangulation, and (b) walls blocking cells whose walk surface was far **below** or
**above** the wall's base (sub-floor slab, raised-stage drops).

### Layout after the fix (walkability grid, 36×41 cells @50 u)

`blocked=483` (from 694/877); `hits over grid=109`; spawn `(-50,1000)` resolves to
`y=100`. Center hall rows become a large contiguous walkable region with scattered
columns; sub-floor structure no longer blocks.

| State | Count | Meaning |
|-------|-------|---------|
| Floor resolvable + not blocked | 414 | actual walkable cells |
| Floor resolvable but wall-blocked | 445 | floor cells under interior columns/stage sides |
| Blocked, no floor | 38 | room boundary / solid volume |
| No floor, no block | 579 | void space (not standable) |

### Movement integrity on the real mesh (scripted legs from spawn)

All 7 cardinal legs (8×25 u/frame, halfExtent=12, stepHeight=30) kept `floorHeightAt`
true every sub-step and stopped against walls; `totalFails=0`. Full demo run at 120
frames: stable 2 draw calls/frame, 15,185 tris (15,173 real + 12 box degenerates).

## Changes (byte/semantic level)

1. **`ClipBridge::buildFromMesh` → two-pass rasterization.**
   - Pass 1 (`rasterizeTriangle`): up-facing triangles only → per-cell heightfield with
     a **7×7 sub-sample grid** per cell (was cell-center only). Takes the highest plane
     Y seen per cell. Solves the real-mesh floor under-coverage.
   - Pass 2 (`rasterizeWall`, new): near-vertical triangles only (`|ny|<=0.25`, was 0.5)
     → blocks a cell only if the wall passes through the cell's inner 60% core AND the
     wall's vertical extent intersects the walk band (slab margin below, stand height
     above, `kStandHeight=220`, `kSlabMargin=15`). Sub-floor slabs and decoration above
     stand height no longer block.
   - Both `upFloor`/`wall` classification moved to their own passes so floor heights are
     complete before walls are scored against them.
2. **`PlayerController` (new, `ico::game`)** — owns a `GObj` (via `IsysGObjRuntime::add`),
   registers one `ProcessNode` (typeMask 1, priority 0); `setMove()` sets a pending
   vector consumed inside the GObj process callback through `ClipBridge::move`. This is
   the Fase A seam where real `boy.c` logic will attach (Fase C). Adds `spawn()` with
   bounded outward spiral (maxRing=2) so off-room requests fail instead of teleporting;
   failure no longer clobbers `x_/z_/y_`.
3. **`main.cpp`** — builds `clip` from `fitPiece` (p1), `IsysGObjRuntime(0x40,0x40)`,
   spawns player at camera-fit center `(-50,1000)`, maps WASD → `setMove(step=25)`;
   camera poll renamed to `pollDebugCameraInput`; red marker quad replaced by a
   3D AABB box drawn via `drawIndexed` (8 verts, 6 faces × 4 = 36 indices).
4. **Tests.** `clip_bridge_test` restructured: `makeTestRoom(ClipBridge&)` (non-copyable),
   all floor/tier triangle windings corrected up-facing, wall-band and step-height
   assertions against the real model. `player_controller_test` tests spawn rejection,
   GObj-backed dispatch, and wall-stop movement.
5. **Pre-existing breakage fixed:** `TestBackend` in `gif_packet_test.cpp` and
   `gif_executor_test.cpp` did not override `RenderBackend::drawStrips` (added Rev.147),
   leaving the classes abstract (build broken since Rev.147). Both now implement
   `drawStrips` with counters; full suite builds and passes.

## What is confirmed

- The real p1 floor is **not** a single plane but a tiered multi-level surface resolvable
  only with dense per-cell sampling; the 7×7 probe yields continuous floor coverage.
- Walls must be scored against the local walk surface height; pure AABB/core-overlap
  shock-absorbs the whole room into 52–59% blocked cells and makes the hall unnavigable.
- Player spawns `(-50,100,1000)` on the actual room floor and moves collision-constrained;
  `floorHeightAt` never drops mid-move.
- Pre-push gate: clean Debug configure+build+CTest = **25/25 PASS** (previously the
  `gif_packet`/`gif_executor` targets failed to build).

## What is probable

- The multi-tier surface (`a…g` Y-band map) corresponds to this room's platforms and
  central raised area; Y resolution is per-cell max, so overlapping levels resolve to the
  highest walk surface in the cell (satisfactory for ground movement, imperfect at
  under-ramp boundaries).

## What is possible

- Wall mis-classification remains possible where a wall's XZ extent is thinner than the
  cell core (20 u inner box) — a ≤20 u-thin freestanding column could be missed; the
  footprint corner probes + sub-stepping mitigate tunneling but do not prove it impossible.

## What is unknown

- Correct step/ledge Y tolerances for every tier transition (tuned with
  `stepHeight=30`, `kSlabMargin=15` by experiment, not measured from gameplay).
- Whether the real `_Clip` treats thin columns as solid the same way.

## What is discarded

- The old single-pass wall classification (`|ny|<=0.5`) → 877 blocked cells, spawn-abort.
- Cell-center floor probing → 7 hits, spawn-abort. Both replaced.

## Next minimum test

1. Interactive: `./build/ico_native --p2o assets/170_st00a_p1.p2o` (window opens, spawn OK,
   WASD moves the box across the hall without sinking or clipping through the floor/columns).
2. Fase C seam: feed real `boy_hC/hB/hA` state machine read from the walk input instead of
   the raw `setMove` vector (attaches to the same ProcessNode callback).
3. Batch-render the ClipBridge blocked/floor grid as a debug overlay to visually confirm
   the walkability map matches the room render.

## Conservative verdict

Fase A (input → GObj dispatch) e Fase B (colisão mínima via ClipBridge) estão fechados
com um personagem visível andando na sala real p1 sem atravessar piso/paredes. A colisão
é uma ponte semântica (não byte-exact) e usa limiares empíricos; Fase C (BOY real) e
revisão de limiares contra jogabilidade medem-se como trabalho seguinte. Gate local:
25/25 testes passando.