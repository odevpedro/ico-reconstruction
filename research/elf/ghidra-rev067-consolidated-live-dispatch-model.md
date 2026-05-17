# Rev.067 — Consolidated live dispatch model: slot table, callbacks, callers, alternate implementation, and runtime probe plan

**Date:** 2026-05-17

## Objective

Consolidate all findings from Rev.066 and the three parallel follow-up analyses into a single integrated model:

1. slot table at `0x00282690` (17 entries, 14 unique callbacks)
2. callback target disassembly and structural classification
3. callers of `0x00166028` (list builder) and `0x00168650` (slot initializer)
4. alternate implementation at `0x00169F80` / `0x0016A058`
5. runtime probe plan for next session

## Scope

- Static disassembly only (Capstone MIPS64 little-endian from ELF)
- No runtime, no PCSX2, no DATA.DF, no assets
- No edits to `src/`

## Sources used

| Source | Use |
|---|---|
| `AGENTS.md` | project caution rules |
| `research/elf/ghidra-rev066-static-live-dispatch-callsite-map.md` | cold paths, GP slot map, dispatch model |
| `research/elf/ghidra-rev065-live-dispatch-followup-plan.md` | follow-up plan |
| `research/elf/ghidra-rev064-cold-paths-and-live-dispatch.md` | prior cold-path model |
| `docs/data-model.md` | data model to verify/refine |
| `docs/backlog.md` | current project state |
| `.local/extracted/SCUS_971.13.elf` | byte-level source |

## Slot table at `0x00282690`

17 entries, stride 0x10, read by dispatcher `0x00166E10` as `[0x00282690 + a1 * 0x10]`:

| Slot | VA | w0 | w1 | w2 | Callback target |
|---:|---:|---:|---:|---:|---:|
| 0 | `0x00282690` | 1 | 0 | 0 | `0x00168DA8` |
| 1 | `0x002826A0` | 1 | 0 | 0 | `0x00168ED0` |
| 2 | `0x002826B0` | 1 | 0 | 0 | `0x001692F0` |
| 3 | `0x002826C0` | 1 | 0 | 0 | `0x00169440` |
| 4 | `0x002826D0` | 1 | 1 | 0 | `0x00169020` |
| 5 | `0x002826E0` | 1 | 1 | 0 | `0x00169190` |
| 6 | `0x002826F0` | 1 | 0 | 0 | `0x001696C0` |
| 7 | `0x00282700` | 1 | 0 | 0 | `0x00169580` |
| 8 | `0x00282710` | 1 | 0 | 1 | `0x00168ED0` (same as 1) |
| 9 | `0x00282720` | 1 | 0 | 1 | `0x00169440` (same as 3) |
| 10 | `0x00282730` | 1 | 0 | 0 | `0x00169800` |
| 11 | `0x00282740` | 1 | 0 | 0 | `0x00169968` |
| 12 | `0x00282750` | 0 | 0 | 0 | `0x00169AA8` |
| 13 | `0x00282760` | 0 | 1 | 0 | `0x00169BD0` |
| 14 | `0x00282770` | 0 | 0 | 0 | `0x00169E58` |
| 15 | `0x00282780` | 0 | 0 | 0 | `0x00169D18` |
| 16 | `0x00282790` | 0 | 0 | 1 | `0x00169AA8` (same as 12) |

**Unique targets:** 14 (some slots share callbacks).

## Slot flag semantics

| Flag | Meaning |
|---|---|
| `w0 == 0` | **Group 2** — elem_size=0x70 struct, JAL `0x1667E0` (orientation/alignment check). Writes `[ctx+0x94]=struct*, [ctx+0x8C]=a1, [ctx+0x90]=a2, [ctx+0x88]=0` |
| `w0 == 1` | **Group 1** — elem_size=0x50 struct, JAL `0x166258` (position/rotation setup). Writes `[ctx+0x88]=struct*, [ctx+0x80]=a1, [ctx+0x84]=a2` |
| `w1 == 1` | **Duplicate guard** enabled — skips if `[ctx+0x74]==a1 AND [ctx+0x78]==a2 AND [ctx+0x7C]==struct*` (slots 4, 5, 13) |
| `w2 == 1` | Stored to `sp+0xCC`, passed through to callbacks. No visible behavioral difference in the callbacks themselves (slots 8, 9, 16 reuse callback code from slots 1, 3, 12) |

## Callback function structure

All 14 unique callbacks share an **identical structural skeleton**:

```asm
addiu   $sp, $sp, -0x90     ; 144-byte frame
sd      $ra, 0x80($sp)
sd      $s0-s7, various offsets
...
ld      $ra, 0x80($sp)
ld      $s0-s7, various offsets
jr      $ra
addiu   $sp, $sp, 0x90
```

These are **not independent callback functions**. They are **parametric variants** from two templates (Group 1 and Group 2), parameterized by mask values in the slot table.

Common internal loop (all callbacks):

```txt
for (a2 = 0; a2 < bound; a2++) {
    index = halfword_table[0x6AB080 + a2]  // runtime-populated BSS table
    if (index < 0) break
    ptr = table_base[index]                 // pointer table at struct+0x18 or +0x1C
    traverse linked entries (terminated by sign bit)
    for each entry:
        struct = array_base + index * elem_size
        if (field_at_+0x48 passes mask test)
            JAL sub_func(a0=ctx, a1=struct, a2=?, a3=?)
}
```

### Group 1 callbacks (w0=1, slots 0-11, elem_size=0x50, JAL 0x166258)

| Slot | VA | Size | Mask condition at +0x48 | a2 on JAL | Guard |
|---:|---:|---:|---|---|---|
| 0 | `0x00168DA8` | 296B | (none — always passes) | 0 | no |
| 1 | `0x00168ED0` | 336B | `0xF000` skip if set; sub-mask `0xF==1` skip | 0 | no |
| 2 | `0x001692F0` | 336B | same as 1 | **1** | no |
| 3 | `0x00169440` | 320B | `0xF000` skip if set | 0 | no |
| 4 | `0x00169020` | 368B | `0xF000` skip; sub-mask `0xF==1` skip; then **triplet guard** | 0 | **yes** |
| 5 | `0x00169190` | 352B | `0xF000` skip; then **triplet guard** | 0 | **yes** |
| 6 | `0x001696C0` | 320B | `0xC000 == 0x4000` to proceed | 0 | no |
| 7 | `0x00169580` | 320B | `0x3000 != 0` to proceed | 0 | no |
| 10 | `0x00169800` | 360B | `0x7000` skip; sub-mask `0xF==1` skip; then `a2 & 0xC000 == 0x8000` | 0 | no |
| 11 | `0x00169968` | 320B | `0xC000 == 0xC000` to proceed | 0 | no |

Writes on success: `[ctx+0x88]=struct*; [ctx+0x80]=a1; [ctx+0x84]=a2`

### Group 2 callbacks (w0=0, slots 12-16, elem_size=0x70, JAL 0x1667E0)

| Slot | VA | Size | Mask condition | a2 on JAL | Guard |
|---:|---:|---:|---|---|---|
| 12 | `0x00169AA8` | 296B | (none) | 0 | no |
| 13 | `0x00169BD0` | 328B | checks `[ctx+0x74/78/7C]` triplet first | 0 | **yes** |
| 14 | `0x00169E58` | 296B | (none) | **1** | no |
| 15 | `0x00169D18` | 320B | `[struct+0x60] & 0xF == 2` → skip | 0 | no |
| 16 | `0x00169AA8` | same as 12 | (same as 12) | 0 | no |

Writes on success: `[ctx+0x94]=struct*; [ctx+0x8C]=a1; [ctx+0x90]=a2; [ctx+0x88]=0`

### Called utility functions

| Function | Stack | Role |
|---|---|---|
| `0x166258` | 0xE0, saves `$f20-$f24` | Position/rotation setup: saves FPU state, reads matrices/positions from struct, calls `0x243B60` (matrix copy/transform), float vector ops on `+0x20`, `+0x40`, `+0x4C` |
| `0x1667E0` | 0x30, no FPU save | Orientation/alignment check: pure float arithmetic (dot products on `a1+0x40/44/48` x `a0+0x20/24/28/0/4/8`), compares against zero |
| `0x243B60` | (matrix lib) | Likely `ApplyMatrix` or similar PS2 SDK matrix math |

### Halfword table at `0x006AB080`

Referenced by all callbacks as a **runtime-populated BSS table** of uint16 entries, linking object IDs to pointer-table indices. This is the real object iteration mechanism — the callbacks do not iterate a hardcoded array but walk this runtime-built index.

## Callers of `0x00166028` (build_runtime_pointer_list)

`0x00166028` builds the pointer list at `0x006AAC80` with count in `gp-25896`.

