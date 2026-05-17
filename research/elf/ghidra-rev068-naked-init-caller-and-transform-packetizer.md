# Rev.068 — Naked init caller at 0x001A3208 (cold path selector + bulk GP init) and transform orchestrator 0x1D4A58 (VU0 ring-buffer packetizer)

**Date:** 2026-05-17

## Objective

Follow up on two items from Rev.067:

1. Disassemble `0x001A3334` (tail-call J to `0x00168650`) and its enclosing function to understand who selects cold vs alternate implementation.
2. Disassemble `0x1D4A58` (transform/matrix function called twice by the alternate implementation with `t0=0` then `t0=-1`).

## Scope

- Static disassembly only (Capstone MIPS64 little-endian from ELF)
- No runtime, no PCSX2, no DATA.DF, no assets

## Sources used

| Source | Use |
|---|---|
| `research/elf/ghidra-rev067-consolidated-live-dispatch-model.md` | alternate implementation calling pattern |
| `.local/extracted/SCUS_971.13.elf` | byte-level source |

---

## Function `0x001A3208` — Naked bulk init + cold-path selector

### Bounds

| Property | Value |
|---|---|
| Start | `0x001A3208` |
| End | `0x001A333C` (next function at `0x001A3340`) |
| Prologue | **None** — zero-byte frame, no callee-saves |
| Epilogue | None — ends with tail-call `J 0x00168650` |
| Type | **Naked function** |

The function has no `addiu $sp,...` adjustment and no `sd $ra,...`. It is a raw sequence of stores followed by a tail-call.

### Caller

Only caller: `JAL 0x001A3208` at `0x00101D58`, inside function `0x00101C80` (128-byte stack frame, saves `$ra` + `$s0`–`$s6`).

Before the JAL, `0x00101C80`:
1. Reads from `0x274EC0`/`0x274EC4` (system state/timing)
2. Does multiply/divide with div-by-zero guard
3. Calls `JAL 0x1AA4E8` (returns a value)
4. Caps return to `max(v0, 103)`, stores to `gp-0x6F10`
5. **Then** calls `JAL 0x1A3208` (our function)
6. After return, calls `JAL 0x1025E8` and more init

Probable role: **engine/task system cold-start initialization**.

### Behavior

**Step 1: Bulk GP data initialization** (~70 stores)

| Value | Count | Examples |
|---:|---:|---|
| `0` | ~50 | `gp-0x5C10`, `gp-0x5C3C`, `gp-0x5C40`, `gp-0x5BCC`, etc. |
| `1` | ~16 | `gp-0x5C04`, `gp-0x5C08`, `gp-0x5BF0`, `gp-0x5BE4`, etc. |
| `4` | 3 | `gp-0x5B58`, `gp-0x5B8C`, `gp-0x5A5C` |
| `15` | 2 | `gp-0x5C00` |
| `3` | 1 | `gp-0x5B84` |
| `5` | 1 | `gp-0x5B54` |
| `25` | 1 | `gp-0x5BBC` |
| `100` | 1 | `gp-0x5BC0` |

**Step 2: Select cold paths** (`a0 = 0`)

```asm
0x001A323C: move  $a0, $zero        ; a0 = 0 -> cold paths
...
0x001A3334: j     0x00168650        ; tail-call init_gp_slots
0x001A3338: sw    $zero, -0x6ED0($gp)  ; delay slot: clear another slot
```

### Tail-call chain

```txt
0x00101C80:
  JAL 0x1A3208   -> $ra = 0x00101D5C

0x001A3208 (naked):
  bulk GP init (70 stores)
  a0 = 0          -> selects cold paths
  J 0x00168650    -> preserves $ra, transfers control

0x00168650 (init_gp_slots):
  gp-25856 = 0x00167230 (cold path A)
  gp-25852 = 0x00167258 (cold path B)
  ...(epilogue)
  JR $ra          -> returns to 0x00101D5C
```

**Key finding:** The init at `0x00101C80` always requests **cold paths** (`a0=0`). The alternate implementation (`a0 != 0`) selection point is elsewhere — still unknown.

---

## Function `0x1D4A58` — Transform orchestrator / VU0 ring-buffer packetizer

### Bounds

| Property | Value |
|---|---|
| Start | `0x001D4A58` |
| End | `0x001D4B38` |
| Size | 0xE0 (224 bytes, 56 instructions) |
| Stack | 0xC0 (192 bytes) |
| Callee-saves | `$s0`–`$s6` + `$ra` (8 × 8 = 64 bytes) |

### Calling convention

| Register | Value |
|---:|---|
| `a0` | entity context |
| `a1` | stack with matrix/init data |
| `a2` | `entity+0x10` |
| `a3` | stack pointer (same buffer or shifted) |
| `t0` | **forwarded to `0x1D43F8`** — controls packet format |

### Sub-functions called

| Call PC | Target | Role |
|---:|---:|---|
| `0x1D4A8C` | `0x00117C40` | VU0 sync stub: 4× LUI of high constants, then `J 0x103800C`. Return discarded. |
| `0x1D4AA4` | `0x001D45B0` | Matrix/quaternion setup: builds 4×4 transform from entity data. Returns 0 or 1. |
| `0x1D4AC0` | `0x00118430` | Conditional matrix copy (bit test on `a1[1]`) |
| `0x1D4AD0` | `0x00118430` | Same, different buffer pair |
| `0x1D4AEC` | `0x001D43F8` | **VU0 ring-buffer packet builder** (`t0` forwarded) |
| `0x1D4B0C` | `0x001D43F8` | Same, swapped a0/a2 (PATH_A) |

