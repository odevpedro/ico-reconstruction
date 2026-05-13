# Ghidra Analysis - rev.013 - Library Calls and Top Functions

## Date
2026-05-12

## Summary
Fixed Ghidra script loading issue by using isolated script folder. Successfully identified top called functions and their relationships.

## Solution Applied
- Created clean script folder: `/tmp/ghidra_scripts_clean/`
- Used one script at a time to avoid OSGi bundle contamination
- Working script: `FindLibraryCalls.java`

## Top Functions by Call Count

| Rank | Function | Address | Callers |
|------|----------|---------|---------|
| 1 | FUN_001b7288 | 0x001b7288 | 8 |
| 2 | FUN_001a6e28 | 0x001a6e28 | 4 |
| 3 | FUN_001b0a80 | 0x001b0a80 | 3 |
| 4 | FUN_0017b230 | 0x0017b230 | 2 |
| 5 | FUN_001b1270 | 0x001b1270 | 2 |
| 6 | FUN_00100230 | 0x00100230 | 1 |
| ... | ... | ... | ... |

## Total Functions
- **3426 functions** identified in SCUS_971.13.elf

## Analysis Notes
- FUN_001b7288 is a stub function (returns immediately)
- FUN_001a6e28 has complex stack management and global data access
- Most functions are called only once

## Next Steps
1. Analyze call graph for potential I/O functions
2. Look for PS2 syscall patterns
3. Search for string references to identify library functions

## Scripts Used
- FindLibraryCalls.java - lists top functions and their call relationships