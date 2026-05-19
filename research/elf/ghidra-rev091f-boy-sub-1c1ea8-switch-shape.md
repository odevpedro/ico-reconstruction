# Rev.091f — BOY sub_1C1EA8 Switch-Shape Experiment

Date: 2026-05-18

## Objective

Test whether rewriting `sub_1C1EA8`'s variant selection from `if / else if / else`
to a `switch` would move the generated code closer to the original ELF shape.

## Scope

Included:

- `src/entity/boy.c`
- `tools/ee_gcc_compile.py`

Excluded:

- runtime probing;
- new gameplay interpretation;
- other entity files;
- broad documentation rewrites.

## Sources Used

| Source | Role |
|---|---|
| `python3 tools/ee_gcc_compile.py src/entity/boy.c --whole-file --fn sub_1C1EA8 --va 0x1C1EA8 --size 0xB0` | Baseline and post-change scoring |
| `src/entity/boy.c` | Local source shape under test |

## Evidence Used

| Evidence | Result |
|---|---|
| Baseline score before the `switch` rewrite | `2600/4300 = 60.47%` |
| Score after the `switch` rewrite | `2950/4300 = 68.60%` |
| Branch shape in generated ASM | `beq` / `beql` chain now closer to target than the prior `bne` / `bnel` chain |

## Byte-Level or Instruction-Level Findings

The rewritten source changed the compiler output around the variant selector:

| Pattern | Before | After |
|---|---|---|
| First compare | `bne` | `beq` |
| Second compare | `bnel` | `beql` |
| Score impact | 60.47% | 68.60% |

The generated code still differs from the target in several places:

| Area | Status |
|---|---|
| Register allocation around `scene_obj` / `state_block` loads | still mismatched |
| Matrix setup / `sub_105278` block | still not structurally exact |
| Tail call to `sub_121D90` | still preserved, but surrounding instruction shape differs |

## What Is Confirmed

1. The `switch` rewrite did move `sub_1C1EA8` closer to the target.
2. The function is still not close enough to justify claiming an exact or near-exact closure.
3. The compiler is sensitive to variant-selection source shape in this function.

## What Is Probable

| Item | Rationale |
|---|---|
| The remaining gap is a codegen-shape problem, not a semantic one | The control flow is still the same and the score improved without changing behavior |
| Another small source-shape rewrite may improve it further | The branch sequence is now closer, which suggests the function is on the right path |

## What Is Possible

| Item | Constraint |
|---|---|
| `sub_1C1EA8` may still be closable with more source tuning | Would require careful control over load ordering and temporary lifetimes |
| The next jump in score may require register hints or asm barriers | The current delta is still dominated by codegen choices |

## What Is Unknown

1. Whether a different `switch` layout or explicit temporary can recover the remaining mismatch.
2. Whether the current gap is small enough to close without introducing non-idiomatic C.

## What Is Discarded

1. The original `if / else if / else` version as the best current shape for this function.
2. Treating the 68.60% score as a stable endpoint.

## Next Minimum Test

Try one more small source-shape adjustment around the matrix-loading block in
`sub_1C1EA8`, then rescore. If the score stalls, move on rather than forcing the
function with heavy inline assembly.

## Conservative Verdict

`sub_1C1EA8` is now a better candidate than it was before the `switch` rewrite,
but it remains a partial match. The experiment is useful because it shows that
branch selection style affects the compiler output materially. The function is
still not a closure.
