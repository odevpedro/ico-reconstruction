# SOTC Tooling Relevance Survey

## Date

2026-05-15

## Objective

Open a separate investigation front to evaluate whether tooling and workflow
from `Fantaskink/SOTC` can help this ICO reconstruction project.

This note evaluates possible tooling gains only. It does not import SOTC
technical conclusions into ICO and does not validate new ICO game behavior.

## Scope

Included:

- public SOTC decompilation repository structure;
- local clone inspection at `/tmp/SOTC`;
- SOTC build/config/diff/tooling scripts;
- comparison against the local ICO ELF shape;
- possible low-risk tests for ICO.

Excluded:

- use of SOTC proprietary input files;
- building SOTC;
- copying SOTC source into this repository;
- gameplay or semantic comparison between SOTC and ICO;
- broad asset investigation;
- runtime PCSX2 validation.

## Sources Used

| Source | Use |
|---|---|
| `https://github.com/Fantaskink/SOTC` | public repository under evaluation |
| `/tmp/SOTC` | shallow local clone for file/tool inspection |
| `/tmp/SOTC/README.md` | setup requirements and project target |
| `/tmp/SOTC/configure.py` | main build graph generation model |
| `/tmp/SOTC/config/SCPS_150.97/SCPS_150.97.yaml` | splat YAML example for PS2 ELF |
| `/tmp/SOTC/config/KERNEL.XFF/KERNEL.XFF.yaml` | XFF/module segmentation example |
| `/tmp/SOTC/requirements.txt` | Python tooling dependencies |
| `/tmp/SOTC/tools/first_diff.py` | Rabbitizer-backed first-difference helper |
| `/tmp/SOTC/tools/elf_patcher.py` | spimdisasm-backed ELF alignment patcher |
| `/tmp/SOTC/tools/parse_xff_relocs.py` | XFF relocation tooling |
| `/tmp/SOTC/scripts/setup_compiler.sh` | decomp.me compiler download/setup model |
| `.local/extracted/SCUS_971.13.elf` | local ICO USA ELF comparison target |
| `research/elf/ghidra-rev038-decompme-scratches.md` | current ICO compiler/decomp.me state |
| `research/ico-decomp-cross-reference-2026-05-14.md` | ICO-decomp cross-reference and source tree |
| `research/elf/ghidra-rev039-cloth-domain-correction.md` | latest domain correction for dispatcher/callback |

## Evidence Used

### SOTC repository facts

| Evidence | Finding |
|---|---|
| `README.md` | SOTC targets the preview main ELF `SCPS_150.97` and `KERNEL.XFF`; setup requires Linux/WSL2, MIPS binutils, Python venv, `ninja`, and `scripts/setup_compiler.sh`. |
| `requirements.txt` | Uses `spimdisasm==1.35.0`, `rabbitizer`, `splat64[mips]==0.34.0`, `tqdm`, `ninja_syntax`, and `mapfile_parser`. |
| `configure.py` | Generates per-target Ninja files from splat linker entries; handles asm/data/c/cpp/databin/rodatabin/textbin segments. |
| `configure.py` | Uses `ee-gcc2.96`, `mips-linux-gnu-*`, custom `regnames.specs`, `objcopy`, `sha1sum`, and compiler/permuter settings generation. |
| `scripts/setup_compiler.sh` | Downloads `ee-gcc2.96` from decomp.me compiler releases into `tools/cc/ee-gcc2.96`. |
| `tools/first_diff.py` | Uses Rabbitizer to decode first binary differences and resolves jump targets through map files. |
| `tools/elf_patcher.py` | Uses spimdisasm ELF parsing to patch section alignment in object files produced by `gas`. |
| `tools/parse_xff_relocs.py` | Dedicated XFF relocation parser, likely useful only for later overlay/module work. |
| `config/SCPS_150.97/SCPS_150.97.yaml` | Demonstrates a PS2 ELF splat config with section order, SDK library segmentation, `gp_value`, compiler/platform settings, and named-register options. |
| `config/KERNEL.XFF/KERNEL.XFF.yaml` | Demonstrates module/XFF segmentation and many `ios/*` source group names. |

