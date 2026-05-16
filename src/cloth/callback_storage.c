#include "structs.h"
#include "../types.h"

/* =================================================================
 * CORRECTED MODEL (Rev.060)
 *
 * The previous Rev.059 model described 0x13F3F0 as a linked-list
 * allocator with stride 0x94. This is INCORRECT in one important way:
 *
 * 0x13F3F0 is a SLOT-BASED POOL MANAGER with stride 0x94.
 * 0x1A6E28 is a DISABLED PRINT STUB (32 bytes, saves regs + returns).
 * The real heap_alloc happens INSIDE 0x13D1B0 via 0x13A0F8.
 *
 * Pool:  gp[-19528] = base pointer, gp[-19524] = capacity
 * Slot is free when [+0x00] == 0 (self-pointer cleared).
 *
 * Each slot links into a per-entity sorted DOUBLY LINKED LIST
 * sorted by t0 (key/priority) at slot[+0x14].
 * ================================================================= */

/* Confirmed pool slot layout (stride 0x94 = 148 bytes) */
struct callback_pool_slot {
    ico_u32 self;           /* +0x00: non-zero = slot in use */
    ico_ptr32 entity;       /* +0x04: entity backref */
    ico_ptr32 next;         /* +0x08: next in per-entity sorted list */
    ico_ptr32 prev;         /* +0x0C: prev in per-entity sorted list */
    ico_u32  flags;         /* +0x10: 0 = heap alloc, non-0 = inline store */
    ico_s32  key;           /* +0x14: priority from t0, sorted ascending */
    ico_u32  active;        /* +0x18: 1 = active */
    ico_ptr32 data;         /* +0x1C: data pointer (callback info) */
    ico_u32  pad_20;        /* +0x20: unused */
    ico_u8   payload[0x70]; /* +0x24: heap-alloc'd payload or inline data */
}; /* total: 0x94 */

/* Per-entity list: entity[+0x2C] = head, entity[+0x30] = tail */

/* =================================================================
 * ASM-HOLD — callback_pool_alloc (0x0013F3F0)
 *
 * 576 bytes (0x240). Stack: 144 bytes (0x90).
 *
 * Slot-based pool allocator with per-entity sorted linked list.
 * Calls 0x13D1B0 (heap_alloc via 0x13A0F8, tag 173, "NodeCallback")
 * and 0x13D3C8 (kernel queue insert via 0x100340).
 *
 * Four assert calls to 0x1A6E28 (disabled print stub) for error paths.
 * ================================================================= */

/* =================================================================
 * ASM-HOLD — callback_registration (0x0013F7A8)
 *
 * 44 bytes. Tail-calls 0x13F3F0 with:
 *   a0 = entity (preserved)
 *   a1 = type (a3 → 0x13 for cloth)
 *   a2 = data (original a1)
 *   a3 = &spill (non-zero → inline storage path)
 *
 * Waits: first call 0x13F3F0, then second call 0x13F3F0(a0=entity+0x10)
 * for the "sister" storage. The second call registers at entity+0x10
 * as well, meaning two separate pool slots for the same callback.
 * ================================================================= */

/* =================================================================
 * ASM-HOLD — callback_system_reg (0x0013F7D8)
 *
 * 36 bytes. Context-free system registration:
 *   a0 = 0x194 (hardcoded system object ID)
 *   a1 = 0 (no type filter)
 *   t1 = 0x1800 (6144 bytes heap alloc size)
 *   Only ONE call to 0x13F3F0 (no sister call)
 * ================================================================= */
