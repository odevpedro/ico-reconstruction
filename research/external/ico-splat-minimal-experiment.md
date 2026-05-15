# ICO Minimal splat Experiment

## Date

2026-05-15

## Objective

Test whether `splat64[mips]`, following the tooling direction opened from the
SOTC survey, can model the local ICO USA ELF well enough to produce an
auditable split around already validated anchors.

This is a tooling experiment only. It does not validate new game semantics and
does not attempt a rebuild.

## Scope

Included:

- local `splat64[mips]` installation in a temporary venv;
- minimal `.text` split of `.local/extracted/SCUS_971.13.elf`;
- full-file section coverage using `databin` for non-text sections and DVP
  overlay payloads;
- validation of known anchors in generated output.

Excluded:

- compiling or linking a rebuilt ELF;
- committing generated split assets;
- decompilation of functions;
- semantic naming;
- DVP overlay analysis beyond confirming splat can carry the bytes as blobs;
- use of SOTC source as ICO evidence.

## Sources Used

| Source | Use |
|---|---|
| `.local/extracted/SCUS_971.13.elf` | local target ELF |
| `readelf -h -S` | verified section boundaries |
| `sha1sum` | target identity |
| `/tmp/ico-tooling-venv` | temporary Python environment |
| `splat64[mips] 0.34.0` | split experiment |
| `spimdisasm 1.35.0` | splat disassembly backend |
| `research/external/sotc-tooling-relevance-survey.md` | motivation |
| `research/external/ico-rabbitizer-spimdisasm-dispatcher-check.md` | anchor expectations |
| `research/elf/ghidra-rev023-dispatcher-table-resolution.md` | dispatcher/table ground truth |
| `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md` | registration callsite expectations |

## Evidence Used

Target hash:

```txt
a4d8fc1948fb2da2395f3863a94a3b93de55de14  .local/extracted/SCUS_971.13.elf
```

Temporary tooling:

```txt
/tmp/ico-tooling-venv
splat64[mips] 0.34.0
spimdisasm 1.35.0
Rabbitizer 1.16.0
```

Working directory:

```txt
/tmp/ico-splat-minimal
```

No generated split output was placed in the repository.

## Test 1: Minimal `.text` Split

### YAML shape

The first config modeled only:

| Segment | File start | VA | Treatment |
|---|---:|---:|---|
| ELF header | `0x000000` | n/a | `databin` |
| `.text` | `0x001000` | `0x00100000` | `code` with one `asm` subsegment |
| end marker | `0x1705d4` | n/a | end of `.text` file range |

The command:

```sh
/tmp/ico-tooling-venv/bin/python -m splat.scripts.split \
  /tmp/ico-splat-minimal/config/SCUS_971.13.minimal.yaml \
  --modes all --verbose --skip-version-check
```

Result:

```txt
Split 1 MB (27.53%) in defined segments
databin: 4 KB
asm:     1 MB
unknown: 0 B
```

Generated files included:

| File | Role |
|---|---|
| `/tmp/ico-splat-minimal/asm/main_text.s` | full `.text` assembly |
| `/tmp/ico-splat-minimal/assets/elf_header.databin.bin` | ELF/header blob |
| `/tmp/ico-splat-minimal/SCUS_971.13.ld` | generated linker script |
| `/tmp/ico-splat-minimal/undefined_syms_auto.txt` | generated symbols, including jump table symbol |

## Test 2: Full File Coverage as Sections/Blobs

### YAML shape

The second config kept `.text` as one asm segment and modeled the rest of the
ELF as `databin` slices based on `readelf` section offsets.

Important ranges:

