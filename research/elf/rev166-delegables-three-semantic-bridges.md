# Rev.166 — subEnemyCollision delegables: 3 byte-exact `.s` + 3 semantic bridges

- **Date:** 2026-09-10
- **Objective:** Close the audit loop of the subEnemyCollision bridge by
  decomposing its five external calls. Three (setup ×2, collision_response,
  counter_inc) were disassembled, preserved byte-exact as `.s`, and converted
  to auditable semantic functions with CTest coverage. No runtime required.
- **Trilha:** `main`/dst (decompilation truth) + native-port tests.

## Scope

Rev.165 left `ico_semantic_subEnemyCollision` with five hook calls and no
knowledge of their targets. This revision resolved the targets from the
byte-exact `src/entity/asm/subEnemyCollision.s`:

| Hook | jal target | Identity | Resolution |
|------|-----------|----------|------------|
| setup_a | `0x0014A100` | `fn_14A100` (new name) | disassembled → byte-exact `.s` → semantic |
| setup_b | `0x0014A100` | same | same |
| collision_check | `0x00168538` | trampoline → `lw $v0,-0x64FC($gp); jalr $v0` (a1=0x0C) | **not** reconstructable without runtime target |
| collision_response | `0x0015BCC8` | `fn_15BCC8` (new name) | disassembled → byte-exact `.s` → semantic |
| counter_inc | `0x00203AA0` | `fn_203AA0` (new name) | disassembled → byte-exact `.s` → semantic |

## Sources

- `.local/extracted/SCUS_971.13.elf` (USA ELF, little-endian R5900)
- `src/core/asm/fn_14A100.s`, `fn_15BCC8.s`, `fn_203AA0.s` (new, byte-exact)
- `src/entity/asm/subEnemyCollision.s` (byte-exact, Rev.164)
- Disassembly via `capstone` MIPS64 + little-endian

## Method

1. Disassembled each untracked VA pair with capstone (LE R5900), recorded the
   instruction stream, and measured exact boundaries:
   - `fn_14A100`: 0x14A100..0x14A174 (0x74 B; `jr $ra` @0x14A170, delay
     `addiu $sp,$sp,0x30`)
   - `fn_15BCC8`: 0x15BCC8..0x15BD44 (0x7C B; `j 0x13FF88` @0x15BD40, delay
     `daddu $a2,$a0,$zero`)
   - `fn_203AA0`: 0x203AA0..0x203B40 (0xA0 B; `jr $ra` @0x203B38, delay
     `addiu $sp,$sp,0x20`)
2. Extended `tools/gen_byteexact_asm.py` targets with a `module` field and
   generated the three `.s` as `core` (named `fn_<VA>` per pipeline
   convention for unnamed functions).
3. Assembled with `ee-gcc 2.9-991111-01 -c`, extracted `.text` via pyelftools,
   compared byte-for-byte vs the ELF slice. **All 3 verified BYTE-EXACT.**

## Verified semantics from the disassembly

### fn_14A100 (setup)
```
move a0,s0; move a1,a2; jal 0x109F10     v0 = lookup(entity, key)     (key=0x2C/0x33)
sll  v0,v0,6                              idx << 6 (stride 0x40)
lw   a0,0x15C(s0)                         work = *(entity+0x15C)
lw   v1,0x0C(a0); addu v1,v0,v1           base = *(work+0x0C) + idx*64
lwc1+swc1 0x30/0x34/0x38 → dst[0..3)      three f32 copied (angle triple)
```
`lookup` (0x109F10) has no symbol; the semantic delegates it to an optional
`IcoSemanticTriFn` (NULL → idx 0). Pointers are modeled as host-width cells
(`sec_cell`), floats as raw f32.

### fn_15BCC8 (collision select)
```
addiu v0,0xA8; beq a1,v0,go; addiu v0,0xAD; bne a1,v0,tail   only 0xA8/0xAD enter
lw v1,0x0C(a0);  addiu v0,1;  bne v1,v0,tail                 requires *(e+0x0C)==1
ld v1,0x470(a2); ld v0,0x480(a2)           a2 = *(e+0x164); f1/f2 u64
dsrl32+andi bit29 both set        → a1 = 0xA9
dsrl32+andi bit27 (movn on f2)    → a1 = 0xAA  (b27(f1) gate + b27(f2) movn)
tail j 0x13FF88 (shared response sink — not modeled, result returned)
```

