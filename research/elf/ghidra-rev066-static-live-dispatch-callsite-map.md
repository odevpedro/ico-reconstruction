# Rev.066 — Static live dispatch callsite map

**Date:** 2026-05-17

## Objective

Perform the static-only follow-up requested after Rev.065:

1. revalidate the cold paths at byte/instruction level;
2. map the GP slots that feed the cold paths;
3. map static `JALR` callsites for those GP slots;
4. annotate the main body `0x00166E10` into operational blocks;
5. confirm the callback dispatch point at `0x00167020`;
6. audit the data regions used by the body without using the emulator;
7. consolidate the result as a new research note.

No runtime was performed in this revision.

## Scope

Included:

- local ELF byte reads from `.local/extracted/SCUS_971.13.elf`;
- Capstone MIPS64 little-endian disassembly;
- direct scan for `j`/`jal` targets, GP-relative loads, pointer literals, and key data regions;
- static audit of `0x00166E10`, `0x00167230`, `0x00167258`, `0x00167270`, `0x00167020`, `gp-25904`, `gp-25856`, `gp-25852`, and `gp-25848`.

Excluded:

- PCSX2/runtime validation;
- DATA.DF, `.gcm`, assets, TM2, visual/UI trails;
- compiler/decomp.me reopening;
- any edits to `src/`.

## Sources used

| Source | Use |
|---|---|
| `AGENTS.md` | project caution rules and documentation discipline |
| `research/elf/ghidra-rev064-cold-paths-and-live-dispatch.md` | prior cold-path and live-dispatch model |
| `research/elf/ghidra-rev065-live-dispatch-followup-plan.md` | follow-up plan and capture targets |
| `docs/data-model.md` | current data-model labels to verify/refine |
| `docs/backlog.md` | current operational project state |
| `research/elf/ghidra-rev062-gp-relative-data-map.md` | GP base and GP-relative context |
| `.local/extracted/SCUS_971.13.elf` | byte-level source |

## Evidence used

| Evidence | Result |
|---|---|
| Section parse | `.text = 0x00100000..0x0026F5D4`, `.data = 0x00274700..0x005536B8`, `.sdata = 0x00631900..0x00633BC6`, `.bss` includes `0x006AAC00`/`0x006AAC80` |
| Direct `j` scan to `0x00166E10` | exactly two: `0x00167250`, `0x00167270` |
| Direct `jal` scan to `0x00166E10` | zero |
| 32-bit pointer scan for `0x00166E10` | zero |
| 32-bit pointer scan for cold entries | `0x00167230` and `0x00167258` each appear once in `.sdata`; `0x00167270` appears zero times |
| GP slot bytes | `gp-25856 = 0x00167230`, `gp-25852 = 0x00167258`, `gp-25848 = 0x00000000` in initial ELF image |
| GP slot load scan | 14 call wrappers for `gp-25856`, 6 call wrappers for `gp-25852`, no call wrapper for `gp-25848` |

## Byte-level and instruction-level findings

### Cold path A: `0x00167230`

Bytes:

```txt
2D 10 80 00 B0 00 40 AC 94 00 40 AC 88 00 40 AC
D7 9A 83 6B D0 9A 83 6F 87 00 43 B0 80 00 43 B4
84 9B 05 08 00 00 00 00
```

Instructions:

```asm
0x00167230: move     $v0, $a0
0x00167234: sw       $zero, 0xb0($v0)
0x00167238: sw       $zero, 0x94($v0)
0x0016723C: sw       $zero, 0x88($v0)
0x00167240: ldl      $v1, -0x6529($gp)
0x00167244: ldr      $v1, -0x6530($gp)
0x00167248: sdl      $v1, 0x87($v0)
0x0016724C: sdr      $v1, 0x80($v0)
0x00167250: j        0x00166E10
0x00167254: nop
```

Confirmed: cold path A clears `context+0xB0`, `context+0x94`, `context+0x88`, copies the 8-byte default state from `gp-25904`, and jumps into `0x00166E10`.

### Cold path B: `0x00167258`

Bytes:

```txt
2D 10 80 00 94 00 40 AC D7 9A 83 6B D0 9A 83 6F
93 00 43 B0 8C 00 43 B4 84 9B 05 08 00 00 00 00
```

