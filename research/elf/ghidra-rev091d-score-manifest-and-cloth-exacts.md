# Rev.091d — score manifest and cloth exact closures

Date: 2026-05-18

## Objective

Record the current local decompilation score state after scorer
normalization updates, corrected `enemy1_hA` range, and two new cloth exact
matches.

## Scope

This revision is limited to the local ee-gcc 2.9 scoring/decompilation
pipeline and `src/cloth` / `src/entity` source islands. It does not investigate
runtime logs, assets, DATA.DF, Yorda, capture, Continue menu, TM2, or new game
subsystems.

## Sources used

| Source | Role |
|---|---|
| `tools/score_all.py --summary` | Batch local scoring |
| `tools/ee_gcc_compile.py` | Compile/disassemble/normalize/score pipeline |
| `tools/diff_classifier.py` | Structural diff classification |
| `src/cloth/near_matches.c` | Cloth near-match source island |
| `src/entity/enemy1.c` | ENEMY1 source island |
| `decompme_submissions/` | Manual decomp.me package |

## Evidence used

| Evidence | Result |
|---|---|
| Local ee-gcc compile of all mapped functions | 37/37 compile and score |
| `cloth_test_variant_field` targeted score | 1000/1000 exact |
| `cloth_test_field0_or_extra` targeted score | 1300/1300 exact |
| Corrected `enemy1_hA` range `0x1CE690..0x1CE6F0` | 2200/2300, 95.65% |

## Instruction-level findings

| Function | VA | Size | Finding |
|---|---:|---:|---|
| `cloth_test_variant_field` | `0x001D3DB0` | `0x28` | Exact after explicit nop barrier in branch delay slot |
| `cloth_test_field0_or_extra` | `0x001D40A0` | `0x38` | Exact after preserving result variable and explicit nop barrier |
| `enemy1_hA` | `0x001CE690` | `0x60` | Previous `0x38` range was too short; corrected range exposes tail-jump path and common epilogue |

## Score manifest

| Function | Score | Status |
|---|---:|---|
| `fn_1CE5F8` | 100.00% | EXACT |
| `boy_set_state` | 100.00% | EXACT |
| `cloth_get_variant` | 100.00% | EXACT |
| `cloth_payload_field0_is_zero` | 100.00% | EXACT |
| `cloth_payload_state_is_two` | 100.00% | EXACT |
| `cloth_test_variant_field` | 100.00% | EXACT |
| `cloth_test_field0_or_extra` | 100.00% | EXACT |
| `enemy1_hA` | 95.65% | NEAR EXACT |
| `boy_hB` | 81.37% | STRUCTURAL OK |
| `boy_hC` | 60.75% | PARTIAL |
| `sub_1C1EA8` | 60.47% | PARTIAL |
| Remaining mapped functions | 4.90%-60.47% | LOW/PARTIAL |

## What is confirmed

| Item | Confidence |
|---|---|
| The local pipeline currently scores 37 mapped functions with zero compile errors | Confirmed |
| Seven functions are exact under local normalized scoring | Confirmed |
| `enemy1_hA` should be scored with a larger range than `0x38` | Confirmed |
| The two newly exact cloth functions require explicit nop barriers to match target delay slots | Confirmed |

## What is probable

| Item | Rationale |
|---|---|
| `enemy1_hA` is the next best near-exact target | It is 95.65% with one extra generated epilogue instruction after the inline tail-jump path |
| More short cloth accessors/tests can close quickly | Existing cloth functions are small and mostly differ by scheduling/delay slots |

## What is possible

| Item | Constraint |
|---|---|
| `enemy1_hA` can be closed with stricter control over the tail jump | May require heavier inline asm or a better C shape that makes ee-gcc emit `j fn_1CE5F8` without an unreachable reload |
| The score of other functions improved from normalization, not source changes | Normalization now handles `li`, `dsrl32`, and `fn_XXXXXXXX` call targets |

## What is unknown

| Item | Reason |
|---|---|
| Whether decomp.me scores all seven exacts identically | Browser flow is manual and the API compiler id remains unknown |
| Whether `enemy1_hA` can be matched cleanly without inline assembly beyond delay-slot barriers | Requires further codegen experiments |

## What is discarded

| Idea | Reason |
|---|---|
| Treating the earlier `enemy1_hA` 82.14% score as the current baseline | It used a too-short `0x38` target range |
| Treating decomp.me submission as a blocker | Local scoring is the authoritative iteration loop for now |

## Next minimum test

Run:

```sh
python3 tools/score_all.py --summary
python3 tools/ee_gcc_compile.py src/entity/enemy1.c --fn enemy1_hA --va 0x1CE690 --size 0x60
```

Then decide whether to pursue the remaining `enemy1_hA` epilogue mismatch or
move to the next short function.

## Conservative verdict

The project now has seven local exact matches, all 37 mapped functions compile,
and `enemy1_hA` is near-exact only after correcting its target range. The
decompilation progress remains small relative to the full game, but the local
pipeline is producing repeatable, instruction-level verified closures.
