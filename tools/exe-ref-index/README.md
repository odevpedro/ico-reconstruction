# exe-ref-index

Metadata-only exact reference scanner for the ICO executable.

The tool reads the local `SCUS_971.13` ELF from a user-owned disc image and searches for explicit string queries and numeric constants. It reports counts, ELF file offsets, approximate virtual addresses, and containing section names.

It does not disassemble code, extract executable bytes, or dump arbitrary strings.

## Usage

```bash
python3 tools/exe-ref-index/exe_ref_index.py \
  --image "/path/to/Ico (USA).bin" \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13 \
  --query DATA.DF \
  --query DFDATAS \
  --constant 539367424 \
  --constant 30224099
```

Default reports are written under:

```text
.local/reports/
```

That directory is ignored by git.

## Safe Output

Reports may include:

- query terms supplied by the user
- match counts
- file offsets
- approximate virtual addresses
- section names
- exact constant values searched

Reports must not include raw executable bytes, disassembly, recovered function bodies, or arbitrary string dumps.
