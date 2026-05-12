# dvp-index

Metadata-only `.DVP.*` overlay metadata indexer for the ICO executable.

The tool reads the local `SCUS_971.13` ELF from a user-owned disc image, summarizes `.DVP.ovlytab`, `.DVP.ovlystrtab`, and `.DVP.overlay...` sections, and checks numeric values against the known `DFDATAS/DATA.DF` size.

It does not extract executable bytes, archive entries, overlay payloads, or game assets.

## Usage

For the current ICO USA BIN/CUE metadata:

```bash
python3 tools/dvp-index/dvp_index.py \
  --image "/path/to/Ico (USA).bin" \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13 \
  --data-df-size 539367424
```

Default reports are written under:

```text
.local/reports/
```

That directory is ignored by git.

## Safe Output

Reports may include:

- `.DVP.*` section names, offsets, sizes, and section types
- `.DVP.ovlytab` entry counts and numeric triples
- `.DVP.ovlystrtab` aggregate string counts and length statistics
- numeric value checks against `DATA.DF` size and common alignment

Reports must not include raw executable bytes, disassembly, archive contents, or extracted overlay payloads.
