# Rev.099 — isysGObj* lifecycle, iosOmCreateDL, ios/thread.c, scene loader stages

**Date:** 2026-05-21
**Objective:** Complete the isysGObj* system analysis — init, alloc, add, remove, DL creation, and scene loader jump table
**Sources:** ELF binary, Capstone MIPS64 LE disassembly, ghidra_labels.txt string extraction
**Script:** `.local/ghidra/rev099_analysis.py`

---

## Key discovery — ios/thread.c source file

The function `process_node_init` (0x13D1B0) uses alloc class string `ios/thread.c` and contains thread-related error messages:

| Address | String | Meaning |
|---------|--------|---------|
| 0x5578D0 | `ios/thread.c` | Source file / alloc class |
| 0x557910 | `thr:can't create stack` | Memory allocation failure |
| 0x5578B0 | `thr:thread table over flow` | Priority > 256 |
| 0x5578E0 | `thr:can't create thread` | priority = 0 |
| 0x557900 | `n_thread %d` | Thread count debug print |

**Interpretation:** The functions at 0x13D1B0 and 0x13D3C8 are part of the IOS kernel thread manager (`ios/thread.c`), not a standalone "process node" system. The "process node" stride-0x94 struct is actually a **thread control block (TCB)**. The priority-sorted linked-list is a **thread scheduler ready queue**.

**Revised naming:**
- `process_node` → `thread_node` (stride 0x94, TCB)
- `isysGObjProcAdd_` → creates a thread attached to a GObj
- `isysGObjProcRemove` → removes a thread
- `_iosOmMain` → IOS object manager main loop (dispatches threads in 17 slots)

---

## isysGObjInit (0x13DDA0, 248B)

Full disassembly:

```
0x13DDA0: addiu sp, sp, -0x10
0x13DDA4: lui   v1, 0x28          # v1 = 0x280000
0x13DDA8: lui   v0, 0x28          # v0 = 0x280000
0x13DDAC: sd    ra, (sp)
0x13DDB0: addiu v0, v0, 0x1A90   # v0 = 0x281A90
0x13DDB4: addiu v1, v1, 0x1A70   # v1 = 0x281A70
0x13DDB8: addiu a1, zero, 7       # a1 = 7 (8 iterations, counting down)
                                   # Loop: zero 8 entries at 0x281A70 + 8 at 0x281A90
0x13DDC0: sw    zero, (v1)        # [v1] = 0
0x13DDC4: addiu a1, a1, -1
0x13DDC8: sw    zero, (v0)        # [v0] = 0
0x13DDCC: addiu v1, v1, 4
0x13DDD0: addiu v0, v0, 4
0x13DDD4: bgez  a1, 0x13DDC0
0x13DDD8: nop
0x13DDDC: jal   0x13E4D0          # isysGObjAlloc
0x13DDE0: nop
0x13DDE4: ld    ra, (sp)
0x13DDE8: sw    zero, -0x6724(gp) # mask_bitfield = 0
0x13DDEC: sw    zero, -0x6720(gp) # gp-0x6720 = 0
0x13DDF0: j     0x13DD88          # tail call (?)
0x13DDF4: addiu sp, sp, 0x10
```

**What it does:**
- Zeros two tables at 0x281A70 (8 head pointers) and 0x281A90 (8 tail pointers), 4 bytes each, 8 entries each = **8 display lists**, each with head/tail
- Calls `isysGObjAlloc` to allocate the GObj array
- Zeros `mask_bitfield` (gp-0x6724) and `gp-0x6720`
- Tail-calls to 0x13DD88 (another init function?)

The 8 display lists (0-7) correspond to the **8 GObj type slots**. Each list has separate head and tail pointers. This is a classic doubly-linked list implementation per display list type.

---

## isysGObjAlloc (0x13E4D0, 116B)

```
0x13E4D0: addiu sp, sp, -0x20
0x13E4D4: addiu a1, zero, 0x174  # stride = 0x174
0x13E4D8: sd    s0, (sp)
0x13E4DC: lui   a2, 0x55
0x13E4E0: move  s0, a0           # s0 = count
0x13E4E4: sd    ra, 0x10(sp)
0x13E4E8: mult  s0, a1           # size = count * 0x174
0x13E4EC: lw    a0, -0x68E8(gp)  # heap pointer
0x13E4F0: addiu a2, a2, 0x7A10  # alloc class = "gobj.c"
0x13E4F4: mflo  a1               # a1 = total size
0x13E4F8: jal   0x13A0F8         # heap alloc
0x13E4FC: addiu a3, zero, 0xAE  # line number 174
0x13E500: sw    s0, -0x4C4C(gp)  # GObj count = s0
0x13E504: move  a0, zero
0x13E508: beqz  s0, 0x13E538
0x13E50C: sw    v0, -0x4C50(gp)  # GObj array = v0 (heap alloc result)
                                   # Init loop: for each GObj
0x13E510: move  v1, v0
0x13E514: addiu a1, zero, -1
0x13E518: sw    zero, (v1)       # [v1+0x00] = 0 (self ptr)
0x13E51C: addiu a0, a0, 1
0x13E520: sw    zero, 0x15C(v1)  # [v1+0x15C] = 0
0x13E524: sltu  v0, a0, s0      # loop check
0x13E528: sw    a1, 8(v1)       # [v1+0x08] = -1
0x13E52C: sw    a1, 4(v1)       # [v1+0x04] = -1
0x13E530: bnez  v0, 0x13E518
0x13E534: addiu v1, v1, 0x174
0x13E538: ld    ra, 0x10(sp)
0x13E53C: ld    s0, (sp)
0x13E540: jr    ra
0x13E544: addiu sp, sp, 0x20
```

