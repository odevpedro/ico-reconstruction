# Rev.129 — `CreateGObj` (0x240D40) and `CreateGObj_v` (0x240EA0): byte-exact `.s` reconstruction (gap #2)

- **Date:** 2026-09-05
- **Objective:** Reconstruct the scene-object factory `CreateGObj` (0x240D40) — hot gap
  #2 — plus its sibling variadic factory (0x240EA0) as byte-exact `.s` sources.
- **Scope:** Byte-exact preservation only. Uses the pipeline's own generator so the
  `.s` is guaranteed to reassemble to identical output.
- **Sources used:**
  - `research/elf/rev113-scene-gobj-factory-contract.md` (documented contract)
  - `research/elf/ghidra-rev052-five-way-consolidation.md` (factory references)
  - `research/elf/rev128-sister-callback-reg-byte-exact.md` (gap #1, the sibling
    wrapper used *by* these factories)
- **Evidence used:** `.local/extracted/SCUS_971.13.elf` (USA ELF, little-endian),
  read via the pipeline's **exact** method (`extract_elf_func` + `generate_asm_source`
  + `assemble_and_verify`).

## 1. Function structure — `CreateGObj` (0x240D40, 0x160 bytes)

Frame 0xA0; args: `a0=v0(descriptor base)`, `a1=fp`, `a2=s7`, `a3=s6`, `t0` incoming.

| VA | instruction | semantic (per Rev.113) |
|----|-------------|------------------------|
| 0x240D40 | addiu $sp,$sp,-0xA0 | frame |
| 0x240D50 | move $fp,$a1 | 2nd arg |
| 0x240D58 | move $s7,$a2 | 3rd arg |
| 0x240D60 | move $s6,$a3 | 4th arg |
| 0x240D7C | beqz $t0 → 0x240D88 | if t0==0, skip (+0x40) load |
| 0x240D84 | lw $s5,0x40($v0) | +0x40 word (only if t0 nonzero) |
| 0x240D88 | lw $a0,0x60($v0) | → isysGObjAdd arg0 |
| 0x240D90 | lw $s0,0x5c($v0) | cbA source |
| 0x240D98 | lw $s1,0x50($v0) | cbB source |
| 0x240D9C | lw $s2,0x4c($v0) | cbC source |
| 0x240DA0 | jal isysGObjAdd (0x13E8D8) | create GObj |
| 0x240DA4 | lw $s3,0x48($v0) | cbD source (delay slot) |
| 0x240DA8 | move $s4,$v0 | s4 = GObj |
| 0x240DAC | lw $t0,-0x4d58($gp) | GObj counter |
| 0x240DC8 | sw -1,0xc($s4) | GObj+0x0C = -1 |
| 0x240DD0 | sw -1,8($s4) | GObj+0x08 = -1 |
| 0x240DD8 | sw 1,0x16c($s4) | GObj+0x16C = 1 |
| 0x240DE0 | sw 0,0x164($s4) | GObj+0x164 = 0 |
| 0x240DE8 | sw 1,4($s4) | GObj+0x04 = 1 |
| 0x240DEC | sw $s4,0($v1) | register GObj ptr in table |
| 0x240DF0 | jal sister_callback_reg (0x13F778) | reg cbA type 0x16 |
| 0x240E04 | jal sister_callback_reg (0x13F778) | reg cbB type 0x17 |
| 0x240E18 | jal sister_callback_reg (0x13F778) | reg cbC type 0x18 |
| 0x240E34 | jal 0x13F130 | direct, +0x48, a3=4th arg |
| 0x240E50 | jal isysGObjProcAdd_Wrapper (0x13F7A8) | cond: if s5 and t0 |
| 0x240E58 | sw $s7,8($s4) | GObj+0x08 = 3rd arg |
| 0x240E60 | jal 0x13E648 | final helper, (s4, fp) |
| 0x240E68 | move $v0,$s4 | return GObj |
| 0x240E9C | nop | padding (function is 0x160) |

## 2. `CreateGObj_v` (0x240EA0, 0x128 bytes) — variadic variant

Frame 0x80; args: `a1=s0`, `a2=s1`, `a3=s2`, `t0=s6`, `t1=s3`, `t2=s4`.
Identical step sequence (isysGObjAdd → 3× sister_callback_reg → 0x13F130 →
conditional 0x13F7A8) but **no** `+0x40` load: the conditional value is taken
directly from incoming `t0` (`move $s6,$t0`). Returns GObj in `v0`.

## 3. PAL symbol reconciliation is NOT reliable here

`reconcile.py` MATCH rows falsely assign `sceSifSendCmd` (0x240D90) and
`isceSifSendCmd` (0x240DD0) — both are **internal offsets inside `CreateGObj`**,
and originate from `vobj.o` (an IOP/SIF module), not the scene-factory module.
These are op_seq false positives (object-range VA collisions, cf. Rev.127's 259
collisions), reinforcing that PAL symbols must never be promoted to ground truth
without curation.

## 4. Byte-exact verification

Both `.s` generated via `generate_asm_source` (the pipeline's own converter,
which emits `loc_*` branch labels and `.word` for absolute jal targets), then
verified with `assemble_and_verify`:

| Function | VA | Size | Result |
|----------|-----|------|--------|
| CreateGObj | 0x240D40 | 0x160 | **100.0 byte-exact** |
| CreateGObj_v | 0x240EA0 | 0x128 | **100.0 byte-exact** |

## 5. Confirmed / probable / unknown

**Confirmed:**
- Byte-identical disassembly of both functions (0x160 + 0x128 bytes).
- `.s` files `src/core/asm/CreateGObj.s` and `src/core/asm/CreateGObj_v.s`
  assemble byte-exact (100%).
- Both factories call `sister_callback_reg` (0x13F778) three times with type
  IDs 0x16, 0x17, 0x18 — the exact hot path reconstructed in Rev.128.
- GObj words set: +0x04=1, +0x08=-1 (later +3rd arg), +0x0C=-1, +0x164=0,
  +0x16C=1; returned ptr registered at `0x00712CC0[gobj_counter]` (gp-0x4D58).

**Probable:**
- +0x5C/0x50/0x4C/0x48 are callback pointers; +0x60 is the `isysGObjAdd` user ptr.
- `CreateGObj_v` is the variadic form used when fields are passed in registers
  instead of loaded from the descriptor struct.

**Unknown:**
- Callees 0x13F130 and 0x13E648 are not yet mapped to names (not in pipeline).
- Semantic meaning of type IDs 0x16/0x17/0x18.

## 6. Next minimum test

Run the full scoring pipeline for the remaining gaps: allocator `0x19F310` (gap
#3), `world_state_load` 0x1AF948 (gap #4), and process-remove unlink helper
0x13F638 (gap #5).

## Conservative verdict

Both scene factories are now preserved as **byte-exact `.s`** (100% reassembly),
consistent with the `rev113-scene-gobj-factory-contract` static analysis. Preserved
verbatim; no semantic C is claimed.