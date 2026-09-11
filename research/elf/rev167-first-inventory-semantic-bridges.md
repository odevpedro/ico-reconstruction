# Rev.167 — First Rev.163-inventory batch: 3 byte-exact .s + 3 semantic bridges

- **Date:** 2026-09-10
- **Branch:** `native-port`
- **Objective:** Start converting the `rev163-static-target-inventory.csv`
  bracket of named-but-not-reconstructed functions into byte-exact `.s` plus
  semantic bridges with CTest, using the same audit discipline of Rev.166.
- **Scope:** `actEnemyFlagOnDead` (0x15D5F0), `AP1JumpReq` (0x1AE3B0),
  `actSt04bEne1Chk` (0x203A10). Two further candidates were inspected and
  deferred (see Discarded).
- **Sources:** USA ELF `.local/extracted/SCUS_971.13.elf`; capstone R5900
  little-endian; `tools/gen_byteexact_asm.py` (extended with the `module`
  field in Rev.166 supported these three new targets); ee-gcc 2.9-991111-01 +
  pyelftools `.text` extraction for byte-exact verification;
  `rev163-static-target-inventory.csv`.

---

## Method

Same as Rev.166: disassemble the authoritative ELF bytes with capstone, hand
reconstruct the control flow, generate `.s` (branches/jal/COP1 emitted as
`.word` where the ee-as backward-branch padding rule applies), assemble with
ee-gcc, and compare the object `.text` against the ELF at the target VA.
Semantics in `src/core/isysgobj_semantic.c` are derived only from the verified
disassembly; anything not visible in the instructions stays a hook.

## Byte-exact results

| Function | VA | Size | Module | Verification |
|----------|-----|------|--------|--------------|
| `actEnemyFlagOnDead` | 0x15D5F0 | 0x2C | entity | assembled 44B == target 44B |
| `AP1JumpReq` | 0x1AE3B0 | 0x34 | entity | assembled 52B == target 52B |
| `actSt04bEne1Chk` | 0x203A10 | 0x48 | entity | assembled 72B == target 72B |

On-disk `.s` total: **736 → 739** (entity 663 → 666, cloth 6, core 67).
All 739 byte-exact. Pipeline `asm_source_score.py --all`: Success 617/617.

New files: `src/entity/asm/actEnemyFlagOnDead.s`,
`src/entity/asm/AP1JumpReq.s`, `src/entity/asm/actSt04bEne1Chk.s`.

## Confirmed semantics (from disassembly)

### actEnemyFlagOnDead (0x15D5F0)

```
sw   $a0, ($sp)                    ; save arg (tail, unused here)
lui  $a0, 0x56                     ; 0x560000
sd   $ra, 0x10($sp)
jal  0x1A6E28            [delay] addiu $a0, $a0, -0x7740   ; a0 = 0x5588C0
jal  0x203AA0            [delay] move  $a0, $zero           ; a0 = 0 (frame_count)
jr   $ra                       [delay] addiu $sp, $sp, 0x20
```

- Sends the enemy-dead message to store `0x5588C0` via delegate `0x1A6E28`
  (unknown target, kept as host hook), then runs the verified Rev.166
  `fn_203AA0` delay with `frame_count == 0` (the "a0==0 policy" reachable
  branch: infinite wait returned as 0 when the vblank gap is 60/frame).
- This confirms `fn_203AA0` is a *shared* frame-delay primitive already
  consumed by gameplay handlers.

### AP1JumpReq (0x1AE3B0)

```
lui   $v0, 0x4b ; addiu $v1, $v0, 0x3d10     ; base = 0x4B3D10
addiu $a1, $zero, -2                          ; mask = 0xFFFF...FFFE
addiu $a0, $zero, 0xb5                        ; loop counter (first dec -> 0xB4)
loop@0x1AE3C0:
  ld    $v0, ($v1)
  and   $v0, $v0, $a1                         ; 64-bit and
  sd    $v0, ($v1)
  addiu $v1, $v1, 0x40
  bgez  $a0, loop                  [delay] nop
jr $ra               [delay] nop
```

- Iterations 0xB4 down to 0 → 0xB5 entries; `base + i*0x40`, clearing **bit0**
  of each u64 via `and -2` (0xFFFFFFFFFFFFFFFE). `a0` is reused as the counter
  inside the loop (decremented in the setup, not the loop body).
- **Correction during this revision:** an earlier draft claimed "clears bit1
  (mask ~2)". `-2` sign-extends to 0xFFFF...FFFE on the 64-bit EE, so the low
  bit is cleared. The semantic + test were corrected to `&= -2` before commit.

### actSt04bEne1Chk (0x203A10)

