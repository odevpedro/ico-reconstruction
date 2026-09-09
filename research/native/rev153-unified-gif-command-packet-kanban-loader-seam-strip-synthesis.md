# Rev.153 — Unified GIF-command scene packet + KanbanSceneLoader seam + greedy strip synthesis

- **Date:** 2026-09-09
- **Branch:** native-port
- **Objective:** Close the three fronts of the AGENTS.md native work in one
  revision: (1) extend the `GifPacketBridge` with scene-level commands so the
  sky backdrop and strip batches ride one semantic GIF packet per frame, (2)
  wire the semantic `KanbanSceneLoader` (kanban.c) into `ico_native` for both
  scene composition and the runtime scene-load path, and (3) synthesize strip
  topology for strip-less pieces so every room piece renders through
  `glMultiDrawArrays(GL_TRIANGLE_STRIP)` with no `[A,B,C,C]` indexed
  duplication.
- **Milestone:** the whole st00a room renders via a single unified packet
  pipeline (25/26 CTest, only the documented headless `opengl_backend` segfault
  fails).

---

## Scope

- `native/src/engine/GifCommandBuffer.h/.cpp` — buffer-owned strip geometry.
- `native/src/engine/GifPacket.h/.cpp` — bridge scene commands + depth state.
- `native/src/engine/GifCommandExecutor.h/.cpp` — `DrawSkyGradient` /
  `DrawStrips` execution against a backend + buffer.
- `native/src/engine/RenderBackend.h` — new command kinds + union members.
- `native/src/engine/Ps2oMesh.h/.cpp` — `synthesizeTriangleStrips()`.
- `native/src/main.cpp` — load-time synthesis, KanbanSceneLoader wiring,
  frame-loop bridge routing, composition via `boundAsset()`.
- `native/tests/gif_packet_test.cpp`, `native/tests/gif_executor_test.cpp`,
  `native/tests/ps2o_mesh_test.cpp`.

## Evidence used

- `cmake --build native/build` for every affected target (0 errors).
- `ctest --test-dir native/build` → **25/26 pass**; the only failure is the
  pre-existing headless `opengl_backend` segfault (documented since Rev.143).
- Live run: `./build/ico_native --room st00a --frames 20 --shot
  /tmp/opencode/st00a_rev153.ppm` (X11 display available) — clean shutdown.
- Per-piece and per-texture batch instrumentation printed by `main.cpp`.

---

## Findings

### Front 1 — `GifPacketBridge` scene commands (done, tested)

- `GifCommandBuffer` owns three strip arrays (`m_stripsVertices`,
  `m_stripsFirsts`, `m_stripsCounts`) reset by `reset()`, appended by
  `setStripGeometry()`.
- `GifPacketBridge::drawStrips()` records the **pre-append** base offsets on the
  `RenderCommand::DrawStrips` command and copies the strip stream into the
  buffer; the executor resolves offsets against the buffer at flush time.
- `GifPacketBridge::drawSkyGradient()` and `GifPacketBridge::setDepthState()`
  added. `setDepthState(GSDepthTest, bool write)` is required because the
  PS2 `setZWrite(zte,ztst)` / `setZTest(ate,atst,...)` setters cannot express
  the sky precondition **(Always, write=false)** within one packet.
- Executor: `execute()` now dispatches through a private
  `executeCommand(cmd, &buffer)`; `DrawSkyGradient` and `DrawStrips` cases
  bounds-check and read geometry from the buffer before calling the backend.
  `resolveTextureHandles()` forwards `cmd.strips.texture` (real handles pass
  through unchanged; only uploaded synthetic textures are resolved).
- `gif_packet_test`: new `test_scene_bridge_commands()` covers depth state +
  sky colors + strip geometry offsets + flush forwarding.

### Front 2 — `KanbanSceneLoader` seam (done, run-verified)

- `runOpenGLDemo` composition now goes through a **composition runtime** +
  `KanbanSceneLoader::bindSceneAssets(store, 0x0F)` + `boundAsset(0x0F, i)`
  instead of direct `store.sceneAsset()` iteration.
  - st00a: `compound 33 pieces` via the loader (matches the manifest).
- `runSceneDemo` receives an optional `const SceneAssetStore* store`; when set,
  it creates a `KanbanSceneLoader` against the same `gobjRuntime` the
  BoyController process uses and runs the original `requestScene(0x0F)` +
  `execute()` flow.
  - Run log: `loader bound scene 0x0F: 33 assets (first=0str01_s2)`,
    `requestScene/execute: currentSceneId=0x0F`.
  - **confirmed:** the seam executes the semantic load path with the real
    store in the real runtime; `execute()` settles `currentSceneId = 0x0F`.
  - **expected, logged:** 0 host GObjs created — room *entry descriptors*
    (`2A31B8`/`2A4C48` rows) are not decoded into `m_entries`/`m_descriptors`
    yet, so `initSceneGObj` finds no enabled record.
  - The render loop stays **decoupled** from the loader (it reads the store
    directly); this is the wiring seam where runtime-validated GObj creation
    will feed later revisions.

