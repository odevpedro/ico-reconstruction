#include "../types.h"
#include "structs.h"

void sub_10ECD8(void);
void sub_10ECB8(struct entity_context *);
void sub_1D1580(struct entity_context *);
void sub_1D19C0(struct entity_context *);

void girl_hA(struct entity_context *entity)
{
    sub_10ECD8();
    sub_10ECB8(entity);
    sub_1D1580(entity);
    sub_1D19C0(entity);
}

void sub_1504D8(ico_ptr32);
void sub_1C62D0(ico_ptr32, ico_ptr32, ico_ptr32);

void queen_hA(struct entity_context *entity)
{
    ico_ptr32 gp_data = *(ico_ptr32 *)0x007191F4;
    if (gp_data)
        sub_1504D8(gp_data);
    sub_10ECD8();
    sub_10ECB8(entity);

    struct entity_state_block *s = get_state_block(entity);
    ico_ptr32 es = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    ico_ptr32 transform = *(ico_ptr32 *)((u8 *)es + 0x844);

    if (*(u32 *)((u8 *)s + 0x0C))
        sub_1C62D0(*(ico_ptr32 *)((u8 *)s + 0x10), transform + 0x40, transform);
    sub_1C62D0(*(ico_ptr32 *)((u8 *)s + 0x14), transform + 0x40, transform);
}

ico_ptr32 sub_202208(void);
void sub_202148(struct entity_context *);
void sub_203AA0(u32);
void sub_203B78(ico_ptr32, u32);
ico_ptr32 sub_1E29E8(struct entity_context *, u32, ico_ptr32);

void queen_init(struct entity_context *entity)
{
    ico_ptr32 s1 = sub_202208();
    sub_202148(entity);
    sub_203AA0(1);

    sub_203B78(0x1A9F80, 20);
    sub_203B78(0x1ABCE0, 21);
    sub_203B78(0x1A9C30, 21);

    *(ico_ptr32 *)((u8 *)s1 + 0x120) = sub_1E29E8(entity, 238, (ico_ptr32)((u8 *)s1 + 0x610));

    ico_ptr32 es = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    *(u32 *)((u8 *)es + 0x7C) = 1;
}

void bga_init(struct entity_context *entity)
{
    (void)entity;
    sub_202208();
    sub_203AA0(1);
}

/* =================================================================
 * BIRD hC (0x197240) — NEAR-STRUCTURAL
 * Constructor for flyer entity. 64B heap alloc, quaternion init,
 * entity_state_reg with flyer params, random timer offset,
 * entity_dispatch_update tail call.
 * ================================================================= */
ico_ptr32 sub_13A0F8(ico_ptr32 heap, u32 size, ico_ptr32 tag, u32 line);
void sub_2641D8(ico_ptr32 dst, u32 fill, u32 size);
void sub_105F00(ico_ptr32 dst, ico_ptr32 src);
void sub_1E4798(struct entity_context *, u32, u32, u32, ico_ptr32, ico_ptr32);
u32 sub_118A68(void);
void sub_1D4B40(struct entity_context *, u32);

ico_ptr32 bird_hC(struct entity_context *entity, ico_ptr32 initializer)
{
    ico_ptr32 heap = *(ico_ptr32 *)0x00719720;
    ico_ptr32 alloc = sub_13A0F8(heap, 64, 0x0056AFD8, 978);

    sub_2641D8(alloc, 0, 64);
    sub_105F00(alloc, initializer);
    *(u8 *)(alloc + 0x10) = 0;

    sub_1E4798(entity, 2119, 2165, (u32)-1, (ico_ptr32)-1, (ico_ptr32)1073);

    {
        ico_ptr32 es = *(ico_ptr32 *)((u8 *)entity + 0x15C);
        *(u32 *)(es + 0x544) = 1;
        *(u32 *)(es + 0x54C) = 0;
        *(u32 *)(es + 0x548) = 1;
        *(u32 *)(es + 0x550) = 0;
    }

    {
        ico_ptr32 es = *(ico_ptr32 *)((u8 *)entity + 0x15C);
        float r = (float)sub_118A68() * 100.0f;
        *(float *)(es + 0x4AC) = r;
        *(float *)(es + 0x4B0) = r;
        *(u32 *)(es + 0x4C4) = 0;
    }

    sub_1D4B40(entity, 3);
    return alloc;
}

/* =================================================================
 * ATTACKCH idx 62 hC (0x1BBE50) — NEAR-STRUCTURAL
 * Constructor for attack chain child entity. 12B heap alloc with
 * backref to parent slot. Clears parent tracking during init.
 * ================================================================= */
ico_ptr32 attackch62_hC(struct entity_context *entity, ico_ptr32 initializer)
{
    (void)entity;
    ico_ptr32 heap = *(ico_ptr32 *)0x00719720;
    ico_ptr32 alloc = sub_13A0F8(heap, 12, 0x006285E8, 27);

    ico_ptr32 parent_slot = *(ico_ptr32 *)(initializer + 0x30);
    *(u32 *)(alloc + 0x00) = parent_slot;
    *(u32 *)(alloc + 0x04) = 0;
    *(u32 *)(alloc + 0x08) = 0;
    *(u32 *)parent_slot = 0;
    return alloc;
}

