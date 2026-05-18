# Rev.091c — Local Scoring Normalization and fn_1CE5F8 Exact Match

## Date

2026-05-18

## Objective

Continue the Rev.090-091 local decompilation scoring work by:

1. validating the current 37-function ee-gcc 2.9 scoring baseline;
2. improving local diff normalization for symbolic targets and local labels;
3. closing `fn_1CE5F8` if the remaining register-allocation difference was controllable;
4. updating browser-ready decomp.me submission files.

## Scope

Included:

- `tools/ee_gcc_compile.py`
- `tools/diff_classifier.py`
- `src/entity/enemy1.c`
- `decompme_submissions/fn_1CE5F8_source.c`
- `decompme_submissions/fn_1CE5F8_target_asm.txt`
- `decompme_submissions/README.md`

Excluded:

- new runtime mining;
- new entity/source islands;
- DATA.DF/assets;
- broad renaming or high-level documentation rewrites.

## Sources Used

| Source | Use |
|---|---|
| `toolchain/ee-gcc2.9-991111-01/bin/ee-gcc` | Local EE GCC 2.9 build 991111-01 scoring compiler |
| `.local/extracted/SCUS_971.13.elf` | Original target instruction bytes |
| `tools/score_all.py` | 37-function batch scoring manifest |
| `tools/ee_gcc_compile.py` | Compile, disassemble, normalize, score |
| `tools/diff_classifier.py` | Diff class breakdown |
| `src/entity/enemy1.c` | `fn_1CE5F8` source |

## Evidence Used

- Local compiler execution with flags:

```txt
-mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2
```

- Batch scoring output:

```txt
Total: 37 functions
Perfect: 5
Errors/NA: 0
```

- Detailed `fn_1CE5F8` scoring:

```txt
Target instructions: 37
Generated instructions: 37
Aligned matches: 37
Score: 3700/3700 = 100.00%
```

## Byte-Level or Instruction-Level Findings

### Local label normalization

Before Rev.091c, the scorer treated local branch labels emitted by GCC (`$L9`,
`$L11`) as mismatches against Capstone absolute branch targets from the ELF.

Rev.091c normalizes local intra-function control-flow targets to instruction
indices:

```txt
beq 2, $zero, 1893972  -> beq 2, $zero, @23
beq 2, $zero, $L9      -> beq 2, $zero, @23
```

External calls remain address-based:

```txt
jal sub_1CF998 -> jal 1898904
```

### `fn_1CE5F8` final register allocation

The only remaining real mismatch was:

| Target | Generated before |
|---|---|
| `lw 3, 16(16)` | `lw 2, 16(16)` |
| `beq 3, $zero, @33` | `beq 2, $zero, @33` |

The exact match was obtained by forcing the cleanup flag load into `$3`:

```c
{
    register u32 cleanup_flag asm("$3");
    cleanup_flag = wk->cleanup_flag;
    if (cleanup_flag)
        sub_1CDB28(entity);
}
```

This preserved all previous frame, delay-slot, call, and local-branch matches.

## Score Matrix

| Function | Status | Score |
|---|---:|---:|
| `cloth_get_variant` | EXACT | 100.00% |
| `cloth_payload_field0_is_zero` | EXACT | 100.00% |
| `cloth_payload_state_is_two` | EXACT | 100.00% |
| `boy_set_state` | EXACT | 100.00% |
| `fn_1CE5F8` | EXACT | 100.00% |
| `enemy1_hA` | STRUCTURAL OK | 82.14% |
| `cloth_test_variant_field` | STRUCTURAL OK | 80.00% |
| `boy_hB` | PARTIAL | 69.61% |
| `cloth_test_field0_or_extra` | PARTIAL | 65.38% |
| `boy_hC` | PARTIAL | 53.27% |
| `sub_1C1EA8` | PARTIAL | 51.16% |

All 37 functions compile and score. The remaining lower-scoring functions are
mostly larger near-structural models with real missing/extra instruction blocks,
not just label formatting issues.

## What Is Confirmed

1. The local ee-gcc 2.9 pipeline can produce five exact matches.
2. `fn_1CE5F8` is now an exact local C match against the original ELF function.
3. Local branch labels and address-like symbolic calls were previously inflating
   mismatch counts.
4. The corrected scorer still reports 37/37 functions compiling with zero errors.
5. The decomp.me submission source for `fn_1CE5F8` is now standalone and exact
   under the local pipeline.

## What Is Probable

1. The next best function-level targets are `enemy1_hA` and
   `cloth_test_variant_field`.
2. Larger entity handlers require structural C work, not just normalization.
3. Remaining local label mismatches in the full queue likely indicate cases where
   generated control-flow structure differs, not simple `$Lxx` formatting.

## What Is Possible

1. Some `REG_ALLOC` differences may be controlled with targeted register
   variables, as with `fn_1CE5F8`.
2. Some `MISSING`/`EXTRA` blocks may be reduced with asm barriers or local
   expression restructuring.
3. Exact decomp.me scores may differ if decomp.me's compiler preset or assembler
   normalization differs from the local package.

## What Is Unknown

1. Whether decomp.me's browser preset for EE GCC 2.9 build 991111-01 matches this
   local package exactly.
2. Whether `enemy1_hA` can be closed without excessive register hints or
   non-idiomatic C.
3. Whether all exact local submissions will score 100% in decomp.me without
   additional formatting adjustments.

## What Is Discarded

1. The previous `fn_1CE5F8` score of 72.37%/87.84% is superseded by corrected
   normalization plus the `$3` register hint.
2. The remaining `fn_1CE5F8` mismatch was not a semantic mismatch.
3. The local `ee-gcc` failure under the default sandbox was an execution
   environment issue, not a source compile failure.

## Next Minimum Test

Submit the five exact-match scratches manually in decomp.me:

1. `cloth_get_variant`
2. `cloth_payload_field0_is_zero`
3. `cloth_payload_state_is_two`
4. `boy_set_state`
5. `fn_1CE5F8`

Then compare decomp.me reported scores against the local pipeline.

## Conservative Verdict

Rev.091c improves the local scoring pipeline and closes `fn_1CE5F8` as the
fifth exact C match. This is a tooling/decompilation-pipeline result, not a new
runtime or gameplay conclusion. The next technical target should be a small
function with high score and few structural differences, preferably `enemy1_hA`
or `cloth_test_variant_field`.
