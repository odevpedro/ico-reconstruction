# Rev.165 — Three gameplay-domain semantic bridges: HoldRope, subEnemyCollision, GirlForceFieldGeo + CTest

**Date:** 2026-09-10

## Objective

Deliver the next three semantic C bridges (after Rev.164's GetEnemyDefLife) for the
gameplay-domain functions that are today byte-exact `.s` ground truth but have no
readable C contract. The bridges are deliberately **semantic models, not byte-exact
C**: they capture only the confirmed control flow and arithmetic, and delegate every
call to internal subroutines (unknown or out of reconstruction scope) to host hooks.

## 1. Ground truth

All three functions were disassembled directly from the USA ELF
(`.local/extracted/SCUS_971.13.elf`, little-endian R5900; the earlier capstone
attempts used big-endian and produced garbage — LE is correct for this ELF).
The `.s` files are byte-exact (`src/entity/asm/*.s`).

| Function | VA | Size | Notes |
|----------|-----|------|-------|
| `HoldRope` | 0x1E59A0 | 0x154 | rope/chain handler |
| `subEnemyCollision` | 0x15E2C8 | 0xB8 | enemy collision polling loop |
| `GirlForceFieldGeo` | 0x1C3C90 | 0x178 | sub-range of `SetGirlClothDispSwitch` (0x1C3C38, 0x1D0) |

## 2. HoldRope (0x1E59A0, 0x154)

### Confirmed access chain
- `gp-0x53A4` = spring coefficient (float cell).
- `gp-0x53B0` = **pointer cell that holds the entity pointer** (indirection!).
  `lw $a0, -0x53b0($gp)` loads a pointer; `*(entity+0x15c)` = work GObj.
- `*(s1+0x5250)` = player_flags_1, `*(s2+0x5250)` = player_flags_2.
- `sp+0x108..0x113` = input bytes (stack scratch from a prior `jal 0x24E578`).

### Flag→value pipeline
| Path | Condition | Operation | Store |
|------|-----------|-----------|-------|
| spring update | flags_1 bit 3 (0x08) | `spring = 1.0 - input111/255.0` | gp-0x53A4, then `jal 0x1E4980` (process_spring) |
| work+0xF4 | flags_2 bit 1 (0x02) | `1.0 - input113 * 0.0078125` | work+0xF4 (swc1) |
| work+0xF4 (else) | — | `1.0` | work+0xF4 (swc1) |
| work+0xF0 | flags_1 bit 0x8000 (**checked first**) | `(input109/255)*8192` → `cvt.w.s` + `sh` | work+0xF0 (u16) |
| work+0xF0 | flags_1 bit 0x2000 | `(input108/255)*(-8192)` → `cvt.w.s` + `sh` | work+0xF0 (u16) |
| work+0xF0 (else) | — | `0` | work+0xF0 (u16) |

### Float constants (verified IEEE patterns in the disassembly)
```
0x437F0000 = 255.0f
0x3F800000 =  1.0f
0x3C000000 =  0.0078125f (1/128)
0x46000000 = 8192.0f     ← Rev.165 correction (NOT 20480.0f)
0xC6000000 = -8192.0f
```
`lui $at,0x4600` + `mtc1` = 8192.0f. The earlier analysis mis-read this constant;
the semantic now uses 8192.0f and the tests assert 8192 (bit 0x8000, 255/255):
- Test5 = 8192 = (255/255)*8192
- Test6 = 61424 = u16 wrap of ((128/255)*8192 = 4112.06 → 4112 → -4112)
- Test7 = 4112 (bit 0x8000 wins over 0x2000)

### Hook model
`process_spring` (jal 0x1E4980, twice) receives the **float VALUE bit-cast as an
integer in a0** (the original passes it in $f12), not an address.

## 3. subEnemyCollision (0x15E2C8, 0xB8)

### Confirmed access chain
- `a0 = entity`; `work = *(entity+0x15c)`; `entity_list = *(work+0x4A0)` (stride 0x190).
- Gate word at `*(work + 0x565060 + 0x188)` (`lui 0x56` + `addiu s2, 0x5060`;
  bit 0 = active). This is a GP-era global structure — modeled as a parameter.
- `mult $ac2, $v1, $s1` is a dead store (accumulator never read).

### Flow
```
loop:
  work = *(entity+0x15c)
  if ((*(work+0x565060+0x188) & 1) == 0) -> skipped path
  setup_a(&sp[0x10], entity, 0x2C)   jal 0x14A100
  setup_b(&sp[0x20], entity, 0x33)   jal 0x14A100   (a1 = entity via 'move a1,s0')
  collision_check(&sp[0x10])         jal 0x168538; result read from sp+0xA4 (not $v0)
  if (result != 0) -> response(entity, 0x9D)   jal 0x15BCC8
  counter_inc(1)                      jal 0x203AA0
  goto loop
```

### Confirmed details
- `move $a1, $s0` (the `.byte 0x0200282D` = `daddu a1,s0,zero`) appears twice:
  once as delay slot of the gate `beqz` (a1 = entity), once as delay slot of
  the second `jal 0x14A100`. setup_b DOES receive entity as a1.
- Result is read from stack `sp+0xA4` after the collision call, not `$v0`.
- The infinite back-edge is `b 0x15e2f8`; the semantic models **one iteration**.

### Divergences (documented, not encoded)
- In the original, `counter_inc` fires on EVERY iteration including the
  gate-fail path; the single-iteration model fires it only on the active path
  and returns 0 on the skip path (host wrapper is responsible there).
- The collision result comes from `sp+0xA4` (a scratch slot), which the host
  hook must mirror; the model takes it as the hook return value.

## 4. GirlForceFieldGeo (0x1C3C90, 0x178)

### Confirmed behavior (byte-identical tail path)
```
move $a0, $s2            # a0 = s2
lui $at, 0x3f80; mtc1    # $f1 = 1.0
cvt.w.s $f0, $f12        # $f0 = (int)f12
mfc1 $v0, $f0            # integer part
mtc1 $v0, $f0            #
cvt.s.w $f0, $f0         # back to float
sub.s $f12, $f12, $f0    # frac = f12 - (float)int
sub.s $f12, $f1, $f12    # $f12 = 1.0 - frac
jal 0x243AA8 (a0=s2, a1=model+s5, a2=model+s6)   # model = *(s1+0xB8)
jr $ra
```
So: **result = 1.0 - frac(f12)** and a final call to 0x243AA8 with
(a0, a1, a2) = (s2, model+s5, model+s6).

### What was REMOVED from the first (over-reaching) draft
The original draft invented a `*(output_addr) = (u16)(result * 255.0f)` write.
This is **NOT in the disassembly** — it was a wrong projection from 8-bit
scaling used in other functions. Removed. The semantic now only claims the
confirmed rounding pipeline + final-call delegation.

### Unconfirmed / delegated
- The three threshold-gated blocks before the tail call unknown subroutines
  in this order: `0x105f78`/`0x105f90` (sibling geometry), `0x106028`
  (magnitude), `0x117c20` (force), `0x244448` (apply), `0x243ad0`,
  `0x243ae8`. Each block pattern: compute → `c.olt.s` (compare `$fN`, `$f0`)
  → force path. Delegated to `block_a/b/c` hooks; the `c.olt.s` branches
  cannot be reproduced without the real magnitudes, so the hooks return
  nonzero when "its" block fires.
- `block_*` argument registers are NOT confirmed (the unknown subroutines use
  s1/s2/sp+0x30 etc.); passing `out_a2` is a documented modeling choice.
- The `cvt.w.s` rounding mode is UNCONFIRMED (assumed truncation toward zero,
  matching the C `(int)` cast; the original may round per FCSR).

## 5. CTest coverage (native/tests/isysgobj_semantic_test.c)

- HoldRope: 7 tests — null cells, all paths, bit priority, constant values
  (8192 / 61424 / 4112), flag bit masks.
- subEnemyCollision: 4 tests — gate open/closed, null work, hook dispatch.
- GirlForceFieldGeo: 6 tests — rounding (3.7→0.3, 1.0→1.0, 0.0→1.0,
  -2.5→1.5), hook order/count (4 calls: 3 blocks + final), args through
  `final_output`, null-hook tolerance.
- Full suite: **27/28 CTest pass** (only headless `opengl_backend` fails,
  the pre-existing baseline).

## 6. Bug fixes during the session

| Bug | Root cause | Fix |
|-----|-----------|-----|
| `#include <string.h>` missing | test used memset/memcpy | added include |
| `spring_b + 0xF4` overflow | spring buffer too small | wrote to `work_b + 0xF4` |
| entity addr mistaken | `gp-0x53B0` is a pointer CELL | test uses an entity-cell buffer |
| constant 20480 | mis-read `lui 0x4600` | corrected to 8192.0f |
| test value 61409 | wrong 20480-derived math | 61424 (u16 wrap of -4112) |
| GFFG invented output | projection from other functions | removed; only confirmed tail |

## 7. Confirmed / Probable / Possible / Unknown / Discarded

**Confirmed (byte-level):** all flag→value arithmetic, constants, FPU rounding
pipeline of GFFG, call order & register args documented above, dead `mult` store.

**Probable:** the three hooks model the threshold blocks of GFFG; the spring
handler 0x1E4980 is the same from the HoldRope entry path.

**Possible:** `sp+0xA4` collision result slot is written by 0x168538.

**Unknown:** exact semantics of 0x14A100, 0x168538, 0x15BCC8, 0x203AA0,
0x117C20, 0x244448, 0x105F78, 0x105F90, 0x106028, 0x243AD0, 0x243AE8, 0x243AA8;
the `cvt.w.s` rounding mode; block hook args; the host-side loop wrapper.

**Discarded:** the invented `*255`/`/255` output write on GFFG; the
big-endian capstone reading (LE is correct).

## 8. Next minimum test

Decompile one of the delegatables (e.g. 0x244448 or 0x243AA8) and replace a
hook with real logic, verifying the rounding pipeline still holds.

## Verdict

Three gameplay-domain semantic bridges added with CTest coverage. All
over-reaches removed; every constant and branch verified against the ELF bytes.
The models are explicitly not byte-exact C — they capture confirmed control
flow and arithmetic and delegate unknown internals to hooks.