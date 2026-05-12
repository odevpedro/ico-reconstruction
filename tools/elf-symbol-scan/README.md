# ELF Symbol Scanner

Enhanced ELF symbol table scanner for PS2 executables.

## Purpose

Scan ELF files for symbol tables and related structures that may help identify functions. Unlike basic ELF indexers, this tool specifically looks for:

- `.symtab` (standard symbol table)
- `.dynsym` (dynamic symbol table)
- Hash tables for dynamic symbols
- PT_DYNAMIC segments

## Usage

```bash
python3 elf_symbol_scan.py \
  --image '/path/to/disc.bin' \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13
```

For deeper string table analysis:

```bash
python3 elf_symbol_scan.py \
  --image '/path/to/disc.bin' \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13 \
  --scan-strings
```

## Arguments

| Argument | Description |
|----------|-------------|
| `--image` | Path to disc image file (required) |
| `--lba` | Starting LBA of ELF (required) |
| `--size` | Size in bytes of ELF (required) |
| `--sector-size` | Physical sector size (default: 2352) |
| `--data-offset` | Data payload offset (default: 24) |
| `--source-name` | Display name for source (default: SCUS_971.13) |
| `--scan-strings` | Scan for potential symbol string tables (slower) |
| `--output-dir` | Output directory for JSON report (default: .local/reports) |

## Output

JSON report containing:
- List of found symbol tables with offsets and sizes
- Dynamic segment information (if present)
- Potential string tables (if --scan-strings used)
- Summary statistics

## Legal Boundary

This tool performs metadata-only scanning only. It does not:
- Extract symbol names or function bodies
- Disassemble code
- Copy executable contents

## Version

0.1.0