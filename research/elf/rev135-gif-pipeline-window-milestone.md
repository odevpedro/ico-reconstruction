# Rev.135 — First observable native-port milestone: real window + GIF pipeline rendering

Date: 2026-09-05
Trilha: PORT
Scope: `native/` — P1 of the priority directive
(`/home/hoper/Downloads/diretriz-prioridades-ico-native-port.md`)

## Objective

Make the graphic backend part of the default `ico_native` binary and produce
the first observable evidence of the native PC port: a real window with
something drawn through the PS2-style GIF pipeline, captured as a screenshot.

## Confirmed

1. **`ICO_ENABLE_OPENGL` is now ON by default** (`native/CMakeLists.txt`).
   `ico_native` links the OpenGL backend, opens a real GLX window (640x448,
   title "ICO") and renders 180 frames (~3 s) by default. `--frames N` controls
   the duration. Without GL the binary falls back to the headless runtime boot.

2. **Rendering path is the semantic GIF pipeline**, not a special-case draw:

   `main.cpp` -> `GifPacketBridge::startPacketPri` -> `setGsReg(TEX0_1)`
   -> `sprite()` (3 animated quads) -> `flush()` -> command buffer ->
   `GifCommandExecutor` -> `OpenGLBackend::drawSprite` -> batch + swap.

3. **Window output verified frame-exact via ImageMagick histogram**
   (`research/native/ico-native-first-window-2026-09-05.png`):

   | Color | Pixel count | Meaning |
   |-------|-------------|---------|
   | (24,28,36) | 267,820 + background clear | 640x448 clear |
   | (150,200,90) | 6,300 | quad 0 (green), 90x70 = 6,300 |
   | (240,170,90) | 6,300 | quad 1 (orange) |
   | (120,170,240) | 6,300 | quad 2 (blue) |

   Each quad occupies exactly its requested 90x70 pixel rect. 300 frames at
   ~60 fps, exit code 0, clean `OpenGLBackend shutdown`.

4. **21/21 CTest pass** (20 previous + `opengl_backend`), forced clean build.

## Two rendering-blocking bugs found and fixed (OpenGLBackend.cpp)

| Bug | Impact | Fix |
|-----|--------|-----|
| `p_glCreateProgram()` used where `glCreateShader(GL_VERTEX_SHADER/GL_FRAGMENT_SHADER)` was required | shader objects were never created; `glCompileShader`/attach/link all failed; `program=0`; GL 3.3 core drew nothing (silent black client area) | Added `p_glCreateShader` (typedef + dlopen + LOAD_GL_FUNC) and used it in `compileShaderProgram` for both stages |
| GL context made current to the plain X window (`glXMakeCurrent(display, I.window, ...)`, init lines before glxWindow creation) while `present()` swaps `I.glxWindow` | rendered content never reached the visible surface; only stale/black client area displayed despite "1 draw calls, 6 triangles" per frame | After `glXCreateWindow`, re-`glXMakeCurrent(display, I.glxWindow, ctx)` so all drawing lands on the double-buffered GLX window |

Also cleaned: `glEnable(GL_ALPHA_TEST)` is legacy invalid in core profile
(emitted `GL_INVALID_ENUM` once at init); now guarded by `!I.isGL33`.

## Coordinate mapping note (probable, not a bug)

`OpenGLBackend::drawSprite` pre-scales vertices via `sx = mvp.m[0]*2/width` and
then the fragment path applies the full MVP again in the shader
(`gl_Position = uMVP * vec4(aPos, 1.0)`) — a double application. A compensating
projection `Matrix4x4::ortho(-0.5, 1.5, -0.5, 1.5, -1, 1)` maps pixel
coordinates `[0,640]x[0,448]` to full-screen NDC and was used in the demo.
This is a semantic mismatch that should be normalized when real GS-style
coordinates arrive (P2 scene rendering); it is not yet a correctness issue.

## Possible

- The GIF TEX0 register binds textures through `tbp0() & 0x3FFF`, treating the
  TextureHandle as tbp0. The demo binds a 1x1 white texture so vertex colors
  show. Real TM2 uploads (P2) will ride the same `SetTexture` path.

## Unknown

- Whether `waitForVSync`/present pacing is needed; demo used `sleep_for(16ms)`.
- Behavior under a real compositor at other refresh rates.

## Discarded

- The "1.224 functions" byte-exact figure (already corrected in Rev.117/Rev.131)
  was re-confirmed stale in `docs/native-port-roadmap.md:23/85` and
  `native/README.md:26` and updated to 688 / 710.

## Next minimum test

- P2: load a real TM2 texture through `TEX0` + `databin` data and render it on
  the window (requires `DATA.DF` extraction validation or a synthetic TM2
  fixture).

## Conservative verdict

Rev.135 closes P1 of the directive: the default `ico_native` build opens a
real window and presents frames produced by the semantic GIF pipeline. This is
the first observable evidence of the native PC port. No real ICO data was used;
everything drawn is synthetic.