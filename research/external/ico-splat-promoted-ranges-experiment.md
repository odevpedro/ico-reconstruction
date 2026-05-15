# ICO splat Promoted Verified Ranges Experiment

## Date

2026-05-15

## Objective

Test the next conservative `splat64[mips]` step after the minimal split:
promote only a few already verified function/range anchors out of the
monolithic `.text` assembly into separate asm files.

This is still a tooling experiment. It is not a rebuild and does not introduce
new semantic names as conclusions.

## Scope

Included:

- `0x0013f3f0` callback storage function;
- `0x0013f7a8` callback registration wrapper;
- `0x001d37c8` cloth dispatcher;
- `0x001d3a30` cloth update callback;
- full ELF byte coverage with non-text/DVP regions treated as `databin`;
- validation of generated file boundaries and key cross-references.

Excluded:

- compiling or linking;
- committing generated asm/assets;
- promoting broad source files;
- trusting unreviewed auto-discovered boundaries;
- semantic DVP/overlay analysis.

## Sources Used

| Source | Use |
|---|---|
| `.local/extracted/SCUS_971.13.elf` | target ELF |
| `/tmp/ico-tooling-venv` | temporary tooling environment |
| `splat64[mips] 0.34.0` | split/promoted-range experiment |
| `spimdisasm 1.35.0` | disassembly backend |
| `research/external/ico-splat-minimal-experiment.md` | previous successful minimal split |
| `research/external/ico-rabbitizer-spimdisasm-dispatcher-check.md` | instruction anchor validation |
| `research/elf/ghidra-rev023-dispatcher-table-resolution.md` | dispatcher and jump table |
| `research/elf/ghidra-rev025-runtime-confirmed-caller-context.md` | `0x001d3a30 -> 0x001d37c8` |
| `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md` | `0x0013f7a8` registration map |

## Evidence Used

Working directory:

```txt
/tmp/ico-splat-promoted
```

Command:

```sh
/tmp/ico-tooling-venv/bin/python -m splat.scripts.split \
  /tmp/ico-splat-promoted/config/SCUS_971.13.promoted.yaml \
  --modes all --verbose --skip-version-check
```

Final clean-run result:

```txt
Split 5 MB (100.00%) in defined segments
databin: 3 MB (72.55%) 28 split
asm:     1 MB (27.45%) 8 split
unknown: 0 B
```

Important generated files:

| File | Lines | Role |
|---|---:|---|
| `asm/callback_storage_0013f3f0.s` | 174 | isolated `func_0013F3F0` |
| `asm/callback_registration_0013f7a8.s` | 24 | isolated `func_0013F7A8` |
| `asm/cloth_dispatcher_001d37c8.s` | 169 | isolated `func_001D37C8` |
| `asm/cloth_update_callback_001d3a30.s` | 78 | isolated `func_001D3A30` |
| `asm/gap_0013f638_0013f7a8.s` | 125 | verified non-target gap after storage function |
| `asm/main_text_001000_0403f0.s` | 72,754 | text before promoted storage range |
| `asm/main_text_0407d8_0d47c8.s` | 168,735 | text between registration wrapper and dispatcher |
| `asm/main_text_0d4b28_1705d4.s` | 179,754 | text after callback range |

Generated support files:

| File | Size |
|---|---:|
| `SCUS_971.13.promoted.ld` | 17 KB |
| `undefined_syms_auto.txt` | 79 KB |
| `undefined_funcs_auto.txt` | 156 B |

No generated files were copied into the repository.

## Promoted Range Table

| Name in experiment | VA start | VA end | File offset start | File offset end | Status |
|---|---:|---:|---:|---:|---|
| `callback_storage_0013f3f0` | `0x0013f3f0` | `0x0013f638` | `0x0403f0` | `0x040638` | Complete after boundary correction |
| `callback_registration_0013f7a8` | `0x0013f7a8` | `0x0013f7d8` | `0x0407a8` | `0x0407d8` | Complete after boundary correction |
| `cloth_dispatcher_001d37c8` | `0x001d37c8` | `0x001d3a30` | `0x0d47c8` | `0x0d4a30` | Complete |
| `cloth_update_callback_001d3a30` | `0x001d3a30` | `0x001d3b28` | `0x0d4a30` | `0x0d4b28` | Complete |

## Boundary Correction Found During Test

The first promoted YAML used two overly short boundaries:

| Range | Initial end | Problem | Corrected end |
|---|---:|---|---:|
| `0x0013f3f0` | `0x0013f600` | split before common return path/epilogue beginning at `0x0013f604` | `0x0013f638` |
| `0x0013f7a8` | `0x0013f7d0` | split before `jr $ra` and delay-slot stack restore | `0x0013f7d8` |

This is a useful result: promoting ranges forces boundary validation and catches
truncation that a monolithic asm file can hide.

## Anchor Validation

### `0x0013f3f0`

Generated file:

```txt
/tmp/ico-splat-promoted/asm/callback_storage_0013f3f0.s
```

Key facts:

| Item | Output |
|---|---|
| entry label | `glabel func_0013F3F0` |
| prologue | `addiu $sp, $sp, -0x90` |
| epilogue included | `ld $ra, 0x80($sp)` through `jr $ra` / `addiu $sp, $sp, 0x90` |
| padding before next function | `0x0013f634: nop` |