/* =================================================================
 * ENEMY1 hC (0x1CE220) — NEAR-STRUCTURAL
 *
 * Constructor for shadow enemy (ENEMY1, desc idx 4, init_fn=0x164440).
 * 80B heap alloc, 10-element child arrays, resource init, state reg.
 *
 * Control flow:
 *   1. Alloc 80B from heap → s0
 *   2. Zero-fill 80B (2x 32B chunks + 16B remainder = 3 stores)
 *   3. Copy initializer data (64B from initializer to alloc)
 *   4. sub_1CEF90(s0+0x30, 10, 4) — child array batch 1
 *   5. sub_1CEF90(s0+0x58, 10, 4) — child array batch 2
 *   6. sub_1CF288(6)              — resource/event register
 *   7. sub_1CEF90(s0+0x44, s1, 4) — child array from s1-sized batch
 *   8. Init fields at alloc+0x50..0x59, set alloc+0x18=0x3F
 *   9. entity_state_reg(entity, 0, s0) — store s0 as payload
 *  10. Tail: entity_dispatch_update(entity) — continue to update handler
 * ================================================================= */
ico_ptr32 sub_1CEF90(ico_ptr32 dst, ico_s32 count, u32 stride);
void sub_1CF288(u32 resource_id);
void sub_1E46B8(ico_ptr32 heap, ico_ptr32 alloc, u32 size, ico_ptr32 tag, u32 line);
void sub_202A18(struct entity_context *, u32, ico_ptr32);
void sub_106190(struct entity_context *);
ico_ptr32 sub_1D3B28(ico_ptr32);

// NOTE: This is an old incorrect draft. The correct decompilation is in enemy1.c.
ico_ptr32 enemy1_hC(struct entity_context *entity, ico_ptr32 initializer)
{
    ico_ptr32 heap = *(ico_ptr32 *)0x00719720;
    ico_ptr32 s0 = sub_13A0F8(heap, 80, 0x0056B160, 409);

    if (!s0)
        return 0;

    sub_2641D8(s0, 0, 80);
    sub_105F00(s0, initializer);

    sub_1CEF90(s0 + 0x30, 10, 4);
    sub_1CEF90(s0 + 0x58, 10, 4);

    sub_1CF288(6);

    {
        ico_s32 count = *(ico_s32 *)((u8 *)initializer + 0x18);
        if (!count) count = *(ico_s32 *)((u8 *)initializer + 0x1C);
        sub_1CEF90(s0 + 0x44, count, 4);
    }

    *(u32 *)((u8 *)s0 + 0x50) = 0;
    *(u32 *)((u8 *)s0 + 0x54) = 0;
    *(u32 *)((u8 *)s0 + 0x58) = 0;
    *(u8  *)((u8 *)s0 + 0x59) = 0;
    *(u8  *)((u8 *)s0 + 0x18) = 0x3F;
    *(u32 *)((u8 *)s0 + 0x4C) = 0;

    sub_202A18(entity, 0, s0);
    sub_106190(entity);
    return s0;
}

/* =================================================================
 * WOODBOX0 hC (0x1C00C0) — ASM-HOLD
 *
 * Constructor for breakable crate (WOODBOX0, desc idx 17, init_fn=0x17D1D0).
 * ~286 instructions, 400B heap alloc with data copy from 0x4DF560.
 *
 * Key pattern (partial):
 *   1. Alloc 400B from heap → s1
 *   2. Zero-fill 400B (large block via sub_2641D8)
 *   3. Data copy: 384B from 0x4DF560 to s1+0x10 (96 word loop via lw/sw)
 *   4. Init fields at s1+0x04, s1+0x0C, s1+0x08
 *   5. sub_1B7FE8(entity, 0x1C6F40, ...) — spawn child entity
 *   6. sub_1CEF90 × multiple calls for child arrays
 *   7. Multiple resource registrations (sub_1CF288)
 *   8. sub_202A18(entity, 0, s1) — reg payload
 *   9. Tail: entity_dispatch_update
 *
 * Data table at 0x4DF560 (384 bytes, 8 entries × 48B):
 *   Offset 0x00..0x0F: header (zero for all entries)
 *   Offset 0x10: float param1 (0.0 or 1.0f)
 *   Offset 0x14: float param2 (0.0 or 1.0f)
 *   Offset 0x18: float param3 (250.0f for active entry)
 *   Offset 0x1C: s32 count/ID (288 for model entry)
 *   Offset 0x20: char name[28] (model path or "NULL")
 *
 *   Entry 0 (0x4DF560): "NULL" at +0x14 (no model)
 *   Entry 1 (0x4DF590): "NULL" at +0x14
 *   Entry 2 (0x4DF5C0): "NULL" at +0x14
 *   Entry 3 (0x4DF5F0): all zeros
 *   Entry 4 (0x4DF620): 1.0f, 1.0f, 250.0f, 288, "object/sdf/st00a/model/0str16.p2o"
 *   Entry 5 (0x4DF650): continuation + trailing "NULL"
 *   Entry 6 (0x4DF680): "NULL"
 *   Entry 7 (0x4DF6B0): "NULL"
 *
 * NOT YET written as C model. Function is too large for NEAR-STRUCTURAL
 * without deeper analysis of the data copy loop and model path string handling.
 * The hB (0x1C0538, 27 insns) and hA (0x1C05D0, 28+ insns) are shorter.
 * ================================================================= */
/* TO DO: write WOODBOX0 hC C model after full disassembly of the 286-instruction
   body, especially the data copy loop and model path string/table usage. */
