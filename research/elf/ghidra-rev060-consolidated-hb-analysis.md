# Rev.060 — Consolidated hB Analysis + Callback Pool Correction

**Date:** 2026-05-16

## Objective

Complete all remaining items from Rev.059: (1) allocator identification, (2) runtime plan, (3) 0x1B7B88 analysis, (4) ASM-HOLD C models, (5) entity state machine analysis.

---

## Item 1: 0x1A6E28 is a print/assert stub (NOT allocator)

### Previous assumption

Rev.059 claimed 0x1A6E28 is an "allocator called from 0x13F3F0."

### Current correction

0x1A6E28 (32 bytes) is a **disabled print/assert stub**:
```
0x1A6E28: addiu sp, sp, -128    // save 128B
          sd a1, 72(sp)          // save arguments
          sd a2, 80(sp)
          sd a3, 88(sp)
          sd t0, 96(sp)
          sd t1, 104(sp)
          sd t2, 112(sp)
          sd t3, 120(sp)
          jr ra                 // return immediately (assert disabled)
          addiu sp, sp, 128     // delay slot: restore stack
```

The function right after at 0x1A6E50 (240B) is the REAL `printf`-style formatter (parses `%d`, `%x`, `%f`, `%s`, calls 0x264DF8 for string conversion). Both are compiled-in but disabled in retail — 0x1A6E28 is the stripped assert, 0x1A6E50 is the formatted print.

**0x13F3F0 calls 0x1A6E28 four times** for assert/error messages (out-of-pool condition). The real allocator is internal.

### The real pool allocator in 0x13F3F0

0x13F3F0 is a **slot-based pool manager**, not a general linked-list:

```
Pool base:   gp[-19528] (0xB3B8) — pointer to slot array
Pool cap:    gp[-19524] (0xB3BC) — slot count
Stride:      0x94 (148 bytes) per slot
Free check:  slot[+0x00] == 0   — self-pointer cleared = free
```

**0x13F3F0 calling convention:**
```
a0 = entity ptr        → s2
a1 = callback type     → s5
a2 = data ptr          → s4
a3 = flags & 0xFF      → s6 (non-zero = no heap alloc, inline storage at +0x1C)
t0 = priority/key      → s3 (for sorted list insertion)
t1 = heap alloc size   → s7 (passed to 0x13D1B0 for heap_alloc)
```

**Flow:**
```
1. Scan pool: for i in 0..pool_cap:
     if pool[i][+0x00] == 0: goto FOUND
2. No free slot: assert error (0x1A6E28 calls), return 0
3. FOUND: slot = pool[i]
4. if flags == 0:
     s0 = slot + 0x24     // data area start
     call 0x13D1B0(s0, 1, data, slot, t0_key=?, t1_size=?, t2_extra=?)
         // inside 0x13D1B0:
         //   heap_alloc(t0, (int)t1, "NodeCallback", 173)
         //   stores result into s0 area
         slot[+0x1C] = data
     call 0x13D3C8(s0)   // insert into kernel queue?
   else (flags != 0):
     slot[+0x1C] = data   // inline store only (0x13F7A8 path via stack spill)
5. slot[+0x10] = flags    // at field offset
6. slot[+0x18] = 1        // active
7. slot[+0x04] = entity   // backref
8. slot[+0x14] = t0       // priority/key
9. Insert slot into entity's sorted linked list:
   - entity[+0x2C] = head
   - entity[+0x30] = tail
   - Sorted by slot[+0x14] (ascending)
   - Doubly linked: slot[+0x08] = next, slot[+0x0C] = prev
10. return slot
```

**0x13D1B0** is the **heap alloc initializer** (128B stack):
- Calls `heap_alloc(a0=t0, a1=(int)t1, a2="NodeCallback", a3=173)` via 0x13A0F8
- From 0x13F7D8 (system variant): t1 = 0x1800 = 6144 bytes alloc
- From scene loader path: t1 is whatever the caller had in register

**0x13D3C8** jumps to 0x100340 (kernel function):
- Reads [a0+0x34] and [a0+0x30] → jump to 0x100340
- This is a kernel queue/timer insertion (EE SDK kernel dispatcher)

### Slot layout (stride 0x94)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| +0x00 | 4 | self | Self-pointer (0 = free) |
| +0x04 | 4 | entity | Entity backref |
| +0x08 | 4 | next | Next in per-entity linked list |
| +0x0C | 4 | prev | Prev in per-entity linked list |
| +0x10 | 4 | flags | Flags from a3 (0 = heap alloc, non-zero = inline) |
| +0x14 | 4 | key | Priority/key from t0 |
| +0x18 | 4 | active | 1 = active |
| +0x1C | 4 | data | Data pointer |
| +0x20 | 4 | — | Padding/unused |
| +0x24 | 0x70 | payload | Heap-allocated or inline payload |

