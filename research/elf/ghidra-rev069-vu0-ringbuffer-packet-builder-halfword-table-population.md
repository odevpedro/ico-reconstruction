# Rev.069 — VU0 ring-buffer packet builder (0x1D43F8), VU0 kick stub (0x117C40), halfword table population (0x6AB080), alternate constants (0x55F260)

**Date:** 2026-05-17

## Objective

Complete static analysis of remaining unknowns from Rev.068:

1. Read constant blocks at `0x55F260`–`0x55F298` used by the alternate implementation
2. Disassemble `0x1D43F8` — VU0 ring-buffer packet builder
3. Disassemble `0x117C40` — VU0 kick stub
4. Search for all callers of `0x00168650` with `a0 != 0` (alternate selection point)
5. Search for writers of halfword table at `0x6AB080`

## Scope

- Static disassembly and byte reads from ELF only
- No runtime, no PCSX2, no assets

## Sources used

| Source | Use |
|---|---|
| `research/elf/ghidra-rev068-naked-init-caller-and-transform-packetizer.md` | 0x1D4A58 context, alternate calling pattern |
| `.local/extracted/SCUS_971.13.elf` | byte-level source |

---

## Constant blocks at `0x55F260`–`0x55F298`

Section: `.data` at `0x00540000`–`0x005536B8`. These are read by the alternate implementation callers (`0x169F80` and `0x16A058`) via 4-QWORD `LD` sequences.

### Alternate A block (used by `0x169F80`, loaded from `0x55F280`)

| VA | Raw (little-endian qword) | i32 lo | i32 hi | Notes |
|:---|:---|---:|---:|:---|
| `0x55F280` | `0x00000004000001BA` | 442 | 4 | `lo=0x1BA`, `hi=0x4` |
| `0x55F288` | `0x000000020015D5F0` | 1431024 | 2 | `lo=0x15D5F0`, `hi=0x2` |
| `0x55F290` | `0x0015D5F000000000` | 0 | 1431024 | `lo=0`, `hi=0x15D5F0` |
| `0x55F298` | `0x0000000000000000` | 0 | 0 | zero sentinel |

### Alternate B block (used by `0x16A058`, loaded from `0x55F260`)

| VA | Raw (little-endian qword) | i32 lo | i32 hi | Notes |
|:---|:---|---:|---:|:---|
| `0x55F260` | `0x0000000000000000` | 0 | 0 | zero |
| `0x55F268` | `0x0000000000000000` | 0 | 0 | zero |
| `0x55F270` | `0x0000000000000000` | 0 | 0 | zero |
| `0x55F278` | `0x0000018400000000` | 0 | 388 | `hi=0x184` (388) |

**Interpretation:** The constants are small integers — likely object/bone matrix indices, vertex counts, or offset/size parameters for the VU0 transform pipeline. Alternate A has meaningful values in all 4 QWORD slots; Alternate B is mostly zeros with a single `388` in the last QWORD high word.

---

## `0x1D43F8` — VU0 ring-buffer packet builder

### Function signature

| Property | Value |
|---|---|
| Range | `0x1D43F8`–`0x1D45AC` |
| Size | 0x1B4 (436 bytes, 109 instructions) |
| Stack | 0x60 (96 bytes) |
| Callee-saves | `$s0`, `$s1`, `$s2`, `$s3`, `$s4`, `$ra` |

### Calling convention (from `0x1D4A58`)

| Register | Source |
|---:|---|
| `a0` | struct A (matrix buffer, stack local `sp+0x20` or `sp+0x30`) |
| `a1` | struct B (original `a1` from caller, stack data) |
| `a2` | struct C (other matrix buffer, chosen by branch) |
| `a3` | struct D (original `a3` from caller) |
| `t0` | **Pass flag**: 0 (normal) or -1 (terminator) |

### Ring buffer push mechanism (`0x111918`)

```asm
0x111918: lui      $v0, 0x4c
0x11191C: addiu    $v0, $v0, 0x7710    ; v0 = &ring_buf (0x004C7710)
0x111920: lw       $v1, 0x10($v0)      ; v1 = ring_buf.head (32-bit ptr)
0x111924: sd       $a1, ($v1)           ; *(head+0)  = a1  [data qword]
0x111928: addiu    $v1, $v1, 8          ; v1 = head + 8
0x11192C: sw       $v1, 0x10($v0)       ; ring_buf.head = head+8 (tentative)
0x111930: addiu    $a1, $v1, 8          ; a1 = head + 16 (pre-computed for next)
0x111934: sd       $a0, ($v1)           ; *(head+8) = a0  [type tag qword]
0x111938: jr       $ra
0x11193C: sw       $a1, 0x10($v0)       ; ring_buf.head = head+16 (confirmed)
```

