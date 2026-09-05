# Rev.128 — `sister_callback_reg` (0x13F778): byte-exact `.s` reconstruction (gap #1 of 5)

- **Date:** 2026-09-05
- **Objective:** Reconstruct the hot decompilation gap `sister_callback_reg`
  (0x13F778) as a byte-exact `.s` source file, closing verification gap #1 of the
  five hot-function gaps identified from runtime logs.
- **Scope:** Disassembly + byte-exact reconstruction only. This is a small
  specialization wrapper; no semantic C is claimed.
- **Sources used:**
  - `research/elf/ghidra-rev052-five-way-consolidation.md` (prior disassembly)
  - `research/elf/rev113-scene-gobj-factory-contract.md` (factory contract)
  - Sibling `.s`: `src/core/asm/isysGObjProcAdd_Wrapper.s` (verified byte-exact)
- **Evidence used:** `.local/extracted/SCUS_971.13.elf` (USA ELF, **little-endian**),
  read via the **exact LOAD-segment method of `tools/asm_source_score.py`**
  (little-endian `extract_elf_func`; the extracted ELFs are **LSB**, so capstone
  must use `CS_MODE_LITTLE_ENDIAN`, not big-endian).

## 1. Why this function

Runtime capture (`ico-runtime-20260825-152452.jsonl`, 3.65M events) shows
`isys_gobj_proc_add` firing overwhelmingly with `t0` type IDs `0x16/0x17/0x18`
(144K/131K/93K/93K events; `a3` add=280K, remove=243K). `sister_callback_reg`
feeds those type IDs directly and had **no `.s`**. It is a high-frequency hot
path with zero reconstruction presence.

## 2. Disassembly (authoritative, little-endian)

Read via the pipeline's exact method at 0x13F778, **48 bytes** (0x30):

| VA | Encoding | mnemonic | ABI text |
|----|----------|----------|----------|
| 0x0013F778 | `2d40e000` | daddu | move $t0,$a3 |
| 0x0013F77C | `f0ffbd27` | addiu | addiu $sp,$sp,-0x10 |
| 0x0013F780 | `ff00c730` | andi | andi $a3,$a2,0xff |
| 0x0013F784 | `0000bfff` | sd | sd $ra,0x0($sp) |
| 0x0013F788 | `2d30a000` | daddu | move $a2,$a1 |
| 0x0013F78C | `00001809` | addiu | addiu $t1,$zero,0x1800 |
| 0x0013F790 | `fcfc040c` | jal | jal 0x13F3F0  (isysGObjProcAdd_) |
| 0x0013F794 | `2d288000` | daddu | move $a1,$a0  (delay slot) |
| 0x0013F798 | `0000bfdf` | ld | ld $ra,0x0($sp) |
| 0x0013F79C | `0800e003` | jr | jr $ra |
| 0x0013F7A0 | `1000bd27` | addiu | addiu $sp,$sp,0x10  (delay slot) |
| 0x0013F7A4 | `00000000` | nop | nop |

## 3. Relationship to `isysGObjProcAdd_Wrapper` (0x13F7A8)

Both are thin specialization wrappers forwarding to `isysGObjProcAdd_` (0x13F3F0):

| Aspect | sister_callback_reg (0x13F778) | Wrapper (0x13F7A8) |
|--------|--------------------------------|---------------------|
| `$t1` (priority) | **fixed** `addiu $t1,$zero,0x1800` | from caller via `$t0` (`move $t1,$t0`) |
| `$t0` | carries type ID from `$a3` | reassigned `move $t0,$v0` |
| intent | register sibling callbacks w/ fixed priority | general passthrough |

**Correction vs Rev.052:** Rev.052 described `li t1,0x1800`; the verified encoding
is `addiu $t1,$zero,0x1800` (`00001809`). Rev.052 also estimated 44 bytes; the
function is **48 bytes (0x30)** including the trailing `nop` at 0x13F7A4.
No semantic difference — only precision of the recorded instruction/bounds.

## 4. Byte-exact verification

Using `tools/asm_source_score.py` `assemble_and_verify(..., 0x13F778, 0x30)`:

- Requires an explicit trailing `.nop` to cover the 0x13F7A4 padding byte.
- **Result: `success=True`, size 100.0, BYTE-EXACT.**

Comparison of first mismatch-before-fix showed the 44 assembled bytes already
matched byte-for-byte; only the trailing 4-byte `nop` was missing.

## 5. Confirmed / probable / unknown

**Confirmed:**
- Byte-identical disassembly of 0x13F778..0x13F7A4 (48 bytes) from the USA ELF.
- `.s` at `src/core/asm/sister_callback_reg.s` assembles **byte-exact (100%)**.
- Function is a fixed-priority (`0x1800`) specialization of `isysGObjProcAdd_Wrapper`.

**Probable:**
- Called by factories 0x240D40 / 0x240EA0 to register sibling callbacks with
  type IDs 22/23/24 before the principal type-19 callback (per Rev.052).

**Unknown:**
- Exact meaning of the distinct type IDs / category byte.
- Semantic C intent (out of scope here).

## 6. Next minimum test

Run `python3 tools/asm_source_score.py --all --no-save` (or the sibling-category
verification) to confirm the new `.s` stays 100% alongside the 684 existing
verified `.s` files; then proceed to hot gap #2 (`CreateGObj`, 0x240D40).

## Conservative verdict

`sister_callback_reg` (0x13F778) is now reconstructed as a **byte-exact `.s`**
(48 bytes, 100% match), consistent with its sibling wrapper. This is a verified
ground-truth artifact, not a semantic claim.