### Direct JAL callers

| Caller VA | Context |
|---:|---|
| `0x00101ee8` | Main init chain: `JAL 0x1AA098 → JAL 0x166028 → JAL 0x103370 → JAL 0x104C80 → JAL 0x1AF190` |
| `0x001af974` | Entity init: loads fn ptr from `[base+0x154]` at `0x5F2FB8`, JALRs it if non-null, then falls through to `JAL 0x166028` |
| `0x001b7b50` | Subsystem init: last call before epilogue (restores `$ra, $fp, $s7..$s2`) |

### Indirect GP-slot callers (14 wrappers via gp-25856, slots 0-11)

All at `0x001683A8`–`0x00168620`, same structure: `addiu $sp,-0x10 / lw $v0, slot($gp) / sd $ra,($sp) / jalr $v0 / addiu $a1,$zero,N`.

Slot-to-a1 mapping:

| a1 | Stub VA |
|---:|---:|
| 0 | `0x001683B4` |
| 1 | `0x001683D4` |
| 2 | `0x001683F4` |
| 3 | `0x00168454` |
| 4 | `0x001684D4` |
| 5 | `0x00168474` |
| 6 | `0x00168414` |
| 7 | `0x00168434` |
| 8 | `0x001684F8` |
| 9 | `0x00168520` |
| 10 | `0x00168494` |
| 11 | `0x001684B4` |
| 1 (inline) | `0x0016860C` (compound, `a0=$s0` before JALR) |
| 1 (sp) | `0x0016A198` (compound, `a0=$sp` before JALR) |

## Callers of `0x00168650` (init_gp_slots)

`0x00168650` initializes/cold-paths the function pointer slots `gp-25856` and `gp-25852`.

### Direct callers

| Caller VA | Type | Context |
|---:|---:|:---|
| `0x001a3334` | **J** (tail call) | Last instruction of a function that zeros ~20 GP slots. Delay slot: `sw $zero, -0x6ED0($gp)`. Next function starts at `0x001A3340`. |

**Zero JAL callers** — `0x00168650` is only reached via the tail-call J or through indirect GP wrappers.

### Indirect GP-slot callers (6 wrappers via gp-25852, slots 12-16)

| a1 | Stub VA |
|---:|---:|
| 12 | `0x00168544` |
| 13 | `0x00168564` |
| 14 | `0x00168584` |
| 15 | `0x001685A4` |
| 16 | `0x001685C8` |
| 12 (dup) | `0x00168628` (compound) |

## Cold paths vs alternate implementation

### Cold paths (initial ELF image, `a0 == 0` at `0x00168650`)

| Slot | VA | Fields cleared | Default state copy | Tail-call |
|---:|---:|---:|---:|---:|
| gp-25856 | `0x00167230` | `+0xB0`, `+0x94`, `+0x88` | to `+0x80..0x87` | `j 0x00166E10` |
| gp-25852 | `0x00167258` | `+0x94` only | to `+0x8C..0x93` | `j 0x00166E10` |

Both are **leaf fragments**: no prologue/epilogue, no stack frame. They set up specific fields then tail-call `0x00166E10`.

### Alternate implementation (when `a0 != 0` at `0x00168650`)

| Slot | VA | Frame | Diff from cold |
|---:|---:|---:|---|
| gp-25856 → alt | `0x00169F80` | 80B, saves `$ra, $s0, $s1` | JALs cold path A first, then does extra initialization |
| gp-25852 → alt | `0x0016A058` | 80B, saves `$ra, $s0, $s1` | JALs cold path B first, then does extra initialization |

**Both alternates are self-contained functions** with proper prologue/epilogue. They:
1. JAL the cold path (which Js to `0x00166E10`, which runs and returns via epilogue back to the alternate)
2. Load 32 bytes of constants from absolute addresses (different per slot)
3. Call 6 additional sub-functions
4. Call `0x1D4A58` **twice** with `t0=0` then `t0=-1` (first-pass/second-pass transform hierarchy pattern)

Constant blocks:

| Alternate | Block base | Used words |
|---:|---:|---|
| `0x00169F80` | `0x55F280`–`0x55F298` (4 qwords) | 442, 4, 1430000, 2, 388 |
| `0x0016A058` | `0x55F260`–`0x55F278` (shifted -0x20) | (values TBD) |

The alternates are NOT cold paths — they are a **full initialization sequence** that includes the cold-path setup plus additional transform/matrix work.