**Each push writes 16 bytes**: 8 bytes data (`a1`) + 8 bytes type tag (`a0`). Head pointer at `ring_buf+0x10` advances by 16 each call.

### Ring buffer structure at `0x004C7710` (BSS/runtime)

| Offset | Type | Purpose |
|:---|---:|---|
| `+0x00` | (unknown) | Likely base/limit for wrap-around |
| `+0x10` | `uint32*` (ptr) | **Head pointer** — current write position |

### Packet format per invocation

Each call to `0x1D43F8` produces **5 entries** (80 bytes) in the ring buffer:

| Entry | Type tag (`a0`) | Data (`a1`) | Source |
|:---|:---:|---:|---|
| 1 | **0** (header) | Global value from `*(gp-0x54C4)` | scene/entity context |
| 2 | **1** (byte-packed) | 4 byte-sized fields from struct B | vertex attributes/RGBA |
| 3 | **5** (wide-packed) | 3 fields from struct A (matrix) | transform vector, upper 32 bits = `s3[8]` |
| 4 | **1** (byte-packed) | 4 byte-sized fields from struct D | vertex attributes/RGBA |
| 5 | **5** (wide-packed) | 3 fields from struct C (matrix) | transform vector (tail-call to push) |

### `t0=0` (PATH_A) vs `t0=-1` (PATH_B) difference

In PATH_B (`t0=-1`), entries 3 and 5 are modified: the upper 32 bits are forced to `0xFFFFFFFF`:

```asm
; At entry to PATH_B:
0x001D4504: dsll32   $s0, $s0, 0       ; s0 = 0xFFFFFFFF00000000
...
0x001D4544: or       $a1, $a1, $s0     ; a1[63:32] = 0xFFFFFFFF
```

**Effect:** The VU0 microprogram receiving these packets interprets `0xFFFFFFFF` in the upper 32 bits of type-5 entries as a **batch terminator** signal.

### Shared buffer swapping (from `0x1D4A58` branch)

The transform orchestrator `0x1D4A58` branches on return value of `0x1D45B0` (matrix builder):

| Path | s2 | src (`a0`) | dst (`a2`) |
|:---|:---:|:---|---:|
| PATH_A | 0 | `sp+0x20` (struct A) | `sp+0x30` (struct C) |
| PATH_B | !=0 | **swapped**: `sp+0x30` (struct C) → `sp+0x20` (struct A) |

### GP-relative access

Only one GP access in `0x1D43F8` itself:
- `0x1D4420`: `lw $a1, -0x54C4($gp)` — loads header value from `*(gp-0x54C4)` = `*(0x0063342C)`

---

## `0x117C40` — VU0 kick stub (inline asm)

### Decoded instructions

```asm
0x117C40: lui      $v1, 0xE74B         ; GPR = 0xE74B0000
0x117C44: lui      $k1, 0xE64B         ; GPR = 0xE64B0000  (non-standard rs field)
0x117C48: lui      $s3, 0xE54B         ; GPR = 0xE54B0000  (non-standard rs field)
0x117C4C: lui      $t3, 0xE44B         ; GPR = 0xE44B0000  (non-standard rs field)
0x117C50: andi     $zero, $s0, 0x0F4A  ; timing NOP (result discarded)
0x117C54: j        0x03800C            ; tail-call to runtime VIF uploader
0x117C58: nop                          ; delay slot
0x117C5C: nop                          ; padding
```

### Interpretation

The 4 LUI values (`0xE74B`, `0xE64B`, `0xE54B`, `0xE44B`) form a **VIF command sequence**:

| GPR | VF register | LUI value | Pattern |
|:---:|:---:|:---:|---|
| `$v1` (3) | VF3 | `0xE7_4B` | 0xE7 | 0x4B |
| `$k1` (27) | VF27 | `0xE6_4B` | 0xE6 | 0x4B |
| `$s3` (19) | VF19 | `0xE5_4B` | 0xE5 | 0x4B |
| `$t3` (11) | VF11 | `0xE4_4B` | 0xE4 | 0x4B |

The GPR register numbers (3, 27, 19, 11) correspond to VF register numbers used by the paired SQC2 block at `0x117C80`:

```asm
0x117C80: sqc2     $vf3,  -0x10B5($t1)  ; store VF3
0x117C84: sqc2     $vf11, -0x10B5($t1)  ; store VF11
0x117C88: sqc2     $vf19, -0x10B5($t1)  ; store VF19
0x117C8C: sqc2     $vf27, -0x10B5($t1)  ; store VF27
```

The `j 0x03800C` target is **outside the ELF** (below `0x100000`), meaning it's runtime resident code — likely the VIF/VU0 microcode upload manager loaded during IPL/boot.

### Occurrences

| Address | Pattern | Type |
|:---|:---|---|
| `0x117C40` | 4× LUI + ANDI + J | Full stub |
| `0x117C60` | 4× LUI + ANDI + J | Full stub (same) |
| `0x117CE0` | 1× LUI + J | Truncated variant |

