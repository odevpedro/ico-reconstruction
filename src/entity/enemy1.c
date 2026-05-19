// Near-structural decompilation for ENEMY1 (descriptor index 4)
// Based on MIPS64 disassembly (ee-gcc 2.9-991111-01, -O2)
// ELF: SCUS_971.13, single PT_LOAD segment (VA 0x100000 -> file 0x1000)
//
// Functions:
//   init_fn  (0x164440) -- full entity init via descriptor [+0x00]; 278 insns
//   hC       (0x1CE220) -- per-instance constructor  via descriptor [+0x58]; 103 insns
//   hB       (0x1CE3C0) -- per-frame AI+draw          via descriptor [+0x50]; 141 insns
//   hA       (0x1CE690) -- conditional mask handler    via descriptor [+0x48]; 22 insns
//   fn_1CE5F8           -- destruction/cleanup (tail-called from hA); 37 insns
//
// Key observations (Rev.075 confirmed):
//   ENEMY1 is the ONLY descriptor that uses entry flags_0x48 for
//   mask-based slot selection in the live dispatch system (0x166E10).
//
//   hB state machine counter (wk+0x4C, range 0..10):
//     - Entity flags bit 33 set  --> counter resets to 0 (forced revive)
//     - Otherwise increment each frame; >= 11 skips all processing
//
//   wk struct allocated 0x50 bytes, stored at scene_obj[0x800]

#include "../types.h"
#include "structs.h"

// ============================================================================
// Forward declarations
// ============================================================================

// Allocator
ico_ptr32 sub_13A0F8(ico_ptr32 heap, u32 size, ico_ptr32 tag, u32 line);

// Entity core init
ico_ptr32 sub_202208(struct entity_context *);
void sub_202148(struct entity_context *);
void sub_203918(struct entity_context *);
void sub_203910(struct entity_context *);
void sub_203AA0(u32);
void sub_203B78(ico_ptr32, u32);
void sub_1A6E28(ico_ptr32, struct entity_context *);
void sub_15F6F0(struct entity_context *);

// Model/animation
void sub_14B580(struct entity_context *);
void sub_14B1D0(struct entity_context *);
void sub_14B260(struct entity_context *);
void sub_15BCC8(struct entity_context *, u32);
ico_ptr32 sub_1E69D8(u32);
float sub_14B358(void);
ico_ptr32 sub_1CEC60(struct entity_context *);
void sub_1CEC88(struct entity_context *);
void sub_1651C8(struct entity_context *);

// Enemy1 lifecycle
ico_ptr32 sub_1CEF90(s32, s32, s32);
ico_ptr32 sub_1CF288(s32);
ico_ptr32 sub_1CD9B0(struct entity_context *, ico_ptr32);
void sub_1E4798(struct entity_context *, u32, u32, u32, ico_ptr32, ico_ptr32);
void sub_1D4B40(struct entity_context *, u32, u32);

// AI / draw
void sub_1E3FC8(struct entity_context *);
void sub_103F00(struct entity_context *, u32, u32, float, float, float);
u32 sub_1654C8(struct entity_context *);
u32 sub_165540(struct entity_context *);
void sub_104940(ico_ptr32, ico_ptr32, ico_ptr32);
void sub_1CF6C0(ico_ptr32, ico_ptr32);
void sub_1CF548(ico_ptr32);
ico_ptr32 sub_105278(void);
u32 sub_109F10(struct entity_context *, u32);
void sub_1185D0(ico_ptr32, ico_ptr32, ico_ptr32);
void sub_1CF930(ico_ptr32, ico_ptr32, float);

// Destruction / cleanup
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

// Scene_obj offsets used by ENEMY1:
// +0x0C  : model_transform_base
// +0x88  : child_count (for child array)
// +0x220 : model_frame_index
// +0x548 : ai_flag_548 (reset 0)
// +0x54C : ai_param_54C (set 2)
// +0x550 : ai_flag_550 (0 or 1 based on ai_mode==3)
// +0x558 : counter_mod10
// +0x63C : optional_anim_ptr
// +0x800 : enemy1_work* (this alloc)
// +0x840 : collision_info* (vec3 at +0x20, float at +0x30)
// +0x45C-0x468 : death pos 4-floats

