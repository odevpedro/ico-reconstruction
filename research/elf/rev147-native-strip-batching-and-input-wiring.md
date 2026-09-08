# Rev.147 — Native runtime: strip-semantics batch rendering + input wiring (native-port)

- **Date:** 2026-09-07
- **Objective:** Execute the native-port work order that follows Rev.146's push: (1) instrument per-batch triangle/index counts BEFORE any rendering refactor, to resolve whether the `[A,B,C,C]` quad-group duplication is real; (2) finish the started input wiring so a real world-space placeholder moves via keyboard; (3) guided by the instrumentation result, rebuild the mesh batch with strip semantics (`gif_DrawStripF/G`, GIF prim 0xD) so N spine verts render N-2 triangles.
- **Scope:** `native-port` branch only — `native/src/main.cpp`, `native/src/engine/OpenGLBackend.*`, `native/src/engine/RenderBackend.*`, `native/src/engine/Ps2oMesh.*`, `native/src/platform/Input.*`, `native/CMakeLists.txt`, `native/tests/input_test.cpp`, `native/tests/ps2o_mesh_test.cpp`.
- **Sources:** `native/assets/170_st00a_p1.p2o` and its 7 material TM2s; byte-exact `.s` ground truth `src/core/asm/gif_DrawStripF.s` (0x1114D0, 464 B) and `gif_DrawStripG.s` (0x1116A8, 512 B) from Rev.146; `research/elf/rev142-p2o-vertex-layout-and-face-record-structure.md` + Rev.143/144 notes for the strip/UV decode rules.

## Step 1 (instrument) — the duplication IS real

Before touching the render path, `OpenGLBackend::Impl` gained `degenerateTriangleCount`/`realTriangleCount`, and `main.cpp` `TextureBatch` gained `sourceTriangles` plus a per-texture batch report. On `assets/170_st00a_p1.p2o`:

| Metric (flat `[A,B,C,C]` path) | Before instrumentation claim | Measured |
|-------------------------------|------------------------------|----------|
| Source triangles (PS2O) | 15,161 | 15,161 |
| Batch vertices | 60,644 | 60,644 |
| Batch indices | 60,644 | 60,644 |
| verts/srcTri | — | 4.00 (ideal indexed = 3+3) |
| GPU triangles (2 per quad group) | 30,322 | 30,322 |
| real / degenerate | — | 15,161 / **15,161** |

Verdict confirmed by measurement, not inference: `buildBatches` emitted `[A,B,C,C]` (4 verts + 4 indices) per source triangle and the static quad EBO rendered 2 triangles per group of 4 vertices — exactly half the GPU triangles degenerate. The duplication is a **quad-contract artifact in the batch builder + EBO**, not a PS2O decode problem.

One bug was fixed during this step: the degenerate classifier originally ran AFTER `I.batchIndices.clear()`, so it always classified 0 trials. Moved before the clear.

## Step 2 (input wiring) — real placeholder controlled by keyboard

`native/src/platform/Input.h/.cpp`: public `Input` with `NativeKey` (`KeyW/A/S/D`, `KeyArrowUp/Down/Left/Right`) and `setKeyState(u32, bool)` (bounds-guarded). Contract locked by `tests/input_test.cpp` (62 lines, passes):
- `update()` MUST run at the TOP of the frame (snapshots keys into prev) before the platform pump mutates state;
- `pressed` fires on 0→1, `released` on 1→0, held keys do not re-fire;
- out-of-range keys rejected; gamepad stubs inert.

`main.cpp` GLUE layer (kept platform-neutral Input): X11 keysyms → `NativeKey` via `setKeyState` in `feedKey` on KeyPress/KeyRelease, driving a red floor quad marker (half-size 30) that moves with WASD (step 25/cm-scale frame). Controls: WASD marker, arrows orbit camera, Z/+/wheel zoom, Q/Esc quit. CMake target `input_test` + `add_test(NAME input ...)`.

## Step 3 (strip semantics) — N spine verts -> N-2 triangles, no duplication

Guided by Step 1, the batch was rebuilt around the PS2 strip primitive. `gif_DrawStripF/G` (GIF prim 0xD) draw gouraud triangle strips: consecutive spine verts `[s0,s1,s2,...]` form triangles `(s0,s1,s2)(s1,s2,s3)(...)`, adjacent triangles sharing an edge. Native realization:

