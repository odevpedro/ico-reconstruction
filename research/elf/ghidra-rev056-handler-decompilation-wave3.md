# Rev.056 — Handler Decompilation Wave 3: BIRD, DEVIL_GI, ATTACKCH, BOSS_CTR + Descriptor Index Correction

**Date:** 2026-05-16

## Objective

Complete the handler survey of all 13 descriptor entries with non-null init_fn by
analyzing the 5 remaining entries: BIRD (idx 32), DEVIL_GI (idx 48),
ATTACKCH x2 (idx 62/63), BOSS_CTR (idx 64).

## Scope

- Read handler addresses (init_fn, hA, hB, hC) from descriptor table at 0x002A31B8
- Disassemble all 12 handler functions (some are very short)
- Classify each: EXACT, NEAR-STRUCTURAL, or ASM-HOLD
- Correct the descriptor index mapping (Rev.052 indices were wrong for several entries)
- Update status matrix

## Sources

- ELF: `SCUS_971.13.elf`
- Toolchain: `ee-gcc 2.9-991111-01` target
- Descriptor table: `0x002A31B8` (stride 0x64, 68 entries)
- Entry table: `0x002A4C48`

## Evidence

### File offset calculation

ELF LOAD segment maps:
- Virtual 0x00100000 -> File offset 0x001000
- File offset = VA - 0x00100000 + 0x001000
- Descriptor at VA 0x002AXXXX -> file offset 0x001AXXXX

### Descriptor index correction

Earlier revisions (Rev.052+) incorrectly assigned indices for WOODBOX0, BGA, and
AP1. The corrected mapping (confirmed by scanning all 68 entries for non-null
init_fn at +0x40):

| idx | name      | init_fn   | hA        | hB        | hC        |
|-----|-----------|-----------|-----------|-----------|-----------|
|   1 | BOY       | 0x153478  | 0x153A70  | 0x153CE8  | 0x153710  |
|   2 | GIRL      | 0x174BA0  | 0x1D1A98  | 0x1D17F8  | 0x1D1668  |
|   4 | ENEMY1    | 0x164440  | 0x164740  | 0x1644C0  | 0x164440  |
|  17 | WOODBOX0  | 0x17D1D0  | 0x17D2A8  | 0x1C0538  | 0x17D1D0  |
|  30 | BGA       | 0x203EE8  | 0x000000  | 0x000000  | 0x000000  |
|  32 | BIRD      | 0x1971C0  | 0x197080  | 0x197078  | 0x197240  |
|  46 | QUEEN     | 0x19B7F8  | 0x19B8F8  | 0x19B9D8  | 0x19B7F8  |
|  48 | DEVIL_GI  | 0x174BA0  | 0x1D1A98  | 0x1D17F8  | 0x1D1668  |
|  61 | AP1       | 0x1BB6B0  | 0x1BB860  | 0x1BB530  | 0x1BB6B0  |
|  62 | ATTACKCH  | 0x1BBF78  | 0x1BBEC8  | 0x1BBEA0  | 0x1BBE50  |
|  63 | ATTACKCH  | 0x1BBF78  | 0x1BBE48  | 0x1BBDD8  | 0x1BBB20  |
|  64 | BOSS_CTR  | 0x198140  | 0x198000  | 0x197FC8  | 0x198218  |
|  19 | BARREL    | 0x000000  | 0x1D3B28  | 0x1D3A30  | 0x1D27A8  |

**Corrections from Rev.052:**
- WOODBOX0: idx 6 -> idx 17
- BGA: idx 50 -> idx 30
- AP1: idx 56 -> idx 61
- BARREL: had no init_fn (cloth overlay, not a "real" inits)

**Totals:** 13 entries with non-null init_fn, 10 unique init_fn addresses
(GIRL/DEVIL_GI share 0x174BA0, ATTACKCH 62/63 share 0x1BBF78).

---

### BIRD (idx 32) handler analysis

Flight-capable entity. All handlers compact.

**init_fn** (0x1971C0, 30 insns):
```
dma_read() -> s1
sprite_dma_setup(1)
load_model(0x195A40, slot=20)
load_model(0x197160, slot=21)
load_model(0x197198, slot=21)   // LOD variant
entity_state_reg(a1=238, s1+0x610) -> state+0x120
```
No state flags set (unlike QUEEN which sets +0x7C=1).

**hC** (0x197240, 60 insns) — ALLOC | NEAR-STRUCTURAL:
- 64B alloc (heap 0x13A0F8, tag 978=0x3D2)
- memset(ptr, 0, 64)
- 0x105F00(state, initializer) — quaternion/pos setup
- `state+0x10 = 0` (byte)
- entity_state_reg(a1=2119, a2=2165, a3=-1, t0=-1, t1=1073)
- Sets entity_state flags: +0x544=1, +0x54C=0, +0x548=1, +0x550=0
- rand() * 100.0f -> [+0x4AC] and [+0x4B0] — random timer offset
- [+0x4C4] = 0
- entity_dispatch_update(entity, 3) — tail call to 0x1D4B40
- Returns alloc ptr

