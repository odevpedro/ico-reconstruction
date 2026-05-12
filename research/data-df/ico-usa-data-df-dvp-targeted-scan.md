# ICO USA DATA.DF DVP Targeted Scan

> Evidence level: Confirmed
> Source: local user-owned BIN/CUE metadata-only scan
> Date: 2026-05-12

## Purpose

This note records targeted metadata-only scans around numeric tokens found in `.DVP.overlay...` section names.

The goal was to test whether those numeric tokens behave like plausible `DATA.DF` offsets.

## Input

| Field | Value |
|-------|-------|
| Disc image | `/home/peter/Downloads/Ico (USA)/Ico (USA).bin` |
| Source name | `DFDATAS/DATA.DF` |
| LBA | 2,898 |
| Size | 539,367,424 bytes |
| Sector size | 2,352 |
| Data offset | 24 |

## Command

```bash
python3 tools/data-df-index/data_df_index.py \
  --image '/home/peter/Downloads/Ico (USA)/Ico (USA).bin' \
  --lba 2898 \
  --size 539367424 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name DFDATAS/DATA.DF \
  --target-offset 1887731 \
  --target-offset 15101843 \
  --target-offset 30216547 \
  --target-offset 30224099 \
  --target-offset 30224243 \
  --target-offset 483563468 \
  --target-window-bytes 262144
```

## Targeted Window Summary

Each targeted scan used a 262,144-byte window centered on the candidate offset when possible.

| Candidate Offset | Hex | Entropy | Zero Ratio | Offset Table Candidates | Fixed Record Candidates |
|------------------|-----|---------|------------|-------------------------|-------------------------|
| 1,887,731 | `0x001ccdf3` | 7.989932 | 0.001968 | 0 | 0 |
| 15,101,843 | `0x00e66f93` | 7.913830 | 0.002110 | 0 | 0 |
| 30,216,547 | `0x01cd1163` | 7.996811 | 0.004200 | 0 | 0 |
| 30,224,099 | `0x01cd2ee3` | 7.996398 | 0.004131 | 0 | 0 |
| 30,224,243 | `0x01cd2f73` | 7.996397 | 0.004131 | 0 | 0 |
| 483,563,468 | `0x1cd297cc` | 7.939569 | 0.015316 | 0 | 0 |

## Interpretation

- None of the tested DVP numeric tokens exposed a simple local offset table under the current heuristic.
- The targeted windows are mostly high entropy, similar to the `DATA.DF` head window.
- The tokens may still be meaningful, but current evidence does not confirm them as direct `DATA.DF` archive offsets.
- The values may represent IDs, hashes, compressed stream positions, build metadata, non-sector-aligned offsets, or references requiring executable logic to interpret.

## Remaining Uncertainty

- The current heuristic may miss compressed, encrypted, hashed, or non-monotonic table structures.
- The 262,144-byte window size may be too small or too broad for the actual structure.
- The relationship between DVP metadata and `DATA.DF` remains unconfirmed.

## Next Step

Executable-reference analysis is documented in `research/exe-refs/ico-usa-scus-97113-data-df-dvp-references.md`. The next useful step is metadata-only MIPS split-immediate/reference scanning for the confirmed `DATA.DF` and `DFDATAS` virtual addresses.