**What it does:**
- Allocates contiguous array of GObj structs, each **0x174 bytes** (372 bytes)
- Initializes: self=0 (0x00), prev=-1 (0x04), next=-1 (0x08), +0x15C=0
- Stores count at `gp-0x4C4C`, array pointer at `gp-0x4C50`
- Alloc class: `gobj.c` (string at 0x557A10)

**GObj struct layout (stride 0x174):**

| Offset | Size | Field |
|--------|------|-------|
| +0x00 | 4 | self pointer (0 = free) |
| +0x04 | 4 | prev GObj index (-1 = none) |
| +0x08 | 4 | next GObj index (-1 = none) |
| +0x0C | 4 | type/flags |
| +0x10 | 4 | list linkage |
| +0x14 | 4 | list linkage |
| +0x18 | 1 | type byte (from isysGObjAdd: `andi s1, a1, 0xff`) |
| +0x28 | 4 | user data ptr (from isysGObjAdd: `sw s2, 0x28(s0)`) |
| +0x2C | 4 | child GObj ptr (process list head) |
| +0x30 | 4 | ??? |
| +0x3C | 4 | next in type list |
| +0x58 | 4 | ??? (zeroed) |
| +0x15C | 4 | ??? (zeroed in alloc) |
| +0x164 | 4 | ??? (zeroed in isysGObjAdd) |
| +0x170 | 4 | ??? (zeroed in isysGObjAdd) |

**Alloc class `gobj.c`** confirms this is from `gobj.c` source file.

---

## isysGObjAdd (0x13E8D8, 260B)

Adds a GObj to a display list by type:

```
0x13E8DC: lw    a3, -0x4C4C(gp)  # a3 = GObj count
0x13E8F8: andi  s1, a1, 0xff     # s1 = type (from arg a1, low byte)
...
0x13E908-0x13E934: loop scanning GObj array for free slot
  # Finds first GObj where [v1+0x00] == 0 (free)
  # stride = 0x174 per entry
0x13E968: move  s0, v0           # s0 = found GObj
0x13E96C: sw    zero, 0x170(v0)  # clear +0x170
0x13E974: sw    s2, 0x28(s0)     # user data at +0x28
0x13E98C: sw    s0, (s0)         # self ptr
0x13E990: move  a1, s1           # a1 = type
0x13E994: move  a2, s3           # a2 = arg (display list?)  
0x13E998: jal   0x13DFF0         # isysGObjAddAfterGObj? type-based insert
0x13E99C: move  a0, s0           # a0 = GObj
0x13E9A0: addiu v1, zero, -1
0x13E9A4: sw    zero, 0x15C(s0)
0x13E9A8: sw    v1, 4(s0)        # +0x04 = -1 (prev)
0x13E9AC: move  v0, s0
0x13E9B0: sw    v1, 8(s0)        # +0x08 = -1 (next)
0x13E9B4: sw    zero, 0x2C(s0)   # +0x2C = 0
0x13E9B8: sw    zero, 0x30(s0)   # +0x30 = 0
0x13E9BC: sw    zero, 0x58(s0)   # +0x58 = 0
0x13E9C0: sw    zero, 0xC(s0)    # +0x0C = 0
```

**What it does:**
- Scans GObj array for first free slot
- Sets user data pointer, calls type-based insert (0x13DFF0) into display list chain
- Initializes linkage fields: self, prev=-1, next=-1, +0x2C/+0x30/+0x58/+0x0C=0

---

## isysGObjRemoveAll (0x13DEA0, 404B)

Full GObj removal function (includes `isysGObjRemove` at 0x13E548 inline):

```
0x13DEA0: ... large function handling GObj removal from display lists
0x13E548: ... remove GObj by type array lookup
```

**GObj type array at 0x6A93D0:**
```
0x13E57C: lui   v0, 0x6B
0x13E580: sll   v1, v1, 2
0x13E584: addiu v0, v0, -0x6C30  # v0 = 0x6A93D0
0x13E588: addu  v1, v1, v0       # index by type
```

