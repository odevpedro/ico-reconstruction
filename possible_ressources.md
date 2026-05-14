# Possible Resources for ICO Reconstruction

This document catalogs external projects, tools, and communities that could be leveraged to advance the ICO reconstruction project. Each entry includes relevance assessment and integration considerations.

---

## 1. ICO Decompilation Projects

### 1.1 RossyDoubleUnderscore/ICO-decomp

**Repository**: https://github.com/rossydoubleunderscore/ico-decomp

**Status**: Active (last push: 2026-01-08)

**Details**:
- 22 stars, 2 contributors
- Targets 1:1 matching decompilation
- Primary language: C (71.5%), Python (20.6%)
- License: CC0-1.0
- Uses structure similar to other PS2 decomp projects (BitBake, build scripts)

**Relevance to this project**: High. This is the most directly relevant external project. The research documented in Rev.023-Re37 (dispatcher resolution, callback registration paths, ROPE descriptor analysis) could potentially complement or benefit from their existing work. No public documentation indicates how far their analysis has progressed.

**Integration considerations**:
- Contact maintainer to assess current focus and whether collaboration is feasible
- The dispatch system at `0x001d37c8` and callback registration at `0x0013f7a8` could be cross-referenced with their symbol table
- Their build system could serve as a template if matching becomes a goal

---

## 2. Decompilation Infrastructure

### 2.1 decomp.me

**URL**: https://decomp.me/

**Platform stats**: 26,210+ PS2 scratches as of 2026

**Details**:
- Collaborative matching platform with web-based diffing
- Presets available for multiple PS2 games including PaRappa the Rapper 2
- Supports MIPS, PowerPC, ARM targets
- API available via decomp-me-mcp for programmatic access

**Relevance to this project**: High. The addresses documented in Rev.023-Re37 could be submitted as scratches:
- `0x001d37c8` - state dispatcher
- `0x001d3a30` - ROPE callback (unresolved registration)
- `0x0013f7a8` - callback registration function
- `0x001b76f8` - registration path with entry table access

**Integration considerations**:
- Create scratches for each function to crowd-source matching
- Use context from Ghidra analysis as input
- The existing 26k PS2 scratches provide compiler settings reference for ICO's specific toolchain

### 2.2 m2c (formerly mips_to_c)

**Repository**: https://github.com/matt-kempster/m2c

**Details**:
- Decompiler targeting MIPS, PowerPC, ARM
- Supports multiple compiler targets: IDO, GCC, MetroWerks CodeWarrior
- Used alongside splat, asm-differ, decomp-permuter in standard workflows

**Relevance to this project**: Medium. Could be used to generate initial C prototypes for documented functions. ICO likely uses Sony's GCC fork or Metrowerks - compiler identification needed first.

**Integration considerations**:
- Run m2c on extracted assembly for functions like `0x001d37c8`
- Compare output against manual analysis from Rev.024 (state block semantics)

### 2.3 Splat

**Repository**: https://github.com/ethteck/splat

**Details**:
- Binary splitting tool for N64, PSX, PS2, PSP
- 319 stars, actively maintained
- Supports PS2 ELF configuration via YAML
- Generates linker scripts and disassembly

**Relevance to this project**: Low to Medium. Currently the project uses manual ELF analysis via Ghidra. Splat could automate function extraction if decompilation becomes the goal.

**Integration considerations**:
- Would require first identifying all function boundaries in SCUS_971.13
- PS2 support includes overlay handling relevant to ICO's DVP system

---

## 3. PS2 Reverse Engineering Tools

### 3.1 Ghidra Emotion Engine Reloaded

**Repository**: https://github.com/chaoticgd/ghidra-emotionengine-reloaded

**Details**:
- 206 stars, actively maintained (latest release: v2.1.34, March 2026)
- Disassembles/decompiles EE-specific instruction sets (MMI, VU0 macro mode)
- Includes STABS Analyzer for .mdebug sections
- Imports PCSX2 save states
- MIPS-R5900 Constant Reference Analyzer

**Relevance to this project**: High. This is the primary tool already in use. The project likely uses this extension. Mentioning for completeness and to track updates.