### Local ICO ELF facts

`readelf -h -S .local/extracted/SCUS_971.13.elf` reports:

| Item | ICO USA value |
|---|---|
| ELF class | ELF32 |
| Endianness | little endian |
| Type | executable |
| Machine | MIPS R3000 as reported by ELF header tooling |
| Flags | `0x20924001`: `noreorder`, `5900`, `eabi64`, `mips3` |
| Entry | `0x00100008` |
| Program headers | 1 |
| Section count | 27 |
| `.text` | VA `0x00100000`, offset `0x001000`, size `0x16f5d4`, align 64 |
| `.vutext` | VA `0x0026f5e0`, offset `0x1705e0`, size `0x50e0` |
| `.data` | VA `0x00274700`, offset `0x175700`, size `0x2defb8`, align 64 |
| `.rodata` | VA `0x00553700`, offset `0x454700`, size `0x0dd1a8` |
| `.lit4` | VA `0x00630900`, offset `0x531900`, size `0x0fd0` |
| `.sdata` | VA `0x00631900`, offset `0x532900`, size `0x22c6` |
| `.bss` | VA `0x00634000`, size `0x0eab98` |
| DVP overlays | present as `.DVP.ovlytab`, `.DVP.ovlystrtab`, and multiple `.DVP.overlay...` sections |

## Important Separation

| Category | Status |
|---|---|
| Tooling/process lessons from SOTC | Useful evidence for workflow design. |
| SOTC source names or behavior | Not evidence for ICO semantics. |
| SOTC compiler version | Comparable but not directly transferable. |
| SOTC XFF/module tooling | Possible future fit; not current dispatcher evidence. |
| SOTC Team Ico adjacency | Context only, not proof. |

## Tooling Relevance Matrix

| SOTC component | Current ICO relevance | Possible gain | Risk / caution | Minimum test |
|---|---:|---|---|---|
| `splat64[mips]` YAML workflow | High | Generate an independent, auditable split of `SCUS_971.13.elf` into asm/data/rodata regions. | Bad YAML boundaries could create false structure. Treat output as tooling artifact until byte-checked. | Create an experimental ICO YAML that only models known ELF sections and confirms `0x001d37c8`, `0x001d3a30`, `0x0013f7a8`, and `0x00618fb0` offsets. |
| `rabbitizer` | High | Independent instruction decoding for disputed or important MIPS words. | Decoder output still needs raw-byte anchoring and R5900 awareness. | Decode the Rev.023 dispatcher words and verify `sll $3,$3,2` and jump-table load sequence. |
| `spimdisasm` | High | Matching-oriented disassembly and ELF parsing independent of Ghidra. | Initial function detection may not match known boundaries. | Run only on selected ranges first: dispatcher, callback, registration wrappers. |
| `tools/first_diff.py` pattern | Medium | Useful later for first binary mismatch reporting with decoded MIPS instructions. | Requires built output and map files; not useful before a rebuild path exists. | Port concept only after an ICO minimal rebuild emits a map and binary. |
| `tools/elf_patcher.py` pattern | Medium | Shows how to patch ELF object section alignment when binutils output disagrees with expected PS2 alignment. | Direct script is SOTC-specific in assumptions; do not copy blindly. | Keep as reference if ICO object rebuild hits alignment mismatches. |
| `ninja_syntax` build generation | Medium | Reproducible per-target build graph once ICO has split objects. | Premature before split/rebuild strategy is validated. | Generate a dry-run Ninja for a tiny experimental target only. |
| `sha1sum` rebuild check | Medium | Strong final validation model for matching rebuilds. | Requires reconstructing enough binary layout first. | Add only when a build artifact is intended to match original bytes. |
| `ee-gcc2.96` setup | Medium | Demonstrates decomp.me compiler packaging and local compiler pinning. | ICO Rev.038 points to EE GCC `2.9-991111-01`, not `2.96`; using SOTC compiler as if identical would be wrong. | Compare generated code for tiny known functions only after finding/installing the correct ICO compiler if available. |
| `permuter_settings.toml` generation | Medium-later | Helps wire compiler/assembler into decomp-permuter once functions are near matching. | Permuter is late-stage; not a semantic discovery tool. | Do not use until a candidate C function is already close. |
| SDK include tree | Medium | May help recognize PS2 SDK APIs and types. | Header provenance/licensing and version mismatch must be checked before copying. | Use as read-only reference for names/types, then verify against ICO bytes/calls. |
| SDK library segmentation in YAML | Medium | Helps avoid wasting reverse-engineering effort on standard SDK/libc/libgcc code. | Library versions may differ between SOTC and ICO. | Compare byte signatures of known libc/libgcc functions before naming. |
| `parse_xff_relocs.py` | Low now, Medium future | Useful model if ICO overlay/module relocation work becomes active. | Current priority is ELF cloth/callback/runtime, not XFF/DVP overlay extraction. | Defer; revisit only in an overlay-specific note. |
| `fix_xff.py` | Low now | Future XFF/module build workaround reference. | SOTC-specific. | Defer. |
| SOTC `ios/*` source organization | Medium as naming hint | Cross-checks that `ios/thread`, `ios/memory`, `ios/isys` style organization is plausible for Team Ico codebases. | Naming similarity is weak evidence. ICO-decomp is stronger for ICO names. | Prefer ICO-decomp for source mapping; use SOTC only as secondary process context. |
| SOTC gameplay/entity code | Low | May eventually provide broad engine-pattern intuition. | Very high risk of importing wrong semantics. | Do not use for current cloth dispatcher conclusions. |

