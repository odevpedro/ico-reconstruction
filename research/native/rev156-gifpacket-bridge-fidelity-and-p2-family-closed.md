# Rev.156 — GifPacket bridge fidelity (prim/viewport/half-offset) + p2 family discriminator CLOSED

- **Date:** 2026-09-09
- **Trilha:** native-port (P1)
- **Objectives:** (a) bridge `GifPacket.*` to the GIF command-buffer model with the missing byte-exact-faithful semantics; (b) close the Rev.151 open item "p2 wall family needs its own discriminator".
- **Scope:** `native/src/engine/GifPacket.{h,cpp}`, `native/tests/gif_packet_test.cpp`, `native/tests/ps2o_mesh_test.cpp`, `tools/ps2o_family_analysis.py`.
- **Sources:** byte-exact `src/core/asm/gif_*.s` (18 files), `research/elf/rev146-gifpacket-usa-boundaries-byte-exact.md`, `research/elf/rev151-face-uv-correction.md`.

---

## Part 1 — GifPacket bridge fidelity (Rev.156)

### What was missing

The `GifPacketBridge` (Rev.146) maps function name -> `RenderCmd` correctly but
left three instrumentable arguments behavior-less:

1. `startPacketPri(prim)` / `startPacketPriPath1(prim)` discarded `prim`.
2. `setDrawEnvironment(x, y, w, h, ...)` discarded `x`/`y` (viewport origin
   forced to 0,0).
3. `setHalfOffset(h, v)` was a stored-no-op.
4. The `*Offset` emit variants (`makeSpriteOffset`, `spriteOffset`,
   `spriteSensitiveOffset`, `makePoint2DOffset`, `makeLine2DOffset`,
   `makeSpriteNoTextureOffset`, `pointOffset`, `lineOffset`,
   `draw2DUVStripG`) were exact aliases of their base counterparts.

### What was verified in the byte-exact .s

- `gif_Init` / `gif_StartPacketPri*` stream the GIF tag `prim<<6 | 0x406`
  (Rev.146). Capturing `prim`/path on the bridge is therefore faithful to the
  register contract, even though the native executor does not yet consume
  prim-type bits for primitive rasterization policies.
- A constant `0x8000` is added to the packed XYZ2 X/Y fields in a **subset** of
  both base and Offset primitives:
  - with 0x8000: `gif_MakeSprite`, `gif_MakeSpriteOffset`, `gif_Draw2DStripG`
    (lines 51-52), `gif_Draw2DUVStripG` (verified both vertices, lines 56-57,
    68-69), `gif_SpriteSensitive`, `gif_MakePoint2DOffset`, `gif_EndPacket`.
  - without 0x8000: `gif_Sprite` (Rev.145 "half-offset" note refers to
    `gif_SpriteOffset`, not `gif_Sprite`).

**Conclusion (conservative):** the `0x8000` is NOT a verified discriminator
between Offset and non-Offset variants — both families show it in some `.s`.
It is the GS half-pixel/texel-alignment constant applied during XYZ packing in
a per-function manner. The native bridge therefore does **not** claim a
byte-verified automatic pixel translation.

### What the bridge now does

- `currentPrim()` / `currentPath()` accessors record the primitive + path from
  `startPacketPri*` (0 before any start).
- `setDrawEnvironment(x, y, ...)` emits the viewport with the given origin
  (`viewport.x/y` now honored) plus the framebuffer command.
- `setHalfOffset(h, v)` stores a host-side offset; when nonzero it is applied
  on both axes to every `*Offset` emit variant and to `draw2DUVStripG`
  (documented host hook, **not** claimed as byte-verified GS behavior).
- Default `m_offsetX/m_offsetY = 0.0f` => Offset variants remain identical to
  base variants, preserving prior behavior until a caller opts in.

Left open (documented limitation): `draw2DUVStripG` still has no per-vertex UV
carrier in `RenderCmd` line model; UV consumption is a future command-model
extension, the vertex offset behavior is exercised.

### Tests (gif_packet_test.cpp)

| Test | Asserts |
|------|---------|
| `test_packet_prim_and_path` | prim/path captured and retained through endPacket |
| `test_draw_environment_viewport_origin` | viewport x/y/w/h + framebuffer from one call |
| `test_half_offset_fidelity` | 8 Offset variants shift a<-1,2> on both axes; default = base |
| `test_draw2duv_strip_offset` | UV strip shifts with offset; gouraud flag preserved |

