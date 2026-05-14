# rev.042 - Cloth Variant Field Writers

## Date

2026-05-14

## Objective

Map static writers of the field previously described as:

```txt
[state_block + 0x04]
```

Rev.041 showed that, for the `0x90`-byte cloth-domain payload used by
`0x001d37c8`, this field indexes the table at `0x004d4188`.

The minimum question for this revision is:

```txt
Who writes the value that later becomes the cloth variant/mode selector?
```

## Scope

Included:

- byte-level scan for `sw *, 0x04(*)`;
- local data-flow scan for stores to pointers loaded from `[context+0x15c]+0x800`;
- initializer `0x001d27a8`;
- setter-like function `0x001d1ad8`;
- related cloth initializer `0x001d1668`;
- cross-reference to ICO-decomp YAML source-file ranges.

Excluded:

- runtime/emulator validation;
- broad asset or `DATA.DF` analysis;
- assigning gameplay names;
- proving original C names;
- rewriting previous notes.

## Sources Used

| Source | Use |
|---|---|
| `AGENTS.md` | caution rules and project evidence discipline |
| `research/elf/ghidra-rev040-static-cloth-domain-reinterpretation.md` | cloth-domain correction and `0x001d27a8` context |
| `research/elf/ghidra-rev041-cloth-variant-table-004d4188.md` | table indexed by `[state_block+0x04]` |
| `research/elf/ghidra-rev027-rope-state-block-initializer.md` | prior initializer evidence |
| `research/elf/ghidra-rev025-runtime-confirmed-caller-context.md` | runtime-confirmed callback relationship |
| `/tmp/ICO-decomp/config/ICO-PAL.yaml` | source-file range orientation |
| `/tmp/ICO-decomp/config/symbol_addrs.txt` | nearby symbol orientation |
| `.local/extracted/SCUS_971.13.elf` | local USA bytes |
| Capstone 5.0.7 | local disassembly |

## Evidence Used

This revision uses:

- raw ELF instruction words;
- Capstone disassembly;
- simple local data-flow scans over `lw ...,0x800(...)` followed by
  `sw ...,4(...)`;
- source-file range mapping from ICO-decomp YAML.

This revision does **not** use runtime captures.

## Scan Summary

The full ELF contains many `sw *, 0x04(*)` instructions. A raw scan found:

```txt
sw imm=4 count 1345
lw imm=0x800 count 391
```

Most of those stores are unrelated structure writes. A narrower local data-flow
scan searched for:

```txt
lw R, 0x800(base)
...
sw X, 0x04(R)
```

within a short instruction window. It found 13 candidate relationships, but
only these were relevant to the cloth/`+0x800` question:

| Load VA | Store VA | Source range | Interpretation |
|---:|---:|---|---|
| `0x001d1798` | `0x001d17a8` | `sugipon/src/clothAnimation` | writes `1` to `+0x04` of a cloth `+0x800` payload in a related initializer |
| `0x001d1ae4` | `0x001d1b18` | `sugipon/src/clothAnimation` | generic setter writes caller-provided value to `+0x04` |

The scan also found non-cloth or different-structure candidates:

| Store VA | Source range | Reason not promoted to current cloth-variant writer |
|---:|---|---|
| `0x001c0894` / `0x001c08a0` | `common/src/layout_texture` | outside clothAnimation; direct setter for another subsystem |
| `0x001c32e8` / `0x001c3398` | `common/src/staffroll` | iterates records with stride 8; not the `0x90` cloth payload |
| `0x001ce330` | `sugipon/src/cage` | writes a different cage-domain `+0x800` payload |

Separately, a pattern search for payloads installed into `+0x800` and then
written at offset `+0x04` found:

| Install VA | Store VA | Source range | Interpretation |
|---:|---:|---|---|
| `0x001d2804` | `0x001d2858` | `sugipon/src/clothAnimation` | confirmed writer for the `0x90` payload used by Rev.040/041 |

