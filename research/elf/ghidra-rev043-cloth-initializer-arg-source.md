# rev.043 - Cloth Initializer Argument Source

## Date

2026-05-14

## Objective

Follow the static source of the second argument used by `0x001d27a8`, especially:

```txt
[initializer_arg + 0x30]
```

Rev.042 showed that this value is copied into:

```txt
[payload + 0x04]
```

and Rev.041 showed that `[payload + 0x04]` later indexes the table at
`0x004d4188`.

## Scope

Included:

- prologue of `0x001d27a8`;
- known callback slot `+0x48` dispatchers;
- direct call search for `0x001d27a8`;
- data reference search for `0x001d27a8`;
- source-file range orientation from ICO-decomp YAML;
- conservative correction to the current call model.

Excluded:

- runtime/emulator validation;
- broad room/layout data analysis;
- `DATA.DF` or asset extraction;
- assigning gameplay names;
- changing older revision notes.

## Sources Used

| Source | Use |
|---|---|
| `research/elf/ghidra-rev031-record-callback-dispatchers.md` | prior model for slot `+0x48` dispatch |
| `research/elf/ghidra-rev034-callback-signature-and-record-selection.md` | callback signature caution and record selection context |
| `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md` | registration-path caveats |
| `research/elf/ghidra-rev042-cloth-variant-field-writers.md` | immediate source of `[payload+0x04]` |
| `/tmp/ICO-decomp/config/ICO-PAL.yaml` | source-file range orientation |
| `.local/extracted/SCUS_971.13.elf` | local USA bytes |
| Capstone 5.0.7 | local disassembly |

## Evidence Used

This revision uses:

- instruction-level disassembly;
- raw instruction words;
- direct `jal` encoding search;
- raw little-endian word search for `0x001d27a8`;
- static scan for `lw ...,0x48(...)` followed by `jalr`.

This revision does not use runtime captures.

## Starting Point

Rev.042 established this local write:

```asm
0x001d27cc: move  s4,a1
...
0x001d2850: lw    v1,0x30(s4)
0x001d2858: sw    v1,0x04(s6)
```

Therefore:

```txt
[payload + 0x04] = [a1_at_entry_to_0x001d27a8 + 0x30]
```

The unresolved question is where `a1` comes from when `0x001d27a8` is invoked.

## Instruction-Level Findings

### `0x001d27a8` expects two meaningful input registers

Prologue:

```asm
0x001d27a8: addiu sp,sp,-0xd0
0x001d27bc: move  s5,a0
0x001d27cc: move  s4,a1
...
0x001d27ec: lw    s0,0x15c(s5)
...
0x001d2850: lw    v1,0x30(s4)
0x001d2858: sw    v1,0x04(s6)
```

Byte words:

| VA | Word | Instruction |
|---:|---:|---|
| `0x001d27bc` | `0x0080a82d` | `move s5,a0` |
| `0x001d27cc` | `0x00a0a02d` | `move s4,a1` |
| `0x001d2850` | `0x8e830030` | `lw v1,0x30(s4)` |
| `0x001d2858` | `0xaec30004` | `sw v1,0x04(s6)` |

Confirmed:

- `a0` is treated as the object/context pointer because the function later
  reads `[a0 + 0x15c]`;
- `a1` is treated as a second structure pointer because the function later
  reads `[a1 + 0x30]`;
- the value from `[a1 + 0x30]` is the initial variant/mode value copied into
  the Rev.041 table index field.

### Direct call search for `0x001d27a8`

Search for the direct MIPS `jal 0x001d27a8` encoding found:

```txt
none
```

This matches the earlier callback-slot model: `0x001d27a8` is not reached by a
plain direct call in the local ELF.

### Data reference search for `0x001d27a8`

Raw word search for `0x001d27a8` found one loaded-segment occurrence:

| Value | File offset | VA | Interpretation |
|---:|---:|---:|---|
| `0x001d27a8` | `0x001a497c` | `0x002a397c` | `ROPE` record slot `+0x48` |

No raw word references were found for:

```txt
0x002a397c
0x002a3934
```

This means the static link to `0x001d27a8` is still the `ROPE +0x48` function
pointer, not a direct code reference to the data address.

### Known slot `+0x48` dispatcher does not prepare `a1`

The broad scan for:

```asm
lw callback,0x48(base)
...
jalr callback
```

found two slot `+0x48` call sites, both inside `0x0013fc00`.

First path:

```asm
0x0013fc38: lw    v0,0x48(s2)
0x0013fc44: jalr  v0
0x0013fc48: move  a0,s2
```

Second path:

```asm
0x0013fcac: lw    v0,0x48(s0)
0x0013fcb8: jalr  v0
0x0013fcbc: move  a0,s0
```

Byte words:

| VA | Word | Instruction |
|---:|---:|---|
| `0x0013fc38` | `0x8e420048` | `lw v0,0x48(s2)` |
| `0x0013fc44` | `0x0040f809` | `jalr v0` |
| `0x0013fc48` | `0x0240202d` | `move a0,s2` |
| `0x0013fcac` | `0x8e020048` | `lw v0,0x48(s0)` |
| `0x0013fcb8` | `0x0040f809` | `jalr v0` |
| `0x0013fcbc` | `0x0200202d` | `move a0,s0` |

Confirmed:

- both paths set `a0` in the delay slot;
- neither path sets `a1` near the `jalr`;
- the surrounding code uses `a0` as a loop/index register and object pointer,
  but no stable `a1` setup was observed for the callback.

This is a correction to the earlier implicit assumption that the known
`+0x48` dispatcher fully explains the inputs to `0x001d27a8`.

### Source-file range orientation

ICO-decomp YAML maps the relevant ranges as:

| VA | Source-file range | Note |
|---:|---|---|
| `0x0013fc00` | `fumi/ios/shockdriver` | known slot `+0x48` dispatcher |
| `0x00101f40` | `sdk/lib/libkernl/thread` | only direct caller of `0x0013fc00` found by `jal` search |
| `0x001d27a8` | `sugipon/src/clothAnimation` | initializer consuming `a0` and `a1` |

The source-file ranges are orientation only. They do not prove original C
function names.

## Model Correction

Previous safe model:

```txt
record/list traversal
-> load [object_or_record + 0x48]
-> jalr callback
-> for ROPE, callback can be 0x001d27a8
```

Rev.043 correction:

```txt
known +0x48 dispatcher
-> passes a0
-> does not visibly prepare a1
-> cannot, by itself, explain [a1 + 0x30] in 0x001d27a8
```

Therefore the more precise current model is:

```txt
0x001d27a8 requires:
  a0 = object/context pointer
  a1 = initializer argument pointer with meaningful +0x30 field

known static +0x48 dispatcher proves:
  callback is callable through +0x48 with a0 set

known static +0x48 dispatcher does not prove:
  where a1 comes from
```

## Candidate Explanations

| Hypothesis | Status | Reason |
|---|---|---|
| `0x0013fc00` calls `0x001d27a8` with fully prepared `(a0,a1)` | weak | it sets `a0`, but no `a1` setup was observed |
| another dispatcher calls `+0x48` with `(a0,a1)` | possible | no such direct `lw +0x48 -> jalr` path was found in this pass, but a copied/staged callback could evade the simple scan |
| `a1` is inherited from an outer call into `0x0013fc00` | weak | `0x0013fc00` is called directly from `0x00101f40` with no `a1` setup in the delay slot, and callbacks may clobber caller-saved registers |
| `0x001d27a8` is present in `ROPE +0x48` but not invoked by the observed `0x0013fc00` path in the case needing `a1` | possible | static data reference and dispatcher existence do not prove this exact runtime pairing |
| `[a1+0x30]` comes from object layout/room data prepared elsewhere | possible | Rev.042's write proves the field is consumed, but producer remains unmapped |

## What Is Confirmed

1. `0x001d27a8` preserves `a0` into `s5` and `a1` into `s4`.
2. `0x001d27a8` reads `[s4+0x30]` and writes that value to `[payload+0x04]`.
3. There is no direct `jal 0x001d27a8` in the local ELF.
4. The only raw word reference to `0x001d27a8` found in the loaded segment is
   `0x002a397c`, the known `ROPE +0x48` slot.
5. The only simple static slot `+0x48` `jalr` callers found are the two paths
   inside `0x0013fc00`.
6. Those two `0x0013fc00` paths set `a0` but do not visibly set `a1`.

## What Is Probable

1. The known `+0x48` dispatch model is incomplete for explaining the full
   signature of `0x001d27a8`.
2. `[initializer_arg+0x30]` is real and important, but its producer is not
   statically resolved by the current dispatcher notes.
3. Runtime capture at `0x001d27a8` or `0x001d2858` would resolve this faster
   than more broad static scanning.

## What Is Possible

1. A less direct callback path copies `ROPE +0x48` elsewhere before calling it.
2. `a1` may be a pointer to a setup/layout/entry structure not represented by
   the static `ROPE` record itself.
3. The object reaching `0x001d27a8` through `0x0013fc00` may only be valid in a
   context where `a1` is incidentally preserved, but this would be fragile and
   is not a strong compiler-level explanation.

## What Is Unknown

1. The producer of `a1` at `0x001d27a8` entry.
2. Whether `a1` is stable and meaningful when `0x0013fc00` invokes slot
   `+0x48`.
3. Whether `[a1+0x30]` is bounded to `0..7`.
4. Whether the runtime path that initializes the Rev.041 payload uses
   `0x0013fc00` or a different staged callback mechanism.

## What Is Discarded

1. Treating `0x0013fc00` as a complete explanation for the inputs of
   `0x001d27a8`.
2. Claiming that the producer of `[initializer_arg+0x30]` has been found.
3. Treating `ROPE +0x48` as a direct code xref. It is a data function pointer.
4. Assuming `a1` can be ignored. `0x001d27a8` directly dereferences it.

## Next Minimum Test

Sem emulador:

1. Search for staged callback mechanisms where a function pointer loaded from
   slot `+0x48` is stored into another structure before `jalr`.
2. Track writers to object/node fields that may carry an init argument pointer
   near the time `+0x48` callbacks are registered.
3. Revisit the object creation path around `0x0013f7a8` / `0x0013f3f0` for
   stores of an auxiliary pointer that could later become `a1`.

Com emulador:

1. Break at `0x001d27a8` and capture `a0`, `a1`, `ra`, `[a1+0x30]`, and the
   object pointer chain `[a0+0x15c]`.
2. Break at `0x0013fc44` and `0x0013fcb8`; when the callback target is
   `0x001d27a8`, capture `a0`, `a1`, `s0`, `s2`, and `ra`.
3. Break at `0x001d2858` to confirm the copied value and compare it to the
   Rev.041 table bounds.

## Conservative Verdict

Rev.043 does not find the producer of `[initializer_arg+0x30]`. Instead, it
finds an important static gap: `0x001d27a8` clearly consumes `a1`, but the known
slot `+0x48` dispatcher at `0x0013fc00` only prepares `a0`.

The safest conclusion is that the current static callback model is incomplete
for the initializer path. The Rev.041/042 chain remains valid from
`0x001d2858` onward, but the origin of `a1` must stay open until either a staged
callback path is found or runtime captures identify the actual entry registers.