The paired SQC2 block appears at `0x117C80` and `0x1181EC`.

---

## Callers of `0x00168650` (init_gp_slots, alternate selection)

**Result: Only 1 direct caller. Zero indirect callers.**

| Type | Address | Notes |
|:---|:---:|:---|
| **`J`** (tail call) | `0x001A3334` | Inside naked function `0x001A3208`. Always passes `a0=0` (cold paths). |
| **`JAL`** | — | **None found** |
| **LUI+ADDIW as fn ptr** | — | Address only appears in `.rodata` at `0x00613E04` (init-time pointer table) |

**There is no known static code path that calls `0x00168650` with `a0 != 0`.**

The alternate implementation is either:
- Activated by runtime-only path (JALR via function pointer, or self-modifying code)
- Activated by overlay-loaded code (outside the main ELF)
- Not actually used in normal execution

---

## Writers of halfword table at `0x006AB080`

### Result: 2 writer sites found

| Address | Pattern |
|:---:|:---|
| `0x00166D1C` | `LUI 0x006B / ADDIU -0x4F80 → 0x006AB080`; `SH (halfword store)` |
| `0x00166D78` | Same pattern, different code path (same enclosing function) |

Both sites are in a function **immediately preceding the dispatcher `0x00166E10`** (within the same `0x00166CXX`–`0x00166DXX` range).

### Writer code structure

```asm
0x00166D18: lw       $a1, -0x4BC4($gp)  ; a1 = gp-19396 = index counter
0x00166D1C: lui      $v1, 0x006B
0x00166D20: addiu    $v1, $v1, -0x4F80  ; v1 = 0x006AB080 (table base)
0x00166D24: sll      $a0, $a2, 5        ; value = a2 << 5
0x00166D28: sll      $v0, $a1, 1        ; index = counter * 2 (halfword offset)
0x00166D2C: addu     $a0, $a0, $t0      ; value += t0
0x00166D30: addu     $v0, $v0, $v1      ; addr = table + index*2
0x00166D34: addiu    $a1, $a1, 1        ; increment counter
0x00166D38: sh       $a0, 0($v0)        ; *** WRITE halfword to 0x6AB080[counter] ***
```

**Mechanism:**
- Counter at `gp-19396` (`0x00633D2C`) tracks the next write position
- Value written: `(a2 << 5) + t0` — encodes two parameters in a halfword
- Index advances by 1 per write
- Located in the same code region as the dispatcher — halfword table is populated during the same init/iteration pass

### Adjacent table

`0x006AA4B0` is constructed via similar patterns at `0x0014A2B0`, `0x0014AF70`, `0x0014AFB8` — suggesting a **family of BSS halfword tables** in `0x006Axxxx`.

---

## What is confirmed

1. `0x1D43F8` is a VU0 ring-buffer packet builder (96B frame, pushes 5×16 bytes to `0x4C7710`). The only sub-function is `0x111918` (ring_buffer_push).
2. `0x117C40` is an inline asm VU0 kick stub that loads VIF command constants and tail-calls runtime code at `0x3800C`. Return is via preserved `$ra`.
3. The alternate implementation (`a0 != 0` at `0x00168650`) has **no known static caller**. The only path always passes `a0=0`.
4. The halfword table at `0x6AB080` is populated by 2 write sites in the live dispatch code region (before `0x00166E10`). Values are `(a2 << 5) + t0`.

## What is probable

1. The constants at `0x55F260` are bone/object matrix indices for the VU0 transform pipeline.
2. The ring buffer at `0x4C7710` is consumed by a VU0 microprogram via DMA, with the terminator (`0xFFFFFFFF00000000`) signaling end of batch.
3. The halfword table at `0x6AB080` is populated during scene init iteration, and the 14 slot callbacks read from it to know which objects to process.

## What is unknown

1. Who calls `0x00168650` with `a0 != 0` — the alternate implementation selection point remains unknown.
2. The concrete VU0 microcode at `0x3800C` (outside ELF) that consumes the ring buffer packets.
3. The exact semantics of the values in the halfword table at `0x6AB080` (object type indices? entity IDs?).

## Conservative verdict

The static analysis of the live dispatch system's supporting infrastructure is complete:
- Ring buffer packet format and push mechanism mapped
- VU0 kick stub identified and linked to runtime code
- Halfword table population mechanism in the init/iteration loop mapped
- Alternate implementation constants read — small indices/parameters
- No static path to the alternate implementation exists — this is now a runtime question

The next step should be **runtime validation**: capture hits at the dispatcher `0x00166E10`, cold paths `0x00167230`/`0x00167258`, dispatch point `0x00167020`, and the init halfword writer `0x00166D38` to confirm which slots fire and what values populate `0x6AB080`.
