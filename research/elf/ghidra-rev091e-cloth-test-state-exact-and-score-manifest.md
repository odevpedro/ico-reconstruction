# Rev.091e — cloth_test_state_lt_2 Exact Match and Score Manifest Update

Date: 2026-05-18

## Objective

Continue the Rev.091d local scoring step by:

1. rerunning the score manifest baseline;
2. testing whether the remaining `enemy1_hA` epilogue mismatch could be closed cleanly;
3. moving to the next short cloth accessor/test if `enemy1_hA` remained blocked;
4. recording any new exact local closure in the manifest.

## Scope

Included:

- `src/entity/enemy1.c`
- `src/cloth/near_matches.c`
- `tools/ee_gcc_compile.py`
- `tools/score_all.py`
- `decompme_submissions/`

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
| `python3 tools/ee_gcc_compile.py src/entity/enemy1.c --fn enemy1_hA --va 0x1CE690 --size 0x60` | `enemy1_hA` near-exact verification |
| `python3 tools/ee_gcc_compile.py src/cloth/near_matches.c --fn cloth_test_state_lt_2 --va 0x1D3D40 --size 0x30` | New short cloth target scoring |
| `.local/extracted/SCUS_971.13.elf` | Target instruction bytes |
| `toolchain/ee-gcc2.9-991111-01/bin/ee-gcc` | Local compiler |

## Evidence Used

| Evidence | Result |
|---|---|
| Initial `enemy1_hA` score | `2200/2300 = 95.65%` |
| `enemy1_hA` generated extra instruction | `ld $31,16($sp)` after inline tail-jump block |
| C call / `noreturn` alias attempts | Worse score; generated `jal fn_1CE5F8`, not target tail `j fn_1CE5F8` |
| `cloth_test_state_lt_2` after source scheduling fix | `1200/1200 = 100.00%` |
| Updated batch manifest | `38` functions, `8` exact, `30` partial, `0` errors |

## Byte-Level or Instruction-Level Findings

### `enemy1_hA`

The corrected target range remains:

| Function | VA | Size | Status |
|---|---:|---:|---|
| `enemy1_hA` | `0x001CE690` | `0x60` | Near-exact, `95.65%` |

The remaining generated-only instruction is:

```asm
ld $31,16($29)
```

It appears after the inline assembly block that restores `$16`, jumps to
`fn_1CE5F8`, and adjusts `$sp` in the branch delay slot. This is compiler
fallthrough cleanup for a path that is unreachable at runtime because the
inline block performs a tail jump.

Two cleaner C attempts were rejected:

| Attempt | Result |
|---|---|
| Plain `fn_1CE5F8(entity)` call | Generated `jal fn_1CE5F8`, missing target tail-jump epilogue |
| `fn_1CE5F8` alias marked `noreturn` | Still generated `jal fn_1CE5F8`, not the target `j` sequence |

### `cloth_test_state_lt_2`

The target instruction sequence is now matched locally:

| Index | Instruction |
|---:|---|
| 0 | `lw $2,348($4)` |
| 1 | `move $5,$zero` |
| 2 | `lw $3,364($4)` |
| 3 | `beq $3,$zero,@10` |
| 4 | `lw $4,2048($2)` |
| 5 | `ld $2,8($4)` |
| 6 | `bne $2,$zero,@10` |
| 7 | `nop` |
| 8 | `lw $2,72($4)` |
| 9 | `slti $5,$2,2` |
| 10 | `jr $31` |
| 11 | `move $2,$5` |

The C-side changes that closed it were:

| Change | Effect |
|---|---|
| Preload `payload` before reading `extra` | Forces `lw entity`, `move result=0`, `lw extra`, then branch-delay payload load |
| Add explicit `nop` before reading `payload+0x48` | Preserves the target `bne` delay-slot `nop` |
| Normalize `slt rd,rs,N` to `slti rd,rs,N` | Accounts for GAS immediate pseudo-op rendering |

## Score Manifest

| Metric | Rev.091d | Rev.091e |
|---|---:|---:|
| Mapped functions in batch | 37 | 38 |
| Exact local matches | 7 | 8 |
| Partial/non-exact | 30 | 30 |
| Compile/scoring errors | 0 | 0 |

New exact:

| Function | VA | Size | Score |
|---|---:|---:|---:|
| `cloth_test_state_lt_2` | `0x001D3D40` | `0x30` | `100.00%` |

## What Is Confirmed

1. `enemy1_hA` remains near-exact at `95.65%` with the corrected `0x60` range.
2. The remaining `enemy1_hA` mismatch is caused by one generated fallthrough epilogue instruction after the inline tail-jump path.
3. Clean C call shapes tested so far do not reproduce the target `j fn_1CE5F8` tail sequence.
4. `cloth_test_state_lt_2` is now an exact local match.
5. The batch manifest now tracks `38` functions, with `8` exact local matches and `0` compile errors.

## What Is Probable

| Item | Rationale |
|---|---|
| Closing `enemy1_hA` cleanly may require either heavier inline assembly or a different source shape not yet found | GCC 2.9 keeps generating a reachable epilogue or a `jal` call |
| More short cloth helper functions are good next targets | The recent exact closures all came from small cloth predicates/accessors with scheduling differences |
| `slt` with immediate in generated assembly is a GAS pseudo-op for `slti` | Normalization makes the generated and target instruction streams match exactly |

## What Is Possible

| Item | Constraint |
|---|---|
| `enemy1_hA` can be forced to 100% with full assembly | That would be less useful as C decompilation evidence |
| A cleaner C-only `enemy1_hA` form exists | Not found in this pass |
| Decomp.me may score `cloth_test_state_lt_2` differently | Only relevant if its compiler package differs from the local package |

## What Is Unknown

1. Whether the downloaded decomp.me compiler package is byte-identical to the local package used by `tools/ee_gcc_compile.py`.
2. Whether `enemy1_hA` can be closed without replacing the function with assembly.
3. Which short cloth predicate/accessor is the next best target after `cloth_test_state_lt_2`.

## What Is Discarded

1. Treating `__builtin_unreachable()` as available in this ee-gcc build: it compiled as a call, not as unreachable control-flow metadata.
2. Treating a `noreturn` alias to `fn_1CE5F8` as sufficient for the target tail jump: the compiler still emitted `jal`.
3. Pursuing `cloth_dispatcher` or `clothSubForceApply` as the immediate short target: their diffs remain large and structural.

## Next Minimum Test

1. Record the local compiler package identity with hashes if needed for auditability.
2. Search for the next short cloth predicate/accessor adjacent to the known exact range.
3. Revisit `enemy1_hA` only if a source shape can remove the fallthrough epilogue without replacing the function with full assembly.

## Conservative Verdict

Rev.091e adds one new local exact match and updates the batch manifest from
`37/7 exact` to `38/8 exact`. `enemy1_hA` remains near-exact, and the exact
closure came from the smaller cloth helper path rather than from forcing the
tail-jump handler with heavier assembly. The local compiler package is the
working authority for this iteration loop; decomp.me submission is not a blocker
when the package and flags are equivalent. This is a decompilation/scoring
result only; it does not add new runtime or gameplay semantics.
