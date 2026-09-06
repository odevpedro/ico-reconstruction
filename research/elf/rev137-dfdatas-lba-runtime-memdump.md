# Rev.137 — DATA.DF lane: ISO/LBA confirmation, static exhausted, runtime memdump probe

- **Date**: 2026-09-05
- **Objective**: Close the DATA.DF encoding question for P2 (first real TM2 render) by
  confirming the container geometry and moving the lane to runtime evidence.
- **Scope**: ISO9660 re-derivation, payload format ruling-out, main-ELF reference audit,
  instrumented PCSX2 memdump probe, native-port commit-history rewrite + AGENTS.md policy.

## Sources used

- `Ico (USA).bin` ISO9660 PVD (sector 16) + root directory records
- `tools/dfdatas-index/dfdatas_index.py` catalog (`/tmp/dfdatas-usa.json`, 169 records)
- USA ELF `.local/extracted/SCUS_971.13.elf` (sections, LUI/gp-relative reference scans)
- `/tmp/df-read2` (majo33) + `astrange.ithinksw.net/ico/df-read.c` + astrange page (libarc/TRFILE)
- Fork source `.local/pcsx2-ico-logpoints-fork/pcsx2/x86/ix86-32/iR5900.cpp`

## Evidence used / findings

### ISO9660 re-derivation (LBA confirmed)

| Item | Value |
|------|-------|
| PVD root dir LBA (LE32 @ +158) | 22 (0x16) |
| Root dir `DATA.DF;1` record | LBA 2898 (0xB52), 539,367,424 B |
| Root dir `DFDATAS` subdir | LBA 23, 1 sector (points to same file; path is virtual) |
| byte base | `2898*2352+24` = 6,816,120 |
| ELF map sanity | `SCUS_971.13;1` LBA 25 / 5,481,608 B (matches `.local/extracted` ELF) |

→ The offsets/sizes used since Rev.136 are byte-accurate. `off[i+1]−off[i]−size[i]`
residuals = 1441/1250/1656 B → byte-granular offsets with ~1.2–1.7 KB inter-file slack.

### Payload ruling-out (confirmed negatives)

- **No `0xEC` first-byte** in any `.DF` (histogram: zero 0xEC); astrange's "buf[0]==0xEC →
  inflate" precondition does not hold on this copy.
- **No deflate** (raw −15 or zlib 15) at any plausible offset; sliding scan of STGST04A
  first 512 KB → zero hits; COMMON.DF single blob @0x77FC0 (21,624 B) is isolated;
  8,884 byte-0xEC positions in COMMON.DF → zero deflate hits.
- **No TIM2** anywhere (data.jim/STG*/COMMON/boy_blk.ico).
- **No repeating XOR keystream**: IC/autocorrelation sweep shifts 2–512 → no periodicity.
- **`.int` overlay block is high-signal**: `77_2/79/80/81/82/83.int` + `85.int` start
  `0xFFFFFFFF…`; `STGST24B/18a.int` = `FFFFFF, 38…, 524288, 524288`; ballooned
  `39_6` (19.9 MB)/`40` (10 MB). Not plausible compressed/ELF content → replaced/
  corrupted/encrypted region hypothesis (needs runtime/overlay evidence to decide).
- `icon.sys`/`02_1*.int`/`84/55.int` head `0x20C`(=524) — readable residues only.

### Main-ELF reference audit (reader not in main ELF)

- No JALs to PAL libarc VAs (TRFILE addresses are PAL-only; 0x138b80/0x139048 disassemble
  as garbage in USA `.text`).
- No absolute `lui` (whole ELF file) into string region; **no gp-relative (`$28`) loads**
  into 0x5569f0–0x556c40 / 0x6127d0–0x612810 / 0x559b00–0x559bf0 / 0x55a600–0x55a650
  (0 hits across all instruction encodings with rs==28 and memory opcodes).
- `.rodata` strings (`DFDATAS/DATA.DF`@0x556a20, `DFDATAS/COMMON.DF`@0x6127e8,
  `DFDATAS/%s`@0x556a10, `ios/inflate.c`@0x556b00, etc.) are unreferenced from main code →
  the reader lives in a DVP overlay loaded into main address space.

→ Static lane on the disc side is **exhausted**. Remaining evidence must come from runtime
  (decompressed buffers) or overlay disassembly.

## Runtime instrumentation added (validated fork)

File: `pcsx2/x86/ix86-32/iR5900.cpp` (rebuilt `build-runtime`; `pcsx2-qt` relinked 3/3).

- New `IcoRuntimeMemDumpWorldState()`: on first 3 `world_state_load` (0x1af948) hits,
  writes a **full 32 MB EE RAM binary snapshot** to
  `$ICO_PCSX2_DUMP_DIR/world_state_01..03.bin` (8592 pages × 4 KiB via `memRead32`).
  Disabled unless `ICO_PCSX2_DUMP_DIR` is set. Forward declaration + call inserted in
  `IcoRuntimeProbeMaybeLog`.
- Verification: `strings` shows `ICO_PCSX2_DUMP_DIR` + `world_state memdump`; preflight
  `run-ico-pcsx2-logpoints.sh --check` = READY. `halfword_second_caller_entry` still present.

## Commit history rewrite (native-port)

Rewrote the two thin pushed subjects with detailed bodies (Rev.132 style) and corrected the
revoked claim: **`icon.sys` does NOT start with `PS2D`** (starts `0c 02 00 00` repeating);
`PS2D` appears only inside `.int` payloads. `git push --force-with-lease` applied with user
consent. New history:

```
e2868b3 docs: commit message discipline documented in AGENTS.md (Rev.132-style detailed bodies)
4a52bd7 core: Rev.136 — DATA.DF container TOC decoded ... (body; PS2D claim corrected)
7499ce2 core: Rev.134-135 — moveImage/CopyTexture ... (body added)
```

AGENTS.md gained a mandatory "Commit message discipline" section (multi-line bodies, root
causes, artifact names, verification evidence; no silent amend — follow-up commit or consent).

## Confirmed / probable / possible / unknown

- **Confirmed**: LBA 2898 + 539,367,424 B += byte base 6,816,120; TOC = 40 B records
  (name[32]+off+size); inter-file slack ~1.2–1.7 KB; no deflate/TIM2/0xEC/keystream in
  payload; reader strings unreferenced in main ELF (abs + gp); PAL TRFILE VAs invalid in USA.
- **Probable**: reader + decompressor live in a DVP overlay; overlay payloads stored with a
  replacement/encryption transform (all-FF `.int` block).
- **Possible**: stage payloads are transformed by an export/`BMIR`-style protection distinct
  from libarc deflate.
- **Unknown**: overlay load path/VA; decode transform; decompressed buffer location (target
  of the memdump probe).

## Next minimum test

Launch runtime session with `ICO_PCSX2_DUMP_DIR` armed, play through ≥3 room transitions,
then scan `world_state_0*.bin` for TIM2/PS2D/ELF/heap signatures and diff transitions to
locate the decompressed STG buffer.

## Conservative verdict

The disc-side format and alignment are now confirmed to byte level; the data transform is
not recoverable by static means available and requires runtime memory evidence (prepared)
or overlay disassembly.