// ============================================================================
// hA (0x1CE690) -- conditional mask handler
//
// Called from live dispatch when mask selects this path.
// Dispatch table slot selection for ENEMY1 uses flags bit 33:
//   - bit 33 clear --> return immediately (skip destruction)
//   - bit 33 set   --> check readiness; if ready, cleanup
//
// The flag bit is managed by init_fn based on entry flags_0x48 bit 18.
// ENEMY1 is the ONLY descriptor using this mechanism (Rev.075).
// ============================================================================
void enemy1_hA(struct entity_context *entity)
{
    ico_ptr32 inner = *(ico_ptr32 *)((u8 *)entity + 0x164);
    ico_u64 flags = *(ico_u64 *)((u8 *)inner + 0x18);

    if (!((flags >> 33) & 1))
        return;

    sub_1BB7E0();
    __asm__ __volatile__("nop");
    if (sub_165F88(entity))
        return;

    fn_1CE5F8(entity);
}

// ============================================================================
// fn_1CE5F8 -- destruction/cleanup
//
// Tail-called from hA when entity is ready to die.
// 1. Conditional: sub_1224E0 if cleanup_state set
// 2. Destroy child sprites if coll->0x30 == 0.0
// 3. Release model resource
// 4. Full entity destruction if cleanup_flag set
//
// NOTE: asm nop barriers used to match target's delay slot scheduling.
// ee-gcc 2.9 fills delay slots aggressively; target uses .set noreorder.
// ============================================================================
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

// ============================================================================
// hC (0x1CE220) -- per-instance constructor
//
// Called from descriptor[+0x58]. Returns enemy1_work ptr.
// 1. Allocate 80B work struct
// 2. Create 2 child sprites (sub_1CEF90)
// 3. Create model resource (sub_1CF288)
// 4. Allocate/init child pointer array from scene definition
// 5. Init model/animation params
// 6. Seed mod-10 counter: (seed + 2) % 10
// ============================================================================
ico_ptr32 enemy1_hC(struct entity_context *entity, ico_ptr32 initializer)
{
    struct enemy1_work *wk;
    ico_ptr32 scene_obj;
    s32 child_count;
    ico_ptr32 buffer;
    s32 count_from_def;
    ico_ptr32 alloc;
    int i;

    alloc = sub_13A0F8(HEAP_PTR, 0x50, (ico_ptr32)0x00618CF0, 0x25D);
    wk = (struct enemy1_work *)alloc;

    scene_obj = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    *(ico_ptr32 *)((u8 *)scene_obj + STATE_BLOCK_OFFSET) = alloc;

    wk->field_1C = 0;
    wk->child1 = sub_1CEF90(10, 0, 10);
    wk->field_24 = 0;
    wk->child2 = sub_1CEF90(10, 0, 10);
    wk->model = sub_1CF288(6);

    wk->cleanup_state = 0;
    wk->anim_guard = 1;
    wk->state_counter = 0;
    wk->cleanup_flag = 0;
    wk->field_3C = 0;
    wk->field_40 = 0;
    wk->field_44 = 0;

    count_from_def = *(s32 *)((u8 *)initializer + 0x30);
    child_count = *(s32 *)((u8 *)scene_obj + 0x88);
    buffer = sub_13A0F8(HEAP_PTR, child_count * 4, (ico_ptr32)0x00618CF0, 0x104);
    wk->child_array = buffer;

    wk->count_init = count_from_def;
    if (child_count > 0)
        for (i = 0; i < child_count; i++)
            *(u32 *)((u8 *)buffer + i * 4) = 0;
    wk->next_free = 0;

    wk->result_08 = sub_1CD9B0(entity, count_from_def);
    sub_1E4798(entity, 0x71C, 0x839, 0x18, (ico_ptr32)0x24, (ico_ptr32)0x314);

    {
        u32 seed = COUNTER_SEED;
        u32 rem;
        *(u32 *)((u8 *)scene_obj + 0x558) = seed;
        rem = (seed + 2) % 10;
        COUNTER_SEED = rem;
        *(u32 *)((u8 *)scene_obj + 0x550) = 0;
        sub_1D4B40(entity, 2, rem);
    }

    return alloc;
}