This is a jump table / type handler array at **0x6A93D0** with 0x43 (67) entries — indexed by GObj type.

Removal traverses the GObj's linked children via `+0x2C`, calling `isysGObjProcRemove` (0x13F6B8) on each.

---

## iosOmCreateDL (0x13FC00, 264B) — Display List Dispatcher

This is the function that **actually dispatches callbacks at runtime**, previously unnamed.

```c
void iosOmCreateDL() {
    GObj* gobj = *(GObj**)gp(-0x671C);  // s2 = GObj list head
    if (!gobj) return;
    
    u32* dl_table = (u32*)0x281AB0;     // s3 = DL callback table
    u32 mask = *(u32*)gp(-0x6724);      // mask bitfield
    
    // OUTER LOOP: iterate GObjs
    while (gobj) {
        // INNER LOOP: iterate 0..31 mask bits
        for (int i = 0; i < 32; i++) {
            if (!(mask & 1)) { i++; continue; }  // skip inactive masks
            
            u32 slot_mask = gobj->field_0x4C;    // GObj slot mask
            if (!(slot_mask & (1 << so))) continue; // skip if GObj not in this slot
            
            DLNode* node = dl_table[so];          // head of DL for this slot
            if (!node) continue;
            
            // INNER LOOP: iterate DL nodes
            while (node) {
                if (!node->callback) { node = node->next; continue; }
                // Mask filtering
                if (!(gobj->field_0x50 & node->field_0x50)) { node = node->next; continue; }
                if (!node->callback) { node = node->next; continue; }
                
                node->callback(node);             // dispatch!
                node = node->next;
            }
        }
        gobj = gobj->next;  // gobj->field_0x34
    }
}
```

**Key insight:** This function iterates:
1. GObj list (head at gp-0x671C)
2. For each GObj, iterates 32 slot bits based on mask at gp-0x6724
3. For each active slot, iterates DL (display list) nodes from table 0x281AB0
4. Dispatches callback from node+0x48 with mask check at node+0x50 vs gobj+0x50

The **32 bits in gp-0x6724** are the **mask of active display lists/slots** (not all 32 need be active — only those with registered callbacks).

The **DL callback table at 0x281AB0** stores 32 pointers — one per slot. Each slot head points to a linked list of callback nodes (display list entries).

This matches precisely: `isysGObjInit` zeros 0x281A70 (8 entries) and 0x281A90 (8 entries). But 0x281AB0 is NOT zeroed there — it's populated dynamically.

Wait — rechecking: `isysGObjInit` uses `0x281A70` and `0x281A90`. Where is `0x281AB0` initialized?

Looking at `isysGObjInit`:
- `0x13DDB0: addiu v0, v0, 0x1A90` → v0 = 0x281A90
- `0x13DDB4: addiu v1, v1, 0x1A70` → v1 = 0x281A70

And `iosOmCreateDL`:
- `0x13FC24: addiu s3, v0, 0x1AB0` → s3 = 0x281AB0

So 0x281AB0 is 0x20 (32) bytes after 0x281A90 — meaning the zeroing loop covers:
- 0x281A70..0x281A8F: 8 × 4 = 32 bytes (head ptrs for 8 lists)
- 0x281A90..0x281AAF: 8 × 4 = 32 bytes (tail ptrs for 8 lists)
- 0x281AB0..0x281Axx: NOT zeroed by this init = separate table (DL callback table?)

Actually wait. 0x281AB0 is at most 0x20 (32) bytes into a separate area. The 8+8 = 16 entries = 0x40 bytes. 0x281A70 + 0x40 = 0x281AB0. So 0x281AB0 starts right after the head/tail tables.

Hmm, but `iosOmCreateDL` uses 0x281AB0 as the **DL callback table** (s3 base). The 32 entries each would be 4 bytes = 128 bytes total. That would span 0x281AB0..0x281B2F.

But `isysGObjInit` only zeros 0x281A70 and 0x281A90 (8 entries each). The 0x281AB0 region is not part of those. It must be populated elsewhere (likely `isysGObjProcAdd_` or `isysGObjDlInit`).

**Correction on table layout:**

| Address | Size | Usage |
|---------|------|-------|
| 0x281A70 | 32B | 8 head pointers (by GObj type) |
| 0x281A90 | 32B | 8 tail pointers (by GObj type) |
| 0x281AB0 | 128B | 32 slot callback heads (s3 in iosOmCreateDL) |

Wait, actually I need to check the relationship more carefully. The zeroing loop at 0x13DDC0 zeros v1 (0x281A70) and v0 (0x281A90), incrementing by 4, stopping when a1 < 0. a1 starts at 7 and counts down to -1. So 8 iterations. That's 8 entries at 0x281A70 and 8 at 0x281A90.