## Possible Immediate Gains for ICO

### 1. Experimental splat configuration

Potential gain:

- produce a second, reproducible disassembly/splitting path;
- make function package generation less ad hoc;
- prepare eventual matching rebuild infrastructure.

Minimum safe prototype:

| Requirement | Conservative constraint |
|---|---|
| Target file | `.local/extracted/SCUS_971.13.elf` |
| Output location | local experimental folder, not mixed into established notes |
| Initial scope | section boundaries only; no broad invented function names |
| Validation anchors | `0x001d37c8`, `0x001d3a30`, `0x0013f7a8`, `0x0013f3f0`, `0x00618fb0` |
| Acceptance | generated offsets must match existing byte-level notes |

### 2. Independent MIPS decoding harness

Potential gain:

- reduce reliance on Ghidra output;
- provide compact scripts for byte/instruction tables in future notes;
- prevent repeats of the old `0x001d3800` transcription issue.

Minimum safe prototype:

| Target | Expected verification |
|---|---|
| `0x001d37c8..0x001d3814` | dispatcher prologue and table dispatch sequence |
| `0x001d3800` | `sll $3,$3,2`, not `sll $3,$0,2` |
| `0x0013f7a8` | wrapper argument shuffle into `0x0013f3f0` |
| `0x001d3b04` | direct `jal 0x001d37c8` from callback |

### 3. SDK/library recognition pass

Potential gain:

- identify standard library regions;
- reduce false “game function” claims;
- improve future splat YAML segmentation.

Minimum safe prototype:

| Step | Evidence required |
|---|---|
| Pick one known library-like function | byte signature or instruction sequence, not name intuition |
| Compare with SOTC SDK/lib sections | exact or near-exact instruction-level match |
| Document result | separate confirmed/probable/possible classification |

### 4. decomp.me compiler packaging model

Potential gain:

- use decomp.me compiler archives consistently;
- generate local `permuter_settings.toml`;
- standardize scratches and local matching experiments.

Caution:

SOTC uses `ee-gcc2.96`. ICO Rev.038 currently identifies:

```txt
ee-gcc 2.9-991111-01
-march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2
```