### fn_203AA0 (frame-delta counter)
```
lui 0x27; lw v0,0x4EC0(v1)        count   = *(0x274EC0)   (VBlank counter)
lw  a1,4(v1)                       divisor = *(0x274EC4)
beql a1,$zero → break 0,7          divisor==0 → TRAP
div v0,a1 ; div v0,60              v = ((60 - count)/divisor)/60  (signed div)
beqz a0 / movz                      a0==0 → v ; a0!=0 → (v? v:1)
v > 0 → spin: n=v; while(n) jal 0x13D3F0, n--
v == 0 (a0==0) → infinite jal 0x13D3F0 loop
```
The `mult $ac2` at 0x203AB8 and 0x203AE0 are **dead stores** (no `mflo` of
the accumulator) — a confirmed modeling note. The 0x13D3F0 periodic call is
the thread yield; the semantic returns the yield count and the `0`/trap
sentinels instead of spinning.

## Confirmations

- `subEnemyCollision.s` trace (full, Rev.166): the 0x203AA0 counter_inc call
  is **unconditional every loop iteration** (runs on gate-fail and on
  outcome==0 paths); the 0x15BCC8 response call only when outcome (sp+0xA4)
  ≠ 0. The 0x14A100 setup_a/setup_b write `(&sp+0x10, key=0x2C)` /
  `(&sp+0x20, key=0x33)`; between setup_b and collision_check the original
  runs `lwc1 $f0,0x24($sp); sub.s $f0,$f0,$f20; swc1 $f0,0x24($sp)` with
  `$f20 = 0x40A00000 = 5.0f` — modeled as `scratch_b[4] -= 5.0f`.
- In subEnemyCollision the response reaches fn_15BCC8 with `a1 = 0x9D`, which
  is **outside** the select set {0xA8, 0xAD} → the select is inert there; the
  built-in validates state==1 and returns 0x9D unchanged.

## Changes

| Artifact | Change |
|----------|--------|
| `src/core/asm/fn_14A100.s` | new, byte-exact (0x14A100, 0x74 B) |
| `src/core/asm/fn_15BCC8.s` | new, byte-exact (0x15BCC8, 0x7C B) |
| `src/core/asm/fn_203AA0.s` | new, byte-exact (0x203AA0, 0xA0 B) |
| `tools/gen_byteexact_asm.py` | targets now carry a `module` field (core/entity) |
| `src/core/gobj_abi.h` | `u64` typedef; prototypes `ico_semantic_fun14A100` (void* dst), `ico_semantic_fun15BCC8`, `ico_semantic_fun203AA0` |
| `src/core/isysgobj_semantic.c` | helpers `sec_st_float`/`sec_ld_u64`; three delegates; built-in defaults in `ico_semantic_subEnemyCollision` (setup_a/b, collision_response) + verified `-= 5.0f` |
| `native/tests/isysgobj_semantic_test.c` | 12 new tests (delegates + NULL-hook bridge path) |

## Confirmed vs probable vs unknown

**Confirmed (byte/instruction-level):**
- All three delegate boundaries, register flow, and the bit29/bit27 select
  table of fn_15BCC8; the divisor==0 break-0,7 trap and the (60-count)/k/60
  unsigned countdown of fn_203AA0; the idx*64 + *(work+0xC) base advance and
  the 3-float copy of fn_14A100; the byte-exact assembly of all three `.s`.
- 0x203AA0 rereads the VBlank counter pair {0x274EC0, 0x274EC4}.

**Probable:** `fn_14A100` dst triple = angle/position triple (used by the
collision setup; key 0x2C/0x33 select animation/mesh variants). The `idx`
lookup 0x109F10 compares `entity` against a per-entity index.

**Unknown:** 0x109F10 name/semantics (still a delegable); the runtime target
of gp-0x64FC (collision_check trampoline); the 0x13FF88 sink internals; the
0x13D3F0 yield identity; VU/COP2 stub pair {0x243AA8, 0x244448} left for the
render platform layer.

**Modeling choices (documented divergences):**
- counter_inc is modeled only on the active pass (the original fires it every
  iteration incl. gate-fail); pre-existing Rev.165 divergence, kept.
- fn_203AA0 returns the yield count instead of spinning; trap (divisor 0) and
  the infinite-wait branch returned as `0`.
- 0x13FF88 tail not modeled — host is responsible for the response sink.

## Next minimum test

- Add `0x13FF88` (response sink) to the byte-exact corpus, then bind the
  fn_15BCC8 result into the host pipeline.
- Assign symbols to 0x109F10 / 0x13D3F0 / 0x13FF88 via PAL MAIN.MAP/SRCFILE
  if present.

## Verdict

Conservative: the three delegates are byte-exact preserved and their verified
decision logic is now an auditable, tested part of the semantic core. The
remaining unimplemented links (0x109F10, gp-0x64FC, 0x13FF88, 0x13D3F0) are
explicitly listed as unknown/delegable — none is upgraded to a conclusion.