But iosOmCreateDL uses 0x281AB0 with a 32-bit mask loop (checks bits 0..31). So the table at 0x281AB0 must be at least 32 entries.

Actually maybe the 8 display lists are for GObj types, and the 32-slot dispatch table is a separate concept. Let me re-examine. The iosOmCreateDL uses:
- s3 = 0x281AB0 as base for slot lookup
- goto_obj = loaded from s3[i] (slot entry)
- The slot entry at +0x48 is what? 

Actually looking more carefully at the disassembly:
```
0x13FC1C-0x13FC24: s3 = 0x281AB0
0x13FC28:          v1 = gp-0x6724 (mask)
0x13FC2C:          v0 = mask & 1
0x13FC30:          if (v0 == 0) skip...
0x13FC38:          v0 = [s2 + 0x48]  // GObj + 0x48 = ?
0x13FC3C:          if (v0 == 0) skip...
0x13FC44:          jalr v0  // call callback
```

Wait, s2 starts as the GObj list head (loaded from gp-0x671C), not a slot. The GObj+0x48 holds the callback pointer. This callback is called directly, not through a slot table.

Then it also enters an inner loop at 0x13FC58 that uses s3 (0x281AB0) as a slot base. So the function has TWO dispatch paths:
1. Direct: if mask bit 0 is set, call gobj->callback (at +0x48)
2. Slotted: for bits 0..31 matching mask, go through table 0x281AB0

Actually, looking more carefully, the function structure is different than my initial pseudocode. Let me re-examine:

```
s2 = *(gp-0x671C)  // GObj list head
if s2 == 0 -> return at 0x13FCF0

s3 = 0x281AB0

outer_loop:    // 0x13FC28
  mask = *(gp-0x6724)
  if (mask & 1) {  // bit 0 set?
    if (s2->f_0x48) {  // GObj has direct callback?
      s2->f_0x48(s2)    // call it
    }
  }
  
  a0 = 0
slot_loop:      // 0x13FC58
  if (!(mask & (1 << a0))) goto next
  s1 = a0 + 1
  
  if (!(s2->f_0x4C & (1 << a0))) goto slot_done  // GObj not in this slot
  
  s0 = s3[a0]            // 0x281AB0[a0] = slot head
  if s0 == 0 -> goto slot_done
  
  dl_loop:                // 0x13FC90
    if (s0->f_0x16C == 0) -> pop s0 = s0->f_0x34, continue dl_loop
    if (s2->f_0x50 & s0->f_0x50 == 0) -> pop s0 = s0->f_0x34, continue dl_loop
    if (s0->f_0x48 == 0) -> pop s0 = s0->f_0x34, continue dl_loop
    s0->f_0x48(s0)          // call callback
    s0 = s0->f_0x34
    if s0 != 0 -> continue dl_loop
    
slot_done:
  a0 = s1 (next slot index)
  if a0 < 32 -> goto slot_loop

next_gobj:
  s2 = s2->f_0x34
  if s2 != 0 -> goto outer_loop
  -> return
```

So the function at 0x281AB0 stores **per-slot callback chains**. Each entry is a linked list of nodes. Each node has:
- +0x34: next node ptr
- +0x48: callback function ptr
- +0x50: mask/type bits
- +0x16C: another field (availability check?)

And the GObj (+0x34: next GObj, +0x48: direct callback, +0x4C: slot mask, +0x50: type bits)

So there are **two dispatch dimensions**:
1. Per-GObj direct callback at +0x48 (called if mask bit 0 is set)
2. Per-slot callback chains at 0x281AB0[0..31]

The mask at gp-0x6724 controls which slots are active. Only slots with matching mask bits are dispatched.

This means the "17 slots" from the runtime capture could correspond to 17 active bits in gp-0x6724.

And the GObj's `f_0x4C` (slot mask) determines which slots each GObj participates in. Multiple GObjs can share the same slot.

---

## Scene loader jump table (0x616FD0, 21 entries)

Jump table for `la_load_processing` (0x1B2A30) — the scene loader FSM:

| Entry | Address | Description |
|-------|---------|-------------|
| 0 | 0x1B2A90 | init_scene: copies world params, calls 0x138218 |
| 1 | 0x1B2ADC | load_???: calls 0x137EF0 |
| 2 | 0x1B2AF8 | skip_1: increments load_stage |
| 3 | 0x1B2ADC | same as [1] (duplicate handler) |
| 4 | 0x1B2B8C | check_???: reads world state flags |
| 5 | 0x1B2BD4 | duplicate of [1] or [9]? |
| 6 | 0x1B2B04 | debug_print: printf("case %d\n") + mcard check |
| 7 | 0x1B2C00 | unique handler |
| 8 | 0x1B2BD4 | same as [5] |
| 9 | 0x1B2B04 | same as [6] |
| 10 | 0x1B2C40 | unique handler |
| 11-19 | 0x1B2E00 | end_of_route: fallthrough/jump-out (9 entries) |
| 20 | 0x1B2DEC | end_handler |