### Corrected 0x13F7A8 model

```
0x13F7A8(a0=entity, a1=data, a2=unused, a3=type):
  // Tail call to 0x13F3F0:
  //   a2 = a1 (data preserved)
  //   a1 = a3 (type=0x13)
  //   a3 = &spill (non-0 flags → inline path)
  //   t0, t1 = whatever caller had
  j 0x13F3F0
  addiu a3, sp, 0x10    // delay slot
```

Since a3 = sp+0x10 (stack address → non-zero → inline path), the scene loader's callback registration stores data **inline** at slot[+0x1C] without a heap alloc. The heap alloc path (flags=0) is used when 0x13F7A8 is called with a3=0, which happens from the system variant 0x13F7D8 with t1=0x1800.

---

## Item 2: Runtime validation plan

### Breakpoints for next session

| Address | Capture | Question |
|---------|---------|----------|
| 0x1B76F8 | a0, a1, pool_base(gp-19528), pool_cap(gp-19524) | Confirm s2 = buffer ctx, verify pool state during load |
| 0x13F3F0 | a0-a3, t0, t1 | Capture allocator params: entity, type, data, flags, key, size |
| 0x1B7D00 | s0-s4, gp[-19528], gp[-19524] | Capture 4-phase init: which slots are allocated per phase |
| 0x13D1B0 | a0-a3, t0-t2 | Confirm heap_alloc size and tag (173 = "NodeCallback") |
| 0x1C00C0 (WOODBOX0 hC) | a0, sp area | Capture 400B alloc params and children count |

### Useful log format for each hit

```
[addr] a0=0x%08x a1=0x%08x a2=0x%08x a3=0x%08x t0=0x%08x t1=0x%08x sp=0x%08x
```

---

## Item 3: 0x1B7B88 — re-init Fase 2

```
0x1B7B88(a0=type_id):
  1. entry = entry_table[type_id] (0x002A4C48 + type_id * 0x4C)
  2. type_idx = entry[+0x46]
  3. desc = descriptor_table[type_idx] (0x002A31B8 + type_idx * 0x64)
  4. if desc[+0x44] == 0: return   // inactive descriptor
  5. if entry[+0x44] == 0: return   // zero entry count
  6. if type_idx == 4: return       // type 4 skip
  7. p1 = 0x13EAE8(type_id)         // lookup function
  8. p2 = 0x13EAE8(entry[+0x44])    // lookup on entry count
  9. if p2 == NULL: return
  10. if p1 == p2:
        // Same: log, then init A
        0x1A6E28(...)        // assert (disabled)
        0x1AD768(desc)       // some init
        0x263FF0(desc, ...)  // kernel/setup
      else:
        // Different: link objects
        0x1A6E28(...)        // assert (disabled)
        p1[+0x15C].payload[+0x00] = p2  // link object
        p1[+0x15C].payload[+0x04] = 0
      
  11. return
```

Called from scene loader Phase 2 (loop s0=2..5) as a second initialization pass for entity types 2-5.

---

## Item 4: ASM-HOLD analysis

### WOODBOX0 hC (0x1C00C0, 286 insns)

18 JAL targets:

| Target | Role |
|--------|------|
| 0x13A0F8 | heap_alloc (payload alloc) |
| 0x19F310 | entity_state_reg |
| 0x1B7FE8 ×2 | Some init function |
| 0x1BDA70 | Child object init |
| 0x1BDC58 | Child object init |
| 0x1BD408 | Child object init |
| 0x1BCC18 | Child object init |
| 0x1A6E28 | Assert stub (disabled) |
| 0x19F4E8 | entity_state_reg variant |
| 0x105F00 ×2 | SDK function |
| 0x2641D8 | SDK function |
| 0x10E158 | Matrix/transform |
| 0x102858 | SDK function |
| 0x118A68 ×2 | Copy/memset |
| 0x1BFFE8 | Finalize init |

Pattern: allocs 400B heap, then creates 2 child objects (0x1BDA70/0x1BDC58 for child sub-init), sets up model data from 0x4DF560 table.

### AP1 hC (0x1B8720, 367 insns)

The most complex constructor seen. 27+ JAL targets:

| Pattern | Count | Description |
|---------|-------|-------------|
| 0x13A0F8 | 6× | heap_alloc for various subsystems |
| 0x139598 | 4× | Some object copy/init |
| 0x19F310 | 2× | entity_state_reg |
| 0x105F00 | 1× | SDK |
| 0x118678 | 2× | memcpy |
| 0x102850 | 1× | SDK |
| 0x104508 | 1× | SDK |
| 0x104140 | 1× | SDK |
| 0x118648 | 2× | memcpy/set |
| 0x109F10 | 1× | Child index lookup |
| 0x264D60 | 1× | rand/srand |
| 0x1AD748 | 1× | Init |
| 0x263FF0 | 1× | Kernel queue |
| 0x1CEF90 | 1× | Collision/shape init |