- **`Ps2oMesh`** (decode): preserved strip topology. `Ps2oStrip { spine, material, uvs }` per p2o frame primitive (spine in record order, degenerate inner pairs KEPT — GL skips zero-area); flat `triangles` list unchanged (tests/fallback). Per-spine-vertex UVs = `UVarray[k + offset[material]]` (Rev.143 `UV = k + offset[f]`, material base now exposed as `materialUVBase`). UI path teaches the loop to emit a strip when `mesh.strips` is non-empty.
- **`RenderBackend::drawStrips`** (new virtual): concatenated spine stream + per-strip `firsts[i]/counts[i]` spans; drawn as ONE `glMultiDrawArrays(GL_TRIANGLE_STRIP, ...)` per texture (GL 1.4+, resolved via dlopen; per-strip `glDrawArrays` fallback if the extension is missing). Texture binding mirrors `drawIndexed`. Triangle accounting: N-2 per strip, degenerates classified identically to the quad stream. `RenderStubBackend` counts N-2.
- **`main.cpp`**: `StripBatch` + `buildStripBatches` per texture; render loop uses strips when present, else falls back to the flat TextureBatch path.

### Verification: strip path on `assets/170_st00a_p1.p2o`

| Metric | Flat (Rev.146 era) | Strip (Rev.147) |
|--------|--------------------|-----------------|
| Source triangles | 15,161 | 15,161 |
| Batch vertices | 60,644 | **31,005** |
| verts/srcTri | 4.00 | **2.05** |
| GPU triangles | 30,322 | **15,163** |
| real / degenerate | 15,161 / 15,161 | **15,163 / 0** |

(`+2` GPU triangles = the input-wired red marker quad.) 7,922 strips per frame, 1 `glMultiDrawArrays` call per texture — down from one 4-index quad group per triangle. The 2.05 verts/tri vs. the flat 4.00 confirms the non-duplication: each interior strip vertex is shared between two triangles.

## What is confirmed

- The `[A,B,C,C]` GPU duplication was real (measured 2× triangles, 4 verts+idx per source tri).
- Strip semantics (`gif_DrawStripF/G` model) eliminate it: N spine verts -> N-2 triangles, 0 degenerates, on the real room piece (15,163 GPU tris = 15,161 src + marker quad).
- `glMultiDrawArrays(GL_TRIANGLE_STRIP)` is available (GL 3.3 core context) and batched per texture.
- Input contract (`update()` top-of-frame, `setKeyState` pump) is locked by `input_test` and drives a live red marker via WASD.

## What is probable

- The per-spine UV contract `UV = UVarray[k + offset[f]]` (Rev.143) holds for the strip path since strip UVs mirror the flat `triVertUVs` sampling; a textured-room visual pass is still the minimum confirmation.
- Degenerate inner spine pairs (kept in `Ps2oStrip::spine`) are zero-area and safely rendered; GL skips them.

## What is unknown

- Whether `glMultiDrawArrays` is available on the fallback GL 2.1 path of the backend (per-strip `glDrawArrays` fallback exists but is unexercised in testing).
- p2 wall-family files (`type 0x00` a==b / `type 0x01` a!=b) still need the per-file family discriminator before they can use the strip path (Rev.142 caveat, unchanged).

## What is discarded

- The hypothesis that the observed duplication was a read/report artifact of the quad-group contract: discarded by direct per-batch counts in Step 1.
- The hypothesis that the wild quick measure "1 vert/tri ideal" should drive batch layout: discarded — short strips average ~2 verts/tri; the correct invariant is `< 3` verts/tri with zero degenerate GPU triangles.

## Next minimum test

- Run `./build/ico_native --p2o native/assets/170_st00a_p1.p2o --frames -1` (headful) or `--frames N` (headless) and confirm the report: `2 draw calls, 15163 triangles (15163 real, 0 degenerate)`.
- Run full CTest; `ps2o_mesh_test` asserts strip topology (1 strip, spine [0,1,2,3], N=4 -> 2 tris).

## Conservative verdict

The three-part work order is closed on `native-port` with measured evidence at every step. The render pipeline now honors the PS2 strip primitive (N -> N-2, shared edges) instead of a duplicating quad contract, and the input seam is a working platform-neutral `Input` + X11 glue. The flat quad path remains as fallback for files without strip topology. CTest: 23/24 pass (only the pre-existing headless `opengl_backend` segfault, known/AGENTS.md).

**Note:** the working tree on `native-port` also still contains the empty-p4-commit / purge follow-up item from Rev.143-146 (git history rewrite) — unchanged by this revision.