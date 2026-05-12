# MIPS Call Graph Analyzer

Metadata-only MIPS function call graph analyzer for PS2 executables.

## Purpose

Analyze MIPS function calls (jal, jalr) to build a call graph and identify which functions call specific target functions, useful for understanding code flow without symbol information.

## Usage

```bash
python3 mips_call_graph.py \
  --image '/path/to/disc.bin' \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13 \
  --prologue-report .local/reports/...-mips-prologue-scan.json \
  --target 0x001321c8 \
  --target 0x00132630
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
| `--prologue-report` | Path to mips-prologue-scan JSON report (required) |
| `--target` | Target function address (hex). Repeatable. |
| `--max-calls` | Max calls to include (default: 1000) |
| `--output-dir` | Output directory for JSON report (default: .local/reports) |

## Output

JSON report containing:
- Summary statistics (total calls, target calls, unique callers)
- Callers grouped by target function
- All detected calls (limited by --max-calls)

## Legal Boundary

This tool performs metadata-only pattern scanning only. It does not:
- Extract or disassemble code
- Identify function names
- Copy executable contents

## Version

0.1.0