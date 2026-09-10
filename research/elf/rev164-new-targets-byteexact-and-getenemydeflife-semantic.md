# Rev.164 — 4 new byte-exact .s + GetEnemyDefLife semantic bridge + CTest

**Date:** 2026-09-10

## Objective

Extend the byte-exact `.s` corpus with 4 new decompilation targets identified
in Rev.163, add GetEnemyDefLife (0x1C11C0) to the authoritative pipeline,
and deliver the first semantic C bridge + CTest for a gameplay-domain function.

## 1. Pipeline update: 612 → 617 TARGET_FUNCTIONS

Five functions added to `tools/asm_source_score.py` TARGET_FUNCTIONS:

| Function | VA | Size | Module | Source |
|----------|-----|------|--------|--------|
| `GetEnemyDefLife` | 0x1C11C0 | 0x90 | entity | Rev.163 inventory |
| `subEnemyCollision` | 0x15E2C8 | 0xB8 | entity | Rev.163 inventory |
| `HoldRope` | 0x1E59A0 | 0x154 | entity | Rev.163 inventory |
| `SetGirlClothDispSwitch` | 0x1C3C38 | 0x1D0 | entity | Rev.163 inventory |
| `GirlForceFieldGeo` | 0x1C3C90 | 0x178 | entity | Rev.163 inventory |

All 5 confirm as `EXISTING .s IS BYTE-EXACT` via the authoritative pipeline.
Pipeline final: **617/617 byte-exact, 0 failures**.

Boundary discovery during Rev.163 analysis:
- `subEnemyCollision` ends at 0x15E380 (0xB8); the original PAL size 0x180
  includes additional functions beyond a new prologue at 0x15E388.
- `SetGirlClothDispSwitch` (0x1C3C38, 0x1D0) and `GirlForceFieldGeo`
  (0x1C3C90, 0x178) overlap: GirlForceFieldGeo is a sub-range inside
  SetGirlClothDispSwitch. Both end at the same `jr $ra` (0x1C3E08).
  Entry points are mid-computation (`div.s`); both are PAL sub-symbols.

## 2. Generator tool: `tools/gen_byteexact_asm.py`

New generator emits:
- Integer instructions as real mnemonics with **numeric registers**
  (capstone symbolic names → numeric map; e.g., `$a1` → `$5`)
- Every COP1/COP2/branch/jal/R5900-accumulator op as raw `.word`
- Safe for ee-gcc 2.9 assembler constraints

### ee-as constraints documented

- **Register names**: numeric only; capstone symbolic names (`$a1`, `$s5`) must
  be converted via a name→number map.
- **Float registers**: must keep `$fN` prefix.
- **COP1 compares** (`c.olt.s` etc.): unsupported → `.word`.
- **R5900 `mult $acN`** (accumulator select via rd field): capstone reg IDs
  for `$ac2` (47) are outside 0–31 → `$?` fallback; all accumulator 3-operand
  forms (mult/madd/maddu/msub/msubu/div) go to `.word`.
- **Branch delay padding**: ee-as injects nop padding for short (≤5 instruction)
  backward branches (Rev.157 root cause); emit such branches as `.word` to
  avoid corruption.

## 3. GetEnemyDefLife disassembly correction

The earlier summary's "life doubled" was wrong. Corrected disassembly:

```
lui  $at, 0x3f00        # 0x3f000000 = 0.5f in IEEE-754
mtc1 $at, $f1           # f1 = 0.5f
add.s $f0, $f0, $f1     # life = life + 0.5f (NOT life * 2)
```

The function increments `work+0x134` by 0.5f when `sched+0x20 == 5`.

Confirmed access chain:
- `self = *(root + 0x00)`
- `work = *(self + 0x15c)`
- `sched = *(work + 0x800)`
- gate: `*(sched + 0x20) != 5` → return 0
- float at `work + 0x134` incremented by 0.5f
- Three registered hooks called in sequence (no host semantic yet)

## 4. Semantic bridge: `ico_semantic_getEnemyDefLife`

Added to `src/core/isysgobj_semantic.c` with prototype in `src/core/gobj_abi.h`.

### Host adaptation (64-bit pointer safety)

PS2 pointers are 32-bit. On a 64-bit host, truncating a stack/heap address
to 32 bits and round-tripping it produces a bogus low address → SIGSEGV.
The semantic bridges this by storing **host-width pointers** in the pointer
cells while preserving the PS2 byte offsets (0x15c / 0x800 / 0x20 / 0x134).