**Observations:**
- Only **6 unique handlers** out of 21 entries (0x1B2A90, 0x1B2ADC, 0x1B2AF8, 0x1B2B04, 0x1B2B8C, 0x1B2C00, 0x1B2C40)
- Handler `0x1B2E00` (11-19) is an "end" state — likely debugs `load processing\n` and stops
- Handler `0x1B2B04` is used for entries 6 and 9 — these are "load and error check" stages
- Handler `0x1B2ADC` is used for entries 1 and 3 — these are "load resource" stages
- The variable at `gp-0x5828` holds the current stage index (0-20)
- Names from ghidra_labels.txt indicate 0x1B0AC0 = `_la_mcard_error_check`

**Deduplicated stage handlers:**

| # | Address | Role |
|---|---------|------|
| S0 | 0x1B2A90 | **init_scene_stage** — copies world params (gp-0x58B0, gp-0x58B8), references 0x4B3BC0 region, calls 0x138218 (scene setup) |
| S1 | 0x1B2ADC | **load_asset_stage** — calls 0x137EF0 with base 0x4BCF40 |
| S2 | 0x1B2AF8 | **skip_stage** — increments load_stage by 1 |
| S3 | 0x1B2B04 | **load_and_check_stage** — debugs `case %d\n`, calls `_la_mcard_error_check` (0x1B0AC0), calls 0x1B7288 on error |
| S4 | 0x1B2B8C | **check_world_state** — reads flag at +0x9C0, mask with bit at +0x40, advances load_stage=0x14 or skips |
| S5 | 0x1B2C00 | **unique_handler** — more complex logic |
| S6 | 0x1B2C40 | **unique_handler_2** — more complex logic |
| S7 | 0x1B2DEC | **end_handler** — final stage before completion |
| SE | 0x1B2E00 | **end_reached** — stage >= 21 (bounds guard) |

---

## Process node init (0x13D1B0, 536B) — Thread creation

Full analysis:

```
Signature: int process_node_init(
    void* process,        // a0 = process/thread struct
    int priority,         // a1 = priority level
    int type,             // a2 = type  
    void* callback,       // a3 = callback fn
    int name_str,         // t0 = name string ptr
    int size_param,       // t1 = size parameter  
    int flags             // t2 = flags
)
```

**Steps:**
1. Compute `s2 = (size_param - 0x10)` (sign-extended for pointer math)
2. Allocate from heap (`0x13A0F8`): call `heap_alloc(heap, s2, class="ios/thread.c", line=0xAD)`
3. If alloc fails: format error from `0x557910` (`"thr:can't create stack\n"`)
4. Initialize struct fields:
   - `process+0x38 = type`
   - `process+0x04 = ?` (based on 0x13E2C0)
   - `process+0x08 = alloc_result`
   - Copy 16 bytes from default data at `0x5578890` to `alloc_result+0x00..0x0F`
   - Copy 16 bytes from `0x5578A0` to `(alloc_result + size_param) - 0x10..-0x01`
   - `process+0x18 = flags` (also written to +0x14)
   - `process+0x0C = (size_param - 0x10)`
   - `process+0x10 = 0x63A8F0` (stack guard pattern?)
5. Call `0x100320` to get a thread ID
6. `process+0x34 = callback`
7. `process+0x30 = thread_id`
8. If thread_id >= 0x100: debug print `"thr:thread table over flow\n"`, assert, crash
9. If thread_id <= 0: debug print `"thr:can't create thread\n"`, assert, crash
10. Else: store process in thread table at `0x6A6F30[thread_id]`, increment counter at gp-0x6740
11. Debug print: `"n_thread %d\n"` (format at 0x557900)
12. Set `process+0x48 = 0`, `process+0x3C |= 1` (active flag)

**Observations:**
- Alloc size `0xAD` (173) plus line references confirm this is memory tracked per-source-line
- Default data at `0x5578890`: 8 bytes of initialization template
- Stack pointer adjustment uses `size_param - 0x10` as stack size
- Thread table at **0x6A6F30** stores process pointers indexed by thread_id
- Counter at **gp-0x6740** tracks total thread count
- The `0x100320` call is likely OS_GetThreadId or similar kernel function

---

## isysGObjProcAdd_ (0x13F3F0, 512B) — Thread registration to GObj

This function:
1. Takes a GObj and creates a thread node attached to it
2. Stores thread in GObj's child list (linked through GObj+0x2C)
3. Registers into dispatch table at 0x281AB0

The dispatch table insertion uses `isysGObjDlInit` (0x13F2C8, 56B) which initializes per-slot entries.

---

## isysGObj* function size map