| Range | File start | Treatment |
|---|---:|---|
| `.text` | `0x001000` | `code` / `asm` |
| `.vutext` | `0x1705e0` | `databin` |
| `.data` | `0x175700` | `databin` |
| `.rodata` | `0x454700` | `databin` |
| `.lit4` | `0x531900` | `databin` |
| `.sdata` | `0x532900` | `databin` |
| `.reginfo` | `0x534c0c` | `databin` |
| `.DVP.ovlytab` | `0x534c24` | `databin` |
| `.DVP.ovlystrtab` | `0x534cb4` | `databin` |
| `.DVP.overlay...` payloads | `0x534e42..0x539e42` | `databin` |
| `.shstrtab` | `0x539e42` | `databin` |
| section headers | `0x53a050` | `databin` |
| end marker | `0x53a488` | end of file |

The command:

```sh
/tmp/ico-tooling-venv/bin/python -m splat.scripts.split \
  /tmp/ico-splat-minimal/config/SCUS_971.13.sections.yaml \
  --modes all --verbose --skip-version-check
```

Result:

```txt
Split 5 MB (100.00%) in defined segments
databin: 3 MB (72.55%) 28 split
asm:     1 MB (27.45%) 1 split
unknown: 0 B
```

This confirms that `splat` can tolerate the ICO ELF file layout, including the
DVP payload region, when non-text regions are carried as blobs. It does not mean
DVP overlays have been semantically modeled.

## Anchor Validation

### Generated symbol / assembly hits

| Anchor | Generated output |
|---|---|
| `0x0013f7a8` | `glabel func_0013F7A8` in `asm_sections/main_text.s` |
| `0x001d37c8` | `glabel func_001D37C8` in `asm_sections/main_text.s` |
| `0x001d3800` | `sll $v1, $v1, 2` in `func_001D37C8` |
| `0x001d3b04` | `jal func_001D37C8` |
| `0x00618fb0` | `jtbl_00618FB0_main_text = 0x618FB0` in `undefined_syms_auto.txt` |

### Registration callsites

The generated assembly also preserves the five known static callsites of
`0x0013f7a8`:

| VA | Generated output |
|---:|---|
| `0x001b7ab0` | `jal func_0013F7A8` |
| `0x001b7acc` | `jal func_0013F7A8` |
| `0x00201ed4` | `jal func_0013F7A8` |
| `0x00240e50` | `jal func_0013F7A8` |
| `0x00240f90` | `jal func_0013F7A8` |

### Dispatcher excerpt

Generated `splat`/spimdisasm output around the dispatcher:

```asm
glabel func_001D37C8
    /* D47C8 001D37C8 A0FFBD27 */  addiu      $sp, $sp, -0x60
    /* D47E4 001D37E4 5C01428E */  lw         $v0, 0x15C($s2)
    /* D47E8 001D37E8 0008538C */  lw         $s3, 0x800($v0)
    /* D47EC 001D37EC 40007126 */  addiu      $s1, $s3, 0x40
    /* D47F0 001D37F0 0800238E */  lw         $v1, 0x8($s1)
    /* D47F4 001D37F4 0500622C */  sltiu      $v0, $v1, 0x5
    /* D47F8 001D37F8 07004010 */  beqz       $v0, .L001D3818
    /* D47FC 001D37FC 6200023C */   lui       $v0, %hi(jtbl_00618FB0_main_text)
    /* D4800 001D3800 80180300 */  sll        $v1, $v1, 2
    /* D4804 001D3804 B08F4224 */  addiu      $v0, $v0, %lo(jtbl_00618FB0_main_text)
    /* D4808 001D3808 21186200 */  addu       $v1, $v1, $v0
    /* D480C 001D380C 0000648C */  lw         $a0, 0x0($v1)
    /* D4810 001D3810 08008000 */  jr         $a0
    /* D4814 001D3814 00000000 */   nop
```

### Jump table in extracted `.rodata` blob

The full-section split extracted `.rodata` as:

```txt
/tmp/ico-splat-minimal/assets_sections/rodata.databin.bin
```

The jump table local offset inside this blob is:

```txt
0x519fb0 - 0x454700 = 0x0c58b0
```

Raw words at that offset:

| Entry | Word |
|---:|---:|
| 0 | `0x001d3818` |
| 1 | `0x001d3844` |
| 2 | `0x001d391c` |
| 3 | `0x001d39e0` |
| 4 | `0x001d3a10` |

