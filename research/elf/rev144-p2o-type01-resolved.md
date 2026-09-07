# Rev.144 — PS2O TYPE 0x01 semantics resolved: no separate decode; `b` does not generate triangles (native-port)

- **Date:** 2026-09-07
- **Objective:** Close Rev.142's open item — "TYPE 0x01 SEMANTICS is the next step for the static-mesh milestone".
- **Scope:** `171_st00a_p2.p2o` (p2) and `170_st00a_p1.p2o` (p1) face blocks; hypothesis: type-01 records encode quad/fan/dual-strip geometry requiring a `b`-column decode.
- **Sources:** PAL `.DF` container unpack → `/tmp/df-stgst00a/170_st00a_p1.p2o` (1,065,408 B), `171_st00a_p2.p2o` (476,656 B), `097_box_water.p2o`; existing `native/src/engine/Ps2oMesh.cpp` (Rev.142/143 decoder).
- **Evidence:** raw 8×u16 record bytes; strip/type counters; edge-manifoldness (boundary-edge ratio) as decode-quality metric.

## Finding

**TYPE 0x01 has no separate decode. It is decoded exactly like TYPE 0x00: a triangle strip whose spine is the `a` (u16[3]) column. The `b` (u16[5]) column does not generate triangles.**

The boundary-edge ratio — fraction of mesh edges shared by != 2 triangles — cleanly separates the correct decode from all alternatives:

### p1 (`170_st00a_p1.p2o`, 13,877 verts) — known-good renderer (Rev.143 texturized room)

| Model | tris | edges | boundary | bnd_ratio |
|-------|------|-------|----------|-----------|
| **M-A a-col spine only** | **15,161** | **22,939** | **582** | **0.025** |
| M-B twin (a+b) | 22,395 | 36,674 | 13,764 | 0.375 |
| M-C interleave (a0,b0,a1,b1…) | 36,959 | 59,394 | 30,199 | 0.508 |
| M-D quad fwd (a_k,b_k,a_{k+1},b_{k+1}) | 36,918 | 59,356 | 30,179 | 0.508 |
| M-E quad alt | 36,918 | 59,324 | 38,153 | 0.643 |

M-A reproduces **exactly** the 15,161 tris documented in Rev.142/143 and rendered by the native demo. Its bnd_ratio of **0.025** indicates a nearly closed room shell. All `b`-consuming models add thousands of spurious triangles and push the boundary ratio into garbage range — the `b` column is not hidden geometry.

### p2 (`171_st00a_p2.p2o`, 6,330 verts, 3,562 strips: 2,972 type-01 + 590 type-00)

| Model | tris | edges | boundary | bnd_ratio |
|-------|------|-------|----------|-----------|
| **M-A a-col spine only** | **7,881** | **12,608** | **1,583** | **0.126** |
| M-B twin (a+b) | 9,793 | 14,904 | 7,080 | 0.475 |
| M-C interleave | 15,692 | 25,841 | 13,292 | 0.514 |
| M-D quad fwd | 15,648 | 25,800 | 13,272 | 0.514 |
| M-E quad alt | 15,648 | 26,449 | 18,894 | 0.714 |

p2's 0.126 is higher than p1's 0.025 because p2 is a *pieces/objects* payload (open boundaries expected), but it is far below the 0.47+ of every `b`-consuming model, confirming a-col-only is the correct decode here too.

## Why the earlier "parallel wall" observation was a red herring

During Rev.142-143 investigation, hand-inspected strips B/C of p2 appeared to show the `b` column forming a second planar wall quad (e.g. strip C: a-quad z4020–4255, b-quad z3850–4295, all x=-90). This was an artifact of sampling strips whose vertices happened to be coplanar in the same wall plane (all x=-90). Global manifold evaluation proves the b-column triangles are incoherent when measured across the full file, so the coplanarity was coincidental geometry, not a second surface.

## Frames analysis

- p2 type-01 strips: 2,972 of 3,562 (83.4%). nrec mostly 3–4. a<b in 3,737 records, a>b in 7,085, a==b in 24 → the `a`/`b` relation in p2 is *not* a mirror (`a==b` rare), reinforcing that `b` is unrelated carry data, not a mirrored spine.
- p1 type-01: 7,338 of 7,922 strips; a<b in 25,209 records vs a>b in 591, a==b in 113 — b ranges over valid vertex indices but produces incoherent geometry when decoded.
- `m` (u16[4]) is constant within 97.7% of strips (p2: 3,481/3,562; p1: 7,219/7,922) and spans a wide range (p2: 0..1497, distinct 1498) — it is a per-strip counter/tag (likely a piece/partition id), not a vertex index and not a per-record field in the spine.
- Terminator records are `[0, T, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF]` with T ∈ {3,4,5,6,7,8,9,10,11,12,14,16,20,32,…} — already handled by `t2 > 0x01`.

## Current decoder alignment

`native/src/engine/Ps2oMesh.cpp` (lines 255–323) already implements M-A exactly:

- spine = `a` values of consecutive records; triangles `(v0,v1,v2)(v1,v2,v3)…`
- degenerate (equal-index) inner triangles dropped
- terminator and false-head hunting via `t0 > 0x01` and `c0 != 0xFFFF`
- `b` (u16[5]) read and bounds-checked but not added to the spine

No decoder change is required for this milestone.

## Confirmed

1. TYPE 0x01 decodes identically to TYPE 0x00 → spine-a triangle strip.
2. `b` (u16[5]) never contributes triangles (all b-consuming models degrade mesh coherence catastrophically on both files).
3. `m` (u16[4]) is a per-strip tag/counter; not geometry.
4. p1 M-A reproduces the exact tutorial triangle count (15,161).

## Probable

- `b` may be a per-vertex attribute id (e.g. UV/attribute index or normal/secondary channel) or an overage/duplicate vertex reference used by the original toolchain for export-order reasons. No consumer found yet; not required for geometry.
- `s` (u16[6]) shows a per-record increment in some p1 strips (e.g. strip @0x6a6be: s=1, @0x6a6ce: s=2) — possibly a slot/reference counter; unused by geometry.

## Unknown

- Exact meaning of `b` and `s` fields (not needed for static geometry render).
- `+0x1c` object sub-mesh delimiter semantics (still open from Rev.142).

## Discarded

- Quad/fan/dual-strip interleaved interpretations of type-01 (M-B..M-E) — all shown incoherent.
- "M-column = per-record counter" — actually constant per strip.
- Earlier hand-sample suspicion that b-columns are a second parallel wall (coplanarity artifact).

## Next minimum test

None required for the geometry milestone — decoder is already correct. Follow-on backlog item (already open): replace per-triangle draw calls with batch rendering.

## Conservative verdict

TYPE 0x01 is not a distinct primitive semantic; do not add a `b`-column decode to `Ps2oMesh`. The static-geometry milestone (asset → geometry → render) is complete as of Rev.143; the only rendering debt is batching, not decode correctness.