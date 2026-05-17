# Rev.073 — Main Loop Dispatch Chain and Corrected Callback Mask Model

**Date:** 2026-05-17

---

## Objective

Consolidate the full main loop dispatch chain from 0x00101C80 and correct the callback mask model for the live dispatch system's 14 unique callbacks.

---

## Scope

- Full 12-step main loop dispatch chain
- Corrected bit positions for callback mask tests
- Secondary pointer table location (BSS)
- Complete struct field maps (80-byte Group 1, 112-byte Group 2)
- VU0 kick trigger identification
- GP variable system map
- All 14 callbacks re-verified

---

## Sources Used

- ELF disassembly via Python decoder
- Research/elf/ghidra-rev067-consolidated-live-dispatch-model.md
- Research/elf/ghidra-rev072-callback-systems-descriptor-table-and-entry-table.md
- AGENTS.md primary context

---

## Evidence

### 1. Full Main Loop Dispatch Chain

The main loop at 0x00101C80 executes this 12-step pipeline every frame:

```txt
Step   Address   Function       Stack   Role
----   -------   --------       -----   ----
  1    0x1AA098  framebuf_reset   0     Clear flag block + HW register at 0x10000000
  2    0x166028  live_dispatch   96     Entity pool iteration, collect live entities
  3    0x103370  entity_regtab   64     Index entities into type-based slots (max 64)
  4    0x104C80  entity_xform   352     COP1-heavy matrix/transform prep (60.0, 20.0)
  5    0x1AF190  game_systems    48     Input polling, frame timer, audio tick
  6a   0x129A78  vu0_kick_cond   16     If gp-28384==0: cleanup + j 0x117768 (VU0 kick)
  6b   0x129AA8  scene_cleanup   80     If gp-28384==0: entity flags reset, counters
  7    0x13FBF8  →0x13F9D0      160     Callback dispatch (linked list A, two-level)
  8    0x129C90  scene_proc2     64     If gp-28384==0: additional scene processing
  9    0x13FC00  cb_dispatch2    —      Same as step 7 (second pass, full function)
 10    0x102680  table_clear    128     Zero 256-entry table, set completion flag
 11    0x13D3F0  vsync_wait      48     Busy-wait for VSync (idle loop)
```

**Critical observations:**

- **Step 2 (0x166028) runs BEFORE entity registration (step 3).** This means the live dispatch iterates the raw entity pool, while step 3 builds the indexed slot table afterward.
- **VU0 kick (step 6a) is CONDITIONAL** on gp-28384 == 0 (gameplay mode). During menus/cutscenes, VU0 is NOT kicked.
- **Callback dispatch (steps 7, 9) is UNCONDITIONAL** — runs every frame regardless of mode.
- **Steps 6-9 include BOTH** the game systems preamble (0x1AF190) AND the callback dispatch, suggesting the callbacks are entity simulation steps.

### 2. GP Variable System Map

```txt
Offset      Access  Functions      Role
------      ------  ---------      ----
-28384      R+BEQ   129A78/129AA8/129C90   Mode flag: 0=gameplay, else=menu/cutscene
-28176      R+BEQ   main loop               Loop termination flag
-28112      R+SW    102680                  Table iteration counter
-28108      SW      102680                  Table clear completion flag
-28084      R+SW    103370                  Entity registration counter (max 64)
-26936      R       1AF190                  Interrupt/timer state
-26404      R       13F9D0/13FC00           Callback bitmask (8 bits)
-26396      R       13F9D0/13FC00           Linked list head ptr
-25896      R+SW    166028                  Live dispatch counter (max 256)
-23628..-23648  SW  1AA098                  Flag block (7× zeros)
-23548      R+BNE   1AF190                  Frame counter
-23588      R+BNE   1AF190                  Input mode flag
```

### 3. Corrected Callback Mask Model

**Critical correction:** All previous Rev.064-067 mask values were WRONG. The bit positions are:

- **Upper nibble (bits 28-31):** major category selection
- **Second nibble (bits 24-27):** sub-category within major
- **Third nibble (bits 20-23):** further subdivision

The correct mask values (tested against struct+0x48 for Group 1, +0x60 for Group 2 slot 15):

```txt
Slot   Callback    Group   Mask Condition (hex)          Mask Value
----   --------    -----   -----------------------       ----------
  0    0x168DA8     G1     none (no flags test)           n/a
  1    0x168ED0     G1     flags & 0xF0000000 == 0       0xF0000000
                           AND flags & 0x000F0000 != 0x00010000
  2    0x1692F0     G1     same as slot 1, a2=1          same
  3    0x169440     G1     flags & 0xF0000000 == 0       0xF0000000
  4    0x169020     G1     same as slot 1, triplet guard same
  5    0x169190     G1     same as slot 3, triplet guard same
  6    0x1696C0     G1     flags & 0xC0000000 == 0x40000000  0xC0000000
  7    0x169580     G1     flags & 0x30000000 != 0       0x30000000
 10    0x169800     G1     flags & 0x70000000 == 0       0x70000000
                           AND flags & 0xC0000000 == 0x80000000
 11    0x169968     G1     flags & 0xC0000000 == 0xC0000000 0xC0000000
 12    0x169AA8     G2     none (no flags test)           n/a
 13    0x169BD0     G2     triplet guard only             n/a
 14    0x169E58     G2     none, a2=1                     n/a
 15    0x169D18     G2     flags+0x60 & 0x000F0000 == 0x00020000  0x000F0000
 16    0x169AA8     G2     same as slot 12                n/a
```