## Instruction-Level Findings

### `0x001d27a8` - confirmed initializer writer for the Rev.041 payload

This is the strongest writer found in this pass.

Relevant instructions:

```asm
0x001d27e8: jal   0x0013a0f8
0x001d27ec: lw    s0,0x15c(s5)
0x001d27f0: move  s6,v0
0x001d2804: sw    s6,0x800(v1)
...
0x001d2850: lw    v1,0x30(s4)
0x001d2858: sw    v1,0x04(s6)
0x001d2860: lw    v1,0x04(s6)
0x001d2864: bnel  v1,v0,0x001d2980
```

Byte words:

| VA | Word | Instruction |
|---:|---:|---|
| `0x001d27e8` | `0x0c04e83e` | `jal 0x0013a0f8` |
| `0x001d2804` | `0xac760800` | `sw s6,0x800(v1)` |
| `0x001d2850` | `0x8e830030` | `lw v1,0x30(s4)` |
| `0x001d2858` | `0xaec30004` | `sw v1,0x04(s6)` |
| `0x001d2864` | `0x54620046` | `bnel v1,v0,0x001d2980` |

Confirmed effect:

```txt
payload = 0x0013a0f8(...)
[context+0x15c + 0x800] = payload
[payload + 0x04] = [initializer_arg + 0x30]
```

For the Rev.041 table relationship:

```txt
cloth_variant_index_candidate = [initializer_arg + 0x30]
```

This is a confirmed static write to the same payload that Rev.040 tied to
`0x001d37c8`, `0x001d3a30`, `0x001d2bf0`, and `0x001d3bf0`.

Immediate behavior after the write:

| Condition | Observed branch |
|---|---|
| `[payload+0x04] == 1` | continues into heavier setup path before `0x001d2980` |
| `[payload+0x04] != 1` | branches to `0x001d2980` via delay-slot load from `s5+0x08` |

This matches Rev.040/041: value `1` has a special role, but the field is not
only a boolean because Rev.041 proved it indexes an 8-entry table.

### `0x001d1ad8` - generic setter for three payload fields

`0x001d1ad8` is a small setter-like function:

```asm
0x001d1ad8: lw    v1,0x15c(a0)
0x001d1ae0: beq   a1,1,0x001d1b1c
0x001d1ae4: lw    v1,0x800(v1)
...
0x001d1b14: jr    ra
0x001d1b18: sw    a2,0x04(v1)
0x001d1b1c: jr    ra
0x001d1b20: sw    a2,0x0c(v1)
0x001d1b24: jr    ra
0x001d1b28: sw    a2,0x18(v1)
```

Byte words:

| VA | Word | Instruction |
|---:|---:|---|
| `0x001d1ad8` | `0x8c83015c` | `lw v1,0x15c(a0)` |
| `0x001d1ae4` | `0x8c630800` | `lw v1,0x800(v1)` |
| `0x001d1b18` | `0xac660004` | `sw a2,0x04(v1)` |
| `0x001d1b20` | `0xac66000c` | `sw a2,0x0c(v1)` |
| `0x001d1b28` | `0xac660018` | `sw a2,0x18(v1)` |

Observed selector behavior:

| `a1` | Written field | Value |
|---:|---:|---|
| `0` | `[payload+0x04]` | `a2` |
| `1` | `[payload+0x0c]` | `a2` |
| `2` | `[payload+0x18]` | `a2` |

Direct `jal` callers found for `0x001d1ad8`:

```txt
0x00216f24
0x00216f34
0x00216f44
```

The ICO-decomp YAML maps `0x00216f24` to:

```txt
fumi/src/way_llf + 0x284
```

Caller snippet:

```asm
0x00216f1c: lw    a0,-0x6e08(gp)
0x00216f20: addiu a1,zero,1
0x00216f24: jal   0x001d1ad8
0x00216f28: move  a2,zero

0x00216f2c: lw    a0,-0x6e08(gp)
0x00216f30: move  a1,zero
0x00216f34: jal   0x001d1ad8
0x00216f38: move  a2,zero

0x00216f3c: lw    a0,-0x6e08(gp)
0x00216f40: addiu a1,zero,2
0x00216f44: jal   0x001d1ad8
0x00216f48: move  a2,zero
```

Confirmed effect of the middle call:

```txt
0x00216f34 calls 0x001d1ad8 with a1=0, a2=0
therefore [payload+0x04] is set to 0 for the object in gp[-0x6e08]
```

What remains unknown:

- whether `gp[-0x6e08]` at that point is always the same runtime object as the
  ROPE/cloth payload tracked in Rev.040/041;
- whether this path executes in normal gameplay for the same object instance;
- whether this reset is part of teardown, scene transition, scripted control,
  or a broader cloth reset.

### `0x001d1668` - related cloth initializer, different records

`0x001d1668` is in `sugipon/src/clothAnimation`, but data references show it
belongs to records named:

```txt
GIRLDEMOCTRL
ENEMY1
SKELTEST
```

It installs a smaller payload:

```asm
0x001d1694: jal   0x0013a0f8        ; size 0x20
...
0x001d16b8: sw    a0,0x00(s1)
0x001d16c4: sw    s1,0x800(v0)
```

For one branch, it writes `1` to offset `+0x04`:

```asm
0x001d1788: lw    v0,0x15c(s2)
0x001d1798: lw    v1,0x800(v0)
0x001d17a8: sw    t2,0x04(v1)       ; t2 = 1
```

Interpretation:

- this is a real `+0x800` payload field writer inside `clothAnimation`;
- it is **not confirmed** to be the same `0x90` payload initialized by
  `0x001d27a8`;
- because its record references differ from ROPE and its allocation size is
  `0x20`, it should not be merged into the Rev.041 variant-table model without
  runtime evidence.

## Writer Classification

| Writer | Writes | Value source | Payload relation | Confidence |
|---:|---:|---|---|---|
| `0x001d2858` | `[payload+0x04]` | `[initializer_arg+0x30]` | confirmed `0x90` payload used by Rev.040/041 | high |
| `0x001d1b18` | `[payload+0x04]` | caller `a2` when `a1==0` | generic setter via `[context+0x15c]+0x800`; direct caller resets it to `0` | medium |
| `0x001d17a8` | `[payload+0x04]` | constant `1` | related cloth `+0x800` payload, but different records/allocation size | low for Rev.041 payload, high as a related writer |
| `0x001c0894` / `0x001c08a0` | `[payload+0x04]` | caller `a1` | `common/src/layout_texture`, outside cloth-domain | discarded for Rev.041 |
| `0x001c32e8` / `0x001c3398` | `[record+0x04]` | constant `1` | `common/src/staffroll`, iterated stride-8 records | discarded for Rev.041 |
| `0x001ce330` | `[payload+0x04]` | `0` | `sugipon/src/cage`, different source range | discarded for Rev.041 |

## Relationship to Rev.041 Table

Rev.041 established:

```txt
entry_ptr = 0x004d4188 + ([state_block+0x04] * 0x14)
```

Rev.042 adds:

```txt
initial [state_block+0x04] for the 0x90 payload comes from [initializer_arg+0x30]
```

The current conservative chain is:

```txt
record/init argument
-> [arg + 0x30]
-> 0x001d2858
-> [payload + 0x04]
-> 0x001d2bf0 / 0x001d3bf0 table index
-> 0x004d4188 + index * 0x14
```

This means the next non-runtime static target is not the table itself, but the
producer of the initializer argument passed to `0x001d27a8`.

## What Is Confirmed

1. `0x001d2858` writes `[payload+0x04]` for the `0x90` payload installed at
   `[context+0x15c]+0x800` by `0x001d27a8`.