Instructions:

```asm
0x00167258: move     $v0, $a0
0x0016725C: sw       $zero, 0x94($v0)
0x00167260: ldl      $v1, -0x6529($gp)
0x00167264: ldr      $v1, -0x6530($gp)
0x00167268: sdl      $v1, 0x93($v0)
0x0016726C: sdr      $v1, 0x8c($v0)
0x00167270: j        0x00166E10
0x00167274: nop
```

Confirmed: cold path B clears `context+0x94`, copies the 8-byte default state into the `+0x8C..+0x93` range, and jumps into `0x00166E10`.

### `0x00167270` correction

`0x00167270` is an instruction inside cold path B:

```asm
0x00167270: j        0x00166E10
0x00167274: nop
```

Static evidence does **not** support treating `0x00167270` as an independently referenced third cold path in the initial ELF image:

| Check | Result |
|---|---|
| Pointer scan for `0x00167270` | 0 occurrences |
| `gp-25848` initial value | `0x00000000` |
| `gp-25848` call wrappers | 0 |
| Instruction at `0x00167278` | starts a different function/prologue-like body |

Conservative correction: `0x00167270` remains a useful breakpoint if one wants to observe the final jump from cold path B, but it should not currently be described as a third cold entry point. If runtime or another static source later proves a caller jumps directly to `0x00167270`, this point can be reopened.

## GP slot map

Using `GP = 0x006388F0`:

| GP slot | VA | Initial u32 | Static role |
|---:|---:|---:|---|
| `gp-25904` | `0x006323C0` | `0x00000000` plus next word `0xFFFFFFFF` | 8-byte default state |
| `gp-25896` | `0x006323C8` | `0x00000000` | count for `.bss` context pointer list |
| `gp-25888` | `0x006323D0` | `0x00000065` | nearby constant, not resolved here |
| `gp-25856` | `0x006323F0` | `0x00167230` | function pointer slot for cold path A |
| `gp-25852` | `0x006323F4` | `0x00167258` | function pointer slot for cold path B |
| `gp-25848` | `0x006323F8` | `0x00000000` | counter/state used by the following function, not a cold-path pointer under current evidence |

The initializer at `0x00168650` writes the two function pointer slots:

```asm
0x00168650: lui      $v0, 0x16
0x00168654: lui      $v1, 0x16
0x00168658: addiu    $v0, $v0, 0x7230
0x0016865C: addiu    $v1, $v1, 0x7258
0x00168660: sw       $v0, -0x6500($gp)   ; gp-25856 = 0x00167230
0x00168664: beqz     $a0, 0x168684
0x00168668: sw       $v1, -0x64fc($gp)   ; gp-25852 = 0x00167258
0x0016866C: lui      $v0, 0x17
0x00168670: lui      $v1, 0x17
0x00168674: addiu    $v0, $v0, -0x6080   ; 0x00169F80
0x00168678: addiu    $v1, $v1, -0x5fa8   ; 0x0016A058
0x0016867C: sw       $v0, -0x6500($gp)
0x00168680: sw       $v1, -0x64fc($gp)
```

Confirmed: the cold path slots are mutable. With `a0 == 0`, the slots remain `0x00167230` and `0x00167258`; with `a0 != 0`, they are overwritten by `0x00169F80` and `0x0016A058`. Runtime needs to capture the current slot values before assuming which implementation is active.

## Static `JALR` callsite map

All statically detected direct wrappers that load `gp-25856` or `gp-25852` and call it by `JALR`:

| Load PC | GP slot | `JALR` PC | Delay-slot argument | Notes |
|---:|---:|---:|---:|---|
| `0x001683AC` | `gp-25856` | `0x001683B4` | `a1 = 0` | wrapper |
| `0x001683CC` | `gp-25856` | `0x001683D4` | `a1 = 1` | wrapper |
| `0x001683EC` | `gp-25856` | `0x001683F4` | `a1 = 2` | wrapper |
| `0x0016840C` | `gp-25856` | `0x00168414` | `a1 = 6` | wrapper |
| `0x0016842C` | `gp-25856` | `0x00168434` | `a1 = 7` | wrapper |
| `0x0016844C` | `gp-25856` | `0x00168454` | `a1 = 3` | wrapper |
| `0x0016846C` | `gp-25856` | `0x00168474` | `a1 = 5` | wrapper |
| `0x0016848C` | `gp-25856` | `0x00168494` | `a1 = 10` | wrapper |
| `0x001684AC` | `gp-25856` | `0x001684B4` | `a1 = 11` | wrapper |
| `0x001684CC` | `gp-25856` | `0x001684D4` | `a1 = 4` | wrapper |
| `0x001684F0` | `gp-25856` | `0x001684F8` | `a1 = 8` | also stores incoming `a1` to `gp-19388` first |
| `0x00168518` | `gp-25856` | `0x00168520` | `a1 = 9` | also stores incoming `a1` to `gp-19388` first |
| `0x0016853C` | `gp-25852` | `0x00168544` | `a1 = 12` | wrapper |
| `0x0016855C` | `gp-25852` | `0x00168564` | `a1 = 13` | wrapper |
| `0x0016857C` | `gp-25852` | `0x00168584` | `a1 = 14` | wrapper |
| `0x0016859C` | `gp-25852` | `0x001685A4` | `a1 = 15` | wrapper |
| `0x001685C0` | `gp-25852` | `0x001685C8` | `a1 = 16` | also stores incoming `a1` to `gp-19388` first |
| `0x00168604` | `gp-25856` | `0x0016860C` | `a1 = 1` | compound wrapper with stream setup |
| `0x00168620` | `gp-25852` | `0x00168628` | `a1 = 12` | compound wrapper with stream setup |
| `0x0016A190` | `gp-25856` | `0x0016A198` | `a1 = 1` | larger wrapper using stack as context |

Counts:

| GP slot | Count |
|---:|---:|
| `gp-25856` | 14 |
| `gp-25852` | 6 |
| `gp-25848` | 0 |

This reproduces the Rev.064 call-count observation for the first two slots and rejects the tentative third-slot interpretation for `gp-25848`.

## Main body `0x00166E10` block map

High-level static block map:

| Range | Role |
|---|---|
| `0x00166E10..0x00166E8C` | prologue, `a1 * 0x10`, table slot selection from `0x00282690`, save callback from slot `+0x0C` to `sp+0xC0` |
| `0x00166E90..0x00166EB4` | stream/setup calls to `0x00243B60` |
| `0x00166EB8..0x00167088` | loop over `.bss` context pointer list at `0x006AAC80`, bounded by `gp-25896` |
| `0x00166ED0..0x00166F40` | candidate descriptor/context filtering through `[s2+0x15C]`, `+0x70`, `+0x74`, `+0x78`, `+0x80` |
| `0x00166F50..0x00167060` | per-subentry loop, transform/setup helpers, callback dispatch at `0x00167020` |
| `0x00167090..0x001671E4` | fallback/alternate context update path using selected slot data, context fields `+0x80`, `+0x84`, `+0x88`, `+0x8C`, `+0x90`, `+0x94`, `+0x98`, `+0xA0` |
| `0x001671E8..0x0016722C` | final stream cleanup calls and epilogue |

Direct reachability findings:

| Search | Result |
|---|---|
| `j 0x00166E10` | `0x00167250`, `0x00167270` |
| `jal 0x00166E10` | none |
| literal pointer `0x00166E10` | none |

This explains why `0x00166E10` does not appear as a normal static callee even though it is the shared body for the cold entries.

## Callback dispatch at `0x00167020`

Instruction-level confirmation:

```asm
0x00167014: lw       $v1, 0xc0($sp)
0x00167018: move     $a0, $s1
0x0016701C: move     $a1, $s2
0x00167020: jalr     $v1
0x00167024: move     $a2, $s0
0x00167028: beqz     $v0, 0x167048
```

The callback target is loaded earlier:

```asm
0x00166E14: lui      $v1, 0x28
0x00166E1C: sll      $a1, $a1, 4
0x00166E24: addiu    $v0, $v1, 0x2690
0x00166E2C: addu     $a0, $v0, $a1
...
0x00166E64: lw       $v1, 0xc($v1)
0x00166E68: sw       $v1, 0xc0($sp)
```