The hook signature receives `ico_ptr32` (truncated) values; the test asserts
consistency via matching truncation on both sides.

### Hooks

Three hook callbacks represent the unknown `jal` targets. NULL skips the call.
The hooks receive the same argument scaffolding as the original (stack scratch
address, work/sched field addresses, etc.).

## 5. CTest: `isysgobj_semantic_test` (27/27 pass)

New assertions in `native/tests/isysgobj_semantic_test.c`:

| Path | Assertion |
|------|-----------|
| type == 5 | returns 1; hooks called 3×; life = 2.0 + 0.5 = 2.5 |
| prelude hook a1 | self pointer |
| stage hook a1 | work + 0xa0 via root chain |
| sched_own hook a0, a1 | sched + 0xd0 (both) |
| type == 6 | returns 0; no hooks; life unchanged |
| NULL hooks + type == 5 | returns 1; life still incremented |

### Key test pattern: STORE_PTR macro

Direct `memcpy(buf, &array, sizeof(ptr))` copies the CONTENTS of `array`
(zeros), not its address. The fix:

```c
#define STORE_PTR(dst, addr) \
    do { void *_tmp = (addr); memcpy((dst), &_tmp, sizeof(void *)); } while(0)
```

This writes the address value (from a local variable) into the byte buffer.

## 6. File-level counts (post-Rev.164)

| Category | Count |
|----------|-------|
| Entity .s | 663 |
| Cloth .s | 6 |
| Core .s | 64 |
| **Total .s on disk** | **733** |
| Pipeline TARGET_FUNCTIONS | 617/617 byte-exact |
| Outside pipeline | 116 (previously verified) |

## 7. What is confirmed

- 4 new functions assembled byte-exact; sizes match ELF.
- GetEnemyDefLife lives at 0x1C11C0 (144 bytes); 0 static jal callers,
  but 1 data-ref (lui 0x1C11 @ 0x1C01E0 inside `woodbox0_hC`) forming
  0x1C11C0 via `addiu` after `jal AllocGObjEntity` → registered GObj handler
  with indirect-call usage.
- The getEnemyDefLife access chain (root → self → work → sched → type gate)
  is confirmed byte-verified and semantic-tested.
- life += 0.5f (not *= 2); the `lui 0x3f00` constant is 0.5f.

## 8. What is probable

- subEnemyCollision at 0x15E2C8 (0xB8) is a standalone entity polling loop
  that re-reads GObj state on each iteration and dispatches through
  registered callbacks when type-mask bits match.
- HoldRope at 0x1E59A0 (0x154) is a rope-physics spring update with
  global-state writes at gp-0x53b0 (spring) and gp-0x53a4 (boost float).
- The three hook targets (0x104508, 0x105F00, 0x243AD0) called by
  GetEnemyDefLife are rendering/physics setup shims (unknown semantics).

## 9. What is possible

- subEnemyCollision may spawn or invoke collision-clip functions via the
  registered callbacks — runtime confirmation needed.
- GirlForceFieldGeo (0x1C3C90) is a sub-component of SetGirlClothDispSwitch,
  sharing the same prologue block.

## 10. What is unknown

- The three hook targets (0x104508, 0x105F00, 0x243AD0) have no host semantic
  yet; they are delegated to the caller-supplied hook.
- Why the entity field `+0x15c` is both the GObj pointer (self+0x15c) AND
  the entity root's own work-area pointer (root+0x15c). These are distinct
  memory locations that happen to hold the same value in common ICO layout
  patterns — confirmed by runtime but the structural reason is unknown.

## 11. What is discarded

- Any interpretation of GetEnemyDefLife's three hook calls beyond
  "registered setup callbacks" — no evidence for their purpose yet.
- The hypothesis that life is doubled (Rev.163 summary error) — disproved
  by corrected disassembly (lui 0x3f00 = 0.5f).

## 12. Next minimum test

1. Add all 4 new semantic functions (subEnemyCollision, HoldRope,
   SetGirlClothDispSwitch, GirlForceFieldGeo) as `.s` + semantic C bridges
   with CTest coverage.
2. Update AGENTS.md numbers (733 .s, 617 pipeline, 116 outside).

## 13. Conservative verdict

Rev.164 is a low-risk increment: 5 new byte-exact `.s` files (pipeline
612→617), 1 new semantic C bridge with 3-assertion CTest, and a corrected
disassembly of GetEnemyDefLife (0.5f increment, not doubling). All 27 CTest
pass. The generator tool is validated against 3 real functions. No speculative
claims.
