# Rev.048 — C Scratch Model Synthesis and `ico_ptr32` Rule

## Date

2026-05-16

## Objective

Consolidate all C scratch matching results from the cloth function cluster into a
single status matrix with a fixed taxonomy. Establish the `ico_ptr32` type rule,
document GCC 2.95.2 limitations, and clearly separate exact matches from near-matches
and blocked functions.

This revision marks the transition from "experimental C matching" to a documented,
auditable decompilation pipeline with known capabilities and limits.

## Taxonomy

From this revision forward, every C-scratched function receives exactly one status:

| Status | Meaning |
|---|---|
| **EXACT** | All instructions match byte-identical or functionally identical. The C source is a correct decompilation. |
| **NEAR-STRUCTURAL** | Control flow, struct offsets, call sequence, and constants match exactly. Remaining differences are systematic compiler artifacts (register allocation, scheduling, frame size, shift width, constant loading method). The C logic is validated; the assembly is not bit-identical. |
| **NEAR-LOCAL** | Only a portion of the function matches. One or more basic blocks diverge due to unknown callees, missing struct context, or insufficient compiler tuning. |
| **MISMATCH** | C model or struct hypothesis is wrong. The generated assembly does not match the target control flow or data access pattern. |
| **BLOCKED** | Cannot be tested with current pipeline. Requires runtime data, toolchain upgrade, or external validation. |
| **ASM-HOLD** | Best kept in assembly. Suitable for thunks, GP-relative functions, or functions where C introduces too much overhead. |

NEAR-STRUCTURAL does not count as decompiled. It means the reverse-engineered
logic is validated but the compiler-specific differences remain unresolved.
Treating NEAR-STRUCTURAL as "decompiled" would misrepresent the project state.

## Status Matrix

All 9 functions in the cloth cluster that have been tested or triaged:

| Address | Symbol | Size | Status | Notes |
|---|---|---|---|---|
| `0x001D3D70` | cloth_sub_001d3d70 | 16B | **EXACT** | Simple leaf: returns `[payload + 0x28]`. `ico_ptr32` resolves `lw`. |
| `0x001D3D80` | cloth_sub_001d3d80 | 24B | **EXACT** | Simple leaf: writes `a1` to `[payload + 0x2c]`. `ico_ptr32` + `int` pack match. |
| `0x001D3D98` | cloth_sub_001d3d98 | 24B | **EXACT** | Simple leaf: writes `a1` to `[payload + 0x30]`. Same pattern. |
| `0x001D3DB0` | cloth_sub_001d3db0 | 40B | **NEAR-STRUCTURAL** | Bounds check with `xori`/`bnez`/two returns. Best C: `unsigned char` query. 8/10 instructions match; scheduler moves `nop` vs `move` in delay slot. |
| `0x001D3D40` | cloth_sub_001d3d40 | 48B | **NEAR-STRUCTURAL** | Conditional with two branches. Same instruction count (12). 8/12 match; differences: instruction reorder, `move` encoding (`or` vs `addu`). |
| `0x001D40A0` | cloth_sub_001d40a0 | 56B | **NEAR-STRUCTURAL** | Logical OR with early exit. Same instruction count (14). All registers differ (compiler allocation); structure identical. |
| `0x001D4358` | cloth_sub_001d4358 | 160B | **NEAR-STRUCTURAL** | Non-leaf pack color with 3 calls to `func_00111918`. Differences: frame 32 vs 48, `sll` vs `dsll` for shifts, missing tail call optimization, extra mask register. |
| `0x001D3BF0` | cloth_sub_001d3bf0 | 336B | **NEAR-STRUCTURAL** | Largest tested: float constants, table lookup, 9 callees, 128B frame. Control flow, call sequence, struct offsets, and constants match exactly. Differences: float constant method (`mtc1` vs `li+dsll32`), frame size, GP-relative vs literal table address. |
| `0x001D37C8` | cloth_dispatcher | 616B | **BLOCKED / ASM-HOLD** | Jump table with `.word` vs `.dword` issue. Cannot match without exact compiler. The jump table entries must be verified at the assembly level. |
| `0x001D27A8` | cloth_payload_init | 528B | **RUNTIME-MODELED** | Complex stack initializer. `a1 == sp` confirmed at runtime; `[a1+0x30]` captures values 0 and 1. Static C matching blocked by runtime-dependent stack layout. |

## The `ico_ptr32` Rule

**Rule**: All internal ICO pointers stored as 32-bit words in structs must use
`typedef int ico_ptr32` in C scratch sources.

**Rationale**: The ICO binary (ELF32, little-endian, MIPS III/R5900 with EABI64)
stores 32-bit pointers inside structs (`lw`/`sw` access). `int` generates `lw`
(matching the target). `void*` or any 64-bit type generates `ld` (mismatching).

**Evidence**: Confirmed across all 8 tested C scratches:
- 3 EXACT matches depend on `ico_ptr32` for correct `lw` generation.
- 5 NEAR-STRUCTURAL matches use `ico_ptr32` for pointer fields; the loads match.

**Not a final type**: `ico_ptr32` is a provisional alias. It does not model
function pointers (DescriptorRecord callbacks), which remain unverified.