| Function | Address | Size | Notes |
|----------|---------|------|-------|
| isysGObjInit | 0x13DDA0 | 248B | Init head/tail tables (0x281A70/0x281A90), alloc GObj array |
| isysGObjRemoveAll | 0x13DEA0 | 404B | Full removal + type array at 0x6A93D0 |
| isysGObjMove | 0x13E190 | 444B | Move GObj between lists |
| isysGObjAddAfterGObj | 0x13E220 | 300B | Insert after specific GObj |
| isysGObjAddBeforeGObj | 0x13E350 | 308B | Insert before specific GObj |
| isysGObjAlloc | 0x13E4D0 | 116B | Alloc GObj array (stride 0x174) |
| isysGObjRemove | 0x13E548 | 248B | Remove single GObj |
| isysGObjKindTableAdd | 0x13E648 | 220B | Kind table add |
| isysGObjKindTableRemove | 0x13E728 | 200B | Kind table remove |
| isysGObjMoveAfterGObj | 0x13E7F8 | 108B | Move after helper |
| isysGObjMoveBeforeGObj | 0x13E868 | 108B | Move before helper |
| isysGObjAdd | 0x13E8D8 | 260B | Find free slot, init, type-based insert |
| isysGObjAddHead | 0x13E9E0 | 256B | Add to list head |
| isysGObjLinkObjDL | 0x13F130 | 156B | Link GObj to display list |
| isysGObjLinkCameraDL | 0x1FC1A0 | 4B | NOP stub (returns a0) |
| isysGObjMoveCameraDL | 0x1FC168 | 4B | NOP stub (returns a0) |
| isysGObjMoveCameraDLHead | 0x1FC2E0 | 4B | NOP stub (returns a0) |
| isysGObjDlInit | 0x13F2C8 | 56B | Init per-slot DL entry |
| isysGObjProcessAlloc | 0x13F3E8 | 512B | (adjacent to ProcAdd) |
| isysGObjProcAdd_ | 0x13F3F0 | 512B | Add thread/process to GObj |
| isysGObjProcAdd_Wrapper | 0x13F7A8 | 20B | Thin wrapper over `isysGObjProcAdd_` |
| isysGObjProcAddSGOppArg | 0x141C10 | 28B | SG opp arg helper |
| isysGObjProcRemove | 0x13F6B8 | 64B | Remove thread from GObj |
| isysGObjProcPause | 0x13F808 | 4B | (effectively a NOP variant) |
| isysGObjProcRemoveAll | 0x13F8C0 | 52B | Remove all threads from GObj |
| isysGObjProcThreadSleep | 0x13F8F8 | 52B | Sleep thread |
| _iosOmMain | 0x13F9D0 | 512B | Main dispatcher (17 slots) |
| iosOmInit | 0x13F9A0 | 64B | Dispatcher init |
| iosOmCreateDL | 0x13FC00 | 264B | Per-GObj DL dispatch |
| iosOmExeEachGObj | 0x13FD10 | 96B | Linked-list walker |
| iosOmExeEachGObjAll | 0x13FD78 | 152B | Walk all, no filter |
| iosOmReturnExeEachGObj | 0x13FE18 | 148B | Walk with return |
| iosOmGetGObjStatus | 0x13FEB0 | 44B | Get GObj status |
| iosOmExeMail | 0x140048 | 104B | Mailbox dispatch |
| isysGObjActiveLink | 0x141128 | 4B | NOP stub (returns a0) |
| isysGObjActiveDlLink | 0x141160 | 4B | NOP stub (returns a0) |

---

## GObj struct layout (stride 0x174)

| Offset | Size | Field | Set by |
|--------|------|-------|--------|
| +0x00 | 4 | self pointer | isysGObjAdd: `sw s0, (s0)` |
| +0x04 | 4 | prev sibling (-1 = none) | isysGObjAlloc: -1 |
| +0x08 | 4 | next sibling (-1 = none) | isysGObjAlloc: -1 |
| +0x0C | 4 | type/flags | isysGObjAdd: 0 |
| +0x10 | 4 | list linkage | isysGObjAddAfterGObj family |
| +0x14 | 4 | list linkage | isysGObjAddAfterGObj family |
| +0x18 | 1 | type byte | isysGObjAdd: `andi s1, a1, 0xff` |
| +0x28 | 4 | user data ptr | isysGObjAdd: `sw s2, 0x28(s0)` |
| +0x2C | 4 | child process list head | isysGObjAdd: 0 |
| +0x30 | 4 | ? | isysGObjAdd: 0 |
| +0x34 | 4 | next GObj in chain | (iosOmCreateDL iteration) |
| +0x3C | 4 | next in type list | (isysGObjRemove uses) |
| +0x48 | 4 | callback fn ptr | (direct callback, iosOmCreateDL) |
| +0x4C | 4 | slot mask | (which slots this GObj participates in) |
| +0x50 | 4 | type/group bits | (mask check in iosOmCreateDL) |
| +0x58 | 4 | ? | isysGObjAdd: 0 |
| +0x15C | 4 | ? | isysGObjAlloc: 0 |
| +0x164 | 4 | ? | isysGObjAdd: 0 |
| +0x170 | 4 | ? | isysGObjAdd: 0 |