## Consolidated dispatch model

```txt
Caller (direct JAL or indirect GP-slot wrapper)
  │
  │ a0 = context pointer
  │ a1 = slot index (0..16)
  ▼
┌─────────────────────────────────────────────────────────────────────┐
│ GP slot check (0x00166E10 entry via cold paths)                    │
│                                                                     │
│ gp-25856 (= 0x00167230 or 0x00169F80) — slots 0-11                │
│ gp-25852 (= 0x00167258 or 0x0016A058) — slots 12-16               │
│                                                                     │
│ Cold paths: clear fields, copy default state, j → 0x00166E10       │
│ Alternates: JAL cold path, then extra init (transform/matrix)      │
└─────────────────────────────────────────────────────────────────────┘
  │
  ▼
┌─────────────────────────────────────────────────────────────────────┐
│ Main body 0x00166E10                                                │
│                                                                     │
│ 1. Read slot entry: 0x00282690 + a1 * 0x10                         │
│ 2. Load callback target from entry[+0x0C]                           │
│ 3. Load flags from entry[+0x04] (w0, w1, w2)                       │
│ 4. Call 0x243B60 x3 for matrix init                                 │
│ 5. Iterate pointer list at 0x006AAC80 (count in gp-25896)          │
│ 6. For each matching entry:                                         │
│    ├─ Filter by [ctx+0x74], [ctx+0x78], [ctx+0x7C] (if w1==1)     │
│    └─ Dispatch callback: 0x00167020(v1, a0=ctx, a1=entry, a2=idx)  │
│                                                                     │
│ Callback (slot entry[+0x0C]):                                       │
│   └─ Iterates halfword table at 0x6AB080 (BSS)                     │
│      └─ For each valid object ID:                                   │
│         ├─ Compute struct from table_base + index * elem_size       │
│         ├─ Check field at +0x48 against slot-specific mask          │
│         └─ JAL 0x166258 (Group 1, position) OR                     │
│            JAL 0x1667E0 (Group 2, orientation)                     │
│            └─ a0=ctx, a1=struct*, a2=loop_idx, a3=mode             │
└─────────────────────────────────────────────────────────────────────┘
```

## Correction: `desc_array_0x006AAC00`

The label `desc_array_0x006AAC00` is inaccurate after Rev.066-067:

| Item | Old label | Corrected |
|---|---|---|
| Pointer list base | `0x006AAC00` | **`0x006AAC80`** (directly referenced by instructions in `0x00166E10`) |
| List count | — | **`gp-25896`** (`0x006323C8`) |
| List element | full descriptor struct | 4-byte **pointer** (index << 2) |
| Max entries | — | `< 0x100` (confirmed by `slti` at `0x001660C8`) |

`0x006AAC00` may be a region-level `.bss` label containing `0x006AAC80`, but no instruction directly references `0x006AAC00`.

## What is confirmed

1. Slot table at `0x00282690` has exactly 17 entries (slots 0-16), stride 0x10, with 14 unique callback targets.
2. All 14 callbacks are parametric variants of two templates (Group 1: position, Group 2: orientation).
3. All callbacks iterate a runtime-populated halfword table at `0x6AB080` (BSS).
4. `0x00166028` has 3 direct JAL callers + 14 GP-slot indirect callers (slots 0-11).
5. `0x00168650` has 0 direct JAL callers, 1 tail-call J (at `0x001A3334`), and 6 GP-slot indirect callers (slots 12-16).
6. The alternate implementation (`0x00169F80`/`0x0016A058`) is a **full init sequence**: JAL cold path → extra transform/matrix setup via `0x1D4A58` (two-pass pattern).
7. Cold paths are leaf fragments (no stack frame); alternates are self-contained functions (80B frame).
8. `0x006AAC80` is the pointer list base, not `0x006AAC00`.

## What is probable

1. The slot index (`a1` values 0-16) maps to 17 distinct scene initialization phases or object groups.
2. The alternate implementation is activated during a second initialization pass or for specific scene types.
3. The halfword table at `0x6AB080` is populated during scene load by `0x00166028` or a related function.
4. `0x1D4A58` is a transform hierarchy builder (called twice with `t0=0` then `t0=-1`).

## What is possible

1. The flags `w0/w1/w2` may have a more nuanced interpretation when studied in the dispatcher `0x00166E10` context.
2. The alternate constant blocks may differ in values that control specific behavior per slot.
3. The 3 direct JAL callers of `0x00166028` may represent different call sites (scene init, entity init, subsystem init).

