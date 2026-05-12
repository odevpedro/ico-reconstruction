# ICO USA SCUS_971.13 Call Graph Analysis

> Evidence level: Confirmed
> Source: metadata-only MIPS call graph analysis
> Date: 2026-05-12

## Purpose

This note records the call graph analysis to identify which functions call the known DATA.DF/DFDATAS reference functions identified in the previous correlation step.

## Input

| Field | Value |
|-------|-------|
| MIPS call graph report | `.local/reports/*mips-call-graph.json` |

## Results

### Summary

| Metric | Value |
|--------|-------|
| Total calls found | 61,783 |
| Target calls found | 15 |
| Unique callers | 13 |

### Callers by Target Function

| Target Function | Callers | Stack Sizes |
|-----------------|---------|-------------|
| `0x001321c8` | 0 | - |
| `0x00132630` | 7 | 112, 112, 208, 96, 96, 112, 144 |
| `0x00132ff0` | 1 | 32 |
| `0x00185ca8` | 0 | - |
| `0x0019fb34` | 1 | 176 |
| `0x001a0a38` | 2 | 32, 16 |
| `0x0023d468` | 2 | 176, 272 |

### Detailed Caller List

| Target | Caller Function | Caller Address | Stack Size |
|--------|-----------------|----------------|------------|
| `0x00132630` | `0x00138510` | `0x0013857c` | 112 |
| `0x00132630` | `0x00138618` | `0x00138684` | 112 |
| `0x00132630` | `0x00140340` | `0x001406c0` | 208 |
| `0x00132630` | `0x00140748` | `0x00140840` | 96 |
| `0x00132630` | `0x00176600` | `0x00176738` | 96 |
| `0x00132630` | `0x0019fb34` | `0x0019fc1c` | 112 |
| `0x00132630` | `0x001f0e40` | `0x0019fc1c` | 144 |
| `0x00132ff0` | `0x0019db70` | `0x0019db88` | 32 |
| `0x0019fb34` | `0x0019fc78` | `0x0019fee4` | 176 |
| `0x001a0a38` | `0x0017d128` | `0x0017d1b0` | 32 |
| `0x001a0a38` | `0x001ab190` | `0x001ab19c` | 16 |
| `0x0023d468` | `0x001af4a0` | `0x001af684` | 176 |
| `0x0023d468` | `0x0023d420` | `0x0023d450` | 272 |

## Interpretation

- **Function `0x00132630`** is the most called (7 times), suggesting it may be a central file I/O utility
- Functions `0x001321c8` and `0x00185ca8` have no detected callers - they may be leaf functions or called via register indirection
- The call chain shows `0x0019fb34` calling `0x00132630` - interesting since `0x0019fb34` itself accesses DATA.DF
- Stack sizes range from 16 to 272 bytes, indicating simple to moderately complex callers

## Next Step

This completes the metadata-only tooling pipeline. The next major step is **Environment Setup** - configuring a disassembler and emulator to validate and extend this analysis.

(End of file - total 76 lines)