# rev.044 - Staged Callback Path for 0x001d27a8

## Date

2026-05-14

## Objective

Search for a staged callback path that could explain how `0x001d27a8` receives
both:

```txt
a0 = object/context pointer
a1 = initializer argument pointer
```

Rev.043 showed that the known direct `+0x48` dispatcher prepares `a0` but does
not visibly prepare `a1`. This revision checks whether `0x001d27a8` is copied
into an intermediate node or called through another descriptor field that does
prepare `a1`.

## Scope

Included:

- `0x0013f3f0` node registration/storage;
- `0x0013f778`, `0x0013f7a8`, `0x0013f7d8`, and `0x0013f930` wrapper variants;
- `0x0013f9d0 / 0x0013fb70` node callback dispatcher;
- scan for `lw ...,0x48(...)` followed by store or `jalr`;
- descriptor constructor-like path at `0x00129660`;
- ROPE record fields around `0x002a3934`;
- source-file range orientation from ICO-decomp YAML.

Excluded:

- runtime/emulator validation;
- `DATA.DF` / asset extraction;
- broad gameplay interpretation;
- rewriting older notes.

## Sources Used

| Source | Use |
|---|---|
| `research/elf/ghidra-rev033-node-callback-dispatch-chain.md` | node `+0x1c` registration and dispatch model |
| `research/elf/ghidra-rev036-registration-path-survey.md` | `0x0013f7a8` callers and registration caveats |
| `research/elf/ghidra-rev043-cloth-initializer-arg-source.md` | gap: `0x001d27a8` consumes `a1`, known `+0x48` dispatcher does not prepare it |
| `/tmp/ICO-decomp/config/ICO-PAL.yaml` | source-file range orientation |
| `.local/extracted/SCUS_971.13.elf` | local USA bytes |
| Capstone 5.0.7 | local disassembly |

## Evidence Used

This revision uses:

- instruction-level disassembly;
- direct `jal` target search;
- local data-flow scan for `lw ...,0x48(...)` followed by stores or `jalr`;
- raw data dump of the ROPE descriptor record.

No runtime captures were used.

## Question Being Tested

The candidate missing path was:

```txt
ROPE +0x48 = 0x001d27a8
-> copied/staged into another structure
-> later called with a0 and a1
```

If such a path exists statically, it should leave at least one of these traces:

1. a `+0x48` load stored into another callback field;
2. a `+0x48` load called indirectly after `a1` is prepared;
3. a wrapper such as `0x0013f7a8` storing the callback into a node whose
   dispatcher later passes more than `a0`;
4. a descriptor-field call path that uses the same `0x001d27a8` value and
   prepares `a1`.

## Findings

### 1. `0x0013f3f0` stores callbacks, but not an auxiliary `a1`

`0x0013f3f0` receives values from wrapper functions and creates/reuses a node
of apparent stride `0x94`.

Relevant register preservation:

```asm
0x0013f40c: move  s5,a1
0x0013f414: move  s4,a2
0x0013f41c: move  s3,t0
0x0013f424: move  s2,a0
```

Main callback store:

```asm
0x0013f550: sw    s4,0x1c(s1)
0x0013f55c: sw    s6,0x10(s1)
0x0013f560: sw    v0,0x18(s1)
0x0013f564: sw    s2,0x04(s1)
0x0013f56c: sw    s3,0x14(s1)
```

Observed node fields:

| Field | Stored value | Meaning in current model |
|---:|---|---|
| `+0x04` | `s2`, original `a0` to `0x0013f3f0` | object/context pointer |
| `+0x10` | `s6`, masked original `a3` | node category/phase byte |
| `+0x14` | `s3`, original `t0` | ordering/priority-like value |
| `+0x18` | `1` | enabled flag |
| `+0x1c` | `s4`, original `a2` | callback pointer |

No field in this observed store block preserves an auxiliary pointer that the
dispatcher later passes as `a1`.

### 2. Wrapper variants still register a callback, not a two-argument call

Relevant wrappers:

```asm
0x0013f778: move  t0,a3
0x0013f788: move  a2,a1
0x0013f790: jal   0x0013f3f0
0x0013f794: move  a1,a0

0x0013f7a8: move  v0,a3
0x0013f7b8: move  a2,a1
0x0013f7c4: jal   0x0013f3f0
0x0013f7c8: move  a1,a0

0x0013f7d8: move  t0,a3
0x0013f7e8: move  a2,a1
0x0013f7f0: jal   0x0013f3f0
0x0013f7f4: move  a1,zero

0x0013f930: move  v0,a3
0x0013f940: move  a2,a1
0x0013f94c: jal   0x0013f3f0
0x0013f950: move  a1,zero
```