Confirmed static calling convention at the callback point:

| Register | Value at `0x00167020` |
|---|---|
| `v1` | callback target from `[0x00282690 + slot_index * 0x10 + 0x0C]` |
| `a0` | `s1`, original context pointer from cold path/wrapper |
| `a1` | `s2`, current item pointer from the `.bss` context pointer list |
| `a2` | `s0`, subentry/loop index |
| return `v0` | controls whether fallback copy path at `0x00167048` runs |

Do not assign final semantics to the callback yet. Static evidence confirms the call shape, not the meaning of each callback target.

## Data-region audit

### Slot table at `0x00282690`

`0x00166E10` indexes this table by `a1 * 0x10`. The first 17 slots visible in `.data` match the wrapper `a1` values from `0..16`.

| Slot | VA | Word0 | Word1 | Word2 | Callback word3 |
|---:|---:|---:|---:|---:|---:|
| 0 | `0x00282690` | `1` | `0` | `0` | `0x00168DA8` |
| 1 | `0x002826A0` | `1` | `0` | `0` | `0x00168ED0` |
| 2 | `0x002826B0` | `1` | `0` | `0` | `0x001692F0` |
| 3 | `0x002826C0` | `1` | `0` | `0` | `0x00169440` |
| 4 | `0x002826D0` | `1` | `1` | `0` | `0x00169020` |
| 5 | `0x002826E0` | `1` | `1` | `0` | `0x00169190` |
| 6 | `0x002826F0` | `1` | `0` | `0` | `0x001696C0` |
| 7 | `0x00282700` | `1` | `0` | `0` | `0x00169580` |
| 8 | `0x00282710` | `1` | `0` | `1` | `0x00168ED0` |
| 9 | `0x00282720` | `1` | `0` | `1` | `0x00169440` |
| 10 | `0x00282730` | `1` | `0` | `0` | `0x00169800` |
| 11 | `0x00282740` | `1` | `0` | `0` | `0x00169968` |
| 12 | `0x00282750` | `0` | `0` | `0` | `0x00169AA8` |
| 13 | `0x00282760` | `0` | `1` | `0` | `0x00169BD0` |
| 14 | `0x00282770` | `0` | `0` | `0` | `0x00169E58` |
| 15 | `0x00282780` | `0` | `0` | `0` | `0x00169D18` |
| 16 | `0x00282790` | `0` | `0` | `1` | `0x00169AA8` |

This table is currently better described as a 17-entry slot/callback table than as `desc_array_0x006AAC00`.

### Runtime pointer list at `0x006AAC80`

Static instructions reference `0x006AAC80`, not `0x006AAC00`, in the live body:

```asm
0x00166EB8: lui      $a1, 0x6b
0x00166EC8: lw       $s2, -0x5380($a1)   ; 0x006AAC80
...
0x0016706C: lui      $a1, 0x6b
0x00167070: addiu    $a0, $a1, -0x5380   ; 0x006AAC80
```

The list is populated by `0x00166028`:

```asm
0x0016602C: sw       $zero, -0x6528($gp) ; gp-25896 count = 0
0x00166050: addiu    $s0, $v0, -0x5380   ; s0 = 0x006AAC80
...
0x00166098: lw       $v0, -0x6528($gp)
0x0016609C: sll      $v1, $v0, 2
0x001660A0: addiu    $v0, $v0, 1
0x001660A4: addu     $v1, $v1, $s0
0x001660A8: sw       $v0, -0x6528($gp)
0x001660AC: sw       $a0, ($v1)
```

Confirmed static model:

| Element | Address | Evidence |
|---|---:|---|
| pointer list base | `0x006AAC80` | direct `lui/addiu` and `lw -0x5380` references |
| count | `gp-25896` / `0x006323C8` | reset and increment in `0x00166028`, read by `0x00166E10` |
| list element size | 4 bytes | `index << 2` |
| max check | `< 0x100` | `0x001660C8: slti v0, v0, 0x100` |

`0x006AAC00` lies in the same `.bss` region, but this static pass found no direct reference to exactly `0x006AAC00`. Treat existing `desc_array_0x006AAC00` wording as a region-level or stale label until runtime/data-model review refines it.

