# ICO USA DATA.DF Initial Triage

> Evidence level: Confirmed
> Source: local user-owned BIN/CUE metadata-only scan
> Date: 2026-05-12

## Input

`DFDATAS/DATA.DF` was identified by `tools/iso-index/`:

| Field | Value |
|-------|-------|
| Disc image | `/home/peter/Downloads/Ico (USA)/Ico (USA).bin` |
| Source name | `DFDATAS/DATA.DF` |
| LBA | 2,898 |
| Size | 539,367,424 bytes |
| Sector size | 2,352 |
| Data offset | 24 |

The archive candidate was read from the local image for metadata extraction only. It was not copied into the repository.

## Command

```bash
python3 tools/data-df-index/data_df_index.py \
  --image '/home/peter/Downloads/Ico (USA)/Ico (USA).bin' \
  --lba 2898 \
  --size 539367424 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name DFDATAS/DATA.DF
```

## Scan Scope

| Field | Value |
|-------|-------|
| Windows | head, middle, tail |
| Bytes per window | 1,048,576 |
| Total scanned bytes | 3,145,728 |
| Total scan ratio | 0.005832 |

## Window Profiles

| Window | Region Offset | Entropy | Zero Ratio | `0xFF` Ratio | Printable ASCII Ratio | Unique Bytes |
|--------|---------------|---------|------------|--------------|-----------------------|--------------|
| head | 0 | 7.994416 | 0.005119 | 0.005159 | 0.364155 | 256 |
| middle | 269,159,424 | 6.649272 | 0.025793 | 0.017788 | 0.292822 | 255 |
| tail | 538,318,848 | 2.308932 | 0.760003 | 0.065526 | 0.050411 | 256 |

## Candidate Tables

The initial head-window scan found no simple monotonic 32-bit offset table candidates and no fixed-record table candidates under the current heuristic.

## Initial Interpretation

- `DATA.DF` does not expose an obvious simple offset table at the beginning under the current scan heuristic.
- The head window has near-maximum entropy, which may indicate compressed, packed, encrypted, or otherwise dense binary data.
- The tail window is mostly zero bytes, which likely represents padding or unused archive space.
- The archive structure may require executable reference analysis, overlay metadata analysis, or a broader search for tables outside the first megabyte.

## Remaining Uncertainty

- The internal archive format is still unknown.
- No individual entries, file names, asset records, or payloads have been extracted.
- The relationship between `.DVP.overlay...` ELF sections and `DATA.DF` remains unknown.
- The current heuristic may miss non-monotonic, compressed, hashed, encrypted, or externally referenced tables.

## Next Step

Use the `.DVP.*` overlay metadata documented in `research/dvp/ico-usa-dvp-overlay-metadata.md` to drive targeted `DATA.DF` scans around candidate numeric tokens.