### `0x0013f7a8`

Generated file:

```txt
/tmp/ico-splat-promoted/asm/callback_registration_0013f7a8.s
```

Key excerpt:

```asm
glabel func_0013F7A8
    /* 407A8 0013F7A8 2D10E000 */  daddu      $v0, $a3, $zero
    /* 407AC 0013F7AC F0FFBD27 */  addiu      $sp, $sp, -0x10
    /* 407B0 0013F7B0 FF00C730 */  andi       $a3, $a2, 0xFF
    /* 407B4 0013F7B4 2D480001 */  daddu      $t1, $t0, $zero
    /* 407B8 0013F7B8 2D30A000 */  daddu      $a2, $a1, $zero
    /* 407BC 0013F7BC 0000BFFF */  sd         $ra, 0x0($sp)
    /* 407C0 0013F7C0 2D404000 */  daddu      $t0, $v0, $zero
    /* 407C4 0013F7C4 FCFC040C */  jal        func_0013F3F0
    /* 407C8 0013F7C8 2D288000 */   daddu     $a1, $a0, $zero
    /* 407CC 0013F7CC 0000BFDF */  ld         $ra, 0x0($sp)
    /* 407D0 0013F7D0 0800E003 */  jr         $ra
    /* 407D4 0013F7D4 1000BD27 */   addiu     $sp, $sp, 0x10
```

This corrects the earlier scratch-style 40-byte description: the full wrapper
range is 48 bytes when the return and delay slot are included.

### `0x001d37c8`

Generated file:

```txt
/tmp/ico-splat-promoted/asm/cloth_dispatcher_001d37c8.s
```

Key facts:

| Item | Output |
|---|---|
| entry label | `glabel func_001D37C8` |
| jump table symbol | `jtbl_00618FB0_main_text` |
| corrected shift | `0x001d3800: sll $v1, $v1, 2` |
| function end | before `0x001d3a30` |

### `0x001d3a30`

Generated file:

```txt
/tmp/ico-splat-promoted/asm/cloth_update_callback_001d3a30.s
```

Key facts:

| Item | Output |
|---|---|
| entry label | `glabel func_001D3A30` |
| direct dispatcher call | `0x001d3b04: jal func_001D37C8` |
| epilogue included | `ld s3/s2/s1/s0`, `jr $ra`, `addiu $sp,$sp,0x50` |
| function end | before `0x001d3b28` |

## What Is Confirmed

1. `splat` can promote the four selected verified ranges into separate asm files.
2. The promoted files preserve the key known cross-references:
   - `func_0013F7A8 -> func_0013F3F0`;
   - `func_001D3A30 -> func_001D37C8`;
   - `func_001D37C8 -> jtbl_00618FB0_main_text`.
3. Full ELF file coverage remains at 100% with non-text and DVP regions as `databin`.
4. Manual boundary validation is necessary; two initial range ends were too short and were corrected.
5. The full `0x0013f7a8` wrapper range includes `jr $ra` and delay-slot stack restore through `0x0013f7d4`.

## What Is Probable

1. Promoting small, validated ranges is a viable workflow for building a conservative splat config.
2. The next useful promoted ranges should come from already validated notes, not automatic discovery alone.
3. The generated per-function asm can improve decomp.me scratch preparation because each target is isolated.

## What Is Possible

1. `0x001d27a8`, `0x0013fc00`, and other Rev.043/044 adjacent functions can be promoted next, but only after boundaries are checked.
2. A repository-local experimental config may eventually be useful, but generated assets should stay out of git unless a clear policy is set.
3. The promoted asm files could become a staging point for matching C later, once compiler/package questions are solved.

## What Is Unknown

1. Whether the generated linker script can produce a byte-identical rebuilt ELF.
2. Whether all automatically detected function labels inside gap/main text files are accurate.
3. Whether `jtbl_00618FB0_main_text` can be modeled cleanly as typed rodata instead of an auto undefined symbol.
4. Whether this workflow remains stable after many more functions are promoted.

## What Is Discarded

1. Treating the first naive function ends as sufficient without epilogue inspection.
2. Promoting broad source-file regions before validated boundaries exist.
3. Treating successful split as evidence of semantic correctness.

## Next Minimum Test

Promote the next small set of functions only if they are already validated by
recent notes. Candidate set:

| Function | Reason |
|---:|---|
| `0x001d27a8` | Rev.043/044 initializer path; writes `[entity + 0x800]` |
| `0x0013fc00` | known dispatcher/callback-adjacent entry mentioned in Rev.043 |
| `0x001d3b28` | nearby cloth function following callback |

Before promoting each one, verify:

- entry boundary;
- return/epilogue boundary;
- whether branch labels cross the proposed boundary;
- whether the file references symbols in already promoted ranges.

Follow-up completed:

```txt
research/external/ico-splat-adjacent-promoted-ranges-experiment.md
```

That experiment promoted all three candidates above and preserved full ELF
coverage.

## Conservative Verdict

The promoted-range experiment succeeded and is more than cosmetic: it proved
that selected verified ranges can be separated cleanly, and it caught boundary
mistakes in the process.

The workflow is now viable for incremental, evidence-driven splat config
construction. It should remain conservative: only promote ranges with verified
boundaries, keep generated outputs out of git for now, and do not treat splat
auto-labels as semantic truth.
