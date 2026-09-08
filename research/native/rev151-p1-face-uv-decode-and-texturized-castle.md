# Rev.151 — PS2O p1 face/UV record decode (byte-validated) + fully texturized castle render

- **Date:** 2026-09-08
- **Branch:** native-port
- **Objective:** Replace the wrong face/UV record interpretation of p1-room
  `.p2o` (Rev.142/143) with a byte-validated decode, so the room renders
  correctly texturized in the native runtime.
- **Milestone:** first fully texturized castelo/estágio st00a room in
  `ico_native`.

---

## Scope

`native/assets/170_st00a_p1.p2o` face/UV region (bytes 0x6A6A0 .. 0x101580)
and the `Ps2oMesh::loadPs2oMesh` decoder.

## Evidence used

- Raw hexdump + 16-byte walker of the face region.
- UV edge-spread metric (mean Euclidean distance between a triangle's UV
  coordinates, in texture space) as the discriminator between candidate
  models.
- Record-count validation: for every strip header value N, `count x N` must
  equal the observed record total (exact).
- End-to-end pixel histogram of the rendered window (before/after).

## Sources used

- `native/src/engine/Ps2oMesh.cpp` / `.h` (decoder).
- `/tmp/opencode/face_geom.cpp`, `face_hdr.cpp`, `face_uv2.cpp`,
  `face_stats.cpp`, `face_mat.cpp`, `face_final2.cpp`, `face_uvcov.cpp`
  (diagnostic harnesses; the winning model is replicated by `face_final2`).
- `tools/dfdatas-index/dfdatas_unpack.py` + `/tmp/pal-unpacked/` catalog.

---

## Findings

### Correct record layout (supersedes Rev.142/143)

The Rev.142/143 model (`[c, t, 0, a, 1, b, 0, 0]` framed by `0xFFFF`, strip
spine = u16[3]) is WRONG. The real layout, byte-validated:

```
STRIP HEADER (16 B = 8 x u16): [ N, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
                                  0xFFFF, 0xFFFF, 0xFFFF ]
  N = literal number of RECORD rows that follow. N in [2,64] is safe.

RECORD (16 B = 8 x u16): [ 1, 0, a, s, m, b, u6, f ]
  a  = u16[2]  vertex position index            (p1 max 7792  < nv 13877)
  s  = u16[3]  stream id, CONST per strip       (p1 max 6083, 5349 distinct;
               does NOT index materials)          ← do not use as position
  m  = u16[4]  UV index into the UV array       (p1 max 13069 = NUV-1;
               EXACT full coverage 13070/13070)   ← THE UV source
  b  = u16[5]  mirror of a (== a on most rows)
  u6 = u16[6]  CONST per strip, 10 distinct values 0..9; coincidentally
               matches legacy tex=1/tex=2 counts (2154/168) but cannot be
               the material index (10 != 7 names)  ← do not use as material
  f  = u16[7]  material index, EXACTLY 7 distinct values 0..6 (p1) == the
               7 embedded material names            ← THE material source
```

### Strip header N is a literal count, not a type code

| Header N | Header count | Records (count x N) | Exact? |
|----------|--------------|---------------------|--------|
| 3        | 4636         | 13908               | yes    |
| 4        | 2352         | 9408                | yes    |
| 5        | 142          | 710                 | yes    |
| 6        | 330          | 1980                | yes    |
| 7        | 73           | 511                 | yes    |
| 8        | 133          | 1064                | yes    |
| ... up to 32 | ...       | ...                 | yes    |

Every observed header reproduces the record total exactly — N is the
record count.

### Triangle construction (cascade strip)

One strip = header + N records emits **N-2 triangles**:
`(r0,r1,r2)(r1,r2,r3)...(rN-3,rN-2,rN-1)`.
Vertex k of each triple uses `a` (u16[2]) of record r(k) for position and
`m` (u16[4]) of the SAME record row for UV.

### Model comparison (UV edge spread, lower is better)

| Model | Emitted tris | Mean spread | Verdict |
|-------|--------------|-------------|---------|
| Cascade (pos=a, uv=m=u16[4]) | 15,007 | **0.1645** | WINNER |
| Old tolerant walker (spine u16[3]) | 15,161 | 0.171 | wrong spine |
| Strip-window s-equal | 11,450 | 0.189 | — |
| Fan-pivot-first | 15,571 | 0.228 | rejected |

### Validated figures (`170_st00a_p1.p2o`)

| Metric | Value |
|--------|-------|
| vertex positions | 13,877 (nv) |
| UV entries | 13,070 (NUV), full coverage by m |
| strips | 7,877 |
| cascade triangles emitted | 15,007 |
| face region | 0x6A6A0 .. 0x101580 (ends before OBJH @ 0x101620) |
| material indices used | f = 0..6 (7 distinct) |

Material names (extracted in file order == index order):