## What is confirmed

1. `0x00167230` and `0x00167258` are compiler-split cold paths that rejoin `0x00166E10`.
2. `0x00167270` is the `j 0x00166E10` instruction at the end of cold path B, not an independently referenced third cold entry under current static evidence.
3. `gp-25856` and `gp-25852` are the two active cold-path function pointer slots in the initial ELF image.
4. Static wrapper counts are exactly 14 for `gp-25856` and 6 for `gp-25852`.
5. `0x00166E10` has no direct `JAL` caller and no literal pointer in the ELF; it is reached by direct `J` from cold paths.
6. `0x00167020` is a `JALR` callback dispatch using a target selected from `0x00282690 + slot * 0x10 + 0x0C`.
7. The live body statically references a `.bss` pointer list at `0x006AAC80`, with count in `gp-25896`.

## What is probable

1. The wrapper `a1` values `0..16` select the 17 visible entries in the slot table at `0x00282690`.
2. `0x00166028` builds the runtime pointer list consumed by `0x00166E10`.
3. The mutable GP slots at `0x00168650` support two implementations: cold-path implementation (`0x00167230`/`0x00167258`) and alternate implementation (`0x00169F80`/`0x0016A058`).

## What is possible

1. `0x006AAC00` may still be a broader region label for the `.bss` structure that contains the live list at `0x006AAC80`.
2. A non-static/runtime caller could jump directly to `0x00167270`, but there is no current static evidence for that.
3. The alternate GP slot targets may be selected by a runtime mode controlled by the argument to `0x00168650`.

## What is unknown

1. Which mode calls `0x00168650` with `a0 != 0`, if any, in normal runtime.
2. The final semantic names for the 17 slot callbacks in `0x00282690`.
3. Whether every pointer stored in `0x006AAC80` should be called a descriptor, context, entity, or a narrower structure.
4. Whether the old `0x001B76F8` path is ever reached in a runtime session outside current evidence.

## What is discarded

| Discarded idea | Reason |
|---|---|
| `0x00167230` is a standalone function | it is a cold path that jumps to `0x00166E10` |
| `0x00167270` is confirmed third cold path C | no pointer, no GP slot, no wrapper; it is the terminal jump of cold path B |
| `gp-25848` is a function pointer to `0x00167270` | initial value is zero and observed use begins the next function as a counter/state slot |
| `0x00166E10` should have normal direct static callers | it is reached by cold-path `j`, not external `jal` |

## Next minimum test

Without emulator:

1. statically map the callback targets from `0x00282690` entries `0..16`;
2. disassemble `0x00168DA8`, `0x00168ED0`, `0x00169020`, `0x00169190`, `0x001692F0`, `0x00169440`, `0x00169580`, `0x001696C0`, `0x00169800`, `0x00169968`, `0x00169AA8`, `0x00169BD0`, `0x00169D18`, `0x00169E58`;
3. statically map callers of `0x00166028` and `0x00168650`.

With emulator later:

1. capture current values of `gp-25856` and `gp-25852` before wrapper calls;
2. capture hits at `0x001683B4`/representative wrapper, `0x00167230`, `0x00167258`, `0x00166E10`, and `0x00167020`;
3. capture `gp-25896`, the pointer list at `0x006AAC80`, and callback target selected from `0x00282690`.

## Conservative verdict

The static-only follow-up strengthens the Rev.064/Rev.065 correction but also refines it. The live dispatch candidate is not a three-cold-path model under current static evidence. It is a two-slot cold-path model:

```txt
gp-25856 -> 0x00167230 -> 0x00166E10
gp-25852 -> 0x00167258 -> 0x00166E10
```

`0x00167270` is confirmed as the final jump instruction of cold path B, not as an independently referenced third entry. The true static callsite surface is the wrapper cluster that calls `gp-25856` 14 times and `gp-25852` 6 times with slot indices `0..16`. Inside `0x00166E10`, the callback dispatch at `0x00167020` is confirmed, and the callback target comes from the slot table at `0x00282690`, while the runtime pointer list consumed by the loop is at `0x006AAC80` with count `gp-25896`.

Runtime is still required before calling this a confirmed live path, but the next runtime probe can now be narrower and more precise.
