# ICO USA Environment Setup Plan

> Status: In Progress
> Date: 2026-05-12

## Purpose

This document outlines the environment setup for deeper analysis of the ICO executable using disassemblers and emulators. This is the next logical step after the metadata-only tooling pipeline.

## Context

We have identified:
- 7 functions that reference DATA.DF/DFDATAS addresses
- 13 caller functions that invoke those 7 functions
- 3991 total function prologues
- Key virtual addresses to investigate

The next step requires more powerful tools than our metadata-only scanners.

## Disassembler Options

### Ghidra (Recommended)

**Pros:**
- Free and open source
- Excellent MIPS support
- Decompiler for pseudo-C output
- Scriptable for automation
- Strong community support

**Cons:**
- Requires Java runtime
- May have learning curve

**Download:** https://github.com/NationalSecurityAgency/ghidra

### radare2

**Pros:**
- Lightweight, command-line based
- Very powerful scripting (r2pipe)
- Fast and efficient
- Free and open source

**Cons:**
- Steeper learning curve
- No native decompiler (requires r2frida or similar)

**Download:** https://github.com/radare/radare2

### IDA Pro

**Pros:**
- Industry standard
- Best-in-class decompiler
- Excellent debugging integration

**Cons:**
- Commercial (expensive)
- Not open source

## Emulator Options

### PCSX2 (Recommended for PS2)

**Pros:**
- Mature PS2 emulator
- Debugger capabilities (rername branch)
- Can run actual game for validation
- Active development

**Cons:**
- Not designed for deep reverse engineering

**Download:** https://pcsx2.net/

## Planned Workflow

1. **Import ELF into disassembler**
   - Load SCUS_971.13 (extracted via our tools or direct sector read)
   - Apply MIPS processor module
   - Map known function addresses from our analysis

2. **Analyze known functions**
   - Start with 0x00132630 (most-called function)
   - Examine callers: 0x00138510, 0x00138618, etc.
   - Use string references from exe-ref-index as anchors

3. **Validate with emulator**
   - Run game in PCSX2 debugger
   - Set breakpoints on known function addresses
   - Verify runtime behavior matches static analysis

4. **Iterate and expand**
   - Use discovered information to find more functions
   - Build function library incrementally

## Known Function Addresses to Analyze

| Address | Description |
|---------|-------------|
| `0x001321c8` | References DFDATAS (464 bytes stack) |
| `0x00132630` | Most called function (704 bytes stack) |
| `0x00132ff0` | References DFDATAS (592 bytes stack) |
| `0x00185ca8` | References 0x00633b68 (320 bytes stack) |
| `0x0019fb34` | References 0x006127e8 (112 bytes stack) |
| `0x001a0a38` | References 0x00633b68 (176 bytes stack) |
| `0x0023d468` | References 0x00633b68 (320 bytes stack) |

## Key Strings for Anchoring

From previous analysis:
- `DATA.DF` at virtual address 0x00556a28
- `DFDATAS` at virtual addresses 0x00556a10, 0x00556a20

These can be used to locate functions that use these strings in the disassembler.

## Legal Considerations

- All analysis operates on a user-owned local copy
- No game content is extracted or distributed
- Disassembly notes can be committed as documentation
- No proprietary assets or binaries are shared

## Next Steps

1. Install Ghidra and verify it can load the ELF
2. Create import script to map known function addresses
3. Document initial analysis of 0x00132630 function

(End of file - total 93 lines)