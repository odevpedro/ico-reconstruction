# data-df-index

Metadata-only structural triage for `DFDATAS/DATA.DF`.

The tool reads bounded windows from the local `DATA.DF` region inside a user-owned disc image and writes a JSON report with aggregate statistics and possible numeric table patterns.

It does not extract archive contents, save bytes, decode assets, or write internal files.

## Usage

For the current ICO USA BIN/CUE metadata:

```bash
python3 tools/data-df-index/data_df_index.py \
  --image "/path/to/Ico (USA).bin" \
  --lba 2898 \
  --size 539367424 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name DFDATAS/DATA.DF
```

Targeted scan around candidate offsets:

```bash
python3 tools/data-df-index/data_df_index.py \
  --image "/path/to/Ico (USA).bin" \
  --lba 2898 \
  --size 539367424 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name DFDATAS/DATA.DF \
  --target-offset 1887731 \
  --target-offset 15101843 \
  --target-window-bytes 262144
```

Default reports are written under:

```text
.local/reports/
```

That directory is ignored by git.

## Safe Output

Reports may include:

- source LBA and size
- scan window sizes for head/middle/tail samples
- targeted window sizes and offsets
- entropy and byte-class statistics
- SHA-256 of scanned windows
- possible monotonic 32-bit offset table candidates
- possible fixed-record table candidates

Reports must not include raw file bytes or extracted archive entries.
