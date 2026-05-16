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
