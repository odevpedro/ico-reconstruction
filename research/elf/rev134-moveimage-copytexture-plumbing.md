# Rev.134 — `moveImage` CopyTexture plumbing (bridge → command → executor → backend)

- **Date:** 2026-09-05
- **Objective:** Close the Rev.118 "still uses the stub path" gap for
  `GifPacketBridge::moveImage` by modeling the full copy command boundary
  (`RenderCommand::CopyTexture`) with source/destination geometry, and
  forwarding it from the GIF executor to the render backend.
- **Scope:** `native-port` only. No `.s`, no ground-truth binary touched.

## Sources used

- `research/elf/ghidra-rev118-vram-texture-upload-model.md` (stub: line 177)
- `research/elf/ghidra-rev116-gif-transfer-modes-and-gs-register-addresses.md`
- `research/elf/ghidra-rev115-gifpacket-bridge-and-slot-binding.md` (Rev.115
  already documented: `moveImage` emits `CopyTexture` only when `w,h > 0`)

## Evidence used

- `native/src/engine/GifPacket.cpp:336` — `moveImage` only pushed a bare
  `CopyTexture` command; all six geometry parameters were commented out.
- `native/src/engine/GifCommandExecutor.cpp:187` — the `CopyTexture` case was
  an empty `break` (command reached the executor and was dropped).
- `RenderCmd` union had no copy substruct; `RenderBackend` had no copy method.
- `gif_executor_test.cpp:269-272` — asserted CopyTexture was a no-op
  (`m_drawPrimCalls` unchanged).

## What was done

| Layer | Change |
|-------|--------|
| `RenderBackend.h` | New pure virtual `copyTexture(srcX, srcY, dstX, dstY, w, h)`; new `RenderCmd.copy` substruct holding all six floats |
| `GifPacketBridge::moveImage` | Records full source/dest/size geometry into the `CopyTexture` command; guards unchanged (`!m_packetOpen`, `w<=0`, `h<=0` drop) |
| `GifCommandExecutor` | `CopyTexture` now calls `m_backend.copyTexture(...)` with the recorded geometry |
| `RenderStubBackend` (`RenderBackend.cpp`) | Documented no-op override |
| `OpenGLBackend` | Documented no-op override |
| Test backends (`gif_packet_test`, `gif_executor_test`) | Record `m_copyCalls` + last copy geometry |

## Confirmed

- The `CopyTexture` command now carries: `srcX, srcY, dstX, dstY, w, h`
  (asserted on the buffered command in `gif_packet_test::test_geometry_mapping`).
- The executor forwards the copy to the backend with identical geometry;
  `m_drawPrimCalls` is unaffected (copy is not a draw).
- Degenerate/closed-packet guards are unchanged and covered by the new
  `test_move_image_guards`: packet closed, `w==0`, `h<0` all emit nothing;
  a valid move emits exactly one command and `flush()` forwards it with the
  recorded geometry to the backend.
- Full CTest suite: **20/20 pass** (no new test targets; two existing targets
  extended).

## Probable

- The original `GifPacket.o` `moveImage` corresponds to a GS `TRXDIR == 2`
  (VRAM→VRAM) move, where the six-float API works against a preconfigured
  buffer setup. This is the same interpretation Rev.115/116 used; it is not
  re-proven here.

## Possible

- The same `copyTexture` boundary can later serve VRAM-to-VRAM streams emitted
  by the GIF block-transfer model (`handleTrxDir` mode 2), which Rev.118 still
  discards. The executor path would be identical.

## Unknown

- Which source/destination VRAM buffers the game actually uses when calling
  `moveImage` (no runtime stream capture for this function exists yet).
- Whether `moveImage` is hot in any world_state (no PCSX2 probe targets it).

## Discarded

- Implementing a real GL blit in `OpenGLBackend::copyTexture` now: the
  simplified signature carries no `SBP/SBW/SPSM`, so a correct source-buffer
  binding cannot be established without runtime evidence. A speculative blit
  would be unverifiable (violates the confirm-before-reconstruct rule).

## Next minimum test

Runtime session: add a probe/breakpoint on the `moveImage` caller path and
capture `srcX/srcY/dstX/dstY/w/h` plus the active frame-buffer registers to
identify the VRAM buffers; only then implement the real blit in the GL backend.

## Conservative verdict

The command boundary for VRAM moves is now modeled and verified end-to-end
(command → executor → backend) with full geometry. Backend execution remains a
documented no-op until runtime evidence identifies the source/dest VRAM
buffers. Rev.118's "still uses the stub path" statement is superseded for the
*modeling* layer; the actual pixel copy is still not exercised.