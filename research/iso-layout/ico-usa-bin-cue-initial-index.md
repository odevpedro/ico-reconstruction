# ICO USA BIN/CUE Initial Index

> Evidence level: Confirmed
> Source: local user-owned BIN/CUE metadata-only scan
> Date: 2026-05-12

## Input

Local path used for analysis:

```text
/home/peter/Downloads/Ico (USA)/
```

The game image is stored as BIN/CUE, not as a plain ISO file.

CUE metadata:

```text
FILE "Ico (USA).bin" BINARY
TRACK 01 MODE2/2352
INDEX 01 00:00:00
```

## Metadata Verification

Generated with:

```bash
python3 tools/verify-local-copy/verify_local_copy.py '/home/peter/Downloads/Ico (USA)'
```

Observed files:

| File | Size | SHA-256 |
|------|------|---------|
| `Ico (USA).bin` | 638,640,912 bytes | `71df3bf8e3939453c6a5182e45b68948ce118a89fe90bdfaa2893fd6b7757573` |
| `Ico (USA).cue` | 75 bytes | `148517249f925becadaba49889f24088850a0ea9875ab78316cba0c5baa5359b` |

## Disc Index

Generated with:

```bash
python3 tools/iso-index/iso_index.py '/home/peter/Downloads/Ico (USA)/Ico (USA).bin' --cue '/home/peter/Downloads/Ico (USA)/Ico (USA).cue'
```

Detected sector layout:

| Field | Value |
|-------|-------|
| Layout | `raw-2352-data-offset-24` |
| Sector size | 2352 |
| Data offset | 24 |
| Payload size | 2048 |
| System identifier | `PLAYSTATION` |
| Volume space size | 271,531 sectors |
| Root extent LBA | 22 |
| Root size | 2,048 bytes |

Summary:

| Metric | Value |
|--------|-------|
| Directories | 1 |
| Files | 11 |
| Total indexed file bytes | 555,727,712 |

## Root Entries

| Path | Kind | LBA | Size |
|------|------|-----|------|
| `DFDATAS` | directory | 23 | 2,048 |
| `DFDATAS/DATA.DF` | file | 2,898 | 539,367,424 |
| `SCUS_971.13` | file | 25 | 5,481,608 |
| `LIBSD.IRX` | file | 2,874 | 26,285 |
| `MCMAN.IRX` | file | 2,827 | 87,789 |
| `MCSERV.IRX` | file | 2,870 | 6,777 |
| `PADMAN.IRX` | file | 2,805 | 43,861 |
| `IOPRP224.IMG` | file | 2,702 | 201,065 |
| `SIO2MAN.IRX` | file | 2,801 | 6,161 |
| `SNDN2DRV.IRX` | file | 2,887 | 20,925 |
| `SYSTEM.CNF` | file | 24 | 57 |
| `LDUMMY.` | file | 266,261 | 10,485,760 |

## Initial Interpretation

- `SCUS_971.13` is the main executable candidate.
- `DFDATAS/DATA.DF` is the primary large data/archive candidate.
- The project should treat this target as a CD-ROM BIN/CUE workflow, not a DVD ISO workflow.
- The next technical target should be metadata-only inspection of `SYSTEM.CNF` and executable metadata, without committing extracted contents.

## Executable Header Metadata

The indexer identified `SCUS_971.13` as an ELF32 little-endian executable.

| Field | Value |
|-------|-------|
| Entry point | `0x00100008` |
| Machine | `8` |
| Type | `2` |
| Flags | `0x20924001` |
| Program header offset | 52 |
| Program header count | 1 |
| Section header offset | 5,480,528 |
| Section header count | 27 |
| Section name string table index | 26 |

## Remaining Uncertainty

- `DATA.DF` internal structure is unknown.
- Whether there is an external or embedded file table is unknown.
- ELF section metadata for `SCUS_971.13` is documented in `research/elf/ico-usa-scus-97113-elf-metadata.md`.
- Function boundaries and symbolic names for `SCUS_971.13` have not been recovered.
- No runtime or emulator validation has been performed yet.