### 4. Secondary Pointer Table Location

```txt
Address:    0x00633D30 (.sbss+0x130)
Load:       lw $a1, -0x4BC0($gp)    # GP = 0x006388F0, -0x4BC0 = 0x00633D30
Type:       BSS — set at runtime by dispatcher init (0x166E10)
```

The table has two halves:

```txt
Offset  Group 1 usage            Group 2 usage
+0x10   struct array base        —
+0x14   —                        struct array base
+0x18   pointer array            —
+0x1C   —                        pointer array
```

### 5. Struct Field Maps

**80-byte struct (Group 1, stride 0x50):**

```txt
Offset  Size  Field
+0x00    2    Linked list entry (signed byte offset; <0 = chain end)
+0x02    2    Unknown
+0x08    4    First index/link
+0x10   16    Position data (consumed by Group1 template 0x166258)
+0x40    4    Template data
+0x44    4    Template data
+0x48    4    Flags — tested by slot-specific mask (all G1 slots except 0)
+0x74    4    Entity guard 1 (slots 4/5)
+0x78    4    Entity guard 2 (slots 4/5)
+0x7C    4    Entity guard 3 (slots 4/5)
```

**112-byte struct (Group 2, stride 0x70):**

```txt
Offset  Size  Field
+0x00    2    Linked list entry
+0x60    4    Flags — tested by slot 15 mask
```

### 6. Halfword Table and Linked List Flow

```txt
Halfword table base:  0x006AB080 (.bss)
Entry size:           16-bit index (0-1023)
Iteration limit:      counter at 0x00633D2C (.sbss+0x12C)

Data flow (same for all callbacks):
1. halfword = table[loop_counter]           # 16-bit index
2. ptr = pointer_array[halfword]            # from secondary_table+0x18 (G1) or +0x1C (G2)
3. For each entry in linked list at ptr:
   a. entry_val = *ptr (16-bit signed; <0 = chain end)
   b. struct_ptr = struct_array_base + entry_val  # entry_val is pre-multiplied by stride!
   c. Apply mask to struct_ptr->field_48 (G1) or field_60 (G2 slot 15)
   d. If passes: jal template (0x166258 G1, 0x1667E0 G2)
   e. ptr += 2 (next entry in list)
```

### 7. Context Store Pattern

```txt
Group 1 (success):  calling_ctx+0x88 = struct_ptr
                     calling_ctx+0x80 = a1 (caller param)
                     calling_ctx+0x84 = a2 (caller param)

Group 2 (success):  calling_ctx+0x94 = struct_ptr
                     calling_ctx+0x8C = a1
                     calling_ctx+0x90 = a2
                     calling_ctx+0x88 = 0     # Clear Group 1 result!
```

### 8. Dead `mult` Instructions Confirmed

All `mult $v0, 0x50` / `mult $v0, 0x70` instructions in callbacks (and at 0x1AF954) are **dead code** — compiler scheduling artifacts at `-O2`. The linked list entry values are already pre-multiplied byte offsets, not raw indices.

---

## What is Confirmed

- Full 12-step main loop pipeline from 0x101C80
- VU0 kick (0x117768) triggered by conditional step 6a when gp-28384==0
- GP variable map with 13 registered offsets
- Secondary pointer table at 0x00633D30 (BSS)
- All 14 callbacks disassembled with corrected mask values
- 80-byte and 112-byte struct field maps
- Halfword table → linked list → template dispatch data flow
- Dead mult instructions confirmed as compiler scheduling artifacts

## What is Probable

- The 4 major bit-groups of field_48 encode entity/object classification categories
- Group 1 callbacks process renderable/positionable objects (80-byte structs)
- Group 2 callbacks process logical/behavioral objects (112-byte structs)
- Slot mask values select subsets by classification hierarchy
- The triplet guard (slots 4/5/13) prevents re-processing the same entity
- Context+0x88/0x94 store results for downstream consumers

## What is Unknown

- How 0x00633D30 is populated at runtime (dispatcher init 0x166E10?)
- Semantic meaning of each 80-byte struct field
- What the halfword table index 0-1023 actually represents (entity ID? resource ID?)
- Why the linked list uses pre-multiplied byte offsets instead of indices
- Relationship between the linked list and the descriptor/entry tables

## What is Discarded

- Previous mask values using bits 12-15 (wrong bit range)
- The idea that the linked list uses raw indices (entry values are byte offsets)
- The idea that main loop functions at 0x103BF8/0x103FC0/0x101068/0x104D3C are independent functions (they are mid-function labels)
- Previous speculation about the live dispatch being entity/AI state management (it's a parallel processing system for structural objects)

---

## Next Steps

1. **Trace 0x00633D30 population:** Find the writers of this BSS address to understand how the secondary table is initialized
2. **Runtime validation target:** Confirm which slots fire during gameplay via `a1` capture at dispatch point 0x166E10+0x200
3. **Analyze the halfword table population:** Writers at 0x166D1C/0x166D78 (same function as dispatcher)
4. **Map the entity guard triplet:** Find what fields at context+0x74/78/7C are set by upstream code
5. **Identify entity pool writers:** Understand how the entity pool at 0x006AAC00 is populated
