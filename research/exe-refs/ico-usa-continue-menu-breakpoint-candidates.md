# ICO USA Continue Menu Breakpoint Candidates

> Evidence level: Confirmed for tool outputs and PCSX2 breakpoint behavior described below.
> Local-only inputs: user-owned ICO USA BIN and PCSX2 debugger.
> Date: 2026-05-13

## Purpose

This note records the expanded address search after the first `Continue / Yes / No` UI tests did not respond to blind ASCII or TM2 string swaps.

The goal is to provide many candidate addresses for later manual PCSX2 breakpoint validation, while separating confirmed runtime behavior from static candidates.

## Local Reports

Generated local reports:

```text
.local/reports/20260513T150131Z-SCUS_971.13-exe-ref-index.json
.local/reports/20260513T150213Z-SCUS_971.13-mips-immediate-scan.json
.local/reports/20260513T150234Z-function-ref-correlator.json
.local/reports/20260513T150327Z-SCUS_971.13-mips-call-graph.json
```

These reports are ignored by git and should remain local.

## Static Search Scope

The broad `exe-ref-index` pass searched UI, decision, input, and texture terms:

```text
Continue, continues, continue, Yes, No, Yes:O, No:X,
gameover, GameOver, game over,
pad, PAD, Pad,
button, Button,
select, Select, SELECT,
cancel, Cancel,
OK, ok,
decide, Decide,
menu, Menu,
pause, Pause,
pac_continueTag,
TEX/conti_p1.tm2,
TEX2/cont2_p1.tm2,
TEX2/yesno_p1.tm2,
%s.tm2
```

This produced 190 unique virtual addresses used as `mips-immediate-scanner` targets.

## MIPS Immediate Summary

From 190 candidate VAs:

- 27 split-immediate matches were found in `.text`.
- 10 functions were correlated with those references.
- 14 direct calls into those candidate functions were found.

## Correlated Candidate Functions

| Function | Stack | Referenced target(s) | Current interpretation |
|----------|-------|----------------------|------------------------|
| `0x00104b98` | 16 | `0x00553b90` (`Pad`) | Input/pad-related candidate |
| `0x0011a520` | 96 | `0x005551f0` (`pac_continueTag`) | Runtime-tested; too generic |
| `0x0012d218` | 592 | `0x00631e00` (`%s.tm2`) | Runtime-tested; not hit in Continue flow |
| `0x0012f818` | 208 | `0x00556620` (`No`) | New candidate |
| `0x0013a868` | 192 | many `pad` targets | Strong input/pad cluster candidate |
| `0x0013ad58` | 128 | `0x00557808` (`pad`) | Input/pad candidate |
| `0x0013af88` | 160 | `0x00557840` (`pad`) | Input/pad candidate |
| `0x001ac4b8` | 80 | `0x00616568` (`No`) | New `No` candidate |
| `0x001ac688` | 1216 | `0x00616568` (`No`) | New `No` candidate |
| `0x001aca28` | 1216 | `0x00616568` (`No`) | New `No` candidate |

## Caller / Callsite Candidates

| Target function | Caller function | Callsite | Notes |
|-----------------|-----------------|----------|-------|
| `0x00104b98` | `0x00135bf8` | `0x00135c8c` | Calls Pad-related function |
| `0x0011a520` | `0x0011a668` | `0x0011a794` | Runtime-tested; too generic |
| `0x0012d218` | `0x0012fd50` | `0x0012fd58` | Runtime-tested; not hit in Continue flow |
| `0x0012f818` | none found | none found | Test function entry directly |
| `0x0013a868` | `0x0013af88` | `0x0013b008` | Input/pad cluster |
| `0x0013ad58` | `0x00104b98` | `0x00104bbc` | Input/pad chain |
| `0x0013af88` | `0x0013ba20` | `0x0013ba54` | Input/pad chain |
| `0x001ac4b8` | `0x001ad0e8` | `0x001ad250` | `No` candidate |
| `0x001ac688` | `0x001accd0` | `0x001ace50` | `No` candidate |
| `0x001aca28` | `0x001accd0` | `0x001acffc` | `No` candidate |

## Runtime Breakpoint Results So Far

Confirmed in PCSX2 debugger:

| Breakpoint | Result | Interpretation |
|------------|--------|----------------|
| `0x0011a520` | Hit when pressing `New Game` | Valid breakpoint, but too generic for death menu |
| `0x0011a794` | Hit when pressing `New Game` | Calls `0x0011a520`; also too generic |
| `0x0012d218` | Did not hit on `New Game`, death menu, or clicking `Yes` | Not directly involved in observed Continue flow |
| `0x0012fd58` | Did not hit in the tested flow | Callsite not directly involved in observed Continue flow |

## Prioritized Next Breakpoints

The next manual tests should prioritize candidates that were not already disproven and are closer to input or Yes/No state:

1. `0x0012f818`
2. `0x001ac4b8`
3. `0x001ac688`
4. `0x001aca28`
5. `0x0013a868`
6. `0x0013af88`
7. `0x0013b008`
8. `0x0013ad58`
9. `0x00104b98`
10. `0x00104bbc`

Recommended test sequence:

1. Add one execute breakpoint at a time.
2. Check whether it fires on `New Game`.
3. If it does not fire early, keep it active until the death `Continue / Yes / No` menu.
4. If it fires during the menu or when pressing `Yes`/`No`, capture a debugger screenshot with registers.

## Current Conclusion

The first `pac_continueTag` and `%s.tm2` paths did not isolate the death/continue menu.

The next productive front is to test functions tied to `No` literals and input/pad clusters, because the menu decision path should eventually process controller input and branch on the selected option even if the visible text is texture-backed.
