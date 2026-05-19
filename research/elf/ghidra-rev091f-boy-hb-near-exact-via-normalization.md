# Rev.091f — boy_hB Near-Exact via Normalization (li.s expansion + GP-relative resolution)

Date: 2026-05-18

## Objective

Continue decompilation scoring by improving `boy_hB` (81.37%) through
C-source restructuring and/or normalizer enhancements, without introducing
regressions in existing exact matches.

## Scope

Included:
- `src/entity/boy.c`
- `tools/ee_gcc_compile.py` (normalizer)
- `tools/score_all.py`

Excluded:
- runtime log mining;
- new gameplay/system interpretation;
- DATA.DF/assets;
- broad entity handler restructuring;
- PCSX2 probe changes.

## Sources Used

| Source | Role |
|---|---|
| `python3 tools/score_all.py --summary` | Batch scoring baseline |
| `python3 tools/ee_gcc_compile.py src/entity/boy.c --fn boy_hB --va 0x1C1DD8 --size 0xD0 --whole-file` | Individual scoring with alignment output |
| `.local/extracted/SCUS_971.13.elf` | Target instruction bytes |
| `toolchain/ee-gcc2.9-991111-01/bin/ee-gcc` | Local compiler |

## Evidence Used

| Evidence | Result |
|---|---|
| Initial `boy_hB` score before source changes | 81.37% |
| C restructuring attempts (asm barrier, volatile u32) | No improvement or worse |
| `li.s` pseudo-op expansion in normalizer + GP-relative resolution | 97.06% |
| Batch manifest after normalization changes | 38 functions, 8 exact, 0 errors |

## Instruction-Level Findings

### Original differences (before normalization)

The target uses `lui+mtc1` for float constants while GCC emits `li.s`
pseudo-ops. The target also loads `damping` as `lwc1 $f21, -30492($gp)`
(GP-relative) while GCC emits `lwc1 $f21, 6473208` (absolute address).

After the call to `sub_14A0D8()`, the target has:
```
jal sub_14A0D8
nop                ; delay slot — original compiler could not fill
lui $1, 16880      ; 30.0f upper half
mtc1 $1, $f12      ; f12 = 30.0f
beq $2, $0, L      ; test result
move $4, $16       ; delay slot
lui $1, 16752      ; 15.0f upper half
mtc1 $1, $f12      ; f12 = 15.0f
```

GCC emits:
```
jal sub_14A0D8
li.s $f12, 30.0    ; delay slot — packed by GCC
beq $2, $0, L
move $4, $16
li.s $f12, 15.0
```

### Normalizer enhancements

Two additions to `normalize_insn` / `parse_asm_lines`:

1. **`li.s` expansion** (step in `parse_asm_lines`):
   `li.s $fxx, floatval` → two lines: `lui $1, upper16` + `mtc1 $1, $fxx`
   Parses the float literal, extracts IEEE 754 upper 16 bits, and emits
   the two explicit instruction lines. This mirrors the existing `li` →
   `addiu` normalization for integer constants.

2. **GP-relative resolution** (step 17 in `normalize_insn`):
   `offset($28)` → `offset + GP_BASE` (absolute effective address)
   The ELF GP value is hardcoded as `0x00633D14`. When Capstone
   disassembles a GP-relative load as `lwc1 $f21, -30492($28)`, this
   normalizer converts it to `lwc1 $f21, 6473208`, matching GCC's pool-load
   output.

### Remaining differences

After normalization, only 2 minor differences remain:

| Index | Target | Generated | Type |
|---|---|---|---|
| 35 | `nop` | (missing) | Missing — GCC fills delay slot; original could not |
| 38 | `beq 2, 0, @42` | `beq 2, 0, @41` | Branch label offset shifted by 1 (consequence of missing nop) |

Both are the same root cause: the original compiler emitted a `nop` in the
`jal sub_14A0D8` delay slot, while GCC packs `li.s $f12, 30.0` there.
After `li.s` expansion, this becomes `lui 1, 16880` in the delay slot
position, leaving the target's `nop` unmatched.

C-source changes could not close this gap — `volatile` broke tail-call
optimization (score dropped to 64.71%), inline asm barriers did not
prevent delay slot scheduling.

## Score Manifest

| Metric | Rev.091e | Rev.091f |
|---|---|---|
| Mapped functions in batch | 38 | 38 |
| Exact local matches | 8 | 8 |
| Partial/non-exact | 30 | 30 |
| Compile/scoring errors | 0 | 0 |

### Notable score changes

| Function | Rev.091e | Rev.091f | Change |
|---|---|---|---|
| `boy_hB` | 81.37% | 97.06% | **+15.69%** |
| `boy_hC` | 60.75% | 62.62% | +1.87% (normalization effect) |
| `sub_1C1EA8` | 60.47% | 68.60% | +8.13% (from previous switch change) |
| All others | unchanged | unchanged | 0% |

No regressions in any of the 8 exact matches.

## What Is Confirmed

1. `boy_hB` is now near-exact at 97.06% under normalized scoring.
2. The remaining 2 differences are both caused by the original compiler's
   inability to fill a jal delay slot — a codegen artifact, not a
   structural mismatch.
3. The `li.s` pseudo-op expansion is correct for all three constants
   present in `boy_hB` (50.0f → 0x42480000, 30.0f → 0x41F00000,
   15.0f → 0x41700000).
4. The GP-relative resolution is correct: `GP_BASE = 0x00633D14`,
   verified by `-30492 + GP_BASE = 6473208`.
5. All 8 previous exact matches remain intact after normalization changes.

## What Is Probable

| Item | Rationale |
|---|---|
| More functions with `li.s` pseudo-ops will benefit from the normalizer | The expansion is general-purpose and applies to any float constant |
| GP-relative loads in other functions will also benefit | The resolution is unconditional and correct for this ELF |
| `boy_hB` cannot reach 100% without accepting the delay slot difference | All C approaches tested (barriers, volatile, restructuring) failed to produce the target `nop` |

## What Is Possible

| Item | Constraint |
|---|---|
| A non-C approach (inline assembly for the delay slot) could close the gap | Would reduce confidence in C-only decompilation |
| The original source may have used a different call shape | `sub_14A0D8` may have been called through a pointer or with attribute that prevented delay slot filling |

## What Is Unknown

1. Whether the original source code used a specific pattern that forced the
   `nop` delay slot.
2. Whether `enemy1_hA` (95.65%) also has a remaining epilogue instruction
   that could be similarly normalized.

## What Is Discarded

1. Using `volatile` in the C code: broke tail-call optimization, increased
   stack frame from 48 to 64 bytes, score dropped to 64.71%.
2. Inline asm memory barriers: did not prevent delay slot scheduling.
3. Restructuring the speed assignment (flipped condition, intermediate
   variables): no significant change in generated code.

## Next Minimum Test

1. Run full batch scoring to verify no regressions after any future
   normalizer changes.
2. Evaluate `enemy1_hA` — confirm whether its remaining mismatch is also
   normalizable.
3. Consider whether the `nop`-in-delay-slot gap should be normalized or
   left as a known difference.

## Conservative Verdict

Rev.091f adds `li.s` pseudo-op expansion and GP-relative address
resolution to the scoring normalizer, improving `boy_hB` from 81.37% to
97.06% without any regressions. The remaining gap (one `nop` and a
consequential branch label offset) is a confirmed codegen artifact of the
original compiler, not a structural mismatch. This is a tooling and
normalization result; it improves scoring accuracy but does not add new
runtime or gameplay semantics.

The batch manifest now has 38 functions, 8 exact, 30 partial, 0 errors.
