# asset-ref-index

Metadata-only asset reference indexer for the ICO executable.

The tool scans printable strings in a local `SCUS_971.13` ELF and records file/path-like references by extension, prefix, file offset, virtual address, and containing ELF section.

It does not extract assets, decode asset formats, disassemble code, or copy game content into the repository.

## Usage

Direct ELF file:

```bash
python3 tools/asset-ref-index/asset_ref_index.py --elf .local/extracted/SCUS_971.13.elf
```

ELF embedded in a BIN/CUE image:

```bash
python3 tools/asset-ref-index/asset_ref_index.py \
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

- asset reference strings already embedded in the local executable
- file offsets and approximate virtual addresses
- containing ELF section names
- extension and prefix counts
- aggregate totals

Reports must not include raw asset bytes, decoded textures, models, animation data, audio, video, or copied executable code.