// ============================================================================
// hB (0x1CE3C0) -- per-frame update (AI + draw)
//
// Main ENEMY1 processing. 141 insns, called each frame via [+0x50].
//
// Structure:
//   1. State counter: 11-frame active window (bit 33 resets)
//   2. AI state: scene_obj 0x548/0x54C/0x550
//   3. Shadow draw via sub_1E3FC8
//   4. Animation blend: phase * scale {70, 50, 0.5}
//   5. Conditional pose: anim_ptr + guard -> compute matrix, apply
//   6. Model draw sub_1CF548
//   7. Mod-10 counter increment
//   8. Average (x+y+z)/3 from collision data -> sprite brightness
//   9. Two sprites: attr 0x24 (child1), attr 0x25 (child2)
//
// Sprite model ref: 0x004C0DA0
// Frame = scene_obj->base_ptr + attr_result * 64
// ============================================================================
void enemy1_hB(struct entity_context *entity)
{
    ico_ptr32 scene_obj;
    ico_ptr32 inner;
    ico_u64 flags;
    struct enemy1_work *wk;
    u32 ai_mode;
    float phase;
    ico_u64 transform_buf[4];  // 32B temp for pose matrix

    inner = *(ico_ptr32 *)((u8 *)entity + 0x164);
    scene_obj = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    flags = *(ico_u64 *)((u8 *)inner + 0x18);
    wk = (struct enemy1_work *)(*(ico_ptr32 *)((u8 *)scene_obj + STATE_BLOCK_OFFSET));

    if ((flags >> 33) & 1) {
        wk->state_counter = 0;
    } else {
        if (wk->state_counter >= 11)
            return;
        wk->state_counter++;
    }

    *(u32 *)((u8 *)scene_obj + 0x550) = 0;
    *(u32 *)((u8 *)scene_obj + 0x54C) = 2;
    *(u32 *)((u8 *)scene_obj + 0x548) = 0;

    ai_mode = sub_1654C8(entity);
    if (ai_mode == 3)
        *(u32 *)((u8 *)scene_obj + 0x550) = 1;

    sub_1E3FC8(entity);

    phase = wk->phase;
    sub_103F00(entity, 4, 0, phase * 70.0f, phase * 50.0f, 0.5f);

    if (sub_165540(entity)) {
        ico_ptr32 anim_ptr = *(ico_ptr32 *)((u8 *)scene_obj + 0x63C);
        if (anim_ptr && wk->anim_guard) {
            u32 frm_idx = *(u32 *)((u8 *)scene_obj + 0x220);
            ico_ptr32 base = *(ico_ptr32 *)((u8 *)scene_obj + 0x0C);
            ico_ptr32 matrix = (ico_ptr32)((u8 *)base + frm_idx * 64 + 0x30);

            sub_104940((ico_ptr32)transform_buf,
                       (ico_ptr32)((u8 *)scene_obj + 0x1D0),
                       matrix);
            sub_1CF6C0(wk->model, (ico_ptr32)transform_buf);
        }
    }

    sub_1CF548(wk->model);

    {
        u32 ctr = *(u32 *)((u8 *)scene_obj + 0x558);
        *(u32 *)((u8 *)scene_obj + 0x558) = (ctr + 1) % 10;
    }

    {
        ico_ptr32 coll = *(ico_ptr32 *)((u8 *)scene_obj + 0x840);
        float *pos = (float *)((u8 *)coll + 0x20);
        float avg = (pos[0] + pos[1] + pos[2]) / 3.0f;
        ico_ptr32 sprite_ref = (ico_ptr32)0x004C0DA0;
        ico_ptr32 base = *(ico_ptr32 *)((u8 *)scene_obj + 0x0C);
        u32 fi;
        ico_ptr32 si;

        si = sub_105278();
        fi = sub_109F10(entity, 0x24);
        sub_1185D0(si, (ico_ptr32)((u8 *)base + fi * 64), sprite_ref);
        sub_1CF930(wk->child1, sub_105278(), avg);

        si = sub_105278();
        fi = sub_109F10(entity, 0x25);
        sub_1185D0(si, (ico_ptr32)((u8 *)base + fi * 64), sprite_ref);
        sub_1CF930(wk->child2, sub_105278(), avg);
    }
}