2. The value written by `0x001d2858` is loaded from `[s4+0x30]`, where `s4` is
   the second argument preserved by the initializer.
3. `0x001d1ad8` is a generic setter that can write `[payload+0x04]`,
   `[payload+0x0c]`, or `[payload+0x18]` depending on `a1`.
4. `fumi/src/way_llf` calls `0x001d1ad8` three times at `0x00216f24`,
   `0x00216f34`, and `0x00216f44`, with the middle call setting
   `[payload+0x04]` to `0`.
5. `0x001d1668` is another `sugipon/src/clothAnimation` initializer that
   installs a `+0x800` payload and writes `1` to its `+0x04` field for one
   branch.

## What Is Probable

1. `[initializer_arg+0x30]` is the original static/runtime source for the
   Rev.041 cloth variant index.
2. `0x001d1ad8` is a small control/reset setter for selected cloth payload
   fields.
3. The `fumi/src/way_llf` caller resets the variant/mode field to `0` for an
   object obtained from `gp[-0x6e08]`.

## What Is Possible

1. `0x001d1ad8` may affect the same ROPE/cloth object seen in Rev.040/041, but
   this needs runtime confirmation of the `a0` object pointer.
2. `0x001d1668` may represent a sibling cloth payload layout sharing the same
   `+0x800` convention, but not the same `0x90` structure.
3. Values other than `0`, `1`, and the table indices observed in Rev.041 may be
   produced by records not reached in this static pass.

## What Is Unknown

1. Who constructs or fills the initializer argument passed as `s4` to
   `0x001d27a8`.
2. Whether `[s4+0x30]` is bounded to `0..7` before it reaches `0x001d2858`.
3. Whether `0x001d1ad8` writes the same runtime payload as `0x001d27a8` during
   normal gameplay.
4. Whether all runtime values of `[payload+0x04]` are valid table indices for
   `0x004d4188`.

## What Is Discarded

1. Treating `0x001d390c` as a writer to `[state_block+0x04]`.
   In the dispatcher, `$s1 = state_block + 0x40`, so `sw v0,4(s1)` writes
   `[state_block+0x44]`, the countdown/timer field from Rev.024.
2. Treating all `sw *,4(*)` instructions in the ELF as relevant. The raw count
   is too broad and mostly unrelated.
3. Merging the `0x001d1668` payload into the Rev.041 `0x90` payload. The
   allocation size and data-record references differ.
4. Assigning gameplay names to the writers based on record labels alone.

## Next Minimum Test

Sem emulador:

1. Trace the caller path that invokes the `+0x48` record callback
   `0x001d27a8`, and identify where its second argument is built.
2. Search `.data` records around `0x002a397c` for records whose argument block
   exposes a meaningful `+0x30` value.
3. Map whether any static table or record values at `+0x30` are limited to
   `0..7`, matching the Rev.041 table count.

Com emulador:

1. Break at `0x001d2858` and capture `s4`, `[s4+0x30]`, `s6`, and
   `[s6+0x04]` after the store.
2. Break at `0x001d1b18` and capture caller RA, `a0`, `a1`, `a2`, and the
   resolved payload pointer.
3. Confirm whether `0x00216f34 -> 0x001d1ad8` writes the same payload pointer
   later used by `0x001d2bf0` and `0x001d3bf0`.

## Conservative Verdict

The confirmed origin of the Rev.041 variant/mode field for the `0x90` cloth
payload is `0x001d2858`, which copies `[initializer_arg+0x30]` into
`[payload+0x04]`.

There is also a generic setter at `0x001d1ad8` that can overwrite the same
offset through `[context+0x15c]+0x800`; a known `fumi/src/way_llf` caller uses
it to reset the field to `0`. That path is a strong static candidate for
runtime mutation, but it is not yet proven to target the same live object as
the ROPE/cloth payload from Rev.040/041.

The next static step should follow the argument source for `0x001d27a8`,
especially the value at `+0x30`.