## Part 2 — p2 family discriminator CLOSED (Rev.156)

### The open item (Rev.151)

Rev.142 described p2 (171_st00a_p2.p2o) as needing "a per-file family
discriminator" (type 0x00 = long tri-strips a==b; type 0x01 = short quads/fans
a!=b). Rev.151 left "p2 wall family still needs its own discriminator" open.

### Evidence (tools/ps2o_family_analysis.py, aligned Rev.151 columns)

| Metric | p1 (170_st00a_p1) | p2 (171_st00a_p2) |
|--------|-------------------|-------------------|
| header rule `[N,0xFFFF x7]` | 7,877 headers | 3,562 headers |
| record total = sum(N) | 30,761 | 15,005 |
| max pos index a (u16[2]) | 7,792 (< nv) | 4,714 (< nv=6,213) |
| UV coverage m (u16[4]) | 13,070/13,070 (1.000) | 4,844/4,844 (1.000) |
| flag u16[0] 0/1 count | 571 / 7,306 | 590 / 2,972 |
| bnd_ratio (bound vs total tris) | 0.021 | 0.126 (= Rev.144 M-A) |
| distinct f = u16[7] | 7 (0..6) | 6 (0..5) |

**Finding:** p2 decodes through the **exact same** canonical rule as p1
(header `[N,0xFFFF x7]`, record `[1,0,a,s,m,b,u,f]`, spine a=u16[2], UV
=m=u16[4] with 100% UV coverage, material f=u16[7], cascade N-2 tris per
strip). The u16[0] flag (Rev.142 "type") is a per-strip attribute correlated
with material, **not** a decoder branch. bnd_ratio 0.126 on p2 reproduces
Rev.144's M-A metric on the correct column, confirming the earlier "M-A"
reading was right and no alternative p2 rule exists.

**Conclusion:** the Rev.151 "p2 discriminator" open item is **CLOSED — no
decoder change required**. `Ps2oMesh.cpp` already handles p2 correctly with the
p1 rule.

### Test (ps2o_mesh_test.cpp)

`test_two_strip_family_unified` builds a minimal two-strip fixture (flag
u16[0]=0 vs 1) and asserts both decode through the same rule: same spine
topology, distinct per-strip materials from u16[7], per-vertex UVs from u16[4],
flat cascade triangles (2/strip).

---

## Confirmed

- `prim` and path registers are now captured by the bridge (startPacketPri*).
- Viewport origin from `setDrawEnvironment` is honored (x/y).
- `setHalfOffset` is no longer a silent no-op; Offset variants consume the host
  hook and default to identity.
- p2 uses the identical decoder rule to p1; UV full coverage (4,844/4,844)
  and material range 0..5 (6 names) confirmed via the analysis tool.

## Probable

- `0x8000` on XYZ2 packing is the GS half-pixel/texel-alignment constant
  (per-function), applied in a subset of both base and Offset gif functions.
  Its exact native-pixel translation is **not** byte-verified — the bridge
  keeps it as an explicit host hook.

## Possible / unknown

- Prim-type bits driving native rasterization policies (executor side) — not
  implemented, not required for current renderer.
- Per-vertex UV carrier on strips/lines in `RenderCmd` — future command-model
  extension.
- Whether `gif_Clip`-family offset variants ever fire in real ICO gameplay.
- p2 material-name identity (6 names) — extraction needs a p2 asset run.

## Discarded

- "Offset variants universally add 0x8000, base do not" (contradicted by
  `gif_MakeSprite`, `gif_Draw2DStripG`, `gif_SpriteSensitive`).
- "p2 needs a separate decoder / per-file family guard" (falsified by aligned
  column analysis + 100% UV coverage on BOTH files).

## Next minimum test

- Run `gif_packet_test` + `gif_command_test` + `gif_executor_test` and the
  full `build/*_test` suite (27/28, only headless `opengl_backend` excluded).
- Optional: run a `ico_native --frames 1 --shot` demo to confirm no regressions
  in scene rendering after the bridge changes.

## Conservative verdict

Rev.156 delivers two independent, test-covered improvements: (1) the
`GifPacketBridge` no longer drops the `prim`, viewport origin, and half-offset
inputs, with the half-offset level explicitly kept as a documented host hook
rather than an unverified byte translation; (2) the p2 "family discriminator"
open item is closed as unnecessary, since both wall and room families obey the
single canonical Rev.151 rule — backed by a byte-aligned analysis tool and a
two-flag fixture test.