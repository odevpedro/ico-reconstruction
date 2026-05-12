# MIPS Immediate Scanner

Metadata-only scanner for MIPS split-immediate patterns (`lui` + `ori`/`addiu`) in PS2 executables.

## Purpose

Scan MIPS code sections for split-immediate instruction patterns that construct 32-bit addresses from high and low 16-bit immediates. Useful for finding code references to known virtual addresses without performing full disassembly.

## Usage

```bash
python3 mips_immediate_scanner.py \
  --image '/path/to/disc.bin' \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13 \
  --target 0x00556a10 \
  --target 0x00633b68 \
  --output-dir .local/reports
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
| `--target` | Target virtual address to scan for (hex with 0x prefix). Repeatable. |
| `--output-dir` | Output directory for JSON report (default: .local/reports) |

## Output

JSON report containing:
- Target addresses scanned
- List of `lui`/`addiu` pattern matches with file offsets and virtual addresses
- Summary statistics

## Legal Boundary

This tool performs metadata-only pattern scanning only. It does not:
- Execute or modify any code
- Extract proprietary data beyond virtual addresses
- Perform full disassembly

## Version

0.1.0