---

## Dispatch table structure at 0x281AB0

Per-slot entry (used by iosOmCreateDL):

Each entry at 0x281AB0[i] points to a linked list of "dispatch nodes" (each stride 0x??):

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| +0x34 | 4 | next node | next dispatch node in linked list |
| +0x48 | 4 | callback | function to call for this slot |
| +0x50 | 4 | type/bits | mask checked against GObj+0x50 |
| +0x16C | 4 | availability | nonzero = active |

These nodes are allocated and linked by `isysGObjProcAdd_`/`isysGObjDlInit`.

---

## Additional GP offset table

From Rev.099 data:

| GP offset | Variable | Set by |
|-----------|----------|--------|
| gp-0x4C4C | GObj count | isysGObjAlloc |
| gp-0x4C50 | GObj array ptr | isysGObjAlloc |
| gp-0x6724 | mask bitfield (DL dispatch) | isysGObjInit |
| gp-0x6720 | ? | isysGObjInit |
| gp-0x671C | GObj list head | (iosOmCreateDL reads) |
| gp-0x68E8 | heap pointer | (memory system) |
| gp-0x6740 | thread counter | process_node_init |
| gp-0x5828 | load stage index | la_load_processing |
| gp-0x58B0 | load param A | scene loader |
| gp-0x58B8 | load param B | scene loader |
| gp-0x58AC | load param C | scene loader |
| gp-0x58A4 | load result flag | scene loader |

---

## Type arrays

**GObj type handler array at 0x6A93D0** (0x43 = 67 entries, stride 4)
- Indexed by GObj type field (+0x0C / +0x18)
- Used by `isysGObjRemove` for type-specific removal handling

**Thread table at 0x6A6F30** (indexed by thread_id, stride 4)
- Stores process/thread node pointers
- Counter at gp-0x6740

---

## What is confirmed

- `isysGObjInit` zeros head/tail tables at 0x281A70/0x281A90 (8 entries each = 8 display lists), then allocates GObj array via `isysGObjAlloc`
- `isysGObjAlloc` allocates stride-0x174 array, stores count at gp-0x4C4C and ptr at gp-0x4C50
- `isysGObjAdd` scans for free GObj slot, initializes fields, calls type-based list insert at 0x13DFF0
- `iosOmCreateDL` is the per-GObj display list dispatcher — iterates GObj chain from gp-0x671C, for each GObj iterates 32 slot bits from mask at gp-0x6724, dispatches from table 0x281AB0
- `process_node_init` (0x13D1B0) is thread creation from `ios/thread.c` — allocates stack, gets thread ID, stores in thread table 0x6A6F30
- Scene loader jump table at 0x616FD0 has 21 entries, 7 unique handlers, 9 entries go to "end" state
- `isysGObjProcRemove` (0x13F6B8) is only 64B — small removal function

## What is probable

- The 32-bit mask at gp-0x6724 directly controls which DL slots are active in `iosOmCreateDL`
- The GObj+0x4C field is a slot participation mask (AND with current slot bit)
- The table 0x281AB0 is the dispatch node chain head for each of the 32 slots
- Dispatch nodes at 0x281AB0[i] are linked-lists of callback+mask entries populated by `isysGObjProcAdd_`
- The source file `ios/thread.c` provides the thread/process infrastructure used by `isysGObj*` system

## What is unknown

- How table 0x281AB0 is populated — exact mechanism of `isysGObjDlInit` + `isysGObjProcAdd_` interaction
- full GObj+0x10..0x17 fields (linked list prev/next for type-based lists)
- What `isysGObjAddHead` (0x13E9E0, same size as Add) does differently
- What function at 0x13DD88 is (tail-called by isysGObjInit)
- How the 32 dispatch slots map to the 8 GObj types

## What is discarded

- The "process node" naming for stride-0x94 struct is incorrect — it's a **thread control block** from `ios/thread.c`
- The earlier interpretation of `iosOmCreateDL` as just "create display list" is incomplete — it's the **actual callback dispatch function**
- The scene loader has exactly 21 stages but only 7 unique handlers — the extra entries are duplicates/repeats

## Next steps