**Integration considerations**:
- Ensure running latest version for best PS2 support
- STABS analyzer could be tested on ICO ELF if .mdebug section exists

### 3.2 CCC (Chaos Compiler Collection)

**Repository**: https://github.com/chaoticgd/ccc

**Details**:
- 97 stars
- Parses debugging symbols from PS2 games (STABS in .mdebug sections)
- v2.x series handles standard ELF symbols and SNDLL linker symbols
- DWARF support in development

**Relevance to this project**: High if ICO has debug symbols. Could reveal function names, struct layouts, file paths not available in stripped binary.

**Integration considerations**:
- Run on SCUS_971.13 to extract debug info
- Import results into Ghidra via Emotion Engine Reloaded's STABS analyzer

### 3.3 PS2Recomp

**Repository**: https://github.com/ran-j/PS2Recomp

**Details**:
- Static recompiler: MIPS R5900 -> C++
- Supports MMI instructions and VU0 macro mode
- Generates TOML configuration, produces compilable C++ output
- Includes runtime implementation (ps2xRuntime)
- ~3K stars, active development

**Relevance to this project**: Low for current goals. This project focuses on analysis/reconstruction, not native ports. Could be used later for behavioral validation.

**Integration considerations**:
- Could translate specific functions for runtime instrumentation
- Useful for validating dispatcher behavior without PCSX2 breakpoints

---

## 4. PS2 Decompilation Projects (Reference)

These projects serve as reference implementations for PS2 decompilation methodology:

| Project | Stars | Focus |
|---------|-------|-------|
| parappadev/parappa2 | 169 | PaRappa the Rapper 2 |
| TheOnlyZac/sly1 | - | Sly Cooper |
| entriphy/kl2_lv_decomp | 37 | Klonoa 2 |
| Fantaskink/SOTC | 66 | Shadow of the Colossus |
| crash-ps2 | - | Crash Bandicoot: Wrath of Cortex |

**Relevance to this project**: Medium. Reference for:
- Build system structure
- Workflow documentation (see parappa2/docs/decompilation.md)
- PS2-specific compiler flag identification

---

## 5. PS2 Development Ecosystem

### 5.1 PS2SDK / Toolchain

**Repository**: https://github.com/ps2dev/ps2dev

**Toolchain**: https://github.com/ps2dev/ps2toolchain

**Details**:
- Open source toolchain for PS2 homebrew
- Includes EE (Emotion Engine), IOP, DVP compilers
- GCC-based with PS2-specific patches
- Actively maintained (271 stars on toolchain)

**Relevance to this project**: Low for current analysis. Relevant if future decompilation attempts require matching compilation.

### 5.2 PS2 Dev Community

**Discord**: PS1/PS2 Decompilation server (invite via decomp.me)

**Relevance to this project**: Medium. Could provide:
- Direct contact with ICO-decomp maintainer
- Context from researchers who worked on similar Team Ico games (SotC)
- Compiler identification help for ICO's specific toolchain

---

## 6. Assessment Summary

### High Priority
1. Contact ICO-decomp maintainer about potential collaboration
2. Create decomp.me scratches for dispatcher functions
3. Verify if ICO ELF has .mdebug section; run CCC if present
4. Join PS1/PS2 Decompilation Discord

### Medium Priority
5. Study PaRappa the Rapper 2 decompilation docs for methodology
6. Compare Ghidra output with m2c for documented functions
7. Evaluate whether SotC decomp community has Team Ico context

### Low Priority
8. Evaluate PS2Recomp for future behavioral validation
9. Consider splat if project direction shifts to matching decompilation

---

## 7. Next Steps

Before pursuing integration with external resources, the following would provide better context:

1. **Verify compiler**: Determine whether ICO uses Sony's GCC fork or Metrowerks CodeWarrior (affects matching approach)
2. **Check for debug symbols**: Run CCC on SCUS_971.13 to see if .mdebug section contains useful information
3. **Document current scope**: Create a summary of what has been verified vs. what remains hypothesis

This will enable more targeted questions when reaching out to external projects.

---

*Document generated: 2026-05-13*
*Based on web research and project context from AGENTS.md and Rev.023-Re37*