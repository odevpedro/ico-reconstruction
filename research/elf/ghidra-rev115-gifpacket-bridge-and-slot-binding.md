# Rev.115 — GifPacketBridge → GifCommandExecutor → RenderBackend, and WorldState→Slot Binding

## Date
2026-09-03

## Objective
Close the two gaps in the native rendering/runtime path:

1. **Render path**: bridge the PS2 `GifPacket.*` primitives (GS register stream
   and geometry emit) into the `GifCommandExecutor` → `RenderBackend` command
   model, so a reconstructed GIF/DL submission can be executed against the
   native backend.
2. **Dispatch path**: replace the pre-existing (Rev.114-era) world_state→slot
   association in `WorldStateSlotMap.h` with the 20-slot BSS table newly derived
   from the 2,807,253-event runtime capture `ico-rr-20260825-152452.jsonl`.

## Scope
- `native/src/engine/GifPacket.{h,cpp}` — bridge implementation
- `native/src/engine/GifCommandExecutor.cpp` — new command dispatch
- `native/src/engine/RenderBackend.h` — new command types + pure-virtual state methods
- `native/src/engine/RenderBackend.cpp` / `OpenGLBackend.{h,cpp}` — backend impl
- `native/src/engine/GifTag.h/cpp` — GS register address constants + bit-layout fixes
- `native/src/engine/WorldStateSlotMap.h` — 35 bindings / 20 slots
- Tests: `gif_packet_test.cpp`, `gif_executor_test.cpp`, `world_state_slot_test.cpp`
- All 13 test targets green (see Evidence).

## Sources Used
- `.local/pcsx2-logs/ico-rr-20260825-152452.jsonl` (2,807,253 events; 36 world_states)
- `rev114-scene-gobj-static-data-bridge.md` (prior slot association)
- Existing `GifTag.h/cpp` (GS register bit layout)
- Existing `RenderBackend.h` (render command model, Rev.110)

## Evidence
- `ctest` result: 13/13 tests passed (0 failures).
- Byte-level / layout verification via standalone reproductions of the
  `GsFrame` / `GsAlpha` / `GsZbuf` getter-setter round trips (see GifTag fixes).

---

## 1. Render path — GifPacketBridge

### 1.1 Constructor injection
`GifPacketBridge` now takes a `RenderBackend&` and constructs an internal
`GifCommandExecutor` against it. `GifCommandExecutor` (from the earlier GIF
command-buffer work) walks `GifCommandBuffer` commands and calls the matching
`RenderBackend` virtual.

### 1.2 Expanded `setGsReg`
The bridge now decodes not only `TEX0` but the page-2 GS registers:

| GS reg | addr | Command emitted |
|--------|------|-----------------|
| TEX0_1/2 | 0x00/0x01 | `SetTexture` |
| ALPHA_1/2 | 0x42/0x43 | `SetAlpha` (aba..abd, afix) |
| TEST_1/2 | 0x45/0x46 | `SetDepthTest` (ztest→ztst, zte→write) + `SetAlphaTest` (atst, aref) |
| FRAME_1/2 | 0x4C/0x4D | `SetFramebuffer` (fbp, fbw, psm) |
| ZBUF_1/2 | 0x4E/0x4F | `SetZBuffer` (zbp, psm, zmsk) |

### 1.3 Geometry / environment mapping
- `setDrawEnvironment` now emits `SetViewport` + `SetFramebuffer` (was
  viewport only).
- `moveImage` emits `CopyTexture` only when `w,h > 0` (guards degenerate moves).
- `setZWrite(zte, ztst)` maps onto `SetDepthTest` (verify GE testing below).

### 1.4 `flush()` and `backend()`
- `flush()` executes the buffered commands via the executor, then resets the
  buffer. No-op when no commands are open.
- `backend()` accessor exposes the injected `RenderBackend&`.

## 2. RenderBackend interface extension

Three new pure-virtual state methods were added (all backends must implement):

```cpp
virtual void setFramebuffer(u32 fbp, u32 fbw, u32 psm) = 0;
virtual void setZBuffer(u32 zbp, u32 psm, bool zmsk) = 0;
virtual void setAlpha(u32 aba, u32 abb, u32 abc, u32 abd, u32 afix) = 0;
```

New `RenderCommand` values `SetFramebuffer`, `SetZBuffer`, `SetAlpha` and
matching `RenderCmd` union members were added.

Implementations:
- `OpenGLBackend`: fbw/fbp are a no-op binding note; zbuf maps `zmsk`→
  `depthWrite`; alpha selects blend mode / fixed alpha.
- `RenderStubBackend` (RenderBackend.cpp) and `TestBackend`
  (gif_executor_test.cpp): no-op trackers.

