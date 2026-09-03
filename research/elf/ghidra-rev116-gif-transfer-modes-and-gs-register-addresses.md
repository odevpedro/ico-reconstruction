# Rev.116 — GIF Transfer-mode Handling (REGS/IMAGE) and GS Register-address Corrections

## Date
2026-09-03

## Objective
Close the substring of the render path that was left as "unknown" in Rev.115:
the `GifCommandBuffer` handling of the GIF **REGS** and **IMAGE** transfer
modes, plus the misrouting of `Regs` mode to an empty no-op. Also ground the
GS register-address constants in the ratified PS2Tek register list.

## Scope
- `native/src/engine/GifTag.h` — GS register address constants (corrections)
- `native/src/engine/GifCommandBuffer.{h,cpp}` — REGS/IMAGE parsing, A_D routing,
  transfer-mode-dependent data sizing
- `native/tests/gif_command_test.cpp` — REGS, PACKED-A_D, IMAGE, and register
  constant tests

## Sources Used
- PS2Tek "GIF Data Formats" (israpps.github.io/ps2tek) — authoritative transfer
  mode layout and A_D register descriptor
- PS2Tek "GS Register List" — ratified register addresses
- PCSX2 `Gif_Unit.cpp` `Gif_HandlerAD` — A_D address-at-byte-8 convention
- Existing `GifCommandBuffer` / `GifTag` code (pre-existing)

## Evidence
- PS2Tek GIF Data Formats:
  - **PACKED**: data in units of quadwords; `reg=Eh A+D` = bits 0-63 data,
    bits 64-71 register address, bits 72-127 unused.
  - **REGLIST/REGS**: total data = `NREGS * NLOOP` in units of **doublewords
    (8 bytes)**; a doubleword is sent directly to the register descriptor;
    every quadword holds data for two registers.
  - **IMAGE**: total data = `NLOOP` in units of **quadwords**, written to GS
    HWREG (VRAM texture/data upload).
- PCSX2 `Gif_HandlerAD`: `reg = pMem[8] & 0x7f` (address at byte 8), data in
  bytes 0-7.
- PS2Tek GS Register List addresses (see corrections below).
- `ctest`: 13/13 targets pass, 0 failures.

---

## 1. Transfer-mode data sizing (GifCommandBuffer::parsePacket)

The packet byte count was previously hard-coded to `nloop * 16` for every mode.
This under-sized REGS mode (which needs `nreg * nloop * 8` bytes), causing data
to be truncated and vertices/primitives to be dropped. Now sized per mode:

| Mode | Packet bytes per tag |
|------|----------------------|
| Packed / Unpacked | `nloop * 16` (quadword slots, unchanged pre-existing model) |
| Regs (REGLIST) | `nloop * nreg * 8` (doublewords) |
| Image | `nloop * 16` (quadwords) |

## 2. Routing (GifCommandBuffer::processTag)

Previously `flg == Regs` was misrouted to `processImageLoop` (an empty no-op),
so REGS and IMAGE both silently dropped all data. Now:

```cpp
Packed -> processPackedData
Regs   -> processRegsData   (new)
Image  -> processImageLoop  (now consumes NLOOP quadwords)
```

- `processRegsData` iterates the data as 8-byte doublewords, each routed
  positionally to the register named by the tag's descriptor list (PS2Tek
  REGLIST semantics).
- `processImageLoop` consumes the `NLOOP` quadwords to keep the stream aligned;
  the VRAM destination requires BITBLTBUF/TRXPOS/TRXREG, which are not yet
  modeled (documented limitation).

## 3. A_D register dispatch (handleAD / dispatchByAddress)

For PACKED mode, a `reg=Eh` (A_D) descriptor means the data QWORD carries the
target register address at **byte 8** and the value in bytes 0-7 (PCSX2
`Gif_HandlerAD`, PS2Tek). The previous code routed `GifReg::A_D` to
`handleAlpha` — wrong. Now:

- `handleAD(data)` reads `addr = data[8]` and dispatches by address.
- `dispatchByAddress(addr, data)` maps the ratified register addresses
  (PRIM/RGBAQ/STQ/UV/XYZF2/XYZ2/XYZF3/XYZ3/TEX0/TEX1/FOG/ALPHA/TEST/FRAME/ZBUF)
  to the existing handlers. Unhandled addresses (VRAM/bitblt/transfer
  registers 0x50+) are ignored.
- This is the path by which the page-2 registers (FRAME 0x4C, ZBUF 0x4E,
  ALPHA 0x42, TEST 0x47) are reachable, since a REGS descriptor list can only
  name the low 16 descriptors directly.