**Usage pattern**:
```c
typedef int ico_ptr32;

// Loading a 32-bit pointer from a struct field:
struct_object = (SomeType*)*(ico_ptr32*)((char*)base + OFFSET);

// Loading a 32-bit value from a struct field:
value = *(ico_ptr32*)((char*)base + OFFSET);
```

## GCC 2.95.2 PS2 Linux — Confirmed Limitations

The compiler available at `ps2-gcc-295` (GCC 2.95.2 for PlayStation 2 Linux)
differs systematically from the original ee-gcc 2.9-991111-01 in these ways:

| Limitation | Effect |
|---|---|
| **Frame layout** | Different frame size and save order. The original compiler uses larger frames and different register save positions. |
| **Shift width** | `sll` (32-bit) vs `dsll` (64-bit) for shifts of 8/16/24. The original prefers 64-bit shifts even for 32-bit operations. |
| **Float constants** | `lui` + `mtc1` (original) vs `li` + `dsll32` + `sd` (GCC 2.95.2). The older GCC cannot use `mtc1` for constant loading. |
| **Tail call** | Original uses `j` (tail call) for sibling calls. GCC 2.95.2 emits `jal` + `jr ra`. |
| **Unsigned loads** | `(unsigned int)` cast generates `lwu` in GCC; original uses `lw` for all word loads. `int` fixes this. |
| **Absolute addresses** | GCC cannot use `lui` + `addiu` for arbitrary absolute addresses. Requires static pointer in `.data` or `extern` linkage. |
| **Dead multiply** | Original emits `mult` with unused result (possible macro artifact). GCC does not. |
| **Register allocation** | Different assignment of s-registers and caller-saved registers. |
| **`move` encoding** | Original sometimes uses `or rd, rs, zero`; GCC emits `addu rd, rs, zero`. Both encode to the same mnemonic in disassembly; differs only in the assembler. |

These differences are systematic and cannot be tuned away via C source changes.
They represent the compiler version gap, not incorrect decompilation.

## Functions Not Yet Tested

These cloth-cluster functions remain untested and retain their previous triage:

| Address | Size | Status | Why not tested |
|---|---|---|---|
| `0x001D3DD8` | 56B | **BLOCKED** | Depends on state resolver `func_0013EB50` / `EBE0` |
| `0x001D40D8` | 56B | **BLOCKED** | Same dependency |
| `0x001D4170` | 40B | **BLOCKED** | Same dependency |
| `0x001D4348` | 16B | **ASM-HOLD** | Thunk with GP-relative address |
| `0x001D45D0`+ | varies | **BLOCKED** | Larger functions needing more struct context |

Testing these before resolving the dependency chain would produce NEAR-LOCAL or
MISMATCH results with limited diagnostic value.

## Pipeline Status

```
C source → Docker ps2-gcc-295 → ee-gcc -O2 -G0 -mips3 -mgp64 -msingle-float
         → mips64r5900el-ps2-elf-as -march=r5900 -mips3 -mgp64 -mabi=eabi
         → mips64r5900el-ps2-elf-objdump → compare with target
```

**Flags used:** `-O2 -G0 -mips3 -mgp64 -msingle-float -mcpu=r5900 -fno-pic -mno-abicalls`

**Note:** `-march=r5900` is not directly supported by this GCC build.
`-mabi=eabi` is not in the wrapper script and is applied during assembly.

## What Is Confirmed

1. Three functions have exact C matches (EXACT).
2. Five functions have structurally validated C logic (NEAR-STRUCTURAL) with systematic compiler differences.
3. `ico_ptr32` (`typedef int`) is the correct provisional type for internal 32-bit pointers.
4. The GCC 2.95.2 PS2 Linux compiler produces near-match output but cannot close the remaining gaps.
5. The dispatcher (`0x001D37C8`) cannot be C-scratched without addressing the `.word`/`.dword` jump table issue.

## What Is Not Confirmed

1. Whether an ee-gcc 2.9-991111-01 compiler would produce exact matches for the near-structural functions.
2. Whether the remaining cloth functions can be C-scratched without resolver context.
3. The function pointer type for DescriptorRecord callbacks.
4. Whether `ico_ptr32` applies to all pointer fields across all structs in the binary.

## Next Minimum Steps

1. Resolve the ROPE callback gap via runtime session (relaxed `0x0013F7A8` filter in patch).
2. If runtime confirms the registration path, write C for the descriptor iteration chain.
3. After more struct context is validated, retest blocked functions.
4. Do NOT retest near-structural functions — no additional C tuning will close the compiler gap.

## Conservative Verdict

The C scratch experiment is successful as a validation methodology. It confirms
struct hypotheses, type rules, and control flow understanding for 8 of 9 tested
functions at the EXACT or NEAR-STRUCTURAL level.

However, NEAR-STRUCTURAL is not decompiled. The project has 3 exact matches
and 5 validated structural models. Full decompilation requires either:
1. Access to the original ee-gcc 2.9-991111-01 compiler, or
2. A decomp.me match with the correct compiler package, or
3. Manual reconstruction that accepts the compiler gap.

The pipeline is mature enough to test any new C scratch confidently, but the
remaining cloth functions should not be tested until their dependencies
(resolver, runtime, struct context) are resolved.