Allocates 640B heap across 6 heap_alloc calls. Creates 4 child objects with varied roles (collision, AI, animation, seeker). Has randomized frame offset (0x264D60 = rand call).

### ENEMY1 hB (0x1CE3C0, 142 insns)

Full structure documented in Item 5.

---

## Item 5: Entity state machine analysis

### ENEMY1 hB (0x1CE3C0, 112B stack)

**State variable:** [s3+0x4C] — counter, max 10 (checked < 11)
- When a bit in entity[+0x164][+0x18] is set: resets counter to 0
- Otherwise: increments counter, skips entire update body if counter >= 11

**Update sequence when active:**

```
1. Reset AI state flags:
   [payload+0x550] = 0    (attack/aggro flag)
   [payload+0x54C] = 2    (AI mode = 2)
   [payload+0x548] = 0    (sub-state)

2. Call 0x1654C8() → returns AI state
   if return == 3: [payload+0x550] = 1 (aggro)

3. Shadow draw: 0x1E3FC8

4. Movement/collision:
   [s3+0x48] * 0x428C (70.0) → f12
   [s3+0x48] * 0x4248 (50.0) → f13
   0x103F00(entity, 4, zero, f12, f13, 0.5)  // move test

5. Call 0x165540(entity) → returns per-frame action
   if return != 0 AND [payload+0x63C] != 0 AND [s3+0x2C] != 0:
     Read [payload+0x220] → transform matrix
     0x104940(sp, payload+0x1D0, matrix+0x30)  // view transform
     0x1CF6C0(s3[+0x28], sp)  // draw A

6. 0x1CF548(s3[+0x28])  // draw B / reset

7. Modulo frame counter: [payload+0x558] = ([payload+0x558] + 1) % 10

8. Read [payload+0x840] position: (x + y + z) / 3.0 → f20 (avg brightness/height)

9. Two render passes (A + B):
   a. 0x105278() → get result
   b. 0x109F10(entity, 36) → child index lookup (pass A)
      0x109F10(entity, 37) → child index lookup (pass B)
   c. index * 0x40 → add to [payload+0x0C], call 0x1185D0(texture_setup)
   d. 0x1CF930(s3[+0x18], result, f20)  // draw A with brightness
      0x1CF930(s3[+0x20], result, f20)  // draw B with brightness
```

**State machine summary:** 3-mode AI (likely idle/search/attack) with counter-based sleep, 2-pass rendering using child sprite hierarchy. f20 controls brightness/scale based on average position.

### BOY hB (0x1C1DD8)

**Main function (48B stack) — 3 sub-functions:**

```
Sub-fn A (main body, 0x1C1DD8):
  1. 0x1D23E0(entity)     → cloth update for BOY's cape/robe
  2. 0x1E3FC8(entity)     → shadow draw (shared with all entities)
  3. 0x1C1C48(entity)     → child update A
  4. 0x1C12F0(entity)     → child update B
  5. 0x1E4868(entity)     → seeker/target update
  6. 0x1BB8C0(entity, 35, 44, 408) → state/flag check
  7. 0x103D50(entity, 4, 50.0, 50.0, float) → movement/collision
     if hit: 0x13FF88(entity, 6, entity) → event/feedback
  8. 0x14A0D8() → conditional timing
  9. tail 0x103D50(entity, 2, 50|30, 50.0, float) → second collision
                                               ↓
Sub-fn B (0x1C1EA8, 64B stack) — collision/transform:
  • Read [entity+0x15C] → [payload+0x800]
  • Call 0x109F10(entity, 35) → child index
  • Switch on [s0+0x00] (state 1/2/default → different child slots)
  • Compute matrix index = s2 × 0x40
  • 0x105F20(a0=[s0+0x0C], a1=matrix), 0x104F48(0x8000)
  • Tail call 0x121D90(s0, entity+0x15C) → collision detect
                                               ↓
Sub-fn C (0x1C1F58, 96B stack) — head/weapon update:
  If [s2+0x10] != 0 (head exists):
    • Transform setup: 0x102850 + 0x10E158
    • 0x12A060(424, sp, s0) → collision with 424 tag
    • Store result in [s2+0x14]
  
  Else (no head):
    • 0x10ECD8() + 0x10ECB8(entity) → recovery/update
    • Call sub-fn B (0x1C1EA8)
    • Call 0x1C1250(entity) → draw
    • If gp_state == 39 AND [payload+0x644] > 20.0 AND [payload+0x648] != 0:
      Collision check: 0x10D180(payload+0x648)
      If pass: 0x10CB48(s2+0x2C, entity, payload+0x648) + 0x10CF88(s2+0x2C)
```

