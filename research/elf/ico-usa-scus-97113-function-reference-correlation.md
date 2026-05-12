# ICO USA SCUS_971.13 Function Reference Correlation

> Evidence level: Confirmed
> Source: metadata-only correlation of prologue and immediate scan data
> Date: 2026-05-12

## Purpose

This note records the correlation between detected function prologues and known code references to DATA.DF/DFDATAS addresses, identifying which functions contain file I/O and data loading code.

## Input

| Field | Value |
|-------|-------|
| MIPS immediate scan report | `.local/reports/*mips-immediate-scan.json` |
| MIPS prologue scan report | `.local/reports/*mips-prologue-scan.json` |

## Results

### Summary

| Metric | Value |
|--------|-------|
| Total immediate matches | 8 |
| Unique functions with data refs | 7 |
| Targets analyzed | 4 |

### Functions with DATA.DF/DFDATAS References

| Target Address | Target Type | Functions | Stack Sizes |
|----------------|-------------|-----------|-------------|
| `0x00556a10` | DFDATAS | 3 | 464, 704, 592 |
| `0x00556a20` | DFDATAS | 1 | 464 |
| `0x006127e8` | DFDATAS ref | 1 | 112 |
| `0x00633b68` | DFDATAS ref | 2 | 320, 320 |

### Detailed Function List

| Virtual Address | File Offset | Stack Size | References |
|-----------------|-------------|------------|-------------|
| `0x001321c8` | 209352 | 464 | lui/addiu to 0x00556a10, lui/addiu to 0x00556a20 |
| `0x00132630` | 210480 | 704 | lui/addiu to 0x00556a10 |
| `0x00132ff0` | 212976 | 592 | lui/addiu to 0x00556a10 |
| `0x00185ca8` | 552104 | 320 | lui to 0x00633b68 |
| `0x0019fb34` | 658228 | 112 | lui/addiu to 0x006127e8 |
| `0x001a0a38` | 662072 | 176 | addiu to 0x00633b68 |
| `0x0023d468` | 1303656 | 320 | lui/addiu to 0x00633b68 |

## Interpretation

- **7 distinct functions** identified that reference DATA.DF/DFDATAS addresses
- Function at `0x001321c8` is particularly interesting: it references both `0x00556a10` and `0x00556a20` (two DFDATAS addresses)
- Stack sizes range from 112 to 704 bytes, indicating simple to moderately complex functions
- No symbol names available, but these virtual addresses can now be used for deeper disassembly analysis

## Next Step

Use these virtual addresses as seeds for deeper analysis:
1. Extend analysis around these function boundaries in a disassembler
2. Identify caller/callee relationships
3. Correlate with string references from exe-ref-index to validate function purposes

(End of file - total 71 lines)