Therefore SOTC's compiler setup is useful as a packaging pattern, not as proof
that ICO should use `ee-gcc2.96`.

## What Is Confirmed

1. SOTC has a mature PS2 decompilation workflow using `splat`, `ninja`, `spimdisasm`, `rabbitizer`, MIPS binutils, and decomp.me compiler packages.
2. The SOTC workflow includes reproducibility controls: generated Ninja files, linker scripts, map files, binary extraction via `objcopy`, and `sha1sum` checks.
3. SOTC's helper scripts show concrete solutions for first-difference decoding and ELF section alignment issues.
4. The local ICO ELF is structurally suitable for a splat-style experiment: ELF32 little-endian MIPS, fixed section table, known `.text/.data/.rodata/.lit4/.sdata/.bss`, and stable address anchors.
5. SOTC does not provide direct evidence for ICO's `0x001d37c8` cloth dispatcher or `0x001d3a30` callback semantics.

## What Is Probable

1. A minimal splat config for ICO can be created and used as an independent audit path for selected function ranges.
2. Rabbitizer/spimdisasm can improve research-note reproducibility by replacing manual or Ghidra-only instruction listings.
3. SOTC's SDK segmentation strategy can help design an ICO library-recognition pass, especially for libc/libgcc/libkernl/libgraph-like code.
4. SOTC's build scripts can guide a future ICO matching pipeline, but only after the correct compiler version and flags are validated locally.

## What Is Possible

1. Some SOTC IOS/module names may correspond conceptually to ICO `fumi/ios` infrastructure, but ICO-decomp remains the stronger source for ICO-specific source layout.
2. SOTC's XFF relocation tooling may become useful if the ICO project later studies overlay/module relocation formats.
3. SOTC's `elf_patcher.py` alignment strategy may solve future object alignment mismatches in ICO rebuild experiments.

## What Is Unknown

1. Whether `splat64[mips]==0.34.0` can parse the ICO ELF and DVP overlay sections without local extensions.
2. Whether the correct ICO compiler package is available in decomp.me compiler archives.
3. Whether ICO's SDK/library versions match SOTC closely enough for byte-signature transfer.
4. Whether SOTC's XFF tooling is relevant to ICO's DVP overlay sections or only to SOTC's own module format.

## What Is Discarded

1. SOTC gameplay or source names as direct evidence for ICO gameplay semantics.
2. SOTC `ee-gcc2.96` as a confirmed ICO compiler.
3. Any assumption that Team Ico adjacency alone proves shared function behavior.
4. Any use of SOTC tooling output as a conclusion without byte-level validation against `SCUS_971.13.elf`.

## Next Minimum Tests

| Priority | Test | Output |
|---:|---|---|
| 1 | Create a minimal ICO splat YAML for `.local/extracted/SCUS_971.13.elf` using only verified section boundaries. | Experimental split output or documented blocker. |
| 2 | Run Rabbitizer/spimdisasm decoding on the Rev.023 dispatcher range. | Instruction table matching existing byte-level evidence. |
| 3 | Check whether splat can tolerate or model ICO `.DVP.*` sections without corrupting main ELF boundaries. | Clear yes/no and required YAML treatment. |
| 4 | Compare one known SDK/libc-like ICO function against SOTC SDK/lib segmentation methodology. | Confirmed/probable/discarded library-recognition result. |
| 5 | Investigate availability of `ee-gcc 2.9-991111-01` in decomp.me compiler packages. | Compiler availability note; no compiler substitution unless validated. |

## Conservative Verdict

The SOTC repository is relevant to ICO primarily as a tooling and process
reference. The strongest immediate gains are a minimal splat experiment,
independent Rabbitizer/spimdisasm instruction decoding, and a future
SDK/library recognition workflow.

The repository should not be used as semantic evidence for the ICO cloth
dispatcher, ROPE/cloth callback, or gameplay systems. Any adopted tool must be
validated against local ICO bytes and existing research anchors before it is
allowed to influence project conclusions.
