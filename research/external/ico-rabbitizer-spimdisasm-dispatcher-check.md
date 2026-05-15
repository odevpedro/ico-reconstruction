# ICO Rabbitizer / spimdisasm Dispatcher Check

## Date

2026-05-15

## Objective

Revalidate the critical ICO dispatcher/callback instructions with tooling
borrowed from the SOTC workflow: Rabbitizer for MIPS instruction decoding and
spimdisasm as the ELF/tooling dependency installed in the same temporary
environment.

The purpose is to create an independent check against Ghidra-derived listings,
not to infer new semantics.

## Scope

Included:

- `0x001d37c8..0x001d3814` dispatcher entry and switch sequence;
- `0x0013f7a8..0x0013f7cc` callback registration wrapper;
- `0x001d3aec..0x001d3b08` callback gate into dispatcher;
- jump table at `0x00618fb0`.

Excluded:

- decompilation;
- broad function discovery;
- runtime validation;
- SOTC semantic comparison;
- new naming beyond already validated neutral/project names.

## Sources Used

| Source | Use |
|---|---|
| `.local/extracted/SCUS_971.13.elf` | raw bytes |
| Rabbitizer `1.16.0` | independent MIPS instruction decoding |
| spimdisasm `1.35.0` | installed as part of the SOTC-style tooling environment |
| `research/elf/ghidra-rev023-dispatcher-table-resolution.md` | expected dispatcher/table anchors |
| `research/elf/ghidra-rev025-runtime-confirmed-caller-context.md` | expected callback-to-dispatcher call |
| `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md` | expected registration wrapper role |
| `research/external/sotc-tooling-relevance-survey.md` | motivation for tooling check |

## Evidence Used

Tool environment:

```txt
/tmp/ico-tooling-venv
Rabbitizer 1.16.0
spimdisasm 1.35.0
splat64[mips] 0.34.0
```

The script used explicit section mappings from local `readelf` output:

| Section | VA | File offset | Size |
|---|---:|---:|---:|
| `.text` | `0x00100000` | `0x001000` | `0x16f5d4` |
| `.vutext` | `0x0026f5e0` | `0x1705e0` | `0x0050e0` |
| `.data` | `0x00274700` | `0x175700` | `0x2defb8` |
| `.rodata` | `0x00553700` | `0x454700` | `0x0dd1a8` |
| `.lit4` | `0x00630900` | `0x531900` | `0x000fd0` |
| `.sdata` | `0x00631900` | `0x532900` | `0x0022c6` |

## Dispatcher Entry and Switch Sequence

| VA | File offset | Word | Rabbitizer decoding |
|---|---:|---:|---|
| `0x001d37c8` | `0x0d47c8` | `0x27bdffa0` | `addiu $sp, $sp, -0x60` |
| `0x001d37cc` | `0x0d47cc` | `0xffb20030` | `sd $s2, 0x30($sp)` |
| `0x001d37d0` | `0x0d47d0` | `0xffb10020` | `sd $s1, 0x20($sp)` |
| `0x001d37d4` | `0x0d47d4` | `0x0080902d` | `daddu $s2, $a0, $zero` |
| `0x001d37d8` | `0x0d47d8` | `0xffbf0050` | `sd $ra, 0x50($sp)` |
| `0x001d37dc` | `0x0d47dc` | `0xffb30040` | `sd $s3, 0x40($sp)` |
| `0x001d37e0` | `0x0d47e0` | `0xffb00010` | `sd $s0, 0x10($sp)` |
| `0x001d37e4` | `0x0d47e4` | `0x8e42015c` | `lw $v0, 0x15C($s2)` |
| `0x001d37e8` | `0x0d47e8` | `0x8c530800` | `lw $s3, 0x800($v0)` |
| `0x001d37ec` | `0x0d47ec` | `0x26710040` | `addiu $s1, $s3, 0x40` |
| `0x001d37f0` | `0x0d47f0` | `0x8e230008` | `lw $v1, 0x8($s1)` |
| `0x001d37f4` | `0x0d47f4` | `0x2c620005` | `sltiu $v0, $v1, 0x5` |
| `0x001d37f8` | `0x0d47f8` | `0x10400007` | `beqz $v0, . + 4 + (0x7 << 2)` |
| `0x001d37fc` | `0x0d47fc` | `0x3c020062` | `lui $v0, 0x62` |
| `0x001d3800` | `0x0d4800` | `0x00031880` | `sll $v1, $v1, 2` |
| `0x001d3804` | `0x0d4804` | `0x24428fb0` | `addiu $v0, $v0, -0x7050` |
| `0x001d3808` | `0x0d4808` | `0x00621821` | `addu $v1, $v1, $v0` |
| `0x001d380c` | `0x0d480c` | `0x8c640000` | `lw $a0, 0x0($v1)` |
| `0x001d3810` | `0x0d4810` | `0x00800008` | `jr $a0` |
| `0x001d3814` | `0x0d4814` | `0x00000000` | `nop` |

## Callback Registration Wrapper