| f | name      | strips | TM2 size |
|---|-----------|--------|----------|
| 0 | metal     | 3,386  | 10,400 B (128x128) |
| 1 | wall_dec1 | 170    | 10,400 B (128x128) |
| 2 | st0_a     | 2,312  | 43,168 B (256x256) |
| 3 | broken    | 1,019  | 43,168 B (256x256) |
| 4 | wall_fuchi2 | 320  | 1,152 B (64x32) |
| 5 | isikabe   | 320    | 10,400 B (128x128) |
| 6 | pole      | 18     | 4,224 B (128x64) |

(strip counts from the per-strip `f`; small residual counts are the
remaining header rows.)

## Changes

- `native/src/engine/Ps2oMesh.cpp`:
  - Removed the wrong c/t/a/b tolerant walker framed by 0xFFFF.
  - Removed `computeMaterialUVOffsets` heuristic (UV = k + offset[f]) — the
    direct `m` index made it unnecessary.
  - New face walker: row-aligned scan from UV end → first `[N, ffff x7]`
    header (N in [2,64]); consume exactly N records; stop at first
    non-header row.
  - `Ps2oStrip.spine` = `a` (u16[2]), `strip.uvs` = `m` (u16[4]) per vertex.
  - Flat `triangles`/`triMaterials`/`triVertUVs`: cascade N-2 tris, UV = m.
- `native/src/engine/Ps2oMesh.h`: header doc updated to the canonical format;
  removed `materialUVBase`.
- `native/tests/ps2o_mesh_test.cpp`: fixture rewritten to the canonical strip
  format (header `[4, ffff x7]` + 4 records, uv=m, material f=3); asserted
  exact strip UVs and cascade flat UVs.
- `native/assets/`: added `metal.tm2`, `wall_dec1.tm2`, `wall_fuchi2.tm2`,
  `isikabe.tm2`, `pole.tm2` (from PAL `STGST00A.DF/object/sdf/st00a/texture/`).
- `AGENTS.md`: item 50 + canonical PS2O face-record format block (so the
  decode is not re-derived by future agents).
- Screenshot: `research/native/ico-rev151-castelo-texturizado-2026-09-08.png`.

## Validation

- `ps2o_mesh_test`: OK (4 verts, 2 tris) — fixture round-trips.
- CTest: 25/26 (only `opengl_backend` segfaults headless — pre-existing).
- `ico_native` on-disk decode of p1 matches the harness exactly:
  verts=13877 uvs=13070 strips=7877 tris=15007 matMax=6 names=7
  meanSpread=0.1645.
- Runtime window pixel histogram (screenshot, 640x448):
  | Metric | before (no metal/etc) | after (7 textures) |
  |--------|-----------------------|--------------------|
  | white pixels | 5,106 (7.1%) | **0 (0.0%)** |
  | colored pixels | 16,279 (22.7%) | **21,601 (30.1%)** |
  | mean colored RGB | (143,130,115) | (125,112,97) |
- User confirmation: castle structure and brick texture correct; room now
  complete.

## Confirmed

- `m` = u16[4] is the per-vertex UV index (full 13070/13070 coverage; single
  record model consistent across all 7,877 strips).
- `a` = u16[2] is the vertex position index.
- `f` = u16[7] is the material index (exactly 7 values == 7 names).
- Strip header `[N, ffff x7]`, N = record count, cascade N-2 triangles.
- Face region ends at 0x101580 in p1 (before OBJH).

## Probable

- `s` = u16[3] is a stream/part label (const per strip, 5349 distinct) — not
  yet mapped to the OBJH [offset,count] tables at 0x101620/0x103d20/0x1040b0.
- `u6` = u16[6] groups strips into 10 buckets that partially overlap the
  material split; semantics not finalized.

## Possible

- The OBJH submesh stream tables drive submesh→material mapping for p2.

## Unknown

- p2 wall family discriminator (type 0x00 long tri-strips a==b vs type 0x01
  short quads/fans a!=b, Rev.142) — the p1 cascade-a rule over-decodes p2.

## Discarded

- Rev.142/143 `[c,t,0,a,1,b,0,0]` + 0xFFFF-frame framing and u16[3]-spine.
- `computeMaterialUVOffsets` heuristic (UV = k + offset[f]).
- `u16[6]` as material index (10 != 7 names).
- `u16[3]` as vertex position (max 6083, 5349 distinct values cannot index
  13,877 positions coherently).

## Next minimum test

- Add p2 (`170_st00a_p2.p2o`) with a discriminator; validate zero UV
  over-decoding and no degenerate cascade explosion.
- (Nice-to-have) Map `s` (u16[3]) to the OBJH [offset,count] stream tables.

## Conservative verdict

p1 room geometry + UV + material are now decoded with byte-level evidence
(record-count exactness, full UV coverage, spread 0.1645) and render fully
texturized in the native runtime. The remaining p2 wall family is a
separate, still-open problem.