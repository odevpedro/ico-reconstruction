# Ghidra Disassembly Analysis - rev.012

## Analysis Date
2026-05-12

## Top Functions Disassembly Analysis

### FUN_001b7288 (Address: 0x001b7288)
- **Callers:** 8
- **Size:** 8 addresses (small function)

**Instructions:**
```
001b7288: jr ra              # Return immediately
001b728c: _sw a0,-0x578c(gp) # Store $a0 to global offset
001b7298: addiu sp,sp,-0x40  # Stack adjustment
001b729c: li v0,0x4          # Load immediate
001b7408: addiu sp,sp,-0x60  # Stack adjustment
001b76f8: addiu sp,sp,-0x130 # Stack adjustment
001b76fc: li v1,0x4c        # Load immediate
001b7f20: addiu sp,sp,-0x50  # Stack adjustment
...
```

**Analysis:** Small stub/wrapper function that stores parameter $a0 to a global variable. Called frequently (8x) - likely a global state setter or debug function.

---

### FUN_001a6e28 (Address: 0x001a6e28)
- **Callers:** 4
- **Size:** 4 addresses (but spans multiple code paths)

**Instructions:**
```
001a6e28: addiu sp,sp,-0x80  # Allocate 128 bytes stack
001a6e50: addiu sp,sp,-0xf0  # Allocate 240 bytes stack
001a7008: addiu sp,sp,-0x70  # Allocate 112 bytes stack
001a76b8: addiu sp,sp,-0xd0  # Allocate 208 bytes stack
001a76bc: li v0,0x128        # Load immediate 296
001a7ad0: addiu sp,sp,-0x40  # Allocate 64 bytes stack
001a7ad4: lui v0,0x27        # Load upper immediate
001a7bb0: addiu sp,sp,-0x1a0 # Allocate 416 bytes stack
001a7bb4: lui t0,0x61        # Load upper immediate
001a7df0: addiu sp,sp,-0x50  # Allocate 80 bytes stack
001a7df4: daddu a3,a0,zero   # Copy $a0 to $a3
001a7f20: addiu sp,sp,-0x70  # Allocate 112 bytes stack
001a7f24: lw a1,-0x5db8(gp)  # Load from global pointer
001a80d0: addiu sp,sp,-0x50  # Allocate 80 bytes stack
001a80d4: lw v1,-0x5d9c(gp)  # Load from global pointer
...
```

**Analysis:** Larger function with complex stack management. Accesses global data via $gp. Could be a memory allocation, file I/O, or data processing function.

---

### FUN_001b0a80 (Address: 0x001b0a80)
- **Callers:** 3
- **Size:** 24 addresses (medium function)

**Analysis:** Medium-sized function, needs more instructions to analyze.

---

## Known Function Summary

| Address | Function | Callers | Notes |
|---------|----------|---------|-------|
| 0x001b7288 | FUN_001b7288 | 8 | Global variable setter/stub |
| 0x001a6e28 | FUN_001a6e28 | 4 | Complex function, global data access |
| 0x001b0a80 | FUN_001b0a80 | 3 | Medium function |
| 0x001321c8 | FUN_001321c8 | 0 | Unknown |
| 0x00132ff0 | FUN_00132ff0 | 0 | Unknown |
| 0x001a0a38 | FUN_001a0a38 | 0 | Unknown |
| 0x0023d468 | FUN_0023d468 | 1 | Unknown |

## Next Steps
1. Cross-reference with PS2 SDK documentation
2. Identify patterns for file I/O, memory allocation
3. Search for string references in these functions
4. Consider using decompiler for higher-level analysis

## Files Updated
- research/elf/ghidra-function-analysis-rev012.md (created)
- research/elf/ghidra-disassembly-analysis-rev012.md (this file)