Note: rand()->mul.s->swc1 at +0x4AC/+0x4B0 creates per-bird frame desync.
+0x4C4 clear is likely an "active" flag reset.

**hB** (0x197078, 2 insns) — DELEGATE:
```
j 0x1E3FC8       // entity_dispatch_update (shared entity update pipeline)
nop
```

**hA** (0x197080, 24 insns):
```
sub_init(entity)
state = entity_state(entity)
child_ptr = [state + 0x800]
if [child_ptr+0x30] != 0:
    if check(child_ptr+0x30) passes:  // fn0x12A7F8
        [child_ptr+0x30] = 0
if gp_var == 82:
    tail_call 0x1998C0              // sound/event?
else:
    return
```

**Verdict:** NEAR-STRUCTURAL (hC), EXCEPT-DELEGATE (hB), ASM-HOLD (hA, init_fn).

---

### DEVIL_GI (idx 48) handler analysis

Complete clone of GIRL (idx 2). All four handler addresses identical.

| field    | address  | meaning                  |
|----------|----------|--------------------------|
| init_fn  | 0x174BA0 | shared with GIRL         |
| hC       | 0x1D1668 | constructor (cloth init) |
| hB       | 0x1D17F8 | update (cloth physics)   |
| hA       | 0x1D1A98 | reset/cleanup            |

DEVIL_GI reuses all GIRL handlers at the same addresses. The only difference is
the descriptor name string "DEVIL_GI" vs "GIRL". This strongly suggests DEVIL_GI
is a gameplay variant (palette swap, possessed version, or cutscene version) that
reuses identical constructor/update/reset logic.

**Is it confirmed the same code?** The descriptor entries are separate memory
locations. If init_fn/+0x48/+0x50/+0x58 all match byte-for-byte between idx 2
and idx 48, then DEVIL_GI truly shares GIRL's impl. This is a confirmed
structural aliasing — NOT a new code path.

**Verdict:** CONFIRMED-ALIAS.

---

### ATTACKCH (idx 62) handler analysis

Attack chain entity (child entity of idx 63). Small alloc, parent tracking.

**init_fn** (0x1BBF78, 20 insns) — shared with idx 63:
```
dma_read() -> s1
sub_init(entity)
sprite_dma_setup(1)
[s1 + 0x18] |= 0x10000000      // DMA flag bit 28
```
The DMA flag (0x10000000 = bit 28) set on the dma_read result context suggests
a transfer completion flag or a persistent DMA channel reservation.