**State variables used:**
- [s0+0x00] — child state (1/2/default)
- [s2+0x10] — head attached flag
- [s2+0x14] — collision result
- gp_state == 39 — world/room state check
- [payload+0x644] — position/height threshold (20.0)
- [payload+0x648] — collision target

### GIRL hB (0x1D17F8, 113 insns)

Very similar to BOY hB structure with 16 JAL targets:

```
1. 0x1D23E0(entity)     → cloth update (same as BOY)
2. 0x1E3FC8(entity)     → shadow draw
3. 0x1BB8C0(entity, 35, 44, 408) → state check (same as BOY)
4. 0x103D50(entity, 4, ...) → collision/move
   if hit: 0x13FF88(entity, 6, entity) → feedback
5. 0x14A0D8() → timing check
6. Repeat collision: 0x103D50 + 0x13FF88
7. 0x174D78(entity)     → animation blend / ragdoll?
8. 0x109F10(entity, 36) ×2 → child index lookups
9. 0x243AE8(entity) + 0x243950(entity) → GIRL-specific AI calls
10. 0x105FE0(entity)     → draw
11. 0x10A4E0(entity)     → post-draw cleanup
12. 0x1D14B8(entity)     → cloth cleanup/update
```

**Key differences from BOY:**
- Has 2 cloth-related calls (0x1D23E0 + 0x1D14B8) vs BOY's 1
- GIRL-specific AI: 0x243AE8 + 0x243950 (probably follow/guard behavior)
- Animation: 0x174D78 (blend/animation)
- NO seeker update (0x1E4868) or child collision chain — simpler body

---

## hB structure patterns across all 5 analyzed entities

| Entity | Stack | JALs | Cloth | Shadow | Move/Coll | Draw | AI | Children | State var |
|--------|-------|------|-------|--------|-----------|------|-----|----------|-----------|
| BOY | 48+64+96 | 27 | 1 (0x1D23E0) | 1 | 3+ calls | 1 (0x1C1250) | 2 (0x1E4868/0x1E4938) | 3-body(head/sword) | Child state, head flag |
| GIRL | ~48 | 16 | 2 (0x1D23E0+0x1D14B8) | 1 | 2+ calls | 1 (0x105FE0) | 2 (0x243AE8/0x243950) | None | AI state 2-mode |
| ENEMY1 | 112 | 17 | 0 | 1 | 2+ calls | 3 (0x1CF6C0+0x1CF930×2) | 1 (0x1654C8/0x165540) | 2 child sprites | Counter ÷10, AI mode 3-state |
| WOODBOX0 | hC only | 18 | 0 | 0 | 0 | 0 | 0 | 2 crate parts | n/a (constructor) |
| AP1 | hC only | 27+ | 0 | 0 | 0 | 0 | 0 | 4 varied | n/a (constructor) |

---

## What is confirmed

- 0x1A6E28 = disabled print/assert stub (not allocator)
- 0x13F3F0 = slot-based pool allocator (stride 0x94), NOT linked-list heap
- Pool base at gp[-19528], capacity at gp[-19524]
- t1 controls heap_alloc size in 0x13D1B0 (0x1800 from system variant)
- 0x1B7B88 = re-init Fase 2 (links sibling objects for types 2-5)
- BOY hB: 3 sub-functions (cloth+movement, collision/transform, head/weapon)
- GIRL hB: BOY-similar but with GIRL-specific AI (0x243AE8/0x243950)
- ENEMY1 hB: 3-state AI (idle/search/attack), counter-based sleep, 2-pass child sprite render

## What is probable

- 0x103D50 is the shared movement/collision function (called by BOY, GIRL, ENEMY1)
- 0x109F10 is a child-index-by-tag lookup (called with tags 35, 36, 37)
- 0x13FF88 is an event/feedback function (called with type 6)
- 0x1E3FC8 is the universal shadow draw (BOY, GIRL, ENEMY1 all call it)
- 0x1CF930 uses f20 (avg pos) as brightness/alpha for child sprites

## What is unknown

- What 0x1654C8 returns (3-state: idle/search/attack?) and how the 3 states map
- What gp_state=39 means (room/world state for BOY's collision response)
- The exact meaning of [s3+0x4C] in ENEMY1 (timer? aggro cooldown?)
- Full semantics of AP1's 4 children (states/sub-types not mapped)
- How BOY's head detachment/recovery works

## Next minimum test

- Runtime: capture 0x13F3F0 to see t0 (key) and t1 (size) values during scene load
- Runtime: capture 0x1654C8 return value and map ENEMY1's 3 AI states
- Runtime: capture gp_state at BOY hB to identify state 39
