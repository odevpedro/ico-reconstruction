# Rev.132 — Function-boundary audit: canonical `.s` truncation, root cause of phantom inflation, TARGET realignment

Date: 2026-09-05

## Objective

Audit every `.s` in `src/{entity,cloth,core}/asm/` (710 files) against the USA
ELF to detect the Rev.131 audit class of bug: a `.s` captured by scanning for
the *first* `jr $ra` can over-run the real function end when the function
finishes with a **tail-jump** (`j 0x...`) instead of `jr $31`, silently
swallowing the NEXT function into the same `.s` file.

## Scope

| Directory | Files |
|-----------|-------|
| `src/entity/asm/` | 658 |
| `src/cloth/asm/` | 6 |
| `src/core/asm/` | 46 |
| **Total** | **710** |

## Sources used

- `.local/extracted/SCUS_971.13.elf` — USA ELF (target binary)
- `.local/reports/elf_real_prologues.json` — 3846 canonical prologue starts
- `tools/asm_source_score.py` — `TARGET_FUNCTIONS` table, `assemble_and_verify`
- ee-gcc 2.9-991111-01 (`toolchain/ee-gcc2.9-991111-01/bin/ee-gcc`)
- capstone (MIPS64, little-endian) for cross-checks

## Root cause of the phantom "+12 bytes" inflation

**Hypothesis RETRACTED.** The earlier working theory that "3 .s files fail
byte-exact verification because ee-gcc 2.9 inflates `mult` by 1 instruction
and synthesizes `cvt.w.s`/`cvt.s.w`" is **WRONG**.

The real cause was a **regex bug in the eager truncation script itself**:

```python
# broken (single-dot)
INSN = re.compile(r"^\t(?:\w+\.?\w*|\.word)\t")
```

This regex does NOT match multi-dot mnemonics like `cvt.w.s` and `cvt.s.w`
(`\w+\.?\w*` matches at most one dot). Any line carrying such a mnemonic fell
through the "non-instruction" else-branch and was **re-appended** when the
truncated source was rebuilt, silently re-inserting up to 3 real instructions
→ phantom `cvt.w.s / cvt.s.w / cvt.w.s` bytes after `assemble_and_verify`.

**Fixed regex (multi-dot):**

```python
INSN = re.compile(r"^\t(?:\w+(?:\.\w+)*|\.word)\t")
```

### Consequence of the fix

The 3 files previously reported as "divergent" were **never inflated**. After
the regex fix, `canonical_end` lands at their natural function end; they
rewrite identically (empty git diff) and verify byte-exact at their original
sizes:

| File | VA | Size | Instructions |
|------|-----|------|--------------|
| `type23_hB.s` | 0x1BC130 | 772 | 193 |
| `eBrainGetTargetGeneratorFromLabelStage.s` | 0x191B70 | 432 | 108 |
| `boy_hB.s` | 0x1C1DD8 | 704 | 176 |

The claimed ee-gcc `mult`/`cvt` inflation was an artifact of the script regex,
not of the assembler. GNU-as 2.9 assembles these one-shot, 1:1.

## Canonical boundary method (final)

For each `.s`:

1. **Recover VA** (priority):
   - internal `loc_XXXXXXXX:` anchors (label VA = base + 4*insn_index);
   - trailing 6-hex-digit in file name (e.g. `boyAI_sub_1446E0`);
   - `TARGET_FUNCTIONS` table in `tools/asm_source_score.py`;
   - byte fingerprint (assemble first ~16 instructions, search the ELF load
     image for a unique exact-byte match).
2. **Count instructions** with the multi-dot regex.
3. **Compute canonical end**:

   ```text
   block  = va + 4 * count_insns(body)
   last   = addr(last function-level terminator)
            where terminator = jr/jalr $31
            or `j tgt` with tgt outside [va, block)
   heuristic = last + 8
   bound     = next canonical start > va (prologue set)
   canonical = min(heuristic, bound) if either exists
   ```

   Two fixes over the earlier heuristic:
   - **out-of-block tail-jump**: the old fixed 0x200000 band classified far
     in-ELF tails (e.g. `j 0x2641d8` from `bossctrl_hB`) as in-block and never
     recognised them as terminators;
   - **min-bounded by next canonical start**: prevents over-running into a
     following prologue-less leaf (e.g. `attackch62_hC` → next function at
     0x1BBEA0 starts straight with `lw $v0,0x15c($a0)`).

4. **Classify**: `exact` / `INFLATED` (size > canonical) / `truncated`
   (size < canonical) / `no-terminator` / `misfiled-no-va`.

