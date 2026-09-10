# Rev.161 — Boy character mesh: verified loader + static texturized render

- **Date**: 2026-09-10
- **Branch**: `native-port`
- **Objective**: Replace the BoxMarker placeholder with the real ICO boy character model (multi-OBJH `.p2c`), render it static at native scale inside the room scene, and validate texture colors on screen.
- **Scope**: mesh loader auto-detect; strip/render integration; texture path; scale; visual validation; LCD-symptom resolution.
- **Sources**: `native/src/engine/Ps2oMesh.cpp`, `native/src/main.cpp`, `native/tests/ps2o_mesh_test.cpp`, `native/assets/boy/*` (local, gitignored), Rev.151 canonical PS2O block, TRILHA:PORT.
- **Evidence**: pixel histograms of real renders, PSMT4/CLUT byte dumps, ASCII downsample, user visual confirmation.

## Summary

The boy model renders statically in the room demo with real textures at ~1.2 m
scale and confirms the character mesh decode (Rev.142 p2 family + Rev.151
canonical rules) on a second, entirely different asset family (character
`.p2c` vs room `.p2o`). The "LCD / epilepsy colors" the user saw in the room
demo were the `--uv-test` checkerboard diagnostic flag left enabled — **not a
texture bug**. Real-texture renders are correct (warm browns/skin gradient,
pixel-verified). A new user observation — *"as texturas parecem esticadas"*
(textures look stretched) — is recorded as an open, deferred item.

## Character asset (local, gitignored)

| Field | Value |
|-------|-------|
| File | `native/assets/boy/boymodel.p2c` |
| Structure | PS2O header + 5 OBJH submeshes |
| Positions | 3,210 (16-B `x,y,z,1.0` floats) |
| UVs | 1,560 (16-B `u,v,0,0` floats, following positions) |
| Records | 3,777 (`[fl,0,a,a,m,0xFFFF,0,f]`; `a`==u16[2]==u16[3] 100%; `m`=u16[4]; u16[5]=0xFFFF; `f`=u16[7] local) |
| Strips | 526 (header `[N,0xFFFF×7]`, N=record count, cascade N-2 tris) |
| Triangles | 2,725 cascade (flat list) |
| Submeshes | 5 (OBJH tags at objh 0xCD0/0x5210/0x20AA0/0x45520/0x48830) |
| Materials | 15 global names (b_mantle … tape_boro), first-occurrence order |

`findObjs`/`isNameAt` auto-detect in `loadPs2oMesh` splits the single vertex/UV
stream across the OBJH region slice; each OBJH contributes its own (positions,
UVs, face region). Same canonical rule as Rev.151 — no per-file discriminator
needed for this family.

## Texture path — PSMT4 verified LINEAR (not swizzled)

All 15 boy TM2s are **PSMT4** with a 16-entry **RGBA32 CLUT** (64 B), e.g.
`b_suit` 128×128, `b_face2` 128×128, `b_arm` 64×64 (PIL/byte inspection, exact
TM2 v4 layout of `Tm2Format.h`).

- The CLUT (linear RGBA, A=0x80 on nearly every entry) is **correct**:
  `b_arm` = `#000000` transparent + skin ramp `#966B3F…#B1B2A2` (avgSat 0.18);
  `b_suit` = brown ramp 0B0A07→A0482C; `b_face2` = flesh tones; `b_pants` =
  light desaturated ramp. No palette byte-order bug.
- **The PSMT4 pixel data is already linear/row-major** in these files (the byte
  block-forward viewer decodes b_face2 as an unmistakable face, b_arm/b_pants/
  b_suit as coherent fabric shapes). `convertPSMT4` reads `src[i/2]` low/high
  nibble linear — matches. The (untested) `deswizzlePSMT4` variant that assumes
  GS 32-col blocks over-reads the buffer and is **not** wired into
  `convertImage`; it must NOT be enabled for these files.
- Room `PSMCT32` textures remain the swizzled family; both coexist.

## Scale and placement (world is PS2 cm-scale)

- `kBoyScale = 1.2f` (was 12.0f), `kBoyFootOffset = 0.65f * kBoyScale`.
  Boy native ~124 units ≈ 1.24 m; scale 1.2 ⇒ ~1.4 m in a cm-scale room.
- Strips are re-emitted each frame inside the scene GIF packet at the
  BoxMarker/GObj position (`markerX/Y/Z`), transformed by `kBoyScale`.
- `BoxMarker` now draws only when no boy mesh is available
  (`drawBoxMarker` gated on `!hasBoyMesh`).

## LCD / epilepsy symptom — resolved

The room demo was running with `--uv-test` (blue/green 8×8 checker with whitish
borders) as a mapping-diagnostic. That checker IS the "viagem de lcd". After
relaunching with real textures the user confirmed the LCD effect is gone.

Pixel-verified standalone render (`boy_real1.png`, textures loaded via
`--tex-dir assets/boy/texture`): boy bbox x279–369 y10–431; per-band median RGB
`(126,99,64) → (115,68,52)/(109,60,41) → (115,84,52)/(129,84,52) → (65,42,25)`
with ~9–12 distinct tone buckets per band — a coherent brown/skin gradient, not
random noise.

## Confirmed

- Character `.p2c` decode works with the canonical Rev.151 rule (records,
  cascade, m=u16[4] UV, f=u16[7] local material), validated on a non-p1 asset.
- Boy PSMT4 textures are linear (row-major); CLUT RGBA32 linear is correct.
- Static texturized render at correct scale; textures are brown/skin-correct.
- `ps2o_mesh_test`: `test_real_boymodel` OK (3,210 verts, 1,560 uvs, 526
  strips, 5 submeshes, 2719 tris, 15 mats); `render_backend_test` OK.

## Probable / Possible

- The "stretched" look (below) probably comes from UV orientation or per-part
  material↔TM2 association, not from index mapping (which is exact).
- The room demo camera frames the whole scene, so the boy is a small figure;
  full-frame standalone renders are available for close inspection.

## Unknown / Discarded

- Unknown: whether all ICO PSMT4 assets are linear or if some are GS-swizzled.
  Current loaders handle the observed (linear) case.
- Discarded: palette byte-order bug, UV index/heuristic bug, LCD = render bug.

## Deferred (user observation, not fixed)

> "o efeito lcd passou mas as texturas parece esticadas"

Textures look **stretched** on the boy in the room demo. Recorded for a later
revision. Candidate directions (not yet verified): texcoord v-orientation,
per-part material↔TM2 association, or per-submesh UV base. No code change made
for this yet.

## Next minimum test

1. Confirm with the user whether the standalone full-frame boy
   (`boy_real1.png` path, `--p2o assets/boy/boymodel.p2c --tex-dir
   assets/boy/texture`) shows the same stretching; the room shot makes the boy
   small and hard to judge.
2. Reproduce the stretch in a zoomed view, then test v-flip and
   material↔TM2 per part.

## Verdict

Milestone reached: real boy mesh renders statically with correct warm
textures at native scale; the perceived "LCD" was the checker diagnostic, not
a defect. The texture-stretch observation is a tracked loose end, not a
blocker for subsequent milestones.