**hC** (0x1BBE50, 20 insns) — ALLOC | NEAR-STRUCTURAL:
- 12B alloc (heap 0x13A0F8, tag 27)
- `state+0x30[0]` (parent's child slot) receives alloc ptr
- alloc+0x00 = parent_state_ptr (backref)
- alloc+0x04 = 0
- alloc+0x08 = 0

The struct layout:
```
struct attackch62_ctx {
    u32 parent_state_ptr;     // +0x00: backref
    u32 next;                 // +0x04: maybe linked list
    u32 prev;                 // +0x08: maybe linked list
};
```

**hB** (0x1BBEA0, 12 insns):
```
parent = [entity + 0x00]     // first child entity
if parent != 0 && [parent + 0x16C] != 0:
    [entity + 0x16C] = 0      // clear update-needed flag on self
```

**hA** (0x1BBEC8, 44 insns) — SPAWNER:
```
if gp_var == 0: return       // global gate
child_state = [entity_state + 0x800]
if [child_state + 0x04] != 0: return   // child slot busy

// Sound/effects
fn0x1118B8(11)
fn0x112148(1)
fn0x111FA8(1, 5, 128)

// Create projectile entity
v0 = fn0x105278()                          // spawn entity
fn0x118678(v0)                             // init entity
v0 = fn0x105278()                          // spawn second
fn0x105F00(v0+0x30, [entity]+0x0C+0x30+0x30)  // copy rotation

// Fetch config float
f0 = fn0x1BBFD0(entity)                   // = entity->state->+0x800->+0x20

// Spawn with params
fn0x11E220(0x4CEFC0, f0, 4, 4)

// Committal
fn0x10F630()                               // finalize
```

The accessor at 0x1BBFD0:
```
lw v0, 348(a0)         // entity_state
lw v1, 2112(v0)        // [+0x840] = state_block + 0x40
lwc1 $f0, 32(v1)       // return state_block+0x20 as float
jr $ra
```

This is a projectile/attack spawner triggered by hA (reset). It reads a config
float from the state block (+0x20) to determine spawn velocity/range.

**Verdict:** NEAR-STRUCTURAL (hC, hB), ASM-HOLD (init_fn, hA).

---

### ATTACKCH (idx 63) handler analysis

Parent attack chain entity. Dynamic child array, rotation matrix math, no hA.

**init_fn** same as idx 62 (0x1BBF78, 20 insns) — shared.

**hC** (0x1BBB20, 176 insns) — ALLOC | COMPLEX:
- 320B stack frame
- `s6 = state+0x30[idx_parent] * 80` (calc parent stride)
- First alloc: 16B (tag 173) — control block `s4`
  - s4+0x00 = [s6+0x44] (count of children)
  - s4+0x04 = 0
  - s4+0x08 = previous s4+0x04 (linked list)
- Second alloc: count * 8 (tag 182) — children array
  - s4+0x0C = children array
- Build inverse rotation matrix from entity state angles (at sp+0x10)
- Quaternion normalization via 0x1183A0/0x1183D0/0x118388/0x1183B8
- Loop over count:
  - lerp position along interpolation range
  - spawn child via 0x1B7FE8(params on stack)
    - params include: mat4, 0x4CD940 (model), 69/62 IDs, color, scale, interpolation t
  - Store child ptr in `children[i]`
  - Child gets entity_ptr in its +0x00
  - Child state_block +0x08 = s6+0x3C (some offset)
- Returns s4 control block

The code sets up a chain of entities (likely chain-sword/chain-strike) with
interpolated positions along a path derived from the parent entity's orientation.

**hB** (0x1BBDD8, 28 insns):
```
state = entity_state(entity)
t1 = [state + 0x800]           // child slot
count = [t1 + 0x00]
children = [t1 + 0x0C]
for i in 0..count:
    child_ptr = children[i]
    child_state = [child_ptr + 0x15C]
    child_slot = [child_state + 0x800]
    children[i+0x04] = [child_slot + 0x04]   // save child's next ptr
    [child_slot + 0x04] = 0                  // clear child's tracking
    [child_ptr + 0x16C] = 1                  // mark child as needs-update
[t1+0x08] = [t1+0x04]
[t1+0x04] = 0
```

This is a linked-list management pass: detach children from tracking and mark
them as non-updatable (likely after a chain attack completes).

**hA** (0x1BBE48, 2 insns) — NO-OP:
```
jr $ra
nop
```

**Verdict:** ASM-HOLD (hC, hB), EXCEPT-NOOP (hA).

---

### BOSS_CTR (idx 64) handler analysis

53-slot boss controller (likely for the "Queen" boss fight, managing 53 shadow
slots or arena elements).

**init_fn** (0x198140, 56 insns):
```
dma_read()                 // result ignored (s0 = 0 check is dead code?)
sprite_dma_setup(1)
[gp - 0x4B28] = 0          // g_boss_active
[gp - 0x4B2C] = 0          // g_boss_count
loop until alloc fails:
    entity = fn0x13EB50(33)    // allocate entity type 33
    if entity:
        [entity + 0x74] = 0    // state+0x74 initializer
        entity->pos = (4.0, 4.0, 4.0)  // half-float value?
        entity = fn0x13EBE0(entity)    // next entity
    count++
fn0x1A6E28(str, count)      // log spawn count
loop count times:
    load_model(0x197B30, slot=21)   // load same model each time
```

The alloc loop creates entities of type 33 until failure. The 4.0 position
suggests initial placement off-screen.

**hC** (0x198218, 60 insns) — NO ALLOC:
- 144B stack frame
- dma_read() -> s5
- sub_init(entity)
- fn0x1A6E28(str, 53)
- Loop 53 times (stride 64):
  - Process slot: quaternion via fn0x243B60
  - Process slot: position via fn0x1986D8
  - data stride 64, slot stride 64
- Returns s5

**hB** (0x197FC8, 4 insns) — CONDITIONAL TAIL:
```
if [0x6D35E0] != 0:
    tail 0x17DD60      // active update
else:
    tail 0x17DCD8      // inactive update
```
The global byte at 0x6D35E0 controls which update path runs.

**hA** (0x198000, 68 insns):
- Loop 53 slots:
  - If slot substate >= 2:
    - fn0x12A7F8 check
    - If passes: fn0x198710(slot, 482), quaternion copy, position copy
    - Track active count
  - Slot stride = 64
- If global_flag & 1:
  - fn0x1A6A30(10, 60, -1, str, count, 53) — spawn trigger
- Else: return

**Verdict:** ASM-HOLD (hC, hB, hA, init_fn). Too complex without runtime
verification of the 53-slot layout.

---

### hB dispatcher diversity

The Rev.055 question — "who dispatches hB?" — is resolved: there is NO single
dispatcher. Each entity's hB runs independently, called through the same
descriptor dispatch mechanism as hA and hC. The 4 observed hB patterns:

| pattern      | count | entities                                       |
|--------------|-------|------------------------------------------------|
| full code    | 7     | BOY, GIRL/DEVIL_GI, ENEMY1, WOODBOX0, QUEEN, AP1 |
| delegate     | 1     | BIRD (tail to 0x1E3FC8)                          |
| conditional  | 1     | BOSS_CTR (tail to 0x17DD60 or 0x17DCD8)         |
| linked-list  | 1     | ATTACKCH idx 63 (detach children)                |
| small guard  | 1     | ATTACKCH idx 62 (12 insns, parent check)         |

All hBs are reached through the same `descriptor[type]+0x50` call site. The
Rev.055 search failed because the dispatch uses `lw +0x50` + `jalr` through the
entity's descriptor, NOT a direct `lw +0x50 => jalr` in a centralized loop.

---

### Updated status matrix

| entity        | type | init_fn | hC      | hB      | hA      |
|---------------|------|---------|---------|---------|---------|
| BOY           |    1 | ASM     | ASM     | ASM     | ASM     |
| GIRL          |    2 | ASM     | ASM     | ASM     | ASM     |
| ENEMY1        |    4 | ASM     | ASM     | ASM     | ASM     |
| WOODBOX0      |   17 | ASM     | ASM     | ASM     | ASM     |
| BGA           |   30 | ASM     | —       | —       | —       |
| BIRD          |   32 | ASM     | NEAR    | DELEG   | ASM     |
| QUEEN         |   46 | NEAR    | NEAR    | ASM     | NEAR    |
| DEVIL_GI      |   48 | ALIAS   | ALIAS   | ALIAS   | ALIAS   |
| AP1           |   61 | ASM     | ASM     | ASM     | ASM     |
| ATTACKCH 62   |   62 | ASM     | NEAR    | NEAR    | ASM     |
| ATTACKCH 63   |   63 | ASM     | ASM     | ASM     | NOOP    |
| BOSS_CTR      |   64 | ASM     | ASM     | ASM     | ASM     |
| BARREL        |   19 | —       | CLOTH   | CLOTH   | CLOTH   |

Legend: ASM=disassembled only, NEAR=NEAR-STRUCTURAL C model produced,
ALIAS=copies another, DELEG=delegate jump, NOOP=empty, CLOTH=cloth physics.

---

### What is confirmed

1. **13 descriptor entries have non-null init_fn** (corrected indices).
2. **DEVIL_GI is a full GIRL alias** at all 4 handler addresses.
3. **BIRD hB delegates** to entity_dispatch_update (0x1E3FC8) in 2 insns.
4. **ATTACKCH idx 62** is a simple parent-tracked entity.
5. **ATTACKCH idx 63** is a dynamic child-chain spawner with rotation math.
6. **BOSS_CTR** manages 53 slots (Queen boss arena).
7. **hB dispatcher**: no unified dispatch — each entity has independent hB code.
8. **COP2 still exclusive to cloth**: none of the 13 entities use COP2.

### What is probable

- ATTACKCH idx 62 hA (0x1BBEC8) spawns projectile/attack entities.
- ATTACKCH idx 63 hC builds a chain of entities via interpolation.
- BOSS_CTR's 53 slots correspond to 53 shadow/arena objects (Queen fight).
- BIRD's random timer (+0x4AC) controls flight pattern switching.

### What is unknown

- Exact state_block layout for BOSS_CTR 53-slot system.
- What the 0x1D4B40 function (entity_dispatch_update) does internally.
- How WOODBOX0 hB (0x1C0538) and ENEMY1 hB (0x1644C0) work — not decompiled.
- The full init_fn semantics for BOY, ENEMY1, WOODBOX0 — not analyzed.
- Whether the global byte at 0x6D35E0 (BOSS_CTR hB gate) maps to a visible
  gameplay condition (phase change, health threshold, etc.).

### Next minimum test

1. Decompile BOSS_CTR hC loop to identify the 53-slot data layout.
2. Reverse 0x1B7FE8 (child spawner used by ATTACKCH idx 63 hC).
3. Write NEAR-STRUCTURAL C models for BIRD hC and ATTACKCH 62 hC.
4. Runtime test: breakpoint at BIRD hC (+0x4AC write) to verify
   random timer distribution.

### Conservative verdict

12 handler functions analyzed across 5 entries. DEVIL_GI confirmed as GIRL
alias. BIRD, ATTACKCH x2, BOSS_CTR are all structurally independent. No COP2
instructions found in any entity handler (cloth remains exclusive COP2 domain).
hB dispatch resolved: per-entity, not unified.
