# ICO USA SCUS_971.13 Symbol Table Analysis

> Evidence level: Confirmed
> Source: metadata-only ELF symbol scan
> Date: 2026-05-12

## Purpose

This note records the search for symbol tables in the main executable `SCUS_971.13`, including standard symbol tables (`.symtab`), dynamic symbol tables (`.dynsym`), and related structures.

## Input

| Field | Value |
|-------|-------|
| Disc image | `/home/peter/Downloads/Ico (USA)/Ico (USA).bin` |
| Source name | `SCUS_971.13` |
| LBA | 25 |
| Size | 5,481,608 bytes |

## Command

```bash
python3 tools/elf-symbol-scan/elf_symbol_scan.py \
  --image '/home/peter/Downloads/Ico (USA)/Ico (USA).bin' \
  --lba 25 \
  --size 5481608 \
  --sector-size 2352 \
  --data-offset 24 \
  --source-name SCUS_971.13
```

## Results

| Check | Result |
|-------|--------|
| `.symtab` (SHT_SYMTAB) | Not found |
| `.dynsym` (SHT_DYNSYM) | Not found |
| Hash tables (SHT_HASH/SHT_DYNHASH) | Not found |
| `PT_DYNAMIC` segment | Not found |
| Potential string tables | None |

### Summary

- **Symbol tables found**: 0
- **Dynamic segments**: 0
- **Potential string tables**: 0

## Interpretation

The executable `SCUS_971.13` is **completely stripped** of all symbol information. This is typical for commercial PlayStation 2 games:

- Reduces binary size
- Prevents easy identification of functions by name
- Deters casual reverse engineering

The absence of a dynamic symbol table (`.dynsym`) indicates the executable does not use runtime dynamic linking. The absence of `PT_DYNAMIC` confirms this.

## Implications for Analysis

Without symbol names, function identification must rely on:

1. **String cross-references** - Use known strings (DATA.DF, DFDATAS) to locate functions that reference them
2. **Code pattern analysis** - Identify function prologues (`addiu $sp, $sp, -X` sequences)
3. **Import/export tables** - Check for any exported functions (not found in this scan)
4. **Call graph reconstruction** - Build call relationships from control flow analysis
5. **Static analysis with disassembly** - Use tools like IDA Pro, Ghidra, or mips-disasm for deeper analysis

The 8 MIPS split-immediate references found in the previous scan (`lui`/`addiu` patterns) remain the primary confirmed linkage between code and known data sections.

## Next Step

Without symbols, the next practical approach is to correlate the known code references (from `mips-immediate-scanner`) with code structure analysis using a full disassembler or by extending tooling to identify MIPS function prologues.

(End of file - total 79 lines)