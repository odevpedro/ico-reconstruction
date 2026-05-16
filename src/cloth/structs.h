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
 * Physics object type table entry (Rev.049)
 * Base: 0x001A48A0, stride: 0x64, 31+ entries
 * Each entry has 3 handler functions for a physics object type.
 * ================================================================= */
struct physics_type_entry {
    ico_u32 count;          /* +0x00: usually 1 */
    ico_ptr32 handler_a;    /* +0x04: post-dispatch / cleanup */
    ico_ptr32 pad_08;       /* +0x08: null */
    ico_ptr32 handler_b;    /* +0x0C: update / main callback */
    ico_ptr32 pad_10;       /* +0x10: null */
    ico_ptr32 handler_c;    /* +0x14: init / payload init */
    ico_u32  pad_18;        /* +0x18: null */
    ico_u32  pad_1C;        /* +0x1C: null */
    char     name[8];       /* +0x20: e.g. "ROPE\0\0\0\0" */
};

/* Known physics type names (from Rev.049 scan):
 *   WOODBOX01, ROTOBJEC, BARREL, ROPE, CHAIN,
 *   FLEVER, WLEVER, NONE, CAMERADU, SEFFECT,
 *   BIRD, GENERATO, CANDLE, MOBJ, CHANDELI,
 *   WORM, POOL, DARKVOLU, MCOLTEST, ROPEFIX,
 *   CAGE, DYNAMICM, QUEEN, QUEENDEM, CAGEFIX, CLOTHTES
 */

#endif /* ICO_CLOTH_STRUCTS_H */
