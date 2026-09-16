# Rev.173 — Sky gradient quad reused the last bound piece texture (static "cloud")

Date: 2026-09-16

## Objective

Fix a host-rendering defect in the native runtime where the room sky was
reported by the user as a single giant, static "cloud" covering the viewport.

## Scope

- `native-port` host renderer only (no byte-exact `.s` change, no `main`/`master`
  reconstruction change).
- Affected scene: `st00a` scene composition via `assets/scene/stgst00a.manifest`
  (`--scene assets/scene/pieces --frames 0`).
- Function under change: `OpenGLBackend::drawSkyGradient` in
  `native/src/engine/OpenGLBackend.cpp`.

## Sources used

- `native/src/engine/OpenGLBackend.cpp` (`drawSkyGradient` ~1545,
  `flushBatch` ~1647, `bindTexture` ~1050).
- `native/src/engine/GifPacket.cpp` (`drawSkyGradient` command emit ~394),
  `native/src/engine/GifCommandExecutor.cpp` (~193).
- `native/src/main.cpp` (sky emit ~1495-1505; per-piece draws ~1521/1537/1584).
- Runtime log `/tmp/rev173_window.log`.

## Evidence used

- Instruction-/line-level code inspection of the batch texture state machine.
- Direct runtime observation (interactive GTK/X11 window) by the user, who is
  the project's visual validator.

## Findings

### Root cause

`flushBatch()` binds the solid white texture only when no texture is bound:

```cpp
// OpenGLBackend.cpp ~1670
if (I.whiteTexture && I.boundTexture == kNullTexture) {
    p_glBindTexture(GL_TEXTURE_2D, I.whiteTexture);
}
```

`OpenGLBackend::drawSkyGradient` appends a full-screen NDC quad (u,v = 0..1)
into the same batch and calls `flushBatch()` to draw it, but it **never binds a
texture and never resets `I.boundTexture`**. `I.boundTexture` is left over from
the last `drawStrips()`/`bindTexture()` call of the previous frame, so from
frame 2 onward the background quad was drawn with that **piece texture stretched
across the full screen**. That is:

- static on screen (the quad is emitted in NDC with identity matrices);
- invariant under camera zoom (not world geometry);
- visually a single large texture blob ("cloud").

### Fix

Set the untextured path before flushing the background quad:

```cpp
flushBatch();                       // draw the previous batch with its own texture
I.boundTexture = kNullTexture;      // force white texture for the gradient quad
... append 4 NDC vertices ...
flushBatch();                       // flushBatch binds I.whiteTexture
```

`I.boundTexture = kNullTexture` is sufficient because `flushBatch()` then takes
the white-texture branch, so only the per-vertex gradient colours modulate the
quad.

## Status table

| Item | Status | Evidence |
|------|--------|----------|
| Background quad emitted in NDC | Confirmed | `drawSkyGradient` ~1566-1608 |
| `drawSkyGradient` never (re)binds a texture | Confirmed | code |
| `flushBatch` reuses `I.boundTexture` when set | Confirmed | ~1670 |
| Static "cloud" disappeared after fix | Confirmed (user visual) | interactive window |
| "Melting" castle symptom in current st00a scene | Not reproducible | user visual |
| Real st00a sky/cloud asset | Unknown | see below |

## What is confirmed

- The sky "cloud" was the previous frame's piece texture stretched full-screen.
- The fix makes the sky show only the host gradient (top/bottom colours).
- The `st00a` room has **no** sky/cloud asset locally: `native/assets/scene/texture/`
  contains no `sky*.tm2`/`cloud*.tm2`, and the `st00a` piece set has no cloud or
  sky-backdrop piece. Sky/cloud assets exist only in other rooms (st17a, st19a,
  st22a, st24a, st26a, st47a).

## What is probable

- The exact texture seen before the fix was whatever piece was last drawn in the
  prior frame (order-dependent), not a specific "sky" texture.

## What is possible

- The original `st00a` sky may come from a globally shared sky asset or engine
  sky-dome not present in the room-specific extraction.

## What is unknown

- The real source of the original `st00a` sky/clouds.
- Whether additional host elements (additive `window_flare`/`window_glow`,
  `*_add` materials) are also rendered incorrectly; those remain drawn
  `RenderList::Opaque` with a fixed 255 tint and no vertex colours
  (`main.cpp:1521/1537/1584`, colours forced at `600/652/1576`).

## What is discarded

- The "melting" castle symptom as a current defect. The user clarified it
  occurred in a previous configuration where other parts of the castle were
  connected / the whole map was rendered; it does not reproduce in the current
  `st00a` scene.

## Next minimum test

- None blocking for this fix.
- Follow-ups (not part of this revision): (a) locate the real `st00a` sky/cloud
  source; (b) model GS per-primitive material state (blend/alpha-test/vertex
  colour), ideally from a PCSX2 GS capture rather than a name heuristic.

## Conservative verdict

The background quad was being textured with a stale bound texture. Forcing the
untextured path removes the artifact. This is a host-renderer fix only and makes
no claim about the original `st00a` sky contents, which remain unknown.