## What is unknown

1. Which runtime conditions cause `a0 != 0` at `0x00168650` (activating the alternate implementation).
2. How the halfword table at `0x6AB080` is populated (caller of `0x00166028`? separate init chain?).
3. The semantic meaning of each slot (what does slot 0 vs slot 12 actually represent in gameplay terms).
4. Whether the alternate `0x1D4A58` calls are matrix hierarchy operations for cloth/rope/scene objects.
5. The concrete constant values at the alternate blocks (`0x55F260`–`0x55F298`).

## What is discarded

| Discarded idea | Reason |
|---|---|
| Callback targets are independent functions | All 14 are parametric variants of 2 templates |
| `desc_array_0x006AAC00` is the correct label | The list base is `0x006AAC80` |
| `0x00168650` has direct callers | Only reached via tail-call J or GP-slot wrappers |
| Alternate implementation is another pair of cold paths | Alternates are full functions (80B frame, 6+ callees, proper epilogue) with extra init |
| Cold paths are standalone functions | They are leaf fragments (no stack, no epilogue, terminal J to shared body) |

## Runtime probe plan

### Breakpoint targets by priority

| Priority | PC | What to capture |
|---:|---:|---|
| 1 | `0x00167230` | Cold path A hit. Capture `a0`, `gp`, and current slot value at `gp-25856`. Confirm if cold or alternate is installed. |
| 1 | `0x00167258` | Cold path B hit. Same capture. |
| 2 | `0x00166E10` | Main body entry. Capture `a0` (ctx), `a1` (slot index), `s0-s7` if possible. |
| 3 | `0x00167020` | Callback dispatch. Capture `v1` (callback target), `a0` (ctx), `a1` (entry), `a2` (index). |
| 3 | `0x001683B4` | Wrapper for slot 0 (most basic, no guard). Capture `a0`, `a1`, target from GP slot. |
| 4 | `0x00169F80` | Alternate implementation for slot A. Capture if hit during gameplay. |
| 4 | `0x0016A058` | Alternate implementation for slot B. Capture if hit. |
| 5 | `0x00168650` | Slot initializer. Capture `a0` (mode switch). Track if both modes appear in a session. |
| 5 | `0x00166028` | List builder. Capture to see what populates the halfword table at `0x6AB080`. |

### Capture fields per breakpoint

| PC | Minimum capture |
|---:|---|
| `0x00167230`/`0x00167258` | `a0` (ctx ptr), `gp`, `gp-25856` value, `gp-25852` value |
| `0x00166E10` | `a0`, `a1` (slot index), `ra` (caller) |
| `0x00167020` | `v1` (callback target), `a0`, `a1`, `a2`, `ra` |
| `0x001683B4` | `a0`, `a1`, target function from GP slot |
| `0x00169F80`/`0x0016A058` | `a0`, `ra` |
| `0x00168650` | `a0`, `gp-25856` old value, `gp-25856` new value |
| `0x00166028` | `a0`, `gp`, `gp-25896` (count), first few entries of `0x006AAC80` |

### Questions to answer

1. Which slots (a1 values) appear during normal gameplay vs cutscenes vs menus?
2. Are the cold path slots ever swapped to the alternate implementation during a session?
3. Does slot 0 (`0x00168DA8`, Group 1, no mask, no guard) fire most frequently?
4. Which Group 2 slots (12-16) fire, and what extra conditions do they check?
5. Does the halfword table at `0x6AB080` contain entity/object type indices?
6. What is the real callback distribution at `0x00167020`?

## Conservative verdict

The live dispatch model is now more complete and internally consistent than any prior model. The slot table at `0x00282690` is confirmed as a 17-entry, stride-0x10 configuration table with 14 unique callback targets that are parametric variants of two templates (position and orientation). The callbacks iterate a runtime-populated halfword table at `0x6AB080`, not a hardcoded array. The cold paths (`0x00167230`/`0x00167258`) are confirmed as leaf fragments that tail-call `0x00166E10`. The alternate implementation (`0x00169F80`/`0x0016A058`) is confirmed as a full init sequence with extra transform/matrix work. The pointer list for the dispatcher loop is confirmed at `0x006AAC80` (not `0x006AAC00`).

The next step is runtime validation: capture hits at the cold paths, main body, and dispatch point to confirm which slots fire during normal gameplay and whether the alternate implementation is ever reached.
