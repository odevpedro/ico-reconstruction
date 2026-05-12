# elf-index

Metadata-only ELF32 indexer for local user-owned inputs.

The tool reads an ELF either from a direct local file or from a file region inside a local disc image, then writes a JSON report with ELF header, program header, section header, and symbol table metadata.

It does not extract or save executable contents.

## Usage

Direct ELF file:

```bash
python3 tools/elf-index/elf_index.py --elf /path/to/local.elf
```

ELF embedded in a BIN/CUE image:

```bash
python3 tools/elf-index/elf_index.py \
  --image "/path/to/Ico (USA).bin" \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13
```

Default reports are written under:

```text
.local/reports/
```

That directory is ignored by git.

## Safe Output

Reports may include:

- ELF class, endianness, machine, flags, and entry point
- program header types, offsets, virtual addresses, file sizes, and memory sizes
- section names, types, flags, addresses, offsets, and sizes
- symbol table presence and counts

The tool does not list individual symbol names by default.