// ============================================================================
// init_fn (0x164440) -- full entity initializer
//
// Called once per entity via descriptor [+0x00] during scene setup.
// Heavier than hC. Handles:
//   1. Core entity data alloc (sub_202208)
//   2. Three-pass model parameter setup (sub_14B358 x3)
//   3. Parameter block copy (128B from save area to stack base)
//   4. Scene object field population
//   5. flags_0x48 processing (ENEMY1-specific bit 18 check)
//   6. Callback registrations (sub_203B78 x{0,3,4})
//   7. Scale init with 10.0f minimum clamp
//   8. Optional quaternion transform (s0_flag controlled)
//
// flags_0x48 processing (at 0x164730, ENEMY1-only):
//   Loads entry_record[type]->flags at +0x48
//   Checks bit 18 -> s0_flag (controls quaternion init at +0x16482C)
// ============================================================================
void enemy1_init(struct entity_context *entity)
{
    ico_ptr32 alloc;
    s32 entity_type;
    struct entity_context *ec;
    s32 s0_flag;
    float f20_val;
    ico_ptr32 res;
    ico_ptr32 inner;
    ico_ptr32 scene_obj;
    u32 param[64];
    int i;

    ec = entity;

    sub_1A6E28((ico_ptr32)0x005591D8, ec);
    alloc = sub_202208(ec);
    sub_202148(ec);
    sub_203918(ec);
    sub_203910(ec);

    inner = *(ico_ptr32 *)((u8 *)ec + 0x164);
    scene_obj = *(ico_ptr32 *)((u8 *)ec + ENTITY_STATE_OFFSET);

    entity_type = *(s32 *)((u8 *)ec + 0x08);
    if (entity_type == 0xD7D) {
        ico_u64 fl = *(ico_u64 *)((u8 *)alloc + 0x20);
        fl |= (ico_u64)0x20000;
        *(ico_u64 *)((u8 *)alloc + 0x20) = fl;
    }

    sub_14B580(ec);
    f20_val = FLOAT_CONST;
    sub_14B1D0(ec);
    sub_14B260(ec);

    res = sub_1E69D8(1);
    {
        ico_ptr32 p = *(ico_ptr32 *)((u8 *)inner + 0x678);
        *(ico_ptr32 *)((u8 *)p + 0x378) = res;
    }

    // --- 3 model parameter setup calls ---
    // Params written at param[0x80/4..0xFF/4], then copied to param[0..0x7F]

    sub_14B358();  // call 1: attr 0x12, params at 0x80+ area
    param[0xA8 / 4] = 0x13;
    param[0xAC / 4] = 0x32;
    param[0x94 / 4] = 0;  // result of call 1
    param[0x98 / 4] = *(u32 *)&f20_val;
    param[0x9C / 4] = 1;
    param[0xA0 / 4] = 1;
    param[0xA4 / 4] = 0;
    param[0xB0 / 4] = 0;

    sub_14B358();  // call 2: attr 0x13

    param[0xD0 / 4] = 0x32;
    param[0xC0 / 4] = 2;
    param[0xC8 / 4] = 0x16;
    param[0xB4 / 4] = 0;  // result of call 2
    param[0xB8 / 4] = *(u32 *)&f20_val;
    param[0xBC / 4] = 0;
    param[0xC4 / 4] = 0x1D;
    param[0xCC / 4] = 0x19;

    sub_14B358();  // call 3: attr 0x14

    param[0xD4 / 4] = 0;  // result of call 3
    param[0xE0 / 4] = 2;
    param[0xE4 / 4] = 0x1C;
    param[0xE8 / 4] = 0x16;
    param[0xD8 / 4] = *(u32 *)&f20_val;
    param[0xDC / 4] = 0;
    param[0xF0 / 4] = 100;
    param[0xF4 / 4] = 0x7F7FFFFF;  // FLT_MAX from 0x006323AC
    param[0xF8 / 4] = 0x43AB8000;  // 343.0f as u32 bits
    param[0xFC / 4] = 1;
    param[0xEC / 4] = 0;

    // Copy 128 bytes from param+0x80 to param (4 x 32B loop)
    for (i = 0; i < 4; i++) {
        ico_u64 w0 = *(ico_u64 *)((u8 *)param + 0x80 + i * 32 + 0);
        ico_u64 w1 = *(ico_u64 *)((u8 *)param + 0x80 + i * 32 + 8);
        ico_u64 w2 = *(ico_u64 *)((u8 *)param + 0x80 + i * 32 + 16);
        ico_u64 w3 = *(ico_u64 *)((u8 *)param + 0x80 + i * 32 + 24);
        *(ico_u64 *)((u8 *)param + i * 32 + 0) = w0;
        *(ico_u64 *)((u8 *)param + i * 32 + 8) = w1;
        *(ico_u64 *)((u8 *)param + i * 32 + 16) = w2;
        *(ico_u64 *)((u8 *)param + i * 32 + 24) = w3;
    }

    // Store model params (from copied block at param+0x00)
    s0_flag = 1;
    {
        ico_ptr32 p = *(ico_ptr32 *)((u8 *)inner + 0x670);
        ico_ptr32 coll = *(ico_ptr32 *)((u8 *)scene_obj + 0x840);

        *(u32 *)((u8 *)p + 0x1DC) = s0_flag;
        *(float *)((u8 *)p + 0x1D8) = *(float *)((u8 *)coll + 0x20);
        *(u32 *)((u8 *)p + 0x1E0) = param[0x20 / 4];
        *(u32 *)((u8 *)p + 0x1E8) = param[0x24 / 4];
        *(float *)((u8 *)alloc + 0x1D4) = *(float *)&param[0x34 / 4];
        *(float *)((u8 *)alloc + 0x1D4) += *(float *)&param[0x38 / 4];
        *(u32 *)((u8 *)p + 0x1EC) = param[0x28 / 4];
        *(u32 *)((u8 *)p + 0x1F0) = param[0x2C / 4];
        *(u32 *)((u8 *)p + 0x1F4) = param[0x30 / 4];
        *(u32 *)((u8 *)p + 0x1F8) = param[0x38 / 4];  // float->int via cvt.w.s
        *(u32 *)((u8 *)p + 0x204) = 3;
    }

    // Update entity flags from param[15] bit 0 -> flag bit 46
    {
        ico_u64 fl = *(ico_u64 *)((u8 *)alloc + 0x18);
        u32 bv = param[0x3C / 4] & 1;
        fl = (fl & ~((ico_u64)1 << 46)) | ((ico_u64)bv << 46);
        *(ico_u64 *)((u8 *)alloc + 0x18) = fl;
    }

    // Conditional death anim pos init
    {
        ico_ptr32 p = *(ico_ptr32 *)((u8 *)inner + 0x670);
        if (*(u32 *)((u8 *)p + 0x1DC) == 3) {
            float dv = DEATH_FLOAT;
            *(float *)((u8 *)scene_obj + 0x45C) = dv;
            *(float *)((u8 *)scene_obj + 0x460) = dv;
            *(float *)((u8 *)scene_obj + 0x464) = dv;
            *(float *)((u8 *)scene_obj + 0x468) = dv;
        }
    }

    {
        ico_ptr32 p = *(ico_ptr32 *)((u8 *)inner + 0x670);
        *(ico_ptr32 *)((u8 *)p + 0x1E4) = ENTITY6E_PTR;
    }
    sub_15F6F0(ec);

    // === flags_0x48 processing ===
    // ENEMY1-specific: reads entry_record[type]->flags at +0x48
    // Checks bit 18 -> s0_flag
    // If set, the init_fn also generates mask-building code (0x16475C-0x164794)
    // that evaluates to identity on this compiler version (AND with all-ones)
    {
        s32 typ = *(s32 *)((u8 *)ec + 0x08);
        u32 entry_flags = *(u32 *)((u8 *)ENTRY_TABLE_BASE + typ * ENTRY_STRIDE + 0x48);
        s0_flag = ((entry_flags >> 18) & 1) ? 1 : 0;
    }

    sub_203AA0(1);

    {
        ico_ptr32 s1_val = (ico_ptr32)(0x00290000 - 0x2BA8);
        *(ico_ptr32 *)((u8 *)alloc + 0xC0) = s1_val;

        if (GLOBAL_FLAG)
            sub_203B78((ico_ptr32)0x00161F10, 0x14);

        sub_203B78((ico_ptr32)0x0015FD58, 0x15);
        sub_203B78((ico_ptr32)0x001600F8, 0x15);
        sub_203B78((ico_ptr32)0x0015C7C8, 0x15);

        *(ico_ptr32 *)((u8 *)alloc + 0xC4) = (ico_ptr32)((u8 *)s1_val + 0x78);
        *(u32 *)((u8 *)alloc + 0x44) = 2;
    }

    // Scale init (clamped to minimum 10.0f)
    {
        float sv = sub_1CEC60(ec);
        float *p1 = (float *)((u8 *)alloc + 0x1D0);
        float *p2 = (float *)((u8 *)alloc + 0x1D4);
        *p2 = sv;
        *p1 = sv;
        if (sv < 10.0f) {
            *p1 = 10.0f;
            *p2 = 10.0f;
        }
    }

    *(u32 *)((u8 *)alloc + 0x340) = 0;
    sub_15BCC8(ec, 0xB7);

    // Optional quaternion init (when entry flags bit 18 was set)
    if (s0_flag) {
        ico_u64 qdata[2];
        qdata[0] = *(ico_u64 *)((u8 *)0x00558FA0);
        qdata[1] = *(ico_u64 *)((u8 *)0x00558FA8);
        sub_104360((ico_ptr32)qdata, (ico_ptr32)qdata);
        sub_1CEC88(ec);
        sub_1651C8(ec);
    }

    sub_203AA0(0);
}
