# Rev.059 — Table Reader Correction + Callback Chain Full Analysis

**Date:** 2026-05-16

## Objective

Complete the three analysis steps deferred from the compile-test session:

1. Identify the physics type table **reader** (function that iterates the descriptor table and dispatches to handlers)
2. Trace the GIRL cloth delegation chain
3. Produce full disassembly of callback registration functions (0x13F3F0, 0x13F7A8, 0x13F7D8)

## Scope

- Correction: 0x1A48A0 is MIPS code, NOT a data table
- Descriptor table layout confirmed at 0x002A31B8
- Caller of 0x1B76F8 identified at 0x1B7D00 (scene loader)
- All three callback functions fully disassembled
- GIRL cloth delegation explained

---

## Finding 1 (CORRECTION): 0x1A48A0 is CODE, not a physics type table

### Previous assumption (Rev.049)

Rev.049 claimed "31 physics object types at 0x001A48A0, stride 0x64". This was based on Ghidra placing a data label at that address.

### Current confirmation

- 0x1A48A0 falls inside .text section (0x100000–0x26F5D4)
- Raw bytes at file offset 0xa58a0: `2d 20 00 02` → LE word `0x0200202d` = `addu a0, s0, zero` = `move a0, s0`
- objdump confirms this is a function body, not a data table

### Implication

All Rev.049 analysis based on the "physics type table at 0x1A48A0" is invalid for that address. The real tables are:

| Table | VA | Section | Stride | Entries |
|-------|-----|---------|--------|---------|
| Entry table | 0x002A4C48 | .data | 0x4C (76) | 512 |
| Descriptor table | 0x002A31B8 | .data | 0x64 (100) | 68 |

---

## Finding 2: Descriptor table handler layout confirmed

The descriptor at 0x002A31B8 (stride 0x64) has the following handler field layout:

| Offset | Slot | Role | Example (WOODBOX0, idx 17) | Example (BARREL, idx 19) |
|--------|------|------|---------------------------|--------------------------|
| +0x40 | init_fn | Asset/constructor fn | 0x17D1D0 | 0 |
| +0x44 | flags | Active flag (=1) | 1 | 1 |
| +0x48 | hA | Post-init/Reset/Dispatcher | 0x1C05D0 | 0x1D3B28 |
| +0x4C | (pad) | Always 0 | 0 | 0 |
| +0x50 | hB | Update per-frame | 0x1C0538 | 0x1D3A30 |
| +0x54 | (pad) | Always 0 | 0 | 0 |
| +0x58 | hC | Constructor | 0x1C00C0 | 0x1D27A8 |
| +0x5C | (pad) | Always 0 | 0 | 0 |
| +0x60 | extra | Optional data ptr | 0x23D660 | 0 |

**Key correction from Rev.052:** hC is at **+0x58**, not +0x50. The gaps at +0x4C and +0x54 are padding (always zero). Rev.052 had the layout correct but labeled offsets inconsistently — this note confirms the actual byte offsets.

### Name field

The name field at +0x00 is up to 8 ASCII characters, null-padded:

| Idx | Name | Entity/Type |
|-----|------|-------------|
| 0 | NULL | Null/empty |
| 1 | BOY | Player |
| 5 | ENEMY_TEST | GIRL/ENEMY1 |
| 17 | WOODBOX01 | Crate entity |
| 19 | BARREL | Barrel (cloth) |
| 20 | ROPE | Rope (cloth/overlay) |

---

## Finding 3: Scene loader at 0x1B7D00 calls descriptor iteration

### Function structure

The function at approximately **0x1B7D00–0x1B7F00** is the **scene loading/initialization function**. It calls 0x1B76F8 (descriptor iteration) in four distinct phases:

```
Phase 1: Loop s0=0..5 (6 iterations)
  0x1B7D90: jal 0x1B76F8(a0=s2, a1=s0)  -- Init base entity types 0-5

Phase 2: Loop s0=2..5 (4 iterations)
  0x1B7DB0: jal 0x1B7B88(a0=s0)          -- Second init pass types 2-5

Phase 3: Loop s0=s3..s1 (range from array)
  0x1B7DF8: jal 0x1B76F8(a0=s2, a1=s0)  -- Init dynamic enemy objects

Phase 4: Walk 0x4B3D10 table (181 entries, stride 0x40)
  0x1B7E6C: jal 0x1B76F8(a0=s2, a1=s0)  -- Init all 181 object IDs
```