## Output Inventory

Important generated files:

| Path | Size | Use |
|---|---:|---|
| `/tmp/ico-splat-minimal/asm_sections/main_text.s` | 23 MB | full `.text` disassembly |
| `/tmp/ico-splat-minimal/SCUS_971.13.sections.ld` | 16 KB | generated linker script |
| `/tmp/ico-splat-minimal/undefined_syms_auto.txt` | 79 KB | generated data/jump-table symbols |
| `/tmp/ico-splat-minimal/undefined_funcs_auto.txt` | 104 B | generated unresolved funcs file |
| `/tmp/ico-splat-minimal/assets_sections/rodata.databin.bin` | 905,728 bytes | `.rodata` blob |
| `/tmp/ico-splat-minimal/assets_sections/data.databin.bin` | 3,010,488 bytes | `.data` blob |
| `/tmp/ico-splat-minimal/assets_sections/vutext.databin.bin` | 20,704 bytes | `.vutext` blob |

## What Is Confirmed

1. `splat64[mips] 0.34.0` can split the ICO USA ELF `.text` section into a full assembly file without blocking.
2. `splat` recognizes function labels at key anchors including `func_001D37C8` and `func_0013F7A8`.
3. `splat`/spimdisasm independently preserves the corrected dispatcher instruction `0x001d3800: sll $v1,$v1,2`.
4. `splat` generates an automatic symbol for the dispatcher jump table: `jtbl_00618FB0_main_text`.
5. A full-file YAML can reach 100% file coverage if non-text sections and DVP payloads are treated as `databin`.
6. DVP payload bytes can be carried by splat as blobs without preventing the main `.text` split.

## What Is Probable

1. A more refined ICO splat config is feasible.
2. The SOTC workflow pattern is practically useful for ICO tooling, at least through the split/disassembly stage.
3. `splat` can become the basis for future decomp.me scratch generation and SDK/library segmentation.

## What Is Possible

1. Future configs can promote selected ranges from monolithic `asm` into named files once boundaries are validated.
2. `.rodata` can later be modeled as `rodata` instead of `databin` after symbol and table handling is understood.
3. DVP overlays may need custom treatment if the project moves beyond byte-carrying blobs into overlay analysis.

## What Is Unknown

1. Whether this config can be linked back into a byte-identical ELF.
2. Whether generated function boundaries are all correct; only known anchors were checked.
3. Whether `splat` can model ICO's `.DVP.*` sections semantically rather than as raw blobs.
4. Whether the correct ICO compiler package is available for future matching.
5. Whether auto-generated labels in the monolithic asm align with Ghidra/ICO-decomp boundaries outside the checked anchors.

## What Is Discarded

1. The concern that `splat` cannot parse the ICO ELF at all.
2. The concern that DVP payload sections necessarily block a basic splat split.
3. Any interpretation of this experiment as a rebuild or source reconstruction milestone.

## Follow-up Completed

The proposed next test was completed in:

```txt
research/external/ico-splat-promoted-ranges-experiment.md
```

That follow-up promoted a few verified functions/ranges out of the monolithic
`main_text.s`, starting with:

| Range | Reason |
|---|---|
| `0x001d37c8` | cloth dispatcher and jump table user |
| `0x001d3a30` | runtime-confirmed callback |
| `0x0013f7a8` | registration wrapper |
| `0x0013f3f0` | callback storage function |

It also found and corrected two initial boundary mistakes. Do not attempt broad
source-file reconstruction until function boundaries and compiler setup are
validated.

## Conservative Verdict

The minimal splat experiment succeeded. `splat64[mips]` is viable as an
independent split/disassembly path for ICO USA, and it can carry the full ELF
layout as defined segments when non-text regions are treated conservatively as
raw blobs.

This materially improves the tooling plan, but it does not change any semantic
conclusion about the cloth dispatcher, callback registration gap, or gameplay
systems.
