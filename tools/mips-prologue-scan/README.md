# MIPS Prologue Scanner

Metadata-only scanner for MIPS function prologues in PS2 executables.

## Purpose

Scan `.text` sections for MIPS function prologues by detecting stack allocation patterns (`addiu $sp, $sp, -N`). This helps map code structure without symbol information.

## Usage

```bash
python3 mips_prologue_scan.py \
  --image '/path/to/disc.bin' \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13
```

With custom range:

```bash
python3 mips_prologue_scan.py \
  --image '/path/to/disc.bin' \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13 \
  --min-offset 32 \
  --max-offset 256 \
  --max-results 1000
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
| `--min-offset` | Minimum stack offset (default: 16) |
| `--max-offset` | Maximum stack offset (default: 32768) |
| `--check-sequence` | Check for register save sequences (slower) |
| `--max-results` | Max prologues in report (default: 500, 0 for all) |
| `--output-dir` | Output directory for JSON report (default: .local/reports) |

## Output

JSON report containing:
- List of detected prologues with file offsets and virtual addresses
- Stack distribution statistics
- Summary of total prologues found and unique stack sizes

## Legal Boundary

This tool performs metadata-only pattern scanning only. It does not:
- Extract or disassemble code
- Identify function names
- Copy executable contents

## Version

0.1.0