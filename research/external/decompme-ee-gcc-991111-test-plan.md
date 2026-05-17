# Test Plan — ee-gcc 2.9-991111-01 via decomp.me

**Date:** 2026-05-16  
**Objective:** Confirm that the ee-gcc 2.9-991111-01 compiler package on decomp.me produces bit-identical output for ICO functions, and establish the correct workflow for future matching.  
**Scope:** decomp.me scratch upload only. No local compiler rebuild, no ISO extraction, no C code changes.  

---

## Background

Rev.038 concluded that "decomp.me has no ee-gcc compiler packages". This was incorrect. The ee-gcc 2.9-991111-01 (Sony R5900-patched GCC 2.95.2) is available on decomp.me through game-specific presets, notably:

- **Klonoa 2** (uses ee-gcc for PS2)
- **PaRappa the Rapper 2** (uses ee-gcc for PS2)

The confusion arose because the generic PS2 preset on decomp.me uses Metrowerks `mwcps2`, not ee-gcc. Only game-specific presets expose the ee-gcc compiler package.

The project's local toolchain situation:

| Toolchain | Version | Purpose | For matching? |
|-----------|---------|---------|--------------|
| ps2dev (modern) | GCC 15.2.0 | Local tooling, splat, build system | No (different codegen) |
| PS2 Linux GCC | GCC 2.95.2 (patched) | Local approximation | Possible but unverified |
| decomp.me ee-gcc | GCC 2.9-991111-01 | **Primary matching target** | To be tested |

---

## Test Procedure

### Prerequisites

- Existing scratch packages in `/tmp/decompme_scratches/` from Rev.038 (6 functions)
- A function with EXACT status from previous testing (0x1D3D70, 0x1D3D80, or 0x1D3D98)
- A decomp.me account capable of selecting game-specific compiler presets

### Step 1: Select target function

Pick a function already confirmed as **EXACT** (bit-identical with ps2dev GCC 15.2.0):

| Function | VA | Status | Size |
|----------|-----|--------|------|
| `func_001D3D70` | 0x1D3D70 | EXACT (Rev.048) | ~20 insns |
| `func_001D3D80` | 0x1D3D80 | EXACT (Rev.048) | ~8 insns |
| `func_001D3D98` | 0x1D3D98 | EXACT (Rev.048) | ~12 insns |

Prefer the largest function for a more discriminating test.

### Step 2: Upload scratch to decomp.me

1. Navigate to decomp.me
2. Create new scratch
3. Select game preset: **Klonoa 2** (or **PaRappa the Rapper 2**)
4. Verify the compiler shows `ee-gcc 2.9-991111-01`
5. Paste the C source from `src/cloth/near_matches.c` for the chosen function
6. Paste the MIPS assembly from the scratch `.s` file
7. Set compiler flags:
   ```
   -march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2
   ```
8. Submit

### Step 3: Evaluate result

| Outcome | Action |
|---------|--------|
| **MATCH** (100%) | Compiler confirmed. Update project compiler to `ee-gcc 2.9-991111-01` via decomp.me. Begin submitting ASM-HOLD functions for crowd matching. |
| **NEAR-MATCH** (>80%) | Check flags. Try flag variations: remove `-msingle-float`, try `-msoft-float`, try `-G8` instead of `-G0`. Re-submit. |
| **NO MATCH** (<50%) | Either the decomp.me preset uses different patches, or the flags are wrong. Cross-check with SOTC compiler packaging. Try different game presets. |

### Step 4: Document result

Write a research note (`research/external/decompme-ee-gcc-991111-result.md`) with:

- Date, compiler preset used, game preset used
- Function tested (VA, name, size)
- Match percentage
- Compiler flags used
- Any flag variations attempted
- Verdict: COMPILER CONFIRMED / COMPILER REJECTED / NEEDS INVESTIGATION

---

## Compiler Flag Reference

From ICO-decomp splat YAML and byte-level analysis:

| Flag | Value | Evidence |
|------|-------|----------|
| `-march` | `r5900` | R5900-specific instructions (LQ, SQ, etc.) |
| `-mips3` | `mips3` | MIPS III ISA (64-bit, but ICO uses mostly 32-bit) |
| `-mgp64` | `gp64` | 64-bit GPR, gp-relative addressing |
| `-mabi` | `eabi` | EE Embedded ABI |
| `-msingle-float` | `single-float` | No double-precision in FPU |
| `-G0` | `0` | No gp-relative for objects >0 bytes |
| `-O2` | `O2` | Optimization level 2 |

If matching fails, try these variations:

1. Remove `-msingle-float` (some PS2 games omit this)
2. Try `-G8` instead of `-G0`
3. Try `-mno-check-zero-division`
4. Try `-fno-delayed-branch`

---

## Risk Assessment

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| ee-gcc preset has different patches | Medium | Medium | Try multiple game presets; compare compiler version metadata |
| Flags are wrong | High | Low | Cross-reference with ICO-decomp splat YAML; try variations |
| decomp.me platform limitation | Medium | Low | Alternate approach: build ee-gcc 2.9-991111-01 locally from PS2 Linux SDK |
| No game preset exposes correct compiler | High | Low | Fall back to PS2 Linux GCC 2.95.2 local build |

---

## Non-goals

- Do not rebuild or patch the compiler locally
- Do not create non-decomp.me test harnesses
- Do not modify C source files during this test
- Do not extract game assets or ISO contents
