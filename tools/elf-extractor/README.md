# ELF Extractor

Extract ELF from disc image for use with external disassemblers.

## Purpose

Extract the main executable (SCUS_971.13) from the BIN/CUE disc image for analysis in disassemblers like Ghidra, radare2, or IDA Pro.

## WARNING

This tool extracts raw executable bytes. Handle with care:
- **For local analysis only**
- **Do not distribute the extracted ELF**
- The extracted file is not committed to the repository

## Usage

```bash
python3 elf_extractor.py \
  --image '/path/to/Ico (USA).bin' \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13 \
  --output-dir .local/extracted
```

## Arguments

| Argument | Description |
|----------|-------------|
| `--image` | Path to disc image file (required) |
| `--lba` | Starting LBA of ELF (required) |
| `--size` | Size in bytes of ELF (required) |
| `--sector-size` | Physical sector size (default: 2352) |
| `--data-offset` | Data payload offset (default: 24) |
| `--source-name` | Display name for output (default: SCUS_971.13) |
| `--output-dir` | Output directory (default: .local/extracted) |

## Output

- `SCUS_971.13.elf` - The extracted ELF file
- `SCUS_971.13-extract-report.json` - Metadata report with hashes

## Using with Ghidra

1. Launch Ghidra
2. File → Import File
3. Select `SCUS_971.13.elf`
4. Configure:
   - Language: MIPS little-endian
   - Processor: MIPS
   - Compiler: GCC
5. Analyze and navigate to known addresses:
   - 0x00132630 (most-called function)
   - 0x001321c8 (DFDATAS references)
   - 0x0019fb34 (data loading)

## Version

0.1.0