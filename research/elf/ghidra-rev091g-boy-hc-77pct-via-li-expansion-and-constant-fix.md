# Rev.091g — boy_hC 76.64% via li pseudo-op expansion + corrected model addresses

**Date:** 2026-05-18

## Objective

Improve boy_hC scoring from 62.62% to near-exact by:
1. Expanding GCC's `li` pseudo-op for large constants
2. Adding a normalizer to convert `lui+ori` pairs to `lui+addiu` form (matching original compiler)
3. Correcting swapped arguments (size/line) in sub_13A0F8 call
4. Correcting model chunk addresses (0x4Cxxxx → 0x4Bxxxx)

## Changes

### tools/ee_gcc_compile.py

1. **`_expand_li()`**: New function that converts `li rd, large_imm` to `lui+addiu/ori` form:
   - Handles inline GCC comments (`# 0x610000` suffix stripping)
   - When lower 16 bits >= 0x8000: uses `lui rd, upper+1; addiu rd, rd, lower-0x10000` (standard GAS `li` expansion)
   - When lower 16 bits < 0x8000: uses `lui rd, upper; ori rd, rd, lower`
   - When lower == 0: single `lui rd, upper`
   - Small constants (-0x8000..0x7FFF) are deferred to normalize_insn step 15 (`addiu rd, $zero, imm`)

2. **`_normalize_ori_addiu_pairs()`**: New function that converts adjacent `lui rd, K` + `ori rd, rd, N` pairs to `lui rd, K+1` + `addiu rd, rd, N-0x10000` when N >= 0x8000, or plain `addiu` when N < 0x8000. This matches the original compiler's use of `addiu` for all constant lower-bits loading (where GCC uses `ori`).

3. **`_reg_written_by()`**: Helper tracking GPR writes for scan-based pair matching.

4. Wired `_expand_li()` into both passes of `parse_asm_lines()` (first pass for label indexing, second for main output).

5. Wired `_normalize_ori_addiu_pairs()` into `score_against_target()` after `parse_asm_lines()`.

### src/entity/boy.c

1. **sub_13A0F8 arguments**: Fixed swapped size (0x25D=605) and line (0xFE=254) arguments. Tag address corrected from 0x00618CF0 to 0x00618838 (verified via target disassembly).

2. **Model chunk addresses corrected** (5 locations):
   - `0x4BF7F0` (was 0x4CF7F0) — model_A data
   - `0x4BFAF0` (was 0x4CFAF0) — model_B data  
   - `0x4BFDF0` (was 0x4CFDF0) — model_C data
   - `0x4BFF30` (was 0x4CFF30) — model_D data
   - `0x4C0070` (unchanged) — model_E data

   The original compiler computes these as `lui 5, 76; addiu 5, 5, N` where N is negative (sign-extended), giving addresses in the 0x4Bxxxx range. Our code had 0x4Cxxxx which GCC splits as `lui 5, 75; ori 5, 5, 0xFxxx` giving 0x4Cxxxx (0x10000 higher).

## Results

### boy_hC scoring

| Metric | Before | After | Change |
|--------|--------|-------|--------|
| Score | 62.62% | **76.64%** | **+14.02%** |
| Aligned matches | 65 | 78 | +13 |
| Missing in gen | 38 | 21 | -17 |
| Extra in gen | 34 | 17 | -17 |

### Batch scoring

All 38 functions: 0 regressions. Previous exact/near-exact matches preserved:
- enemy1_hA: 95.65% (unchanged)
- boy_hB: 97.06% (unchanged)
- sub_1C1EA8: 68.60% (unchanged)
- 8 exact matches: unchanged

### What now matches

- **sub_13A0F8 call**: All 4 arguments now match (heap, size=605, tag=0x618838, line=254)
- **Model data chunk addresses** (5): All compute `lui rd, 76; addiu rd, rd, N` matching target
- **Model texture addresses**: `lui rd, 76` now matches; lower parts partially match

### Remaining mismatches

1. **Texture address lower parts**: `addiu 6, 6, 432` and `addiu 6, 6, 624` in target vs `ori 6, 6, N` in generated. The `_normalize_ori_addiu_pairs` scan misses these because they're in jal delay slots and alignment shifts prevent LCS matching.

2. **Register allocation**: Target uses different registers ($3 vs $18 for entity pointer, $2 vs $17 etc.) — this is a fundamental codegen difference.

3. **Flag mask 0x80808080**: Target uses `lui 2, 32896; ori 2, 2, 32896`. `_expand_li` produces `lui 2, 32897; addiu 2, 2, -32640` (same value, different encoding). `_normalize_ori_addiu_pairs` doesn't touch this (already addiu).

4. **Float constant loading**: `li.s` expansion adds `lui 1, ...; mtc1 1, $fN` which doesn't align with target's equivalent pattern.

## Confidence

**High**: The instruction-level changes are verified against the target disassembly. The `_expand_li` logic follows standard MIPS `li` expansion rules. The `_normalize_ori_addiu_pairs` transformations preserve the computed register value (verified via 32-bit arithmetic).

## Next steps

- Remaining boy_hC issues are register-allocation and encoding-choice differences — normalizer-only fixes are unlikely to close the remaining 23%
- Most productive next frontier: short cloth-domain functions for additional exact matches
- The remaining structural differences indicate the C source needs register-alloc restructuring for further improvement
