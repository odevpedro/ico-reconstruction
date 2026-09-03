# Rev.117 — Byte-exact recount: 684 of 701 .s (correction of Rev.106f "1224")

- **Date:** 2026-09-03
- **Objective:** Re-verify how many `.s` files match the USA ELF byte-exactly,
  after the AGENTS.md and backlog.md claimed **1224** (Rev.106f).
- **Scope:** All 701 `.s` in `src/**/asm/` vs `.local/extracted/SCUS_971.13.elf`.
- **Sources used:**
  - `tools/asm_source_score.py` (`--all` + `assemble_and_verify`)
  - USA ELF `.local/extracted/SCUS_971.13.elf` (sha256
    `0eb85d98…d574fdaed52e`, one PT_LOAD segment, vaddr 0x100000,
    filesz 0x533bc6)
  - The 701 `.s` on disk (git-committed, stable across Rev.106→116)
- **Evidence used:**
  - `asm_source_score.py --all --no-save` → **612/612** for the
    `TARGET_FUNCTIONS` pipeline (0 failures), verified live.
  - Per-file `assemble_and_verify(name, va, size=len(assembled))` for every
    `.s` outside `TARGET_FUNCTIONS`, using the VA decoded from the basename
    (or best first-word alignment for symbolic names).

## Findings

The repository on disk holds **701 `.s` files**, not 1224. The "1224" figure
from Rev.106f (393 + 36 + 57 + 738, or "1224 .s functions") does not correspond
to the checked-in tree. The authoritative re-verified count:

| Step | Count | Method |
|------|-------|--------|
| Pipeline functions | 612 | `asm_source_score.py --all` (612/612, 0 fail) |
| Other `.s` (outside `TARGET_FUNCTIONS`) | 72 | `assemble_and_verify` at target VA |
| **Total byte-exact `.s`** | **684 / 701** (97.6%) | vs USA ELF |

## Non-byte-exact remainder (17 of 701)

| Status | Count | Files |
|--------|-------|-------|
| Divergent `.word`-only (COP1/mult; ee-gcc 2.9 cannot assemble one-shot) | 4 | `boyAI_sub_1435A0`, `eBrainProcess`, `girlBrain_sub_16F618`, `girlBrain_sub_16F620` |
| ASM-ERR (COP2/HPI `ld.b $w0` rejected by ee-gcc 2.9) | 4 | `boyAI_sub_1562D4/DC/E0/E8` |
| Trivial stubs (`jr $ra`, ≤8 B) | 4 | `isysGObjActiveLink`, `isysGObjActiveDlLink`, `isysGObjProcPause`, `boyAI_sub_14BB08` |
| Conservative recount reserve (duplicate basenames core vs entity) | 5 | — |

## Three files corrected at Rev.117 (regenerated from the USA ELF)

These were previously divergent to the ELF bytes and are now byte-exact:

- `boyAI_sub_143B58` (regenerated via `convert_function`, BYTE-EXACT)
- `boyAI_sub_15C7C0` (regenerated, BYTE-EXACT)
- `girlBrain_sub_16E6C4` (regenerated, BYTE-EXACT)

The 4 `.word`-only files remain outside the automated scoring pipeline
(documented ee-gcc 2.9 limitation), consistent with prior notes.

## Confirmed

- 612/612 pipeline functions are byte-exact (live re-run).
- 72 other `.s` are byte-exact, verified at their target VA.
- The `.s` files are git-clean except the three regenerated at Rev.117.

## Probable

- The 5-file reserve are duplicate basenames present in both `core` and
  `entity` trees (e.g. `iosOmExeEachGObj`, `iosOmCreateDL`, `isysGObjAlloc`,
  `isysGObjInit`, `isysGObjRemove`); only one copy is authoritative per symbol.

## Possible

- The remaining 4 `.word`-only files are byte-exact **to the ELF bytes** but
  cannot be re-assembled one-shot by ee-gcc 2.9; they would pass a two-step
  (`.word` raw + same-VA) re-assembly.

## Unknown

- How the original "1224" count was derived; likely summed redundant/archived
  entries that were later consolidated or removed.

## Discarded

- The claim that all 1224 files are byte-exact on the current tree.

## Next minimum test

- Re-run `asm_source_score.py --all --no-save` and confirm 612/612 on any future
  change touching `src/**/asm/`.

## Conservative verdict

**684 of 701 `.s` (97.6%) are verified byte-exact against the USA ELF.** Use
this number, not "1224", as the authoritative reconstruction status.
