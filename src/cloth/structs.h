#ifndef ICO_CLOTH_STRUCTS_H
#define ICO_CLOTH_STRUCTS_H

#include "../types.h"

/* =================================================================
 * Cloth struct hierarchy (reverse-engineered, Rev.048-049)
 *
 * context -> entity -> payload (cloth instance data)
 *
 * Offsets verified by 3 EXACT and 5 NEAR-STRUCTURAL C matches
 * against the original EE GCC 2.9-991111-01 binary.
 * ================================================================= */

/* Cloth payload: per-instance data for a cloth/rope object.
 * Initialized by cloth_payload_init (0x001D27A8).
 * Total size unknown (>32 words observed at runtime).
 */
struct cloth_payload {
    ico_u32 field_00;       /* +0x00: tested < 1 */
    ico_s32 variant_id;     /* +0x04: 0 (area A) or 1 (area B) */
    ico_u64 flag_08;        /* +0x08: non-zero = inactive/skip */
    ico_u8  pad_10[0x30];   /* +0x10 to +0x3F: gap */
    ico_s32 field_40;       /* +0x40: returned when variant == 1 */
    ico_u32 state_id;       /* +0x48: 0-4, indexes dispatcher jump table */
};

/* Cloth entity: entity struct with cloth payload pointer.
 * Part of a larger entity struct; only cloth-relevant fields mapped.
 */
struct cloth_entity {
    ico_u8  pad_000[0x800]; /* +0x000 to +0x7FF */
    ico_ptr32 payload;      /* +0x800: points to cloth_payload */
};

/* Cloth context: per-frame context passed to cloth functions.
 * The context is usually a0 of the cloth update function.
 */
struct cloth_context {
    ico_u8  pad_15C[0x15C]; /* +0x000 to +0x15B */
    ico_ptr32 entity;       /* +0x15C: points to cloth_entity */
    ico_ptr32 extra;        /* +0x16C: auxiliary ptr (can be NULL) */
};

/* =================================================================
 * Callback storage node (Rev.059)
 * Managed by 0x13F3F0 (576 bytes). Linked list with stride 0x94.
 * The system at 0x13F7A8 registers callbacks of type 0x13 (cloth).
 * +0x1C in the containing struct is the direct callback slot
 * (maps to ThreadParam.entry in EE SDK).
 * ================================================================= */
struct callback_storage_node {
    ico_u64  next;          /* +0x00: linked list next ptr (8 bytes, ld/sd) */
    ico_ptr32 data;          /* +0x08: callback data pointer */
    ico_u32  type;           /* +0x0C: callback type (e.g. 0x13 = cloth) */
    ico_u8   body[0x84];    /* +0x10: node body (132 bytes, sep alloc) */
}; /* total: 0x94 bytes */

/* Node allocated via 0x1A6E28 (x2 calls: node + body).
 * 3-node scan limit. Fallback: [obj+0x1C] = data.
 */

#endif /* ICO_CLOTH_STRUCTS_H */
