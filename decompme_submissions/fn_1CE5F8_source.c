void fn_1CE5F8(struct entity_context *);
void sub_1224E0(ico_ptr32);
void sub_1CF770(ico_ptr32);
void sub_1CDB28(struct entity_context *);
void sub_1CF998(ico_ptr32);

// hA helpers
void sub_1BB7E0(void);
u32 sub_165F88(struct entity_context *);

// Matrix/quaternion
void sub_104360(ico_ptr32, ico_ptr32);

// GP-relative globals
#define GP_BASE         0x00633D14
#define HEAP_PTR        (*(ico_ptr32 *)(GP_BASE - 0x68E0))
#define COUNTER_SEED    (*(u32 *)(GP_BASE - 0x551C))
#define FLOAT_CONST     (*(float *)(GP_BASE - 0x7BAC))
#define DEATH_FLOAT     (*(float *)(GP_BASE - 0x7BA8))
#define ENTITY6E_PTR    (*(ico_ptr32 *)(GP_BASE - 0x5B84))
#define GLOBAL_FLAG     (*(ico_ptr32 *)(GP_BASE - 0x5BF0))

// ============================================================================
// Struct: private work area per ENEMY1 instance
// Size: 0x50 (80 bytes), heap-allocated in hC
// Stored at: scene_obj[0x800]
// ============================================================================
struct enemy1_work {
    u32    count_init;      // +0x00: initializer[0x30] (instance count)
    u32    next_free;       // +0x04: always 0 (slot index)
    ico_ptr32 result_08;    // +0x08: from sub_1CD9B0
    ico_ptr32 field_0C;     // +0x0C: unused
    u32    cleanup_flag;    // +0x10: if nonzero, fn_1CE5F8 -> sub_1CDB28
    ico_ptr32 child_array;  // +0x14: array of child ptrs (len: scene_obj[0x88])
    ico_ptr32 child1;       // +0x18: child sprite handle 1
    u32    field_1C;        // +0x1C: 0
    ico_ptr32 child2;       // +0x20: child sprite handle 2
    u32    field_24;        // +0x24: 0
    ico_ptr32 model;        // +0x28: model handle (sub_1CF288(6))
    u32    anim_guard;      // +0x2C: 1; checked != 0 in pose update
    u32    field_30;        // +0x30: unused
    u32    field_34;        // +0x34: unused
    u32    cleanup_state;   // +0x38: checked in fn_1CE5F8; if set -> sub_1224E0
    u32    field_3C;        // +0x3C: 0
    u16    field_40;        // +0x40: 0 (halfword)
    u32    field_44;        // +0x44: 0
    float  phase;           // +0x48: animation phase (blend factor)
    u32    state_counter;   // +0x4C: 0..10 frame counter
};
