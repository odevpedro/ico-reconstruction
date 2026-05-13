# Video/Rendering Functions Analysis - rev.016

## Date
2026-05-12

## Video/Rendering Functions Identified

### Top Functions by Caller Count (Potential Video/Rendering)

| Rank | Function | Address | Callers | Notes |
|------|----------|---------|---------|-------|
| 1 | FUN_001b7288 | 0x001b7288 | 8 | Most called - likely utility |
| 2 | FUN_001a6e28 | 0x001a6e28 | 4 | Complex function |
| 3 | FUN_001b0a80 | 0x001b0a80 | 3 | Medium function |
| 4 | FUN_0017b230 | 0x0017b230 | 2 | - |
| 5 | FUN_001b1270 | 0x001b1270 | 2 | - |

### .gcm File Strings Found

The following camera/video data files are referenced in the ELF:

| Address | File |
|---------|------|
| 0x00291a50 | camdata/sacrifice.gcm |
| 0x00291a70 | camdata/boss.gcm |
| 0x00291a90 | camdata/boss.gcm |
| 0x00291ab0 | camdata/sacrifice.gcm |
| 0x00291ad0 | camdata/13c4demo.gcm |
| 0x00291af0 | camdata/25a4demo.gcm |
| 0x00291b10 | camdata/NULL.gcm |
| 0x00291b30 | camdata/athletic.gcm |

## Next Steps

### Second Proof of Concept: Modify String in ELF

Target: Change one of the .gcm file strings in the ELF binary.

Options:
1. Change "sacrifice.gcm" to "TEST____.gcm"
2. Change "boss.gcm" to "BOSS____.gcm"
3. Change "NULL.gcm" to "EMPTY__.gcm"

This will test if the binary can be modified and still function.

## Project Progress

- rev.015: First PoC - String extraction
- rev.016: Video/rendering functions identified (this)

## Files
- research/elf/ghidra-rev015-poc-strings.md
- research/elf/ghidra-rev016-video-functions.md (this)