All four variants pass the candidate callback through `a2` into
`0x0013f3f0`. They differ in metadata (`a1`, `t0`, `t1`, `a3` handling), but
none turns the candidate callback into an immediate two-argument function call.

### 3. The node dispatcher only passes `a0`

`0x0013f9d0 / 0x0013fb70` dispatches callbacks stored in `node +0x1c`:

```asm
0x0013fb64: lw    v0,0x1c(s0)
0x0013fb70: jalr  v0
0x0013fb74: move  a0,s2
```

This is adequate for `0x001d3a30`, whose prologue only requires an
object/context pointer in `a0`.

It is not adequate to explain `0x001d27a8`, because Rev.043 showed:

```asm
0x001d27bc: move  s5,a0
0x001d27cc: move  s4,a1
...
0x001d2850: lw    v1,0x30(s4)
```

Therefore, registering `0x001d27a8` into `node +0x1c` would still not explain
the meaningful `a1` required by the initializer.

### 4. Scan for `+0x48` staging did not find a useful path for `0x001d27a8`

A broad scan found:

```txt
total lw +0x48: 199
loads with store or jalr within local window: 22
```

The only simple `+0x48 -> jalr` paths remain:

| Load | Call | Delay slot |
|---:|---:|---|
| `0x0013fc38` | `0x0013fc44` | `move a0,s2` |
| `0x0013fcac` | `0x0013fcb8` | `move a0,s0` |

Both are inside `0x0013fc00` and were already documented in Rev.043.

Other `+0x48` loads in the scan were copies or flag manipulation, not callback
staging for `0x001d27a8`. Examples:

| VA | Source range | Observed use |
|---:|---|---|
| `0x0011503c` | `seki/src/GsBase` | list pointer splice/copy |
| `0x00192a4c` | `omori/src/camera-ico2` | OR/AND bit manipulation on `entry+0x48` |
| `0x00193be8` | `omori/src/camera-ico2` | OR/AND bit manipulation on `entry+0x48` |
| `0x001b7ed4` | `common/src/gamesys` | ORs flag bit into `entry+0x48`, then stores it back |
| `0x0020300c` | `sugipon/src/weapon` | uses `+0x48` as numeric/offset-like data |

The closest candidate, `0x001b7ed4`, is not a callback call:

```asm
0x001b7ed4: lw    v0,0x48(v1)
0x001b7ed8: or    v0,v0,a2
0x001b7ee0: sw    v0,0x48(v1)
```

This treats `entry+0x48` as flags, not as a function pointer.

### 5. Descriptor `+0x58` call path prepares `a1`, but ROPE does not use it

There is a constructor-like path at `0x00129660`:

```asm
0x00129644: lw    v1,0x00(a0)
0x00129648: lw    v0,0x04(v1)
0x0012964c: mult  v0,t3            ; t3 = 0x64
0x00129650: addu  s0,v0,t2          ; t2 = 0x002a31b8
0x00129654: lw    v0,0x58(s0)
0x00129658: beqz  v0,0x00129674
0x0012965c: move  a0,s1
0x00129660: jalr  v0
0x00129664: addiu a1,sp,0x20
0x00129668: lw    v1,0x00(s3)
0x0012966c: lw    a0,0x15c(v1)
0x00129670: sw    v0,0x800(a0)
```

This is structurally important:

- it calls a descriptor callback with `a0 = s1`;
- it prepares `a1 = sp + 0x20`;
- it stores the callback return value into `[object+0x15c]+0x800`.

That resembles the signature shape needed by `0x001d27a8`.

However, for the static ROPE record:

```txt
record_start = 0x002a3934
ROPE +0x48  = 0x002a397c = 0x001d27a8
ROPE +0x58  = 0x002a398c = 0x00000000
```

Relevant dump:

```txt
0x002a3970: 00000000 001d3a30 00000000 001d27a8
0x002a3980: 00000000 00000000 45504f52 00000000
0x002a3990: 00000000 00000000 00000000 00000000
```

So `0x00129660` is a real two-argument descriptor-callback mechanism, but it
does **not** call `0x001d27a8` for the static ROPE record because it reads
`+0x58`, not `+0x48`.

## Classification of Candidate Paths

