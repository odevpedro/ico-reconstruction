# Function Reference Correlator

Metadata-only correlator for linking function prologues with code references.

## Purpose

Correlate detected MIPS function prologues with known code references (from mips-immediate-scanner) to identify which functions contain references to specific virtual addresses, such as DATA.DF or DFDATAS strings.

## Usage

```bash
python3 function_ref_correlator.py \
  --immediate-report .local/reports/20260512T234508Z-SCUS_971.13-mips-immediate-scan.json \
  --prologue-report .local/reports/20260512T234540Z-SCUS_971.13-mips-prologue-scan.json
```

## Arguments

| Argument | Description |
|----------|-------------|
| `--immediate-report` | Path to mips-immediate-scan JSON report (required) |
| `--prologue-report` | Path to mips-prologue-scan JSON report (required) |
| `--max-details` | Max correlation details to include (default: 20, 0 for all) |
| `--output-dir` | Output directory for JSON report (default: .local/reports) |

## Output

JSON report containing:
- Summary statistics (total matches, unique functions)
- Target-by-target breakdown with function lists
- Detailed per-function information with references

## Legal Boundary

This tool performs metadata-only correlation of existing JSON reports. It does not:
- Extract or disassemble code
- Access the original executable
- Generate new executable content

## Version

0.1.0