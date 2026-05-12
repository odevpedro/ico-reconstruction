# ICO USA SCUS_971.13 MIPS Split-Immediate Patterns

> Evidence level: Confirmed
> Source: metadata-only MIPS immediate pattern scan
> Date: 2026-05-12

## Purpose

This note records the discovery of MIPS split-immediate patterns (`lui` + `addiu`) that construct 32-bit virtual addresses pointing to the known DATA.DF and DFDATAS string locations in `.rodata`.

## Input

| Field | Value |
|-------|-------|
| Disc image | `/home/peter/Downloads/Ico (USA)/Ico (USA).bin` |
| Source name | `SCUS_971.13` |
| LBA | 25 |
| Size | 5,481,608 bytes |

## Command

```bash
python3 tools/mips-immediate-scanner/mips_immediate_scanner.py \
  --image '/home/peter/Downloads/Ico (USA)/Ico (USA).bin' \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13 \
  --target 0x00556a28 \
  --target 0x00556a10 \
  --target 0x00556a20 \
  --target 0x006127e8 \
  --target 0x00633b68
```

## Results

### Pattern Matches Summary

| Target Address | Virtual Address | Matches Found |
|----------------|-----------------|---------------|
| `0x00556a10` (DFDATAS) | 3 | lui $rt5, 0x0055 + addiu pattern |
| `0x00556a20` (DFDATAS) | 1 | lui $rt2, 0x0055 + addiu pattern |
| `0x006127e8` | 1 | lui $rt4, 0x0061 + addiu pattern |
| `0x00633b68` | 3 | lui $rt5/$rt2, 0x0063 + addiu pattern |
| `0x00556a28` (DATA.DF) | 0 | No split-immediate pattern found |

### Notable Code Locations

| Virtual Address | File Offset | Instruction Pattern |
|-----------------|-------------|---------------------|
| `0x0013221c` | `0x0003321c` | lui $rt5, 0x0055 |
| `0x00132248` | `0x0003322c` | addiu $rt5, $rs30, 0x6a10 |
| `0x0013242c` | `0x0003342c` | lui $rt2, 0x0055 |
| `0x00132444` | `0x0003344c` | addiu $rt2, $rs5, 0x6a20 |
| `0x0019fbf4` | `0x000a0bf4` | lui $rt4, 0x0061 |
| `0x0019fc00` | `0x000a0c00` | addiu $rt4, $rs4, 0x27e8 |
| `0x00185fc8` | `0x00086fc8` | lui $rt5, 0x0063 |
| `0x001a0a6c` | `0x000a1a6c` | addiu $rt5, $rs9, 0x3b68 |

## Interpretation

- Multiple code locations reference the DFDATAS string (`0x00556a10`, `0x00556a20`) using `lui`/`addiu` pairs.
- The `.sdata` reference at `0x00633b68` has 3 code references.
- The `.rodata` reference at `0x006127e8` has 1 code reference.
- The `DATA.DF` string at `0x00556a28` was **not** found as a split-immediate pattern in `.text`, suggesting either:
  - The string is accessed through a different mechanism (e.g., direct pointer load from `gp` relative)
  - The code uses a base register that already contains the high bits

## Next Step

Cross-reference these code locations with the ELF symbol table to identify the functions that construct pointers to DATA.DF and DFDATAS, enabling identification of the file I/O and loader logic.