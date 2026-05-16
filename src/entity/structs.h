#ifndef ICO_ENTITY_STRUCTS_H
#define ICO_ENTITY_STRUCTS_H

#include "../types.h"

#define DESC_TABLE_BASE  0x002A31B8
#define DESC_STRIDE      0x64
#define DESC_COUNT       68

#define ENTRY_TABLE_BASE 0x002A4C48
#define ENTRY_STRIDE     0x4C

struct descriptor_record {
    char  name[8];
    u8    pad_08[0x18];
    u8    phy_type;
    u8    pad_21[7];
    float scale;
    float damping;
    u32   count;
    ico_ptr32 init_fn;
    u32   flags;
    ico_ptr32 handler_a;
    u32   pad_4C;
    ico_ptr32 handler_b;
    u32   pad_54;
    ico_ptr32 handler_c;
    u32   pad_5C;
};

struct entry_record {
    float pos_x;
    float pos_y;
    float pos_z;
    float rot;
    float scale_a;
    float scale_b;
    float unk_18;
    float unk_1C;
    float unk_20;
    ico_ptr32 callback_override;
    ico_ptr32 unk_28;
    s32  param_2C;
    s32  param_30;
    s32  unk_34;
    s32  unk_38;
    s32  unk_3C;
    u16  flags_hi;
    u16  unk_42;
    u16  unk_44;
    u8   descriptor_idx;
    u8   subtype;
    u32  flags;
};

struct entity_state_block {
    u32 variant;
    u32 active;
    u32 state_id;
};

struct entity_context {
    u8  pad_000[0x15C];
    ico_ptr32 entity_state;
    ico_ptr32 entity_alt;
    ico_ptr32 extra;
};

#define ENTITY_STATE_OFFSET  0x15C
#define STATE_BLOCK_OFFSET   0x800

static inline struct descriptor_record *get_descriptor(int idx)
{
    return (struct descriptor_record *)(DESC_TABLE_BASE + idx * DESC_STRIDE);
}

static inline struct entity_state_block *get_state_block(struct entity_context *ctx)
{
    ico_ptr32 es = *(ico_ptr32 *)((u8 *)ctx + ENTITY_STATE_OFFSET);
    ico_ptr32 sb = *(ico_ptr32 *)((u8 *)es + STATE_BLOCK_OFFSET);
    return (struct entity_state_block *)sb;
}

#endif