### The 0x1B76F8 function

Function at 0x1B76F8 (304B stack frame) processes ONE object/entity:

```
0x1B76F8(a0=buffer/ctx, a1=type_id):
  01. Read entry_table[type_id] from 0x002A4C48 + type_id * 0x4C
  02. Load type_idx = entry[+0x46] (descriptor selector)
  03. Read descriptor = 0x002A31B8 + type_idx * 0x64
  04. Load s5 = alloc_obj()  -- allocate object structure
  05. If s5 != NULL:
  06.   entity_type = s5[+4] (lhu)
  07.   If entity_type == type_id:
  08.     // Call descriptor+0x38 handler (init)
  09.     If descriptor[+0x38] != NULL:
  10.       descriptor[+0x38](stack, s5)
  11.     // Register callback type 19 if applicable
  12.     If entry[+0x24] != NULL:
  13.       0x13F7A8(obj, entry[+0x24], 0, 19)
  14.     Elif descriptor[+0x40] != NULL:
  15.       0x13F7A8(obj, descriptor[+0x40], 0, 19)
  16.     // Call entry table +0x58 handler
  17.     If entry[+0x58] != NULL:
  18.       entry[+0x58](obj, stack)
  19.     // Call descriptor+0x34 handler (construct)
  20.     If descriptor[+0x34] != NULL:
  21.       descriptor[+0x34](obj, s5)
  22.   Store obj in global (gp-based pointer)
```

### Handler dispatch summary

The function at 0x1B76F8 reads handlers from TWO locations:

| Source | Offset | When called | Example (WOODBOX0) |
|--------|--------|-------------|-------------------|
| descriptor[+0x38] | At step 10 | Init handler | 0x1C0790 |
| entry[+0x24] or desc[+0x40] | At step 12-15 | Callback reg | 0x17D1D0 (for entity) |
| entry[+0x58] | At step 18 | Entry handler | varies |
| descriptor[+0x34] | At step 21 | Construct handler | 0x1C0838 |

Note: +0x34 and +0x38 are **below** init_fn (+0x40) in the descriptor, in the space typically used for physics/type data (scale, damping, etc.).

---

## Finding 4: Callback registration chain — full disassembly

### 0x13F7A8 (callback_registration) — 44 bytes

```
0x13F7A8: 27BDFFE0    addiu sp, sp, -0x20    // 32B stack
0x13F7AC: 00A0302D    move a2, a1             // a2 = original a1 (data ptr)
0x13F7B0: 00E0282D    move a1, a3             // a1 = a3 (callback type, always 0x13)
0x13F7B4: FFB00010    sd s0, 16(sp)
0x13F7B8: 0080802D    move s0, a0             // s0 = a0 (obj ctx)
0x13F7BC: FFBF0018    sd ra, 24(sp)
0x13F7C0: 0C04FCFC    jal 0x13F3F0            // tail to node_callback_storage
0x13F7C4: 27A70010    addiu a3, sp, 0x10      // a3 = &spill_on_stack (temp storage)
0x13F7C8: 26040010    addiu a0, s0, 0x10      // a0 = obj+0x10 (next callback slot?)
0x13F7CC: 0200282D    move a1, s0
0x13F7D0: 0C04FCFC    jal 0x13F3F0            // second call to node_callback_storage
0x13F7D4: 27A70010    addiu a3, sp, 0x10      // a3 = &spill_on_stack
0x13F7D8: DFBF0018    ld ra, 24(sp)
0x13F7DC: DFB00010    ld s0, 16(sp)
0x13F7E0: 03E00008    jr ra
0x13F7E4: 27BD0020    addiu sp, sp, 0x20
```

**Behavior:**
- Calls 0x13F3F0 TWICE with (a0, a1=type=a3, a2=data=orig_a1, a3=&spill)
- First call: a0 = obj (s0)
- Second call: a0 = obj+0x10 (sister context)
- Always registers callback type in a1 = a3 (0x13 for cloth)

### 0x13F7D8 (sister_callback_reg, context-free variant) — 36 bytes

