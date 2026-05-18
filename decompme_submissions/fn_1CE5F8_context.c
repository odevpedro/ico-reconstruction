typedef unsigned char u8;
typedef unsigned int u32;
typedef int ico_ptr32;

struct entity_context;

#define ENTITY_STATE_OFFSET 0x15C
#define STATE_BLOCK_OFFSET  0x800

void sub_1224E0(ico_ptr32);
void sub_1CF770(ico_ptr32);
void sub_1CDB28(struct entity_context *);
void sub_1CF998(ico_ptr32);

struct enemy1_work {
    u32 count_init;
    u32 next_free;
    ico_ptr32 result_08;
    ico_ptr32 field_0C;
    u32 cleanup_flag;
    ico_ptr32 child_array;
    ico_ptr32 child1;
    u32 field_1C;
    ico_ptr32 child2;
    u32 field_24;
    ico_ptr32 model;
    u32 anim_guard;
    u32 field_30;
    u32 field_34;
    u32 cleanup_state;
    u32 field_3C;
    unsigned short field_40;
    u32 field_44;
    float phase;
    u32 state_counter;
};

void fn_1CE5F8(struct entity_context *entity)
{
    struct enemy1_work *wk;

    wk = (struct enemy1_work *)(
        *(ico_ptr32 *)((u8 *)(*(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET)) + STATE_BLOCK_OFFSET)
    );

    if (wk->cleanup_state) {
        __asm__ __volatile__("nop");
        sub_1224E0(*(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET));
        __asm__ __volatile__("nop");
        if (*(float *)((u8 *)(*(ico_ptr32 *)((u8 *)(*(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET)) + 0x840)) + 0x30) == 0.0f) {
            __asm__ __volatile__("nop");
            sub_1CF998(wk->child1);
            sub_1CF998(wk->child2);
        }
    }

    sub_1CF770(wk->model);

    {
        register u32 cleanup_flag asm("$3");
        cleanup_flag = wk->cleanup_flag;
        if (cleanup_flag)
            sub_1CDB28(entity);
    }
}