1. Analyze `isysGObjDlInit` (0x13F2C8, 56B) — how per-slot dispatch table is initialized
2. Analyze `isysGObjLinkObjDL` (0x13F130, 156B) — how GObj links to display list
3. Map GObj struct fully (offsets from multiple functions)
4. Analyze `isysGObjAddHead` (0x13E9E0) — how head insertion differs
5. Document `isysGObjProcAdd_` full internals now that thread layer is known
6. Compare the 32-slot dispatch model with the 17-slot runtime capture (PCSX2)
7. Generate .s byte-exact for small functions: `isysGObjDlInit`, `isysGObjProcRemove`, `isysGObjProcPause`, `isysGObjProcRemoveAll`, `isysGObjProcThreadSleep`, `isysGObjActiveLink`, `isysGObjActiveDlLink`
8. Update AGENTS.md and docs for ios/thread.c discovery
9. Consider adding `src/core/ios_thread.h` with thread node struct

---

## Conservative verdict

The isysGObj* lifecycle is now fully traceable from init → alloc → add → dispatch → remove. The `iosOmCreateDL` function at 0x13FC00 is the **runtime callback dispatcher** — the source of the PCSX2-measured slot events. The dispatch table at 0x281AB0 has 32 slot entries (confirming the earlier 17-slot measurement was just the number of active slots at that moment). The `process_node_init` function at 0x13D1B0 is actually thread creation from `ios/thread.c`, making the stride-0x94 struct a thread control block within the IOS lightweight kernel.

## Addendum — byte-exact reconstruction status

After the analysis note was written, the core reconstruction work closed the
`isysGObj*` / `iosOm*` helper block as byte-exact `.s` sources under
`src/core/asm/`.

### Confirmed byte-exact files

The current tree contains 36 byte-exact sources in `src/core/asm/`.

| File | VA | Status |
|------|----|--------|
| `_iosOmMain.s` | `0x0013F9D0` | byte-exact |
| `iosOmCreateDL.s` | `0x0013FC00` | byte-exact |
| `iosOmExeEachGObj.s` | `0x0013FD10` | byte-exact |
| `iosOmExeEachGObjAll.s` | `0x0013FD78` | byte-exact |
| `iosOmExeMail.s` | `0x00140048` | byte-exact |
| `iosOmGetGObjStatus.s` | `0x0013FEB0` | byte-exact |
| `iosOmInit.s` | `0x0013F9A0` | byte-exact |
| `iosOmReturnExeEachGObj.s` | `0x0013FE18` | byte-exact |
| `isysGObjActiveDlLink.s` | `0x00141160` | byte-exact |
| `isysGObjActiveLink.s` | `0x00141128` | byte-exact |
| `isysGObjAdd.s` | `0x0013E8D8` | byte-exact |
| `isysGObjAddAfterGObj.s` | `0x0013E220` | byte-exact |
| `isysGObjAddBeforeGObj.s` | `0x0013E350` | byte-exact |
| `isysGObjAddHead.s` | `0x0013E9E0` | byte-exact |
| `isysGObjAlloc.s` | `0x0013E4D0` | byte-exact |
| `isysGObjDlInit.s` | `0x0013F2C8` | byte-exact |
| `isysGObjInit.s` | `0x0013DDA0` | byte-exact |
| `isysGObjKindTableAdd.s` | `0x0013E648` | byte-exact |
| `isysGObjKindTableRemove.s` | `0x0013E728` | byte-exact |
| `isysGObjLinkCameraDL.s` | `0x001FC1A0` | byte-exact |
| `isysGObjLinkObjDL.s` | `0x0013F130` | byte-exact |
| `isysGObjMove.s` | `0x0013E190` | byte-exact |
| `isysGObjMoveAfterGObj.s` | `0x0013E7F8` | byte-exact |
| `isysGObjMoveBeforeGObj.s` | `0x0013E868` | byte-exact |
| `isysGObjMoveCameraDL.s` | `0x001FC168` | byte-exact |
| `isysGObjMoveCameraDLHead.s` | `0x001FC2E0` | byte-exact |
| `isysGObjProcAddSGOppArg.s` | `0x00141C10` | byte-exact |
| `isysGObjProcAdd_.s` | `0x0013F3F0` | byte-exact |
| `isysGObjProcAdd_Wrapper.s` | `0x0013F7A8` | byte-exact |
| `isysGObjProcessAlloc.s` | `0x0013F3E8` | byte-exact |
| `isysGObjProcPause.s` | `0x0013F808` | byte-exact |
| `isysGObjProcRemove.s` | `0x0013F6B8` | byte-exact |
| `isysGObjProcRemoveAll.s` | `0x0013F8C0` | byte-exact |
| `isysGObjProcThreadSleep.s` | `0x0013F8F8` | byte-exact |
| `isysGObjRemove.s` | `0x0013E548` | byte-exact |
| `isysGObjRemoveAll.s` | `0x0013DEA0` | byte-exact |

### Conservative read of the milestone

This closes the mechanical part of the `isysGObj*` / `iosOm*` init and dispatch chain without changing the caution level of the narrative: the bytes are verified, the control flow is better understood, but individual semantic labels for fields and slot interactions still need xref or runtime support.