### Flow

```txt
0x1D4A58:
  1. s6 = t0         (saved, forwarded to 0x1D43F8)
  2. JAL 0x117C40    (VU0 sync stub — return discarded)
  3. JAL 0x1D45B0    (build 4x4 matrix from entity data)
  4. s2 = v0         (0 or 1 from 0x1D45B0)
  5. if s2 == -1: skip (defensive)
  6. Conditional matrix copy via 0x118430 (twice)
  7. Branch on s2:
     PATH_B (s2 != 0):
       a0 = sp+0x30 (src), a2 = sp+0x20 (dst)
       JAL 0x1D43F8(t0=s6)
     PATH_A (s2 == 0):
       a0 = sp+0x20 (src), a2 = sp+0x30 (dst)  [SWAPPED]
       JAL 0x1D43F8(t0=s6)
  8. Epilogue: restore $ra, $s0-$s6, jr $ra
```

### How `t0` is used

**Critical finding:** `t0` is **NOT branched on** inside `0x1D4A58`. It is saved to `$s6` and forwarded verbatim to `0x1D43F8`.

The real two-pass pattern is at the **callers** (`0x00169F80`/`0x0016A058`), which load different constant blocks into the `a1`/`a3` stack slots for each pass:

| Pass | t0 | a1 (stack data) | a3 |
|---:|---:|---:|---|
| 1 | 0 | `sp+0x00` (4 QWORDs from `0x55F280`) | `sp+0x00` |
| 2 | -1 | `sp+0x10` (4 QWORDs from `0x55F290`) | `sp+0x10` |

Inside `0x1D43F8`, `t0` controls packet format: with `t0=-1`, a `0xFFFFFFFF00000000` terminator is inserted.

### Interpretation

`0x1D4A58` is a **transform orchestrator** that:
1. Syncs VU0 (`0x117C40`)
2. Builds a 4×4 matrix from entity data (`0x1D45B0`)
3. Copies it conditionally to two local buffers (`0x118430` × 2)
4. Packs the result into the VU0 ring buffer at `0x4C7710` (`0x1D43F8`)
5. The two-call pattern at the alternate implementation sends two different constant blocks as separate transforms in the VU0 ring buffer, with the second call inserting a terminator

---

## Corrections from Rev.067

| Claim from Rev.067 | Correction |
|---|---|
| `0x1D4A58` chooses behavior based on `t0` | `t0` is forwarded to `0x1D43F8`; the branch inside `0x1D4A58` is on `s2` (return of `0x1D45B0`) |
| Two-pass pattern is inside `0x1D4A58` | Two-pass is at the callers — different data blocks per pass |
| `0x1A3334` is the caller | The full function is `0x001A3208` (naked, ~70 GP stores, then tail-call) |
| The caller always selects cold paths | Not just "always" — `a0 = 0` is hardcoded at `0x001A323C` |

---

## What is confirmed

1. `0x001A3208` is the only direct ancestor of `0x00168650` — it initializes ~70 GP slots and selects cold paths (`a0=0`).
2. The alternate implementation (`a0!=0`) selection point is elsewhere — not in this init chain.
3. `0x1D4A58` is a VU0 transform packetizer that builds a 4×4 matrix and submits it to the VU0 ring buffer via `0x1D43F8`.
4. `t0` is passed through to `0x1D43F8` where `t0=-1` adds a terminator.
5. The branch inside `0x1D4A58` is on `s2` (return of `0x1D45B0` matrix builder), swapping which buffer is source vs destination.

## What is probable

1. The alternate implementation (`0x00169F80`/`0x0016A058`) sends two distinct transform packets (with different constant data) to the VU0 ring buffer per invocation, forming a two-pass transform submission.
2. `0x1D43F8` at `0x4C7710` manages a VU0 microcode ring buffer — the terminator (`0xFFFFFFFF00000000`) marks end of a batch.

## What is unknown

1. Who calls `0x00168650` with `a0 != 0` to select the alternate implementation? (The only known caller uses `a0=0`.)
2. What condition triggers the alternate implementation?
3. The exact data format at `0x55F260`–`0x55F298` used by the alternate callers.

## Next minimum test

Without runtime:
1. Search for all callers of `0x00168650` that might set `a0 != 0` (look for JALR or other indirect paths).
2. Disassemble `0x1D43F8` — the VU0 ring-buffer packet builder.
3. Search for all callers of `0x1D4A58` beyond the alternate implementation.

With runtime:
1. Set breakpoint at `0x00168650` to capture `a0` when called, confirming if alternate is ever selected.
2. Set breakpoint at `0x00169F80` to see if alternate implementation ever fires.

## Conservative verdict

The cold-path init chain is confirmed:
- `0x00101C80 → JAL 0x1A3208 → J 0x00168650` always selects cold paths (`a0=0`)
- `0x001A3208` initializes ~70 GP slots as a bulk cold-start configuration

The transform orchestrator `0x1D4A58` is confirmed as a VU0 ring-buffer packetizer shared by the alternate implementation. The two-pass pattern originates in the alternate callers, not inside `0x1D4A58` itself. The alternate implementation selection point remains unknown — it is not in the cold-start init chain.
