# ICO splat Adjacent Promoted Ranges Experiment

## Date

2026-05-15

## Objective

Continue the conservative `splat64[mips]` workflow by promoting the next small
set of already-studied adjacent functions out of monolithic `.text` assembly.

The goal is tooling validation and boundary verification, not semantic
renaming.

## Scope

Included:

- `0x0013fc00` callback slot `+0x48` dispatcher;
- `0x001d27a8` cloth payload initializer candidate;
- `0x001d3b28` cloth state auxiliary function;
- the previously promoted verified ranges from
  `research/external/ico-splat-promoted-ranges-experiment.md`;
- full ELF byte coverage with non-text/DVP regions treated as `databin`.

Excluded:

- committing generated asm/assets;
- compiling or linking a rebuilt ELF;
- promoting broad source-file regions;
- claiming a source-level match;
- treating SOTC/ICO-decomp names as semantic proof beyond the already recorded
  cross-reference context.

## Sources Used

| Source | Use |
|---|---|
| `.local/extracted/SCUS_971.13.elf` | target ELF |
| `/tmp/ico-tooling-venv` | temporary tooling environment |
| `splat64[mips] 0.34.0` | split/promoted-range experiment |
| `spimdisasm 1.35.0` | disassembly backend |
| `research/external/ico-splat-promoted-ranges-experiment.md` | previous promoted-range baseline |
| `research/elf/ghidra-rev025-runtime-confirmed-caller-context.md` | `0x001d3b28` and `0x001d27a8` context |
| `research/elf/ghidra-rev031-record-callback-dispatchers.md` | `0x0013fc00` callback slot dispatcher context |
| `research/elf/ghidra-rev032-static-callback-follow-through.md` | deeper `0x0013fc00` traversal notes |
| `research/elf/ghidra-rev043-cloth-initializer-arg-source.md` | `0x001d27a8` argument-source gap |
| `research/elf/ghidra-rev044-staged-callback-path-001d27a8.md` | staged callback path limits |

## Evidence Used

Working directory:

```txt
/tmp/ico-splat-adjacent
```

Command:

```sh
/tmp/ico-tooling-venv/bin/splat split \
  /tmp/ico-splat-adjacent/config/SCUS_971.13.adjacent.yaml
```

Clean-run result:

```txt
Split 5 MB (100.00%) in defined segments
databin: 3 MB (72.55%) 28 split
asm:     1 MB (27.45%) 13 split
unknown: 0 B
```

Important generated files:

| File | Lines | Role |
|---|---:|---|
| `asm/callback_slot48_dispatcher_0013fc00.s` | 88 | isolated `func_0013FC00` |
| `asm/cloth_payload_initializer_001d27a8.s` | 147 | isolated `func_001D27A8` |
| `asm/cloth_state_aux_001d3b28.s` | 68 | isolated `func_001D3B28` |
| `asm/callback_storage_0013f3f0.s` | 174 | previously promoted storage function |
| `asm/callback_registration_0013f7a8.s` | 24 | previously promoted registration wrapper |
| `asm/cloth_dispatcher_001d37c8.s` | 169 | previously promoted cloth dispatcher |
| `asm/cloth_update_callback_001d3a30.s` | 78 | previously promoted cloth update callback |
| `SCUS_971.13.adjacent.ld` | 485 | generated linker script |

No generated files were copied into the repository.

## Promoted Range Table

| Name in experiment | VA start | VA end | File offset start | File offset end | Status |
|---|---:|---:|---:|---:|---|
| `callback_slot48_dispatcher_0013fc00` | `0x0013fc00` | `0x0013fd10` | `0x040c00` | `0x040d10` | Complete |
| `cloth_payload_initializer_001d27a8` | `0x001d27a8` | `0x001d29b8` | `0x0d37a8` | `0x0d39b8` | Complete |
| `cloth_state_aux_001d3b28` | `0x001d3b28` | `0x001d3bf0` | `0x0d4b28` | `0x0d4bf0` | Complete |

The previous promoted ranges remained present:

| Name in experiment | VA start | VA end | Status |
|---|---:|---:|---|
| `callback_storage_0013f3f0` | `0x0013f3f0` | `0x0013f638` | Preserved |
| `callback_registration_0013f7a8` | `0x0013f7a8` | `0x0013f7d8` | Preserved |
| `cloth_dispatcher_001d37c8` | `0x001d37c8` | `0x001d3a30` | Preserved |
| `cloth_update_callback_001d3a30` | `0x001d3a30` | `0x001d3b28` | Preserved |

## Boundary Validation

| Function | End marker | Boundary evidence |
|---:|---:|---|
| `0x0013fc00` | `0x0013fd10` | epilogue includes `jr $ra` at `0x0013fd04`, delay-slot stack restore at `0x0013fd08`, padding `nop` at `0x0013fd0c`; next label is `func_0013FD10` |
| `0x001d27a8` | `0x001d29b8` | epilogue includes `jr $ra` at `0x001d29ac`, delay-slot stack restore at `0x001d29b0`, padding `nop` at `0x001d29b4`; next label is `func_001D29B8` |
| `0x001d3b28` | `0x001d3bf0` | epilogue includes tail jump path to `func_0010ECB8` and normal `jr $ra` path through `0x001d3bec`; next label is `func_001D3BF0` |

## Byte-Level or Instruction-Level Findings

### `0x0013fc00`

Generated file:

```txt
/tmp/ico-splat-adjacent/asm/callback_slot48_dispatcher_0013fc00.s
```

Key instructions:

| VA | Instruction | Meaning |
|---:|---|---|
| `0x0013fc38` | `lw v0,0x48(s2)` | load callback from slot `+0x48` |
| `0x0013fc44` | `jalr v0` | indirect call |
| `0x0013fc48` | `daddu a0,s2,zero` | pass `s2` as first argument |
| `0x0013fcac` | `lw v0,0x48(s0)` | load callback from slot `+0x48` in nested item |
| `0x0013fcb8` | `jalr v0` | indirect call |
| `0x0013fcbc` | `daddu a0,s0,zero` | pass `s0` as first argument |

This preserves the Rev.031-044 model: the function can call slot `+0x48`
callbacks, but it visibly prepares only `a0` at the call sites. It does not, by
itself, explain the meaningful `a1` consumed by `0x001d27a8`.

### `0x001d27a8`

Generated file:

```txt
/tmp/ico-splat-adjacent/asm/cloth_payload_initializer_001d27a8.s
```

Key instructions:

| VA | Instruction | Meaning |
|---:|---|---|
| `0x001d27bc` | `daddu s5,a0,zero` | preserve first input |
| `0x001d27cc` | `daddu s4,a1,zero` | preserve second input |
| `0x001d27e8` | `jal func_0013A0F8` | request/provider call for payload |
| `0x001d2804` | `sw s6,0x800(v1)` | install returned payload at `[entity + 0x800]` |
| `0x001d2850` | `lw v1,0x30(s4)` | read from second input structure |
| `0x001d2858` | `sw v1,0x4(s6)` | store copied value into payload field `+0x04` |
| `0x001d2944` | `sw s0,0x40(s3)` | write pointer into payload field `+0x40` |
| `0x001d2978` | `swc1 f1,0x44(s3)` | write float-derived value into payload field `+0x44` |

This preserves the important Rev.043 gap: `0x001d27a8` clearly consumes a
meaningful `a1`, so runtime must still identify the real caller/context that
provides it.

### `0x001d3b28`

Generated file:

```txt
/tmp/ico-splat-adjacent/asm/cloth_state_aux_001d3b28.s
```

Key instructions:

| VA | Instruction | Meaning |
|---:|---|---|
| `0x001d3b44` | `lw v0,0x15c(s2)` | read context/entity chain |
| `0x001d3b48` | `lw a0,0x800(v0)` | read installed cloth payload |
| `0x001d3b9c` | `jal func_001D12D8` | call related cloth-domain routine |
| `0x001d3bb4` | `jal func_001D2540` | alternate related cloth-domain routine |
| `0x001d3bd4` | `j func_0010ECB8` | tail-call path |
| `0x001d3be8` | `jr ra` | normal return path |

This supports the existing interpretation that `0x001d3b28` is a sibling
routine in the same cloth payload domain as `0x001d3a30` and `0x001d37c8`.
Its exact role remains unnamed.

## What Is Confirmed

1. The three adjacent candidate ranges can be promoted cleanly with `splat`.
2. Full ELF coverage remains 100%; no unknown bytes were introduced.
3. `0x0013fc00` remains isolated as a callback slot `+0x48` dispatcher with two
   visible `jalr` call sites.
4. `0x001d27a8` remains isolated as the payload installer that writes
   `[entity + 0x800]` and consumes `[a1 + 0x30]`.
5. `0x001d3b28` remains isolated as a routine that reads the same installed
   payload and branches into related cloth-domain routines.
6. The previously promoted ranges still coexist with the new promoted ranges in
   the generated linker script.

## What Is Probable

1. The incremental splat config can now grow around verified function islands
   rather than around a single huge `main_text.s`.
2. `0x001d27a8`, `0x001d3a30`, `0x001d37c8`, and `0x001d3b28` are a coherent
   cloth payload lifecycle cluster.
3. `0x0013fc00` is a useful tooling anchor for the unresolved `+0x48` callback
   path, even though it does not fully explain `a1`.

## What Is Possible

1. A future repo-local experimental splat config could include these promoted
   ranges if the project decides to version tooling config separately from
   generated asm.
2. The isolated files may be useful for decomp.me scratches or later matching C
   attempts.
3. Additional nearby cloth-domain helpers can be promoted once their boundaries
   are checked individually.

## What Is Unknown

1. Whether the generated linker script can rebuild a byte-identical ELF.
2. Who provides the meaningful `a1` observed by `0x001d27a8`.
3. Whether `0x0013fc00` calls the static `ROPE +0x48` slot in the runtime
   scenario of interest.
4. The exact semantic role of `0x001d3b28`.
5. Whether all auto-generated labels in the surrounding gap files are valid
   function starts.

## What Is Discarded

1. Treating `0x0013fc00` as a complete explanation for `0x001d27a8(a0, a1)`.
2. Naming `0x001d3b28` as finalize/update/destructor without runtime or stronger
   static evidence.
3. Treating a successful split as proof of source-level reconstruction.
4. Copying generated asm into the repository before a clear generated-output
   policy exists.

## Next Minimum Test

There are two conservative next tests:

| Test | Purpose |
|---|---|
| Runtime breakpoint at `0x001d27a8` | capture `ra`, `a0`, `a1`, `[a1+0x30]`, and identify the real caller/context |
| Promote one nearby cloth helper at a time | continue growing verified islands only after boundary checks |

The runtime breakpoint is now the higher-value test, because the static split
has made the function cluster cleaner but has not resolved the `a1` source.

## Conservative Verdict

The adjacent promoted-range experiment succeeded.

The practical gain is that the cloth callback/payload cluster is no longer just
buried inside monolithic assembly. The project now has a reproducible way to
isolate verified function islands while keeping the evidence chain conservative.

This does not prove source-level semantics, but it materially improves the
tooling baseline for future runtime capture, decomp.me scratches, and eventual
matching work.
