# Rev.118 — VRAM Texture-upload Model (BITBLTBUF/TRXPOS/TRXREG + IMAGE → TEX0)

## Date
2026-09-03

## Objective
Close the remaining "VRAM destination not modeled" gap left by Rev.115-116 in
the native render path: model the GS **block-transfer registers**
(BITBLTBUF/TRXPOS/TRXREG/TRXDIR), decode a host→VRAM **IMAGE** upload into an
RGBA8 texture, resolve it to a texture handle via **TEX0.tbp0**, and bind that
texture to real draws in the executor. This makes the previously-empty
`GifCommandBuffer::currentTexture()` and `processImageLoop` functional.

## Scope
- `native/src/engine/GifTag.h` / `GifTag.cpp` — new transfer-register structs:
  `GsBitbltBuf`, `GsTrxPos`, `GsTrxReg`, `GsTrxDir` (bitfield accessors)
- `native/src/engine/GifCommandBuffer.{h,cpp}` — transfer state, BITBLTBUF/
  TRXPOS/TRXREG/TRXDIR A_D handlers, IMAGE host→VRAM decode, TEX0→handle
  resolution, texture binding to draws, `uploadedTexture()` accessor
- `native/src/engine/GifCommandExecutor.{h,cpp}` — create/bind real backend
  texture from a virtual buffer handle (cached)
- `native/tests/gif_command_test.cpp` — `test_image_vram_upload`
- `native/tests/gif_executor_test.cpp` — `test_executor_vram_upload`

## Sources Used
- PS2Tek "GIF Data Formats" and "GS Register List" — IMAGE (host→VRAM) transfer
  and the 0x50-0x53 block-transfer register encodings
- PS2Tek GS registers: BITBLTBUF (SBASE/SBW/SPSM/DBASE/DBW/DPSM), TRXPOS
  (SSX/SSY/DSX/DSY/DIR), TRXREG (RRW/RRH), TRXDIR (XDIR)
- Existing `GifTag` / `GifCommandBuffer` / `GifCommandExecutor` code (pre-existing)

## Evidence
- GS register addresses already ratified in Rev.116:
  `kGsAddrBITBLTBUF=0x50`, `kGsAddrTRXPOS=0x51`, `kGsAddrTRXREG=0x52`,
  `kGsAddrTRXDIR=0x53`
- TRXDIR `XDIR`: `0` = local → local / VRAM write (host→VRAM on GIF IMAGE),
  `1` = VRAM → host. Only `xdir=0` is modeled here.
- IMAGE data is `NLOOP` quadwords written to the VRAM write pointer described by
  BITBLTBUF/TRXPOS/TRXREG (PS2Tek GIF Data Formats).
- `ctest`: 13/13 targets pass, 0 failures.

---

## 1. Transfer-register bitfield layouts

New GS registers modeled in `GifTag` (PS2Tek GS Register List):

| Register | Addr | Fields | Bits |
|----------|------|--------|------|
| `GsBitbltBuf` | 0x50 | SBASE | 0-13 |
| | | SBW | 16-21 |
| | | SPSM | 24-29 |
| | | DBASE | 32-45 |
| | | DBW | 48-53 |
| | | DPSM | 56-61 |
| `GsTrxPos` | 0x51 | SSX | 0-10 |
| | | SSY | 16-26 |
| | | DSX | 32-42 |
| | | DSY | 48-58 |
| | | DIR | 59-60 |
| `GsTrxReg` | 0x52 | RRW | 0-11 |
| | | RRH | 16-27 |
| `GsTrxDir` | 0x53 | XDIR | 0-1 |

These are pure bitfield accessors on a `u64 value`, matching the style of the
existing `GsTex0`/`GsFrame`/`GsZbuf` accessor family.

## 2. GifCommandBuffer transfer state and routing

The buffer now tracks the block-transfer configuration:

```cpp
GsBitbltBuf m_transferBitbltBuf;  // destination buffer base/width/PSM
GsTrxPos    m_transferTrxPos;     // destination x/y offset
GsTrxReg    m_transferTrxReg;     // rectangle width/height
GsTrxDir    m_transferTrxDir;     // transfer direction
```

`dispatchByAddress` now routes 0x50-0x53 to `handleBitbltBuf`/`handleTrxPos`/
`handleTrxReg`/`handleTrxDir`. These arrive via the `A_D` (reg=Eh) PACKED path,
which is how page-2 GS registers are reached (as established in Rev.116).

## 3. IMAGE host→VRAM decode

`processImageLoop` previously just consumed quadwords. Now, for a usable
host→VRAM upload:

1. **`xdir == 0`** (host→VRAM write). `xdir != 0` (VRAM→host, local→local)
   stays unmodeled.
2. **`DPSM == PSMCT32`**. Other destination PSM formats (16/8/4, indexed
   palettes) remain unmodeled (see "Not covered").
3. **`RRW`, `RRH`** bound (`1..512`). Decode `NLOOP` quadwords as 32-bit
   little-endian pixels (`R | G<<8 | B<<16 | A<<24`) into a row-major RGBA8
   plane, sized `RRW × RRH`.
4. Register an `UploadedTexture { handle, tbp0 = DBASE, width, height, format,
   rgba }`. The `DBASE` is used as the TEX0.tbp0 key.

```cpp
// per quadword
u32 px[4]; memcpy(px, qw, 16);
for (pi in 0..3): rgba[idx++] = { px&0xFF, (px>>8)&0xFF, (px>>16)&0xFF, (px>>24)&0xFF };
```

## 4. TEX0 → texture-handle resolution

`currentTexture()` no longer returns `kNullTexture`. It is now:

```cpp
TextureHandle GifCommandBuffer::currentTexture() const {
    if (m_currentTex0.tcc() == 0 || m_uploads.empty()) return kNullTexture;
    for (tex : m_uploads) if (tex.tbp0 == m_currentTex0.tbp0()) return tex.handle;
    return kNullTexture;
}
```

`emitSprite` / `emitGouraudSprite` / triangle / line / point now fill
`cmd.*.texture` from `currentTexture()` instead of `kNullTexture`. The uploaded
pixels are exposed via `uploadedTexture(handle, w, h, format, rgba)` and
`uploadedTextureCount()`.

## 5. Executor: virtual handle → real backend texture

`GifCommandExecutor::execute` now resolves, per draw command, any non-null
virtual texture handle against the buffer before dispatching to the backend:

```cpp
resolveTextureHandles(cmd, buffer);
// DrawSprite / DrawSpriteGouraud / DrawPrimitive / DrawIndexed only
if (handle == kNullTexture) return;
if (cached virtual->real) use it;
else {
    buffer.uploadedTexture(handle, w, h, fmt, rgba);
    backend.createTexture({w, h, fmt, rgba});  // real handle
    backend.bindTexture(real, 0);
    cache & replace;
}
```

This keeps `executeCommand(RenderCmd)` (used by direct-command tests) unchanged;
only the `execute(buffer)` path performs buffer-dependent texture resolution.

---

## Confirmed (byte/instruction-level or deterministic)

- BITBLTBUF/TRXPOS/TRXREG/TRXDIR 0x50-0x53 A_D routing reaches the new handlers.
- A host→VRAM IMAGE upload is decoded to RGBA8 PSMCT32 for the given RRW×RRH
  rectangle and recorded with `tbp0 = DBASE`.
- Filling TEX0 (tcc=1, tbp0=DBASE) then drawing makes `cmd.sprite.texture`
  non-null for that buffer (`test_image_vram_upload`).
- The executor creates exactly one real backend texture and binds it before the
  draw, and draws with that handle (`test_executor_vram_upload`).
- All 13 ctest targets pass.

## Probable

- `DBASE` acting as the TEX0.tbp0 key for uploads at destination offset
  (DSX/DSY = 0) is the common whole-texture upload case; offset uploads
  (DSX/DSY ≠ 0) are not validated.

## Possible

- The same model could later be extended to PSMCT16/PSMT8/PSMT4 + CLUT (TEX0
  `cbp/csa/cpsm`) once real CLUT-upload streams are available.

## Unknown

- Whether real ICO scene-load streams drive host→VRAM IMAGE at these exact
  register settings (no runtime stream capture was used in this revision).

## Not covered / Discarded (for now)

- `xdir != 0` transfers (VRAM→host reads, local→local).
- Destination PSM ≠ PSMCT32 (the 16/8/4 indexed paths).
- PS2 GS **swizzling**: PSMCT32 host uploads are stored linear here; the GS
  texture swizzle of PSMCT16/PSMZ* is out of scope.
- `moveImage` (source→dest) in `GifPacketBridge` still uses the stub path.

## Next minimum test
- Feed a synthetic two-frame stream (upload + textured draw in one buffer, then
  a second buffer) and confirm `uploadedTextureCount()` grows per frame and the
  executor's virtual→real cache does not leak.

## Conservative verdict
- The Rev.115/116 "VRAM destination not modeled" note is closed for the modeled
  path (PSMCT32 host→VRAM). The model is deterministic and reproducible against
  synthetic bytes; binding to real ICO scene-load GIF streams remains the
  not-yet-exercised integration step.
