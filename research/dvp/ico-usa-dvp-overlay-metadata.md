# ICO USA DVP Overlay Metadata

> Evidence level: Confirmed
> Source: local user-owned BIN/CUE metadata-only scan
> Date: 2026-05-12

## Input

`.DVP.*` sections were inspected from the embedded `SCUS_971.13` executable:

| Field | Value |
|-------|-------|
| Disc image | `/home/peter/Downloads/Ico (USA)/Ico (USA).bin` |
| Source name | `SCUS_971.13` |
| ELF LBA | 25 |
| ELF size | 5,481,608 bytes |
| Sector size | 2,352 |
| Data offset | 24 |
| `DATA.DF` size context | 539,367,424 bytes |

The executable was read from the local image for metadata extraction only. It was not copied into the repository.

## Command

```bash
python3 tools/dvp-index/dvp_index.py \
  --image '/home/peter/Downloads/Ico (USA)/Ico (USA).bin' \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13 \
  --data-df-size 539367424
```

## ELF Load Context

| Field | Value |
|-------|-------|
| Load range start | `0x00100000` |
| Load range end | `0x0071eb98` |

## DVP Section Summary

| Metric | Value |
|--------|-------|
| `.DVP.*` sections | 14 |
| `.DVP.overlay...` sections | 12 |
| Has `.DVP.ovlytab` | yes |
| Has `.DVP.ovlystrtab` | yes |
| `.DVP.ovlytab` size | 144 bytes |
| `.DVP.ovlytab` entry size | 12 bytes |
| `.DVP.ovlytab` entries | 12 |
| `.DVP.ovlystrtab` size | 398 bytes |
| `.DVP.ovlystrtab` string count | 12 |

## Overlay Table Pattern

The `.DVP.ovlytab` section is structured as 12 entries of 12 bytes. Interpreted as little-endian 32-bit words, each entry has 3 words.

Observed aggregate pattern:

| Measurement | Value |
|-------------|-------|
| Total words | 36 |
| Words numerically below `DATA.DF` size | 36 |
| Words inside ELF load range | 12 |
| Words aligned to 2048 | 12 |

Important interpretation note: being below `DATA.DF` size is not enough to classify a value as a `DATA.DF` offset. The second word of every `.DVP.ovlytab` entry falls inside the ELF load range, which strongly suggests at least part of the table refers to loaded memory addresses rather than archive offsets.

The third word in each entry is frequently `0`, `2048`, `4096`, or `6144`, which may represent overlay size, alignment, or a related small field. This is still unconfirmed.

## Overlay Section Name Tokens

The `.DVP.overlay...` section names include numeric tokens. Some larger values are below the known `DATA.DF` size, including values around 1.8 MiB, 15.1 MiB, 30.2 MiB, and 483.6 MiB.

These values are not consistently 2048-aligned, so they should not be treated as confirmed archive sector offsets yet.

## Initial Interpretation

- The executable contains an explicit DVP overlay table and overlay string table.
- There are 12 overlay sections and 12 overlay string-table entries, suggesting a one-to-one relationship.
- `.DVP.ovlytab` is likely meaningful overlay metadata, but current evidence points partly to ELF memory references rather than direct `DATA.DF` archive offsets.
- Numeric tokens in `.DVP.overlay...` section names may be useful search seeds for `DATA.DF`, but they are not confirmed offsets.
- A targeted `DATA.DF` scan should use these numeric tokens as candidate offsets and nearby windows, not as authoritative table entries.

## Remaining Uncertainty

- Exact `.DVP.ovlytab` field meanings are unknown.
- `.DVP.ovlystrtab` strings were counted, but individual string contents were not recorded in this note.
- Whether large numeric tokens in `.DVP.overlay...` names reference `DATA.DF` positions, virtual memory addresses, build metadata, hashes, or IDs is unknown.
- No overlay payload has been extracted or decoded.

## Next Step

Extend `data-df-index` with targeted window scans around `.DVP.overlay...` numeric tokens and compare entropy/padding/table candidates near those positions.
