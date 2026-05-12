# ICO USA SCUS_971.13 ELF Metadata

> Evidence level: Confirmed
> Source: local user-owned BIN/CUE metadata-only scan
> Date: 2026-05-12

## Input

The executable candidate was identified by `tools/iso-index/`:

| Field | Value |
|-------|-------|
| Disc image | `/home/peter/Downloads/Ico (USA)/Ico (USA).bin` |
| Source name | `SCUS_971.13` |
| LBA | 25 |
| Size | 5,481,608 bytes |
| Sector size | 2,352 |
| Data offset | 24 |

The executable was read from the local image for metadata extraction only. It was not copied into the repository.

## Command

```bash
python3 tools/elf-index/elf_index.py \
  --image '/home/peter/Downloads/Ico (USA)/Ico (USA).bin' \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13
```

## ELF Header

| Field | Value |
|-------|-------|
| Class | ELF32 |
| Endianness | little |
| Type | 2 |
| Machine | 8 |
| Entry point | `0x00100008` |
| Flags | `0x20924001` |
| ELF header size | 52 |
| Program header offset | 52 |
| Program header count | 1 |
| Section header offset | 5,480,528 |
| Section header count | 27 |
| Section name string table index | 26 |

## Program Headers

| Index | Type | Offset | Virtual Address | Physical Address | File Size | Memory Size | Flags | Alignment |
|-------|------|--------|-----------------|------------------|-----------|-------------|-------|-----------|
| 0 | `PT_LOAD` | 4,096 | `0x00100000` | `0x00100000` | 5,454,790 | 6,417,304 | `0x00000007` | 4,096 |

## Section Summary

| Index | Name | Type | Address | Offset | Size | Flags | Alignment |
|-------|------|------|---------|--------|------|-------|-----------|
| 1 | `.text` | `SHT_PROGBITS` | `0x00100000` | 4,096 | 1,504,724 | `0x00000006` | 64 |
| 2 | `.vutext` | `SHT_PROGBITS` | `0x0026f5e0` | 1,508,832 | 20,704 | `0x00000006` | 16 |
| 3 | `.reginfo` | `SHT_UNKNOWN_1879048198` | `0x002746c0` | 5,458,956 | 24 | `0x00000000` | 4 |
| 4 | `.data` | `SHT_PROGBITS` | `0x00274700` | 1,529,600 | 3,010,488 | `0x00000003` | 64 |
| 5 | `.vudata` | `SHT_PROGBITS` | `0x005536b8` | 4,540,088 | 0 | `0x00000003` | 1 |
| 6 | `.rodata` | `SHT_PROGBITS` | `0x00553700` | 4,540,160 | 905,640 | `0x00000002` | 16 |
| 7 | `.lit4` | `SHT_PROGBITS` | `0x00630900` | 5,445,888 | 4,048 | `0x10000003` | 4 |
| 8 | `.sdata` | `SHT_PROGBITS` | `0x00631900` | 5,449,984 | 8,902 | `0x10000003` | 16 |
| 9 | `.sbss` | `SHT_NOBITS` | `0x00633c00` | 5,458,944 | 1,012 | `0x10000003` | 8 |
| 10 | `.bss` | `SHT_NOBITS` | `0x00634000` | 5,458,956 | 961,432 | `0x00000003` | 128 |
| 11 | `.vubss` | `SHT_NOBITS` | `0x0071eb98` | 5,458,956 | 0 | `0x00000003` | 1 |
| 12 | `.DVP.ovlytab` | `SHT_UNKNOWN_2147480608` | `0x0071eb98` | 5,458,980 | 144 | `0x00000001` | 4 |
| 13 | `.DVP.ovlystrtab` | `SHT_STRTAB` | `0x0071eb98` | 5,459,124 | 398 | `0x00000001` | 1 |
| 26 | `.shstrtab` | `SHT_STRTAB` | `0x00000000` | 5,480,002 | 525 | `0x00000000` | 1 |

Additional sections `14` through `25` are `.DVP.overlay...` entries. They appear to be overlay metadata or overlay payload sections and need dedicated investigation before interpretation.

## Symbol Tables

No `SHT_SYMTAB` or `SHT_DYNSYM` section was detected.

## Initial Interpretation

- The executable is an ELF32 little-endian MIPS target.
- The executable has one loadable segment starting at virtual/physical address `0x00100000`.
- Entry point `0x00100008` sits inside the loaded region.
- `.vutext`, `.vudata`, and `.vubss` indicate vector-unit-related code/data sections are present.
- `.DVP.ovlytab`, `.DVP.ovlystrtab`, and multiple `.DVP.overlay...` sections suggest an overlay mechanism or linked overlay metadata exists in the executable.
- No symbol table is present, so function naming will likely require disassembly analysis, references, pattern matching, and manual labeling.

## Remaining Uncertainty

- Exact meaning of the `.DVP.*` sections is unknown.
- The relationship between `.DVP.overlay...` sections and `DFDATAS/DATA.DF` is unknown.
- Program-header memory mapping needs runtime/emulator validation.
- No disassembly or function boundary analysis has been performed yet.

## Next Step

Use the `DATA.DF` triage results and `.DVP.*` section metadata to design a targeted search for archive tables or overlay references.