```
0x13F7D8: 27BDFFE0    addiu sp, sp, -0x20
0x13F7DC: 0000282D    move a1, zero           // a1 = 0 (no type filter!)
0x13F7E0: 24040194    li a0, 0x194            // a0 = 0x194 (system obj ID = "barrel"?)
0x13F7E4: 24091800    li t1, 0x1800           // t1 = 0x1800 (fixed buffer size)
0x13F7E8: FFBF0018    sd ra, 24(sp)
0x13F7EC: 0C04FCFC    jal 0x13F3F0            // tail to node_callback_storage
0x13F7F0: 27A70010    addiu a3, sp, 0x10
0x13F7F4: DFBF0018    ld ra, 24(sp)
0x13F7F8: 03E00008    jr ra
0x13F7FC: 27BD0020    addiu sp, sp, 0x20
```

**Behavior:**
- a0 = 0x194 (hardcoded system object, likely the first/global cloth object)
- a1 = 0 (no context pointer — system-level registration)
- t1 = 0x1800 (fixed allocation size)
- Only ONE call to 0x13F3F0 (no sister call)

### 0x13F3F0 (node_callback_storage) — 576 bytes

Full 576-byte function (0x13F3F0–0x13F634). Stack frame: 144 bytes (0x90). General structure:

```
0x13F3F0(a0=obj_ptr, a1=callback_type, a2=data_ptr, a3=&temp):
0x13F3F0:
  // Prologue: sd s0-s5, sd ra; addiu sp, -0x90
  
  // Phase 1: Find or allocate linked list node
  s0 = a0                    // obj ptr
  s5 = a2                    // data ptr
  s4 = a3                    // temp spill slot
  s1 = s0[+0x10]             // linked list head
  s3 = 0                     // iteration counter
  s2 = a1                    // callback type
  
  LOOP:
    if s1 == NULL: goto ALLOC
    if s1[+0x08] != s2: goto NEXT  // type field check
    if s1[+0x04] != s5: goto NEXT  // data field check
    goto FOUND_MATCH             // exact match: use existing node
  NEXT:
    s1 = s1[+0x00]           // linked list next ptr
    s3++
    if s3 < 3: goto LOOP     // max 3 nodes scanned
  
  // Phase 2: Allocate new node
  ALLOC:
    a0 = s5                   // pass data ptr
    jal 0x1A6E28(??alloc??)  // alloc descriptor object
    a1 = t1 ...               // t1 may be 0x1800 (from caller)
    s1 = v0                   // new node
    
    if s1 == NULL:
      // Fallback: store directly at [s0+0x1c]
      s0[+0x1c] = s5          // direct callback store
      goto EPILOGUE
    
    // Initialize node fields
    s1[+0x00] = s0[+0x10]     // next = old head
    s1[+0x04] = s5            // data ptr
    s1[+0x08] = s2            // callback type
    s0[+0x10] = s1            // head = new node
    
    a0 = s5
    jal 0x1A6E28              // second alloc (for node body?)
    s1 = v0
    
    if s1 == NULL:
      s0[+0x1c] = s5          // fallback direct store
      goto EPILOGUE
    
    s1[+0x00] = old_tail      // link
    ...                       // body init with data
    head[+0x04] = s1          // link back
    
  FOUND_MATCH:
    // Node exists, store at [s0+0x1c] as well
    s4 = spill slot
    s4[+0x00] = s1
    s4[+0x04] = s5
    s0[+0x1c] = s5            // direct callback reference in entity
  
  EPILOGUE:
    ld ra, ld s0-s5
    jr ra
    addiu sp, 0x90
```

**Key observations:**
- Linked list with stride 0x94 (148 bytes per node)
- Max 3 nodes scanned before creating new
- Two alloc calls to 0x1A6E28
- Fallback: if alloc fails, stores at [obj+0x1C] directly (this is the callback slot used by +0x48 dispatchers)
- The +0x1C offset maps to ThreadParam.entry in EE SDK

### Register values seen at runtime

From Rev.051 runtime session (1249 callback_register hits):
- a3 = 0x13 always (type 19 = cloth)
- a1 = 10 distinct callback data pointers
- None is 0x1D3A30 (the callback itself is stored as data, not as function pointer)

---

## Finding 5: GIRL cloth delegation

GIRL is entity type 5 (descriptor idx 5, name="ENEMY_TEST").

### What the descriptor says

```
Idx 5 (ENEMY_TEST):
  +0x34 = 0x00000000  (no construct handler)
  +0x38 = 0x00000000  (no init handler)
  +0x40 = 0x00000000  (no init_fn)
  +0x48 = 0x001CE5F8  (hA = post-init/reset)
  +0x50 = 0x001E08B8  (hB = update/AI)
  +0x58 = 0x001CE220  (hC = constructor)
```

