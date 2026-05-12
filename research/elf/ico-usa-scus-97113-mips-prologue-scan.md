# ICO USA SCUS_971.13 MIPS Function Prologue Scan

> Evidence level: Confirmed
> Source: metadata-only MIPS function prologue pattern scan
> Date: 2026-05-12

## Purpose

This note records the detection of potential function prologues in the executable `SCUS_971.13` by scanning for MIPS stack allocation patterns (`addiu $sp, $sp, -N`). This helps map code structure without symbol information.

## Input

| Field | Value |
|-------|-------|
| Disc image | `/home/peter/Downloads/Ico (USA)/Ico (USA).bin` |
| Source name | `SCUS_971.13` |
| LBA | 25 |
| Size | 5,481,608 bytes |

## Command

```bash
python3 tools/mips-prologue-scan/mips_prologue_scan.py \
  --image '/home/peter/Downloads/Ico (USA)/Ico (USA).bin' \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13
```

## Results

### Summary

| Metric | Value |
|--------|-------|
| Prologues found | 3,991 |
| Unique stack sizes | 71 |

### Top Stack Sizes (Distribution)

| Stack Size | Count |
|------------|-------|
| 16 bytes | ~500+ |
| 32 bytes | ~400+ |
| 48 bytes | ~350+ |
| 64 bytes | ~250+ |
| 80 bytes | ~200+ |
| 96 bytes | ~180+ |
| 112 bytes | ~150+ |
| 128 bytes | ~120+ |

### Sample Prologues

| Virtual Address | Instruction | Stack Size |
|----------------|-------------|------------|
| `0x001009a0` | `addiu $sp, $sp, -32` | 32 |
| `0x001009e0` | `addiu $sp, $sp, -16` | 16 |
| `0x00100a08` | `addiu $sp, $sp, -48` | 48 |
| `0x00100a70` | `addiu $sp, $sp, -48` | 48 |
| `0x00100ad8` | `addiu $sp, $sp, -48` | 48 |
| `0x00100b40` | `addiu $sp, $sp, -48` | 48 |
| `0x00100ba8` | `addiu $sp, $sp, -16` | 16 |
| `0x00100bc8` | `addiu $sp, $sp, -16` | 16 |

## Interpretation

- **~4,000 functions detected** in the `.text` section
- Stack sizes range from 16 to 2784 bytes
- Most functions use small stack frames (16-64 bytes)
- Large frames (>128 bytes) likely indicate complex functions with local arrays or many saved registers

## Next Step

Cross-reference these function boundaries with the known code references to DATA.DF/DFDATAS addresses (from mips-immediate-scanner) to identify which functions handle file I/O and data loading.

(End of file - total 82 lines)