## 4. GS register-address corrections (GifTag.h)

The RAFTED PS2Tek GS Register List differs from the pre-existing constants:

| Constant | Prior | Correct | Fix |
|----------|-------|---------|-----|
| `kGsAddrXYZ2` | 0x04 | 0x05 | Fixed (XYZF2=0x04, XYZ2=0x05) |
| `kGsAddrTEX1_1/2` | 0x08/0x09 | 0x14/0x15 | Fixed |
| `kGsAddrTEX2_1/2` | 0x0A/0x0B | 0x16/0x17 | Fixed |
| `kGsAddrCLAMP_1/2` | 0x0C/0x0D | 0x08/0x09 | Fixed |
| `kGsAddrFOG` | 0x0E | 0x0A | Fixed (0x0E is A_D) |
| `kGsAddrTEST_1/2` | 0x45/0x46 (Rev.115) | 0x47/0x48 | Fixed (0x45=DTHE, 0x46=COLCLAMP) |
| `kGsAddrALPHA_1/2` | 0x42/0x43 | 0x42/0x43 | OK |
| `kGsAddrFRAME_1/2` | 0x4C/0x4D | 0x4C/0x4D | OK |
| `kGsAddrZBUF_1/2` | 0x4E/0x4F | 0x4E/0x4F | OK |

Added: `kGsAddrXYZF2` (0x04), `kGsAddrXYZF3` (0x0C), `kGsAddrXYZ3` (0x0D),
`kGsAddrA_D` (0x0E), and the page-2 transfer registers
`kGsAddrBITBLTBUF/TRXPOS/TRXREG/TRXDIR` (0x50-0x53) for future VRAM work.

**Latent inconsistency (not changed):** the pre-existing `GifReg` enum still
defines `TEX1_1 = 0x08`, `TEX1_2 = 0x09`, `CLAMP_1 = 0x0C`, `CLAMP_2 = 0x0D`,
which do not match the ratified register list (TEX1=0x14/0x15, CLAMP=0x08/0x09).
These are used as PACKED-mode wire descriptor values; changing them is deferred
pending a dedicated check of how PACKED descriptor streams are generated in the
reconstructed pipeline.

## 5. Corrected rendering-side effect (Rev.115 bridge)

Because `GifPacketBridge::setGsReg` switches on the `kGsAddrTEST_1/2`
constants, and those were 0x45/0x46 (wrong) in Rev.115, the bridge was mapping
DTHE/COLCLAMP as TEST. This is now corrected to 0x47/0x48, so the bridge's
`SetDepthTest`/`SetAlphaTest` path now fires on the real TEST register.

---

## What is Confirmed
- REGS (REGLIST) mode now parses 8-byte doublewords routed by the descriptor
  list; PACKED-A_D routes by the embedded byte-8 address to page-2 registers.
- IMAGE mode consumes NLOOP quadwords without misparsing.
- The register-address constants and the bridge now agree with the PS2Tek GS
  register list.
- 13/13 ctest targets pass.

## What is Probable
- Page-2 drawing registers (ALPHA/TEST/FRAME/ZBUF) are normally delivered via
  PACKED-mode A_D descriptors (matching PCSX2), since REGS descriptors are
  limited to the low 16 registers.

## What is Possible
- IMAGE mode's VRAM upload could be made concrete once BITBLTBUF/TRXPOS/TRXREG
  are modeled (0x50-0x53), allowing texture raster writes to a destination.

## What is Unknown
- Whether the reconstructed game pipeline emits REGS mode at all (vs PACKED
  with A_D). No runtime capture has yet exercised REGS/IMAGE path parsing.

## What is Discarded
- The original hard-coded `nloop*16` sizing for ALL modes (under-sized REGS).
- The `Regs -> processImageLoop` misrouting.
- The `A_D -> handleAlpha` misroute.

## Next Minimum Test
1. Capture a real GIF stream (from a scene GObj build) and confirm whether it
   emits PACKED-with-A_D, REGS, or IMAGE; validate against this parser.
2. Model BITBLTBUF/TRXPOS/TRXREG + a VRAM write target so IMAGE mode produces a
   real texture upload instead of a no-op consume.

## Conservative Verdict
The GIF transfer-mode handling is now faithful to the ratified PS2Tek spec for
Packed/Regs/Image sizing and A_D routing, and the register-address constants
are corrected. The remaining open item is a real capture to confirm which mode
the reconstructed game path actually produces, and the VRAM destination model
for IMAGE uploads.
