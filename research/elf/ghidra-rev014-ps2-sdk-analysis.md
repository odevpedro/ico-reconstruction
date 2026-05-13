# PS2 SDK Function Analysis - rev.014

## Date
2026-05-12

## Analysis Summary

### Top Functions Call Analysis
| Function | Address | JAL/JALR Calls | Type |
|----------|---------|----------------|------|
| FUN_001b7288 | 0x001b7288 | 0 | Leaf function (stub) |
| FUN_001a6e28 | 0x001a6e28 | 0 | Leaf function |
| FUN_001b0a80 | 0x001b0a80 | 0 | Leaf function |

**Key Finding:** Top called functions are leaf functions - they don't call other functions.

### Address Distribution
| Range | Functions |
|-------|-----------|
| 0x00100000-0x00200000 | 1915 (56%) |
| > 0x00200000 | 1511 (44%) |

**Total:** 3426 functions

### PS2 SDK Observations
1. **No external library calls detected** - Top functions are self-contained
2. **Most code in main executable** - ~56% in 0x00100000-0x00200000 range
3. **Leaf functions dominant** - Functions don't call other functions
4. **No syscall patterns** - No syscall instructions in top functions

### Likely Interpretations
- FUN_001b7288 with 8 callers could be a global variable setter or simple utility
- The game engine may use inline functions rather than library calls
- PS2 syscalls may be accessed via different patterns

## Next Steps
1. Create first proof of concept (simple string/data modification)
2. Explore data sections for strings
3. Identify potential I/O functions by analyzing data references

## Files
- research/elf/ghidra-rev013-library-analysis.md (previous)
- research/elf/ghidra-rev014-ps2-sdk-analysis.md (this)