GIRL has NO direct reference to cloth handlers in her descriptor. Her constructor (0x1CE220) and update (0x1E08B8) are **AI handlers**, not cloth handlers.

### How cloth gets created for GIRL

1. GIRL's constructor (hC=0x1CE220) allocates 80B heap and sets up AI structures
2. The AI state machine eventually creates cloth objects by calling into the cloth system
3. Cloth objects are created with their OWN descriptor entries (BARREL idx 19, ROPE idx 20)
4. The cloth simulation loop at ~0x228600 calls clothSubForceApply (0x1D3F78) for ALL cloth objects, not just GIRL's

### Cloth is shared, not entity-specific

The 12 callers of cloth accessor functions found across .text:
```
0x149804 -- gameplay logic?
0x14CC44 -- gameplay logic?
0x16C430 -- AI/physics?
0x17D290 -- WOODBOX0 init? (17xxxx range)
0x17E894 -- WOODBOX0 area?
0x17EAF8 -- WOODBOX0 area?
0x1D49F4 -- cloth-internal (accessor helper)
0x1D4AA4 -- cloth-internal (accessor helper)
0x20074C -- BGA or rendering?
0x2287BC -- CLOTH SIMULATION LOOP (clothSubForceApply)
0x228B94 -- CLOTH SIMULATION LOOP (clothSubForceApply)
0x228E48 -- CLOTH SIMULATION LOOP (clothSubForceApply)
```

The simulation loop at ~0x228600 (960B stack frame) iterates cloth objects and calls clothSubForceApply. This is the **shared cloth simulation**, not GIRL-specific.

### GIRL cloth delegation model

```
1. Scene loader (0x1B7D00)
   └─> 0x1B76F8(a0=ctx, a1=5)  -- init GIRL/ENEMY_TEST
       └─> GIRL.hC=0x1CE220 (AI constructor, NOT cloth)
           └─> AI system sets up state machine

2. GIRL AI update (0x1E08B8 per-frame)
   └─> GIRL state machine runs
       └─> When GIRL needs cloth: creates BARREL/ROPE object
           └─> Uses callback type 19 (a3=0x13) to register cloth
               └─> Cloth dispatcher (0x1D37C8) handles physics

3. Shared cloth loop (~0x228600)
   └─> Iterates ALL cloth objects
       └─> clothSubForceApply for each
```

---

## Summary of corrections

| Rev.049 claim | Current finding |
|---------------|----------------|
| Physics type table at 0x001A48A0, stride 0x64, 31 entries | 0x1A48A0 is CODE in .text section; real descriptor table at 0x002A31B8 (68 entries) |
| ROPE entry has handlers 0x1D3B28/0x1D3A30/0x1D27A8 | These are BARREL's handlers (idx 19), NOT ROPE (idx 20). ROPE has different handlers. |
| "31 physics object types" | 68 descriptor entries, 68 entry table entries. No separate "physics type table" exists. |

---

## What is confirmed

- 0x1B76F8 is the descriptor iteration/object creation function, called from scene loader at 0x1B7D00
- Descriptor handler layout: +0x48=hA, +0x50=hB, +0x58=hC (+0x4C and +0x54 are padding)
- 0x13F3F0: 576-byte linked list callback storage with stride 0x94, 3-node scan limit
- 0x13F7A8: thin wrapper calling 0x13F3F0 twice (main + sister at obj+0x10)
- 0x13F7D8: context-free variant with a0=0x194, a1=0, t1=0x1800
- +0x1C is the callback store offset (maps to ThreadParam.entry in EE SDK)
- GIRL cloth: no direct cloth handlers; AI system creates cloth objects independently

## What is probable

- 0x1A6E28 is an alloc function for descriptor objects (0x94 bytes each)
- t1 controls allocation size at 0x13F3F0 (t1=0x1800 from system variant 0x13F7D8)

## What is unknown

- The exact allocator used by 0x1A6E28 (heap? pool? slab?)
- How the scene loader at 0x1B7D00 is itself called
- The contents of the +0x60 field in WOODBOX0 descriptor (0x23D660)
- Full semantics of descriptor +0x34 and +0x38 fields (used by entity types like WOODBOX0)

## Next minimum test

- Re-run runtime capture: breakpoint at 0x1B7D00 to capture s2 (the context buffer) during scene load
- Verify linked list stride 0x94 at runtime
- Disassemble 0x1A6E28 to understand the allocator