| Path | Prepares `a1`? | Can call `0x001d27a8` statically? | Verdict |
|---|---:|---:|---|
| `0x0013fc00` direct `+0x48` dispatcher | no | yes, via ROPE `+0x48` | incomplete signature |
| `0x0013f3f0 -> node+0x1c -> 0x0013fb70` | no, dispatcher passes only `a0` | no direct evidence for `0x001d27a8`; suitable for `+0x40` callbacks | not enough for initializer |
| `0x00129660` descriptor `+0x58` constructor | yes | no, ROPE `+0x58` is zero | structurally similar but excluded for ROPE |
| `entry+0x48` flag manipulation paths | no | no | not callback paths |

## What Is Confirmed

1. `0x0013f3f0` stores a callback pointer into `node+0x1c`.
2. The dispatcher at `0x0013fb70` calls `node+0x1c` with only `a0 = s2`.
3. This staged node path is compatible with `0x001d3a30`, but not with the
   full observed signature of `0x001d27a8`.
4. The only simple `+0x48 -> jalr` paths found remain the two in `0x0013fc00`,
   both passing only `a0`.
5. `0x00129660` is a descriptor-callback path that prepares `a1 = sp+0x20` and
   stores the return value into `[object+0x15c]+0x800`.
6. `0x00129660` reads descriptor field `+0x58`; ROPE's static `+0x58` field is
   zero, while `0x001d27a8` is at ROPE `+0x48`.

## What Is Probable

1. The staged callback machinery documented for `+0x40` should not be reused as
   an explanation for `0x001d27a8`.
2. `0x001d27a8` may be a callback slot whose intended caller is still missing,
   or it may rely on runtime state not visible in the direct static path.
3. `0x00129660` shows that the engine has two-argument descriptor initializers,
   but the ROPE record does not statically connect that mechanism to
   `0x001d27a8`.

## What Is Possible

1. Runtime data may patch or substitute descriptor fields, making a path
   different from the static ROPE dump.
2. A copied/staged path for `ROPE +0x48` may exist but evade the simple local
   scan because the callback pointer is transformed through memory or a table.
3. `0x001d27a8` may only be called in a context where `a1` is preserved by
   convention outside the local static evidence, though this remains weak.

## What Is Unknown

1. The actual runtime caller of `0x001d27a8`.
2. The actual runtime value of `a1` at `0x001d27a8` entry.
3. Whether static ROPE `+0x58 == 0` remains true after any runtime patching.
4. Whether there is another descriptor table or overlay-derived record that
   points to `0x001d27a8` in a field other than `+0x48`.

## What Is Discarded

1. Explaining `0x001d27a8` through the `node+0x1c` staged callback path.
   That dispatcher passes only `a0`.
2. Explaining `0x001d27a8` through `0x00129660` for the static ROPE record.
   That path reads `+0x58`, and ROPE `+0x58` is zero.
3. Treating all `entry+0x48` uses as callback uses. Several are flag
   manipulations.
4. Claiming the static producer of `[a1+0x30]` has been found.

## Next Minimum Test

Sem emulador:

1. Search for other descriptor constructor paths that call fields near
   `+0x48` or `+0x58` with `a1` prepared.
2. Search for additional raw references to `0x001d27a8` outside the main loaded
   segment only if overlay or external extracted code becomes available.
3. Compare neighboring records whose `+0x58` field is non-zero to understand
   the constructor-callback convention.

Com emulador:

1. Break at `0x001d27a8` and capture `ra`, `a0`, `a1`, `[a1+0x30]`, and the
   object pointer chain.
2. Break at `0x0013fc44` and `0x0013fcb8`; when target is `0x001d27a8`,
   capture `a1` to confirm whether it is meaningful or incidental.
3. Break at `0x00129660`; capture descriptor pointer `s0`, callback `v0`,
   and whether any runtime descriptor has `+0x58 == 0x001d27a8`.

## Conservative Verdict

Rev.044 does not find a static staged-callback path that explains
`0x001d27a8(a0, a1)`.

The known `+0x40 -> node+0x1c` staging path remains valid for callbacks like
`0x001d3a30`, but it cannot explain the initializer because its dispatcher only
passes `a0`. A separate descriptor constructor path at `0x00129660` does pass
`a1` and stores the returned payload to `+0x800`, but the static ROPE record has
`0x001d27a8` at `+0x48` and zero at `+0x58`.

The practical next step is runtime capture at `0x001d27a8`. Static analysis has
now isolated the gap precisely: the callback exists in data, and the initializer
uses `a1`, but no static path found so far proves who supplies that `a1`.
