# iso-index

Metadata-only ISO9660 indexer for local user-owned disc images.

The tool reads a local ISO/BIN image, detects a supported sector layout, parses ISO9660 directory records, and writes a JSON report containing names, LBAs, sizes, flags, candidate executable files, and basic ELF header metadata when available.

It does not extract file contents.

## Usage

```bash
python3 tools/iso-index/iso_index.py /path/to/image.iso
python3 tools/iso-index/iso_index.py "/path/to/Ico (USA).bin" --cue "/path/to/Ico (USA).cue"
```

Default reports are written under:

```text
.local/reports/
```

That directory is ignored by git.

## Supported Layouts

Initial detection supports:

- plain ISO data sectors: 2048-byte sectors
- raw CD-ROM images: 2352-byte sectors with common data offsets

## Safe Output

Reports may include:

- image path on the local machine
- sector layout metadata
- volume metadata
- file and directory names
- LBAs
- sizes
- candidate executable names
- basic ELF header metadata for executable candidates

Do not commit generated reports from a real game copy unless reviewed for metadata-only contents.
