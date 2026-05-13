# Ghidra Function Analysis - rev.012

## Analysis Date
2026-05-12

## Top Called Functions

| Rank | Function Name | Address | Callers |
|------|---------------|---------|---------|
| 1 | FUN_001b7288 | 0x001b7288 | 8 |
| 2 | FUN_001a6e28 | 0x001a6e28 | 4 |
| 3 | FUN_001b0a80 | 0x001b0a80 | 3 |

## Analysis

### FUN_001b7288 (0x001b7288)
- **Callers:** 8
- **Description:** Most called function - likely core utility (I/O, memory, or system)
- **Next step:** Decompile and analyze instructions

### FUN_001a6e28 (0x001a6e28)
- **Callers:** 4
- **Description:** Secondary utility function

### FUN_001b0a80 (0x001b0a80)
- **Callers:** 3
- **Description:** Third most called function

## Known Function Status

| Address | Status | Notes |
|---------|--------|-------|
| 0x001321c8 | Function found | FUN_001321c8 |
| 0x00132630 | Not found | Likely data or inline code |
| 0x00132ff0 | Function found | FUN_00132ff0 |
| 0x00185ca8 | Not found | Likely data or inline code |
| 0x0019fb34 | Not found | Likely data or inline code |
| 0x001a0a38 | Function found | FUN_001a0a38 |
| 0x0023d468 | Function found | FUN_0023d468 (1 caller) |

## Next Steps
1. Analyze instructions at 0x001b7288, 0x001a6e28, 0x001b0a80
2. Identify potential library calls (printf, malloc, file ops)
3. Map to PS2 SDK functions if possible

## Scripts Used
- AnalyzeKnownFunctions.java (in Ghidra scripts folder)