# Ghidra Analysis - SCUS_971.13.elf (ICO PS2)

## Analysis Date
2026-05-12

## Environment
- Ghidra: 12.0.4_PUBLIC
- JDK: OpenJDK 21.0.11 (Red Hat)
- Language: MIPS:LE:32:default
- Project: /tmp/ghidra_ico/ICO_Analysis

## Binary Information
- File: SCUS_971.13.elf (extracted from SCUS_971.13)
- Size: 5.48 MB
- Architecture: MIPS32 little-endian
- Type: ELF executable

## Analysis Results

### Function Statistics
- **Total Functions Identified:** 3426

### Known Function Addresses (from previous analysis)
| Address | Ghidra Name | Callers | Notes |
|---------|-------------|---------|-------|
| 0x001321c8 | FUN_001321c8 | 0 | |
| 0x00132630 | - | - | Not identified as function (data?) |
| 0x00132ff0 | FUN_00132ff0 | 0 | |
| 0x00185ca8 | - | - | Not identified as function |
| 0x0019fb34 | - | - | Not identified as function |
| 0x001a0a38 | FUN_001a0a38 | 0 | |
| 0x0023d468 | FUN_0023d468 | 1 | Called by FUN_0023d420 |

### Most Called Functions (Top 15)
| Rank | Function | Address | Callers |
|------|----------|---------|---------|
| 1 | FUN_001b7288 | 001b7288 | 8 |
| 2 | FUN_001a6e28 | 001a6e28 | 4 |
| 3 | FUN_001b0a80 | 001b0a80 | 3 |

### Analysis Notes
- Many addresses from previous analysis not found as functions - likely data or inline code
- Pcode errors during decompilation are normal for stripped binaries
- The binary has no symbol table (.symtab, .dynsym stripped)

## Next Steps
1. Analyze most-called functions (FUN_001b7288, FUN_001a6e28) for potential file I/O
2. Decompile and analyze function signatures
3. Identify library functions (printf, malloc, file operations)
4. Cross-reference with known PS2 SDK functions

## Files Generated
- Ghidra project: /tmp/ghidra_ico/
- Scripts: .local/ghidra/AnalyzeKnownFunctions.java