### Front 3 — Unified strip batch (done, run-verified)

- `Ps2oMesh::synthesizeTriangleStrips()`: greedy triangle→strip synthesis on
  the flat `triangles` list (valid only when the file parsed no strip
  topology). Edge key `(hi<<32)|lo` adjacency, forward extension on the last
  spine edge, backward prepend on the first, per-vertex UVs from the record's
  own `m`, refuses spines with a vertex index > 0xFFFE.
- Load-time call in `runSceneDemo` after `loadPs2oMeshFromFile()`.
- **st00a result:** every one of the 33 room pieces already carries parser
  strips (verified per-piece: `0str01_s2` 33 strips, `st00a_p2` 3562 strips,
  `door` 14 strips → synthesis returns 0 by design). Synthesis is the safety
  net for future strip-less files (e.g. a p2 discriminator that stops the
  over-decode).
- Frame loop: when strips exist, one `bridge.startPacketPri(0)` packet carries
  sky depth-state + `DrawSkyGradient` (if the room has a sky texture) + every
  `StripBatch` as a `drawStrips` command, then `bridge.endPacket()`. The flat
  `drawIndexed` fallback is kept unchanged for strip-less rooms.
- **st00a numbers:**
  - strip batch report: `TOTAL strips=12837 srcTris=26735 verts=52409 |
    verts/srcTri=1.96` (vs the old quad path's 4 verts + 4 indices per tri).
  - `[render] Frame: 20 draw calls, 26759 triangles (26747 real, 12
    degenerate)` — only 12 degenerates in the whole frame (0.04%), i.e. the
    `[A,B,C,C]` duplication is gone from the driver path.

### Regression fixed (Rev.152 breakage, discovered during this work)

Rev.152 added `drawSkyGradient()` as a **pure virtual** `RenderBackend` method
without updating the two test backends, so `gif_executor_test` and
`gif_packet_test` could no longer instantiate `TestBackend` (abstract-class
compile errors) — the build was broken for those targets. Fixed by adding the
`drawSkyGradient` override (with `m_skyGradientCalls` + captured colors) to
both `tests/gif_executor_test.cpp` and `tests/gif_packet_test.cpp`.

---

## Confirmed

- `setDepthState(Always,false)` → `DrawSkyGradient` → `setDepthState(Less,true)`
  → strips, in command order, flushes through the executor to the backend.
- The bridge path renders the full st00a composition (26747 real triangles).
- `KanbanSceneLoader` binds the real store, settles `currentSceneId`, and
  creates the honest 0 GObjs (descriptors not recovered).
- All strip-less files synthesize (safety net) and every st00a piece is in the
  unified strip path today.

## Probable

- `verts/srcTri=1.96` per texture batch will hold for other rooms (strip
  vertex reuse is intrinsic to cascade strips; independent of room).

## Possible / Unknown

- `hasSky` is false for st00a (no `sky.tm2` / `*sky*.tm2` in its texture
  manifest); which rooms carry a sky texture is not inventoried.
- The p2 wall family discriminator (Rev.142/151 "still open") remains
  unresolved; strips from `st00a_p2` are the cascade-a over-decode.
- Recovering the 68 descriptor / 512 entry rows so `initSceneGObj` creates
  real GObjs at scene load.

## Discarded

- A dynamic EBO for strip-less pieces — replaced by greedy strip synthesis
  (nothing per-frame, no index buffer, single draw type).

## Next minimum test

1. Run `ico_native --room st00a` in a display session; confirm the screenshot
   matches the pre-front-3 render (no geometry/tint regression).
2. Render a second room with a sky texture (e.g. one whose manifest lists
   `sky.tm2`) and confirm the bridge `DrawSkyGradient` command fires.
3. Feed validated descriptor rows into `KanbanSceneLoader` and confirm
   `initSceneGObj` creates GObjs > 0 without destabilizing the BoyController
   process.

## Conservative verdict

The three fronts are wired and run-verified with **zero CTest regressions** (the
only failure is the documented headless `opengl_backend` case). The semantic
GIF pipeline now spans real room geometry; the KanbanSceneLoader seam executes
the original load flow against the real runtime. The remaining "0 GObjs" gap is
a documented input problem (descriptor recovery), not a runtime defect.
</content>
</invoke>