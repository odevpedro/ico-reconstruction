# Rev.157 — Root cause of the 4 "divergent .word-only" .s files: ee-as branch padding, not COP1/mult limitation

**Date:** 2026-09-09
**Objective:** Determine why `boyAI_sub_1435A0.s`, `eBrainProcess.s`, `girlBrain_sub_16F618.s`, `girlBrain_sub_16F620.s` did not assemble byte-exact with ee-gcc 2.9, and regenerate them as byte-exact `.s`.
**Scope:** EE assembler behavior (ee-as 2.9-ee-991111-01) for short backward branches; final `.s` for the 4 functions; correction of the "R5900 COP1/mult one-shot limitation" classification.

## Sources used
- `.local/extracted/SCUS_971.13.elf` (USA ELF, `.text`)
- ee-gcc2.9-991111-01 (`toolchain/ee-gcc2.9-991111-01/bin/ee-gcc`)
- rabbitizer 1.16.2, capstone (MIPS64 LE, skipdata)
- `tools/asm_source_score.py` (TARGET_FUNCTIONS + branch-label generation)
- AGENTS.md "Byte-exact reconstruction status" tables

## Evidence used
- Isolation tests of branch assembly with ee-as
- Byte-level comparison of assembled `.o` vs ELF slice at target VA
- Instruction classification of the 4 function ranges

## Root cause

The divergence was NOT "ee-gcc 2.9 cannot assemble COP1/mult one-shot". The ee-as (GNU as, mips, 2.9-ee-991111-01) **macro-expands short backward branch references by injecting nops**, regardless of `.set noreorder`/`.set nomacro`/`.set noat`.

### Empirical rule (isolated tests, all with `.set noreorder/.set nomacro/.set noat`)

For a branch (`bne`, `beq`, `bnel`, `beqz`, `b`, `blez`, ...) whose target label lies **within 5 instructions before the branch**

| dist back | assembled len (words) | clean len (words) | padding nops |
|-----------|----------------------|-------------------|--------------|
| 1         | 14                   | 10                 | 4+           |
| 2         | 14                   | 11                 | 3+           |
| 3         | 14                   | 12                 | 2+           |
| 4         | 14                   | 13                 | 1+           |
| 5         | 14                   | 14                 | 0            |
| 6         | 15                   | 15                 | 0            |

- dist ≥ 6 → clean assembly (len = dist + 9 fixed tail).
- dist ≤ 5 → the assembler pins the branch at a fixed slot (bne@idx=10) and injects nops to fill, producing len=14 regardless of dist.
- Forward branches (label after the branch) assemble cleanly even at short distance.
- Numeric local labels (`1:`/`1b`) do NOT avoid the padding.

This changes the `.o` byte length and shifts all following branch displacements → any `.s` using in-text labels for such branches cannot be byte-exact.

### Consequence for the pipeline

`tools/asm_source_score.py` emits symbolic labels (`loc_%08x`) for in-range branch targets. For the 4 functions below, several backward branches land within the 5-instruction window → the generated `.s` would be padded → the pipeline marks them divergent. This is a generator limitation, not a toolchain preserve-ability limit.

## Solution

Regenerate the 4 `.s` emitting **every** branch, `jal`, `mult`/`multu`, COP1-compare, and COP2/HPI instruction as raw `.word` (exact 32-bit ELF word). No labels, no macro expansion, no padding. Normal instructions emitted via rabbitizer `disassemble()` with ABI→numeric register conversion (`lw $2,0x0($29)` style).

### Resulting functions (byte-exact vs USA ELF)

| Function | VA | Size | bytes | status |
|----------|----|------|-------|--------|
| `boyAI_sub_1435A0` | 0x1435A0 | 0x130 | 304 | BYTE-EXACT |
| `eBrainProcess` | 0x190B30 | 0x258 | 600 | BYTE-EXACT |
| `girlBrain_sub_16F618` | 0x16F618 | 0x258 | 600 | BYTE-EXACT |
| `girlBrain_sub_16F620` | 0x16F620 | 0x258 | 600 | BYTE-EXACT |

**Note on sizes.** The previously declared 0x128/0x254 were too small; the ELF slices of 0x130/0x258 are contiguous code (confirmed by tail words continuing the function, not a clean boundary).

## Confirmed
- ee-as pads short (≤5 insn) backward branches with nops; forward and ≥6-back are clean.
- The 4 `.s` regenerated with `.word`-for-branch strategy assemble byte-exact with ee-gcc 2.9 (304/600/600/600).
- Root cause of the historical divergence: generator/label strategy, not a COP1/mult toolchain limitation.

## Probable / Possible
- The same padding rule may affect any other `asm_source_score.py`-generated `.s` that has a backward branch within the 5-instruction window; those would appear divergent for the same reason. (Would affect files already outside `TARGET_FUNCTIONS`; none observed among the 612 in-pipeline passing files.)

## Unknown
- Exact internal ee-as heuristic (why pin at idx10 / threshold 5); only the observable rule is recorded.

## Discarded
- "ee-gcc 2.9 cannot one-shot assemble COP1/mult/R5900" as the cause of these 4 (toolchain CAN assemble them; `.word` output was a workaround, not a necessity).
- Label naming (symbolic vs `1:` numeric) as the trigger.

## Next minimum test
- Re-run `assemble_and_verify` on the 4 installed `.s` (done: byte-exact).

## Conservative verdict
The 4 functions are now byte-exact `.s` with ee-gcc 2.9. AGENTS.md classification rows updated (moved out of "divergent .word-only"). The other 3 status groups (4 ASM-ERR ld.b, 4 trivial stubs, 5 recount reserve) are unchanged.