| VA | File offset | Word | Rabbitizer decoding |
|---|---:|---:|---|
| `0x0013f7a8` | `0x0407a8` | `0x00e0102d` | `daddu $v0, $a3, $zero` |
| `0x0013f7ac` | `0x0407ac` | `0x27bdfff0` | `addiu $sp, $sp, -0x10` |
| `0x0013f7b0` | `0x0407b0` | `0x30c700ff` | `andi $a3, $a2, 0xFF` |
| `0x0013f7b4` | `0x0407b4` | `0x0100482d` | `daddu $t1, $t0, $zero` |
| `0x0013f7b8` | `0x0407b8` | `0x00a0302d` | `daddu $a2, $a1, $zero` |
| `0x0013f7bc` | `0x0407bc` | `0xffbf0000` | `sd $ra, 0x0($sp)` |
| `0x0013f7c0` | `0x0407c0` | `0x0040402d` | `daddu $t0, $v0, $zero` |
| `0x0013f7c4` | `0x0407c4` | `0x0c04fcfc` | `jal func_8013F3F0` |
| `0x0013f7c8` | `0x0407c8` | `0x0080282d` | `daddu $a1, $a0, $zero` |
| `0x0013f7cc` | `0x0407cc` | `0xdfbf0000` | `ld $ra, 0x0($sp)` |

Rabbitizer renders jump targets with a high virtual prefix in its default
symbol formatting (`func_8013F3F0`). The encoded target index corresponds to
the local project VA `0x0013f3f0` under the project's address convention.

## Callback Gate Into Dispatcher

| VA | File offset | Word | Rabbitizer decoding |
|---|---:|---:|---|
| `0x001d3aec` | `0x0d4aec` | `0x8e22015c` | `lw $v0, 0x15C($s1)` |
| `0x001d3af0` | `0x0d4af0` | `0x8c440800` | `lw $a0, 0x800($v0)` |
| `0x001d3af4` | `0x0d4af4` | `0x8c830004` | `lw $v1, 0x4($a0)` |
| `0x001d3af8` | `0x0d4af8` | `0x38630001` | `xori $v1, $v1, 0x1` |
| `0x001d3afc` | `0x0d4afc` | `0x14600004` | `bnez $v1, . + 4 + (0x4 << 2)` |
| `0x001d3b00` | `0x0d4b00` | `0xdfbf0040` | `ld $ra, 0x40($sp)` |
| `0x001d3b04` | `0x0d4b04` | `0x0c074df2` | `jal func_801D37C8` |
| `0x001d3b08` | `0x0d4b08` | `0x0220202d` | `daddu $a0, $s1, $zero` |

As with the wrapper above, Rabbitizer's display prefix is formatting. The
encoded jump target corresponds to local VA `0x001d37c8`.

## Jump Table

| Entry | VA | File offset | Raw word | Target | Target section |
|---:|---:|---:|---:|---:|---|
| 0 | `0x00618fb0` | `0x519fb0` | `0x001d3818` | `0x001d3818` | `.text` |
| 1 | `0x00618fb4` | `0x519fb4` | `0x001d3844` | `0x001d3844` | `.text` |
| 2 | `0x00618fb8` | `0x519fb8` | `0x001d391c` | `0x001d391c` | `.text` |
| 3 | `0x00618fbc` | `0x519fbc` | `0x001d39e0` | `0x001d39e0` | `.text` |
| 4 | `0x00618fc0` | `0x519fc0` | `0x001d3a10` | `0x001d3a10` | `.text` |

## What Is Confirmed

1. Rabbitizer independently decodes `0x001d3800` as `sll $v1, $v1, 2`, matching the corrected Rev.023 interpretation.
2. The dispatcher sequence computes jump-table base `0x00618fb0` via `lui 0x62` plus signed `addiu -0x7050`.
3. The jump table at `0x00618fb0` contains five `.text` targets: `0x001d3818`, `0x001d3844`, `0x001d391c`, `0x001d39e0`, `0x001d3a10`.
4. The registration wrapper at `0x0013f7a8` delegates to the local target `0x0013f3f0` after argument shuffling.
5. The callback gate at `0x001d3b04` directly calls the local dispatcher target `0x001d37c8`.

## What Is Probable

1. Rabbitizer is suitable for future small-range instruction tables in research notes.
2. The combination of explicit section mapping plus independent decoding is a useful guard against Ghidra-only or manual-transcription mistakes.

## What Is Possible

1. A small project-local helper script could standardize this byte-to-instruction table generation.
2. spimdisasm can likely replace the explicit section mapping once the splat experiment produces stable symbols and sections.

## What Is Unknown

1. Whether spimdisasm's automatic function discovery agrees with the current Ghidra function boundaries.
2. Whether Rabbitizer formatting should be customized to avoid the `func_80...` display prefix in future notes.

## What Is Discarded

1. The old incorrect decoding `sll $3,$0,2` for `0x001d3800`.
2. Any need to revisit the wrong jump-table address `0x00628fb0` for this dispatcher.
3. Any semantic inference from SOTC code; this note only used the tooling style.

## Next Minimum Test

Use `splat64[mips]` on a minimal ICO YAML to check whether the ELF can be
split around the known section boundaries without disturbing the validated
dispatcher/callback anchors.

## Conservative Verdict

The SOTC-style independent decoding path is immediately useful. It confirms
the key Rev.023/025/037 byte-level facts without relying on Ghidra output.
The next tooling step should be a minimal splat experiment, still constrained
to already validated ICO addresses.