```
addiu $sp,$sp,-0x10
move  $a3,$a0                    ; a3 = entity
move  $t0,$a1                    ; t0 = saved arg_b
lw    $v1, 0x164($a3)            ; v1 = work (32-bit load)
lw    $v0, 0x12c($v1)            ; slot busy?
bnez  $v0, busy                  [delay] move $a1,$a2
sw    $a1, 0x130($v1)            ; free: store arg_c at +0x130
move  $a2,$t0                    ; a2 = original arg_b
jal   0x13FF88         [delay] sw $a3, 0x12c($v1)   ; store entity at +0x12c
addiu $v0,$zero,1                ; return 1
busy: move $v0,$zero             ; return 0
ld $ra,($sp) ; jr $ra ; addiu $sp,$sp,0x10
```

- Register-busy slot pattern: `+0x12C` busy/owner cell, `+0x130` payload.
  Returns 1 when the slot was free (and dispatched), 0 when busy.
- The dispatched sink is `0x13FF88` — the **same shared response sink that
  `fn_15BCC8` tail-jumps to** (Rev.166). Host model exposes it as an optional
  hook; the sink receives `(entity, old_a2, old_a1)`.
- All three pointer/adjacent fields (`+0x164`, `+0x12c`, `+0x130`) are
  accessed with `lw`/`sw` (32-bit) on the EE.
- **Host-model consequence (fixed before commit):** `+0x12c` and `+0x130` are
  only 4 bytes apart, so they cannot both be modeled as 8-byte host cells.
  `ico_semantic_actSt04bEne1Chk` stores both as 32-bit cells:
  `sec_st_u32(m+0x130, arg_c)` and `sec_st_u32(m+0x12c, (u32)entity)`.
  An earlier 8-byte `memcpy` at `+0x12c` clobbered `+0x130` (observed with
  gdb byte dumps before the fix). The busy check reads `+0x12c` with
  `sec_ld_u32`, consistent with the EE `lw`.

## Test additions (native/tests/isysgobj_semantic_test.c)

- **Test 13** — `AP1JumpReq`: clears bit0 of 3 u64 entries (stride 0x40),
  leaves other bits intact; NULL guard.
- **Test 14** — `actSt04bEne1Chk`: free slot → 1, `+0x130 == arg_c`,
  `+0x12c == (u32)entity`, sink captured `(entity, arg_c, arg_b)`; busy slot
  → 0 (no sink); NULL entity → 0.
- **Test 15** — `actEnemyFlagOnDead`: fires `flag_send(0x5588C0)` once, then
  runs `fn_203AA0(0, counters)` (count=160/divisor=60 → v=1); hook-less path
  (NULL flag_send) still runs the delay.
- Helper `load_u32` added alongside the existing STORE_* macros.

CTest: 27/28 (only headless `opengl_backend` baseline segfault).

## Also touched

- `src/core/gobj_abi.h`: prototypes + behavior doc for the three new bridges.
- `tools/gen_byteexact_asm.py`: targets extended with the three new functions
  (entity module).

## Probable

- `0x4B3D10` (AP1JumpReq) is likely the AP1 round-state array (0xB5 entries,
  stride 0x40 = one per walkable node), bit0 = jump-request flag.
- `0x5588C0` is a flag/state store read by the enemy-dead path; `0x1A6E28`
  likely posts a message to world/thread state.

## Possible

- `actSt04bEne1Chk` is an area-specific ("st04b") enemy check: registers the
  triggering entity into the work slot and forwards to the shared response
  sink. The `0x9D`/`0xA8/0xAD` protocol seen in Rev.166 likely flows through
  this same sink.

## Unknown

- `0x1A6E28` (flag send) body and its callees.
- `0x13FF88` (shared sink) control flow.
- What `0x4B3D10`'s per-entry layout is beyond bit0.
- AP1 jump state producers/consumers.

## Discarded

- `SetEnemyWingRatio` (0x1C0F08, 0x28 B): its `bc1tl` target lies at
  0x1C0F38, *beyond* the 0x28-byte span — the named function is an entry into
  an inlined region, not a standalone function. Deferring to avoid over-decoding.
- `MultiMatrixByQuaternion` (0x10E708, 0x3C B): body starts with R5900
  `bbit032 $a0, 0xe/0xf, +self` primitives and a 0x138+ continuation; needs a
  proper R5900-family decoder, deferring (platform-y, not gameplay-critical).

## Next minimum test

- Commit and keep pipeline green; then pull the next batch from the same
  inventory by the same rule (conf ≥ 0.95, size ≤ 0x200, gameplay-domain):
  e.g. `SetMotionBlendlessNode` (0x10A048), `RotQuaternionEAX` (0x10EA30),
  `gif_MakeSpriteWithStrip` (0x1101E0).

## Conservative verdict

Three named functions moved from "inventory, no reconstruction" to
**byte-exact `.s` + semantic bridge + CTest** in one pass, with two
self-corrections made from the disassembly before touching the commit:
AP1JumpReq's mask is `-2` (clears bit0, not bit1) and actSt04bEne1Chk's
adjacent fields must be modeled as 32-bit cells on the host. The
`0x13FF88` sink is now confirmed as the shared dispatcher for both the
Rev.166 select path and this Rev.167 register-busy path. `.s` count: 739/739
byte-exact; CTest 27/28.