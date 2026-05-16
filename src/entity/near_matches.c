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
