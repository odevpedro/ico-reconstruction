# ICO USA SCUS_971.13 DATA.DF and DVP References

> Evidence level: Confirmed
> Source: local user-owned BIN/CUE metadata-only scan
> Date: 2026-05-12

## Purpose

This note records exact string and 32-bit constant reference checks inside `SCUS_971.13`.

The goal was to test whether the executable directly references `DATA.DF`, `DFDATAS`, DVP metadata, or DVP numeric tokens observed in previous steps.

## Input

| Field | Value |
|-------|-------|
| Disc image | `/home/peter/Downloads/Ico (USA)/Ico (USA).bin` |
| Source name | `SCUS_971.13` |
| LBA | 25 |
| Size | 5,481,608 bytes |
| Sector size | 2,352 |
| Data offset | 24 |

## Command

```bash
python3 tools/exe-ref-index/exe_ref_index.py \
  --image '/home/peter/Downloads/Ico (USA)/Ico (USA).bin' \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13 \
  --query DATA.DF \
  --query DFDATAS \
  --query SCUS_971.13 \
  --query SYSTEM.CNF \
  --query DVP \
  --constant 539367424 \
  --constant 2898 \
  --constant 1887731 \
  --constant 15101843 \
  --constant 30216547 \
  --constant 30224099 \
  --constant 30224243 \
  --constant 483563468
```

## String Query Results

| Query | Count | Sections |
|-------|-------|----------|
| `DATA.DF` | 1 | `.rodata` |
| `DFDATAS` | 4 | `.rodata`, `.sdata` |
| `SCUS_971.13` | 1 | `.rodata` |
| `SYSTEM.CNF` | 0 | none |
| `DVP` | 26 | `.DVP.ovlystrtab`, `.shstrtab` |

Notable exact locations:

| Query | File Offset | Virtual Address | Section |
|-------|-------------|-----------------|---------|
| `DATA.DF` | `0x00457a28` | `0x00556a28` | `.rodata` |
| `DFDATAS` | `0x00457a10` | `0x00556a10` | `.rodata` |
| `DFDATAS` | `0x00457a20` | `0x00556a20` | `.rodata` |
| `DFDATAS` | `0x005137e8` | `0x006127e8` | `.rodata` |
| `DFDATAS` | `0x00534b68` | `0x00633b68` | `.sdata` |
| `SCUS_971.13` | `0x004577f0` | `0x005567f0` | `.rodata` |

## Constant Query Results

The following exact 32-bit values were searched in little-endian and big-endian form:

| Value | Hex | Matches |
|-------|-----|---------|
| 539,367,424 | `0x20261800` | 0 |
| 2,898 | `0x00000b52` | 0 |
| 1,887,731 | `0x001ccdf3` | 0 |
| 15,101,843 | `0x00e66f93` | 0 |
| 30,216,547 | `0x01cd1163` | 0 |
| 30,224,099 | `0x01cd2ee3` | 0 |
| 30,224,243 | `0x01cd2f73` | 0 |
| 483,563,468 | `0x1cd297cc` | 0 |

## Initial Interpretation

- The executable directly contains the `DATA.DF` and `DFDATAS` strings.
- `DATA.DF` and nearby `DFDATAS` references in `.rodata` are strong seeds for later disassembly or cross-reference work.
- The tested DVP numeric tokens do not appear as direct 32-bit constants in the executable under this exact-match scan.
- `DVP` occurrences are currently confined to DVP metadata/string-table areas, not confirmed runtime code references.
- `SYSTEM.CNF` was not found inside the executable, which is expected because it is a boot configuration file rather than necessarily a runtime string.

## Remaining Uncertainty

- The executable may construct paths or numeric values dynamically.
- MIPS code may store constants through split immediates rather than direct 32-bit values.
- Exact string presence does not identify the functions that use those strings.
- No disassembly or control-flow analysis has been performed yet.

## Next Step

Add a metadata-only MIPS immediate scanner that can find `lui`/`ori` or similar split-immediate patterns for known virtual addresses and constants, starting with the virtual addresses of `DATA.DF` and `DFDATAS` strings.