## 3. GifTag bit-layout corrections (verified vs GS spec)

Standalone reproductions exposed two incorrect getters/setters in the
pre-existing `GifTag.cpp`:

| Field | Prior (buggy) | Correct (GS spec) | Status |
|-------|---------------|-------------------|--------|
| `GsFrame::fbp` | set/read bits 36-44 | set/read bits 0-8 | Fixed |
| `GsFrame::fbw` | bits 32-37 | bits 32-37 (unchanged) | OK |
| `GsFrame::psm` | bits 24-29 | bits 24-29 (unchanged) | OK |
| `GsAlpha::afix` | getter read bits 0-7 | getter reads bits 8-15 | Fixed getter |
| `GsZbuf::zbp` | bits 0-8 | bits 0-8 (unchanged) | OK |

The prior `GsFrame::fbp` at bits 36-44 overlapped `fbw` (bits 32-37) at bits
36-37, making any non-trivial fbp/fbw pair fail to round-trip through
getter+setter. Relocating fbp to bits 0-8 (matching the real PS2 GS FRAME
register: FB at 0-8, PSM at 24-29, FBW at 32-37) removes the collision.
`GsAlpha::afix` setter already wrote bits 8-15; only the getter was wrong.

## 4. Dispatch path — WorldState→Slot binding (Rev.115 replacement)

The prior `WorldStateSlotMap.h` association was superseded by a 20-slot BSS
table derived from the runtime capture:

| Slot | BSS addr | Slot | BSS addr |
|------|----------|------|----------|
| 0x18 | 0x00677DD8 | 0x23 | 0x00679A08 |
| 0x1A | 0x006782F8 | 0x24 | 0x00679C98 |
| 0x1C | 0x00678818 | 0x25 | 0x00679F28 |
| 0x1D | 0x00678AA8 | 0x26 | 0x0067A1B8 |
| 0x1E | 0x00678D38 | 0x29 | 0x0067A968 |
| 0x1F | 0x00678FC8 | 0x2E | 0x0067B638 |
| 0x20 | 0x00679258 | 0x33 | 0x0067C308 |
| 0x21 | 0x006794E8 | 0x34 | 0x0067C598 |
| 0x22 | 0x00679778 | 0x36 | 0x0067CAB8 |
| 0x40 | 0x0067E458 | 0x44 | 0x0067EE98 |

- **35 bindings** across 20 share groups; `kWorldStateSlotBindingCount == 35`
  (not 36). `kWorldStateCount = 35` (miscount correction).
- World state `0x39` (18 events, no `ios_om_main` activity) is excluded.
- Slots without BSS addresses in this capture: 0x19, 0x1B, 0x27, 0x28,
  0x2A-0x2D, 0x2F-0x32, 0x35, 0x37-0x3F, 0x41-0x43.

---

## What is Confirmed
- 13/13 native tests pass after the bridge and slot-binding changes.
- `GifPacketBridge` now requires a `RenderBackend&`; all open regs
  (TEX0/ALPHA/TEST/FRAME/ZBUF) map to typed render commands.
- 20 BSS slot addresses and 35 bindings verified against the 2.8M-row capture.

## What is Probable
- The corrected `GsFrame`/`GsAlpha` bit layouts match the real PS2 GS registers
  (FRAME: FB 0-8, PSM 24-29, FBW 32-37; ALPHA: AFIX 8-15), enabling
  non-overlapping round trips.

## What is Possible
- `OpenGLBackend::setFramebuffer` semantics (fbp/fbw no-op in GL) may need
  refinement once actual framebuffer addressable regions are used.

## What is Unknown
- Whether the GIF `Regs`/`Image` mode paths in `GifCommandBuffer` correctly
  route `A_D` and image loops — not exercised here (the bridge feeds typed
  commands directly).

## What is Discarded
- The pre-existing world_state→slot association in `WorldStateSlotMap.h` (in
  favor of the derived 20-slot table).

## Next Minimum Test
1. Route a real reconstructed `GifPacket` sequence (from a scene GObj build)
   through `flush()` and confirm the `RenderBackend` receives the expected
   state + draw calls.
2. Verify `setZWrite` → `SetDepthTest` mapping against the SCCE/GS Z-test
   semantics.
3. Confirm `GifCommandBuffer` `Regs`/`Image` mode handling (GIF `flg`).

## Conservative Verdict
The render bridge from `GifPacket.*` primitives to the native `RenderBackend`
is now functional and testable. Two real defects in the pre-existing GS
register bit layout were found and corrected using standalone getter/setter
round-trip reproductions. The dispatch slot table is now grounded in the
largest captured session (2.8M events). Remaining GIF mode handling is the next
integration surface.
