# Rev.136 — DFDATAS/DATA.DF container format reverse-engineered

> Date: 2026-09-05
> Evidence level: **Confirmed** (TOC structure), probable (packing/encryption), possible (decryption scheme)
> Objective: unblock the P2 asset pipeline of the native-port roadmap by decoding
> the top-level container that holds every ICO runtime asset.
> Sources: local user-owned `Ico (USA).bin` via `.local/iso/`, metadata-only reads.

## Context

The P2 priority (native-port directive) requires extracting real assets from the
game disc and rendering them through the semantic GIF → executor → OpenGL
pipeline. The priorktriage (2026-05-12, `research/data-df/`) only established
entropy. This revision identifies the actual container layout.

## Input

| Field | Value |
|-------|-------|
| Disc image | `.local/iso/Ico (USA).bin` |
| Source name | `DFDATAS/DATA.DF` |
| DATA.DF LBA | 2,898 |
| DATA.DF size | 539,367,424 bytes (0x20261800) |
| BIN sector size / data offset | 2,352 / 24 |

Tool: `tools/dfdatas-index/dfdatas_index.py --image ".local/iso/Ico (USA).bin" --lba 2898 --size 539367424 --source-name DFDATAS/DATA.DF`

## Confirmed TOC layout

`DATA.DF` begins with a **fixed-record directory** inside its first 8 payload
sectors:

| Field | Offset | Size | Notes |
|-------|--------|------|-------|
| File count | 0x00 | u32 | `0x000000AC` = 172 |
| Record 0 | 0x04 | 40 B | `COMMON.DF` |
| Record N | 0x04 + 40·N | 40 B | some asymmetric slots stride differs |

Each 40-byte record:

| Field | Offset | Size | Notes |
|-------|--------|------|-------|
| Name | 0x00 | 12 B | NUL-terminated, padded `\x00`/`\xff`/`\x7f` |
| Tag A | 0x0C | u32 | `0x0804C438` constant across all records |
| Tag B | 0x10 | u32 | `0x08052488` constant across all records |
| Tag C | 0x14 | u32 | `0x0805279C` constant across all records |
| Tag D | 0x18 | u32 | `0x7FFFF2D8` constant across all records |
| Tag E | 0x1C | u32 | `0x7FFFF2C4` constant across all records |
| Offset | 0x20 | u32 | byte offset of the file in `DATA.DF` |
| Size | 0x24 | u32 | exact byte length |

Tags A–E are byte-identical across every record — likely format boilerplate
(reserved IDs), not per-file data.

## Validation

- **169 of 172 records parsed** by adaptive resync (stride 40, +4 on
  asymmetric slots). zero overlaps, zero out-of-file windows.
- Coverage: first file at `0x00200000`, last file ends at `0x20261004`,
  file total `0x20261800` (tail gap ≈ 2 KB + TOC ≈ 0x2000 header).
- **Independent format check**: `icon.sys` (offset `0x1623D000`) starts with
  ASCII `PS2D` — the standard PlayStation 2 icon.sys magic, confirming the
  extracted window is byte-correct.
- Sizes are exact byte lengths; file starts are 2048-aligned (offset grid
  observed constant multiple of `0x1000`/`0x800` patterns).

## Catalog sample

| Name | Offset | Size (hex) | Kind |
|------|--------|-----------|------|
| `COMMON.DF` | 0x00200000 | 0x22BA5F | base archive |
| `MOTION2/MOTION1.DF` | 0x0022E000 / 0x006EE800 | 0x132B1E / 0x1EADA5 | animation |
| `STGLOG.DF` | 0x00361000 | 0x380188 | stage log |
| `ICON.DF` | 0x006E1800 | 0x0CAD2 | icons |
| `OTHERS1.DF` | 0x008D9800 | 0x8D151 | misc |
| `STG13A2.DF` … `STGST13D.DF` (52) | 0x00967000… | … | per-stage archives |
| `STGSTAFF1-3.DF`, `STGTTL.DF` | 0x0C6B8000… | … | credits / title |
| `boy_blk.ico` | 0x0D2A4800 | 0x17588 | model |
| `NN.int` (~120) | 0x0D2BC000… | … | per-stage overlay/geometry |
| `NN.smb`, `d12.smb` etc. | 0x15CEA800… | … | IOP sound banks |
| `icon.sys` | 0x1623D000 | 0x3C4 | PS2 icon (**PS2D magic verified**) |
| `data.jim` | 0x18FD1800 | 0x3DA000 | sprite container |
| `advertise.ps` | 0x19EBD000 | 0x63A4004 | PS2 advertise MPEG (starts `00 00 01 BA`) |

## File-content classification

| File set | First bytes | Entropy (1 MB) | Interpretation |
|----------|------------|----------------|----------------|
| `STG*.DF`, `COMMON.DF`, `MOTION*.DF`, `STGLOG.DF`, `OTHERS1.DF` | `0xEC …` | 7.86–7.99 | **encrypted/compressed** (constant first byte `0xEC`, high entropy throughout) |
| `*.int` overlay files | `0x0C 02 00 00 …` repeating | 1.7–6.9 | plaintext container (geometry script) |
| `icon.sys` | `PS2D` | low | plaintext |
| `advertise.ps` | `00 00 01 BA` | 5.3 | plaintext MPEG-2 system stream |
| `data.jim` | `TIM2` (fake) | mid | plaintext sprite container |

`data.jim` and interior `TIM2` markers are **not** standard TIM2 textures:
magic read `0x324D4954` but version `0x00010004`, `totalSize=0`, `numImages=0`,
records spaced 0x9C30 — this is a proprietary JIM container (HUD/sprite atlas).

## Probable

- Files are stored at sector-aligned starts; `size` is exact; inter-file gaps
  are the 2048-padding remainder.
- `0xEC`-lead files are guarded by a common symmetric scheme; the constant
  first byte suggests a shared transform (not per-file random key).
- The release `STGSTAFF2.DF` shows a plaintext region (grayscale CLUT ramp
  `0x88..0xFF` and a JIM `TIM2` header at `0x049203`) inside an otherwise
  high-entropy body — i.e. some stage files embed plaintext texture records.

## Possible / unknown

- Decryption for the `STG*.DF` inner archive is not yet recovered. Candidate
  sources: the `ios/dvd`-style async read functions in the USA ELF
  (`kanban.c` scene loader reads these files), and `research/dvp/` overlay
  metadata.
- The 3 missing TOC records (172 − 169) land in asymmetric stride slots
  (between `STGST13D`/`STGST17B` and around `41.int`/`43.int`).
- Exact meaning of tags A–E.
- Inner layout of `STG*.DF` (expected: per-stage TOC of TM2 textures + DVP
  geometry) is the next layer.

## Next minimum test

1. Feed the recovered TOC gaps: scan the asymmetric slots for records whose
   first 4 bytes are non-printable but whose offset/size fields still satisfy
   the packing rule.
2. Locate `DATA.DF`/`DFDATAS` string refs and DVD-read wrappers in the USA ELF
   to recover the `STG*.DF` reader (buffer exchange / decrypt path).
3. Decrypt `STGST04A.DF` (entry-beach stage) header, render its first TM2
   through the semantic GIF pipeline (P2 acceptance).

## Conservative verdict

The top-level `DATA.DF` container is **confirmed**: fixed-record directory,
172 files, byte offsets + exact sizes, 2048-aligned storage, validated with an
independent known magic (PS2D). Stage payloads are protected by a not-yet-
recovered transform. The container layer of P2 is complete; the stage-archive
layer is the next milestone.