## Files corrected byte-exact (12 truncations)

All written via `assemble_and_verify` (0 tolerance) at canonical VA/size:

| File | VA | Size | TARGET old → new |
|------|-----|------|------------------|
| `src/entity/asm/enemy1_hB.s` | 0x1CE3C0 | 0x238 (568) | 0x240 → 0x238 |
| `src/cloth/asm/rope_hC.s` | 0x1D3B28 | 0xC8 (200) | 0x108 → 0xC8 |
| `src/entity/asm/attackch62_hC.s` | 0x1BBE50 | 0x50 (80) | 0x100 → 0x50 |
| `src/entity/asm/bossctrl_hB.s` | 0x197FC8 | 0x34 (52) | 0x40 → 0x34 |
| `src/entity/asm/girlBrain_sub_16F964.s` | 0x16F964 | 0x44 (68) | 0x18C → 0x44 |
| `src/entity/asm/boyAI_sub_1446E0.s` | 0x1446E0 | 0x08 (8) | 0xD0 → 0x08 |
| `src/entity/asm/eBrainInit.s` | 0x1918A8 | 0x44 (68) | 0x50 → 0x44 |
| `src/entity/asm/lightbit_hA.s` | 0x1F0550 | 0x14 (20) | 0xB4 → 0x14 |
| `src/entity/asm/lightbit_hB.s` | 0x1F0540 | 0x24 (36) | 0xC4 → 0x24 |
| `src/core/asm/iosOmInit.s` | 0x13F9A0 | 0x2C (44) | (not in TARGET) |
| `src/entity/asm/GeneratorDL.s` | 0x193600 | 0x130 (304) | (not in TARGET) |
| `src/entity/asm/boyAI_sub_1446E0_2.s` | 0x1446E0 | 0x08 (8) | (not in TARGET) |

`attackch62_hC` was **re-cut to 0x50**, not the naive `last-term+8` value of
0x74: disassembly showed the next function at 0x1BBEA0 is a prologue-less leaf
(`lw $v0,0x15c($a0)`), so the truncated file would otherwise have swallowed
that too.

Disassembly cross-checks of every corrected tail (all end in a clean
`jr $31`+delay-slot epilogue or an out-of-block `j` tail):

```asm
rope_hC:      jr $ra   | addiu $sp,$sp,0x40
enemy1_hB:    jr $ra   | addiu $sp,$sp,0x70
girlBrain_16F964: jr $ra | addiu $sp,$sp,0x1A0
bossctrl_hB:  j 0x2641d8 | addiu $a2,$zero,0xd50
iosOmInit:    j 0x1fc2d0 | addiu $sp,$sp,0x10
boyAI_1446E0: jr $ra   | sw $v1,4($a2)
lightbit_hA:  j 0x1f0260 | addiu $a1,$a1,0x10
lightbit_hB:  j 0x1f0260 | addiu $a1,$a1,0x10
```

## TARGET_FUNCTIONS realignment

`tools/asm_source_score.py` TARGET entries for the 9 in-pipeline files updated
below. Pipeline re-runs clean: **612/612 direct, 0 regeneration** (before the
fix, 9 on-disk .s failed at the stale size and were silently regenerated).

## What is confirmed

- 12 `.s` now byte-exact at canonical boundaries (assemble + verify, 0 bytes
  residual).
- The phantom-inflation root cause was the script regex, not ee-gcc 2.9.
- The pipeline is fully consistent: 612/612 direct PASS.

## What is probable

- The remaining `INFLATED=197` / `truncated=340` audit flags are a mixed
  bag (trailing padding `nop`, multi-function merges, prologue-less leaves,
  `no-terminator` tail-jump files); each needs per-file disassembly before
  any `--write`. **Do NOT mass-truncate.**

## What is discarded

- "ee-gcc 2.9 inflates `mult` / synthesizes `cvt`" (Rev.13x working theory) —
  retracted as a script-regex artifact.
- `attackch62_hC` at 0x74 — over-included the following prologue-less leaf.

## Next minimum test

Run `python3 tools/asm_source_score.py --all --no-save` → expect
`Success: 612/612`, `Failed: 0/612`, zero "regenerating" lines.

## Conservative verdict

Byte-exact `.s` corpus is healthy; 12 boundary corrections are verified.
The function-boundary audit tool is a flagger, not ground truth: canonical
`min(term+8, next-start)` is sound only where the next function has a
recognized prologue; prologue-less leaf boundaries require manual disassembly.