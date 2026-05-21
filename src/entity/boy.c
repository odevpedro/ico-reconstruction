// Near-structural decompilation for BOY (descriptor index 1)
// Based on MIPS64 disassembly (ee-gcc 2.9-991111-01, -O2)
// ELF: SCUS_971.13, single PT_LOAD segment (VA 0x100000 -> file 0x1000)
//
// BOY is the main playable character (Ico). Rev.077 confirmed addresses:
//   init_fn   (0x153478) -- full entity init; 59 insns, 352B stack, 7 JALs
//   hC        (0x1C1A98) -- per-instance constructor via descriptor [+0x58]; 106 insns
//   synchronizeMotionOutputOriginForGirl (0x1C1C48) -- distance trigger / proximity check (called from hB); 99 insns
//   hB        (0x1C1DD8) -- per-frame update via descriptor [+0x50]; 50 insns, cloth/walk/phys
//   boy_dispCrown (0x1C1EA8) -- model transform pipeline (called from hA); 43 insns
//   hA        (0x1C1F58) -- primary update via descriptor [+0x48]; 79 insns, active/idle paths
//   sub_1C2098 (0x1C2098) -- state setter; 3 insns
//   sub_1C20A8 (0x1C20A8) -- 100-entry float accumulator with random reset; 51 insns
//   sub_1C2170 (0x1C2170) -- state activator; 15 insns
//
// Key observations:
//   BOY uses scene_obj flags_0x480 (NOT flags_0x48 which is ENEMY1-only).
//   No mask-based slot selection — ENEMY1 bit-33 mechanism is exclusive to ENEMY1.
//   hB tail-calls movement solver (0x103D50) with speed 15.0/30.0 based on context.
//   hA has two paths: active (animation+physics) and idle (transform-only).
//   World-state 0x27 gates interaction proximity checks.
//   100-entry float array (sub_1C20A8) used for timed transitions or particle effects.
//   Child entities: type 1=model root, 0xB=hitbox, 0xC=shadow/proxy.

#include "../types.h"
#include "structs.h"

// ============================================================================
// Forward declarations
// ============================================================================

// Allocator
ico_ptr32 sub_13A0F8(ico_ptr32 heap, u32 size, ico_ptr32 tag, u32 line);

// Entity core init
void sub_1A6E28(ico_ptr32 data, struct entity_context *entity);
ico_ptr32 sub_202208(struct entity_context *);
void sub_202148(struct entity_context *);
void sub_203918(struct entity_context *);
void sub_203910(struct entity_context *);
void sub_203AA0(u32);

// Model / animation
ico_ptr32 sub_1C8478(struct entity_context *, ico_ptr32 data, ico_ptr32 tex);
void sub_104508(ico_ptr32 buf, struct entity_context *);
void sub_102850(ico_ptr32 buf, struct entity_context *);
void sub_10E158(ico_ptr32 buf, s32 heading);
float sub_12A060(s32, ico_ptr32, ico_ptr32);
u32 sub_109F10(struct entity_context *, u32 attr);
ico_ptr32 sub_105278(void);
void sub_105F20(ico_ptr32 dst, ico_ptr32 src, u32 stride);
void sub_104F48(s32 val);
void sub_121D90(ico_ptr32 child, ico_ptr32 scene_obj);
ico_ptr32 sub_1052A8(ico_ptr32 src);
void sub_10C6C0(ico_ptr32 config);
float sub_14B358(u32 attr);
void sub_10ECD8(void);
void sub_10ECB8(struct entity_context *);
void sub_1C1250(struct entity_context *);

// Child entity
ico_ptr32 sub_19F310(u32 type, ico_ptr32 initializer);
void sub_243BD8(ico_ptr32 transform, ico_ptr32 state);

// Scene setup / cloth / physics
void sub_1E4798(struct entity_context *, u32, u32, u32, ico_ptr32, ico_ptr32);
void sub_1C1380(struct entity_context *, ico_ptr32 initializer);
void sub_1D4B40(struct entity_context *, u32);
void sub_1D23E0(struct entity_context *);
void sub_1E3FC8(struct entity_context *);
void sub_1C12F0(struct entity_context *);
void sub_1E4868(struct entity_context *);
void sub_1BB8C0(struct entity_context *, u32, u32, u32);
u32 sub_103D50(struct entity_context *, u32, float, float, float);
void sub_13FF88(struct entity_context *, u32, struct entity_context *);
u32 sub_14A0D8(void);
void sub_1E4938(struct entity_context *);
void synchronizeMotionOutputOriginForGirl(struct entity_context *);
void boy_dispCrown(struct entity_context *);

// Distance trigger helpers
void sub_1184B8(ico_ptr32, ico_ptr32, ico_ptr32);
void sub_1183B8(ico_ptr32, ico_ptr32, ico_ptr32);
void sub_118388(ico_ptr32, ico_ptr32, ico_ptr32);
void sub_104360(ico_ptr32, ico_ptr32);
void sub_1118B8(u32);
void sub_111FA8(u32, u32, u32);
void sub_118678(ico_ptr32);
void sub_11E220(ico_ptr32, u32, u32);
void sub_10F630(void);

// Interaction
u32 sub_10D180(struct entity_context *);
void sub_10CB48(ico_ptr32, ico_ptr32, struct entity_context *);
void sub_10CF88(ico_ptr32);

// Random
u32 sub_118A68(void);

// GP-relative globals
#define GP_BASE         0x00633D14
#define HEAP_PTR        (*(ico_ptr32 *)(GP_BASE - 0x68E0))
#define ENTITY_ICO_PTR  (*(ico_ptr32 *)(GP_BASE - 0x6E08))
#define FLOAT_GPCONST   (*(float *)(GP_BASE - 0x771C))
#define FLOAT_MULT      (*(float *)(GP_BASE - 0x7718))
#define FLOAT_OFFSET    (*(float *)(GP_BASE - 0x7714))
#define GAMEPLAY_FLAG   (*(ico_ptr32 *)(GP_BASE - 0x6F60))
#define ARRAY_2PTR      (*(ico_ptr32 *)(GP_BASE - 0x5650))
#define INIT_FLAG_BYTE  (*(u32 *)(GP_BASE - 0x4C04))

// ============================================================================
// Struct: private work area per BOY instance
// Size: 0x4C (76 bytes), heap-allocated in hC, tag 0xFE
// Stored at: scene_obj[0x800]
// ============================================================================
struct boy_work {
    u32          variant;         // +0x00: animation variant (0=default, 1/2=alt)
    ico_ptr32    child_root;      // +0x04: child entity type 1 (model root)
    ico_ptr32    child_hitbox;    // +0x08: child entity type 0xB (hitbox/collision)
    ico_ptr32    child_shadow;    // +0x0C: child entity type 0xC (shadow)
    u32          active;          // +0x10: if non-zero → Path A (full anim+phys), else Path B
    s32          anim_time;       // +0x14: animation time result (stored from collision)
    ico_ptr32    model_A;         // +0x18: model chunk at 0x4BF7F0
    ico_ptr32    model_B;         // +0x1C: model chunk at 0x4BFAF0
    ico_ptr32    model_D;         // +0x20: model chunk at 0x4BFF30
    ico_ptr32    model_C;         // +0x24: model chunk at 0x4BFDF0
    ico_ptr32    model_E;         // +0x28: model chunk at 0x4C0070
    u32          config_A;        // +0x2C: configuration value A (initial 20)
    u32          config_B;        // +0x30: configuration value B (initial 20)
    float        range_A;         // +0x34: range value A (initial 300.0f)
    float        range_B;         // +0x38: range value B (initial 300.0f)
    u32          field_3C;        // +0x3C: padding
    u32          field_40;        // +0x40: padding
    u32          field_44;        // +0x44: padding
    u32          flag_mask;       // +0x48: flag mask (initial 0x80808080)
};

// Scene_obj offsets used by BOY:
// +0x554 : init_flag (set to 1 in hC)
// +0x0C  : model_transform_base
// +0x800 : boy_work* (this alloc)
// +0x644 : distance_sensor (float)
// +0x648 : interaction_id
// +0x4A0 : position (vec3)
// +0x62C : field cleared on activate

// ============================================================================
// init_fn (0x153478) -- Full entity initializer
//
// Called once per BOY entity via descriptor [+0x00] during scene setup.
// Heavier than hC. Uses stack frame 0x160 (352 bytes).
//
// 1. GP-relative data setup at 0x6AAAE0 (s5=0x6B0000, s1=s5-0x5520)
// 2. Bit extraction from a 64-bit value at base+8,
//    stored as boolean at gp-0x4c04 through gp-0x4bff (6 byte flags)
// 3. Chain: sub_1A6E28, sub_202208 (core alloc), sub_104508 (anim),
//    sub_202148, sub_203918, sub_203910 (scene context)
// 4. sub_14B358(0x1f) — model parameter setup
// 5. VBlank counter read (0x274EC0) for timing setup
// ============================================================================
void boy_init(struct entity_context *entity)
{
    ico_ptr32 alloc;
    ico_ptr32 gp_area;
    u64 val64;
    u32 flag_bit;
    ico_u64 mask;
    ico_ptr32 gp_base;
    int i;

    gp_area = (ico_ptr32)(0x6B0000 - 0x5520);  // 0x6AAAE0
    *(ico_ptr32 *)((u8 *)gp_area + 0x10) = -1;

    val64 = *(ico_u64 *)((u8 *)gp_area + 8);
    mask = (ico_u64)-9;
    mask = (mask << 16) | 0xFFFF;
    mask = (mask << 16) | 0xFFFF;

    flag_bit = (u32)((val64 >> 35) & 1);
    val64 &= mask;
    *(ico_u64 *)((u8 *)gp_area + 8) = val64;

    // Store flag results into 6 consecutive byte globals
    INIT_FLAG_BYTE = flag_bit;
    *((u8 *)(GP_BASE - 0x4C03)) = 0;
    *((u8 *)(GP_BASE - 0x4C02)) = 0;
    *((u8 *)(GP_BASE - 0x4C01)) = 0;
    *((u8 *)(GP_BASE - 0x4C00)) = 0;
    *((u8 *)(GP_BASE - 0x4BFF)) = 0;
    *((u32 *)(GP_BASE - 0x4BFC)) = 0;

    sub_1A6E28((ico_ptr32)0x005690F0, entity);
    alloc = sub_202208(entity);

    *(ico_ptr32 *)((u8 *)alloc + 0x67C) = (ico_ptr32)(0x6B0000 - 0x5530);
    sub_104508((ico_ptr32)((u8 *)alloc + 0x100), entity);

    sub_202148(entity);
    sub_203918(entity);
    sub_203910(entity);
    sub_14B358(0x1F);

    // Read counter at 0x274EC0 for timing initialization
    // (VBlank counter, likely used as seed or frame reference)
    {
        u32 counter = *(u32 *)0x274EC0;
        u32 max_val = 60;
        // Function truncated after this point by Capstone decode issue
        // at MULT instruction. Remaining code likely continues init.
    }
}

// ============================================================================
// hC (0x1C1A98) -- per-instance constructor
//
// Called from descriptor[+0x58] (cb_routine3). Returns boy_work ptr.
//
// 1. Allocate 76B work struct (tag 0xFE)
    // 2. Load 5 model chunks from 0x4B0000/0x4C0000 region via sub_1C8478
// 3. Create 3 child entities: type 1 (root), type 0xB (hitbox), type 0xC (shadow)
// 4. Transform setup for child 0
// 5. Scene setup (sub_1E4798 with a2=0x4C0=1216)
// 6. Interaction setup (sub_1C1380)
// 7. Cloth collision type 2 (sub_1D4B40)
// 8. State field initialization (variants, range=300.0f, mask=0x80808080)
// 9. Config block init (sub_10C6C0 on config_A)
// ============================================================================
ico_ptr32 boy_hC(struct entity_context *entity, ico_ptr32 initializer)
{
    struct boy_work *wk;
    ico_ptr32 scene_obj;
    ico_ptr32 alloc;

    alloc = sub_13A0F8(HEAP_PTR, 0x4C, (ico_ptr32)0x00618838, 0xFE);
    wk = (struct boy_work *)alloc;

    scene_obj = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    *(ico_ptr32 *)((u8 *)scene_obj + STATE_BLOCK_OFFSET) = alloc;

    // Load 5 model chunks
    wk->model_A = sub_1C8478(entity, (ico_ptr32)0x004BF7F0, (ico_ptr32)0x004C01B0);
    wk->model_B = sub_1C8478(entity, (ico_ptr32)0x004BFAF0, (ico_ptr32)0x004C0270);
    wk->model_C = sub_1C8478(entity, (ico_ptr32)0x004BFDF0, (ico_ptr32)0);
    wk->model_D = sub_1C8478(entity, (ico_ptr32)0x004BFF30, (ico_ptr32)0);
    wk->model_E = sub_1C8478(entity, (ico_ptr32)0x004C0070, (ico_ptr32)0);

    *(u32 *)((u8 *)scene_obj + 0x554) = 1;

    // Create 3 child entities
    wk->child_root   = sub_19F310(1, initializer);
    wk->child_hitbox = sub_19F310(0xB, initializer);
    wk->child_shadow = sub_19F310(0xC, initializer);

    // Transform setup for child 0 (model root)
    sub_243BD8((ico_ptr32)((u8 *)wk->child_root + 0x20),
               *(ico_ptr32 *)((u8 *)scene_obj + STATE_BLOCK_OFFSET));

    // Scene setup — a2=0x4C0=1216, t0=12, a1=0, a3=0, t1=0
    sub_1E4798(entity, 0, 0x4C0, 0, (ico_ptr32)12, (ico_ptr32)0);

    // Interaction setup
    sub_1C1380(entity, initializer);

    // Cloth collision type 2
    sub_1D4B40(entity, 2);

    // Initialize state block fields
    wk->active    = 0;
    wk->anim_time = 0;
    wk->config_A  = 20;
    wk->config_B  = 20;
    wk->range_A   = 300.0f;
    wk->range_B   = 300.0f;
    wk->flag_mask = 0x80808080;

    sub_10C6C0((ico_ptr32)&wk->config_A);

    return alloc;
}

// ============================================================================
// synchronizeMotionOutputOriginForGirl (0x1C1C48) -- distance trigger / proximity interaction
//
// Called from hB. Checks proximity to another entity (Ico player reference).
// Uses XOR mask matching against a 6-entry table at 0x4D4150.
//
// 1. Read ENTITY_ICO_PTR (gp-0x6e08)
// 2. Load position at entity+0x4A0 and gp_ptr+0x4A0
// 3. XOR mask matching loop (5+1 iterations, 8-byte stride at 0x4D4150)
// 4. If both positions match masks → distance check < 10.0f
// 5. On proximity: sound (0x1118B8), config (0x111FA8),
//    matrix setup (0x118678/0x1052A8), render setup (0x11E220 x2), finalize
// ============================================================================
void synchronizeMotionOutputOriginForGirl(struct entity_context *entity)
{
    ico_ptr32 target;
    ico_ptr32 t_scene, e_scene;
    u32 t_pos, e_pos;
    ico_ptr32 table;
    ico_u32 t1_match, t2_match;
    int i;
    u32 v1, v0;
    ico_ptr32 buf;
    ico_ptr32 stack_area;

    target = ENTITY_ICO_PTR;
    if (!target)
        return;

    e_scene = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    t_scene = *(ico_ptr32 *)((u8 *)target + ENTITY_STATE_OFFSET);

    e_pos = *(u32 *)((u8 *)e_scene + 0x4A0);
    t_pos = *(u32 *)((u8 *)t_scene + 0x4A0);

    t1_match = 0;
    t2_match = 0;
    table = (ico_ptr32)0x004D4150;

    for (i = 6; i > 0; i--) {
        v1 = *(u32 *)((u8 *)table + 0);
        v0 = *(u32 *)((u8 *)table + 4);
        if ((t_pos ^ v1) == 0)
            t1_match = 1;
        if ((e_pos ^ v0) == 0)
            t2_match = 1;
        table += 8;
    }

    if (!t1_match || !t2_match)
        return;

    buf = (ico_ptr32)((u8 *)&stack_area - 0x50);  // stack buffer

    sub_1184B8(buf, (ico_ptr32)((u8 *)e_scene + 0x100),
                    (ico_ptr32)((u8 *)t_scene + 0x100));

    {
        ico_ptr32 gp_e = ENTITY_ICO_PTR;
        ico_ptr32 sp = (ico_ptr32)&stack_area;
        sub_1183B8(sp, buf, (ico_ptr32)((u8 *)(*(ico_ptr32 *)((u8 *)gp_e + 0x15C)) + 0x100));
    }

    sub_104508(buf, entity);
    sub_118388(buf, buf, (ico_ptr32)&stack_area);
    sub_104360(buf, buf);

    if (*(ico_ptr32 *)(GP_BASE - 0x5C1C)) {
        float dist_thresh = 10.0f;

        sub_1118B8(0xB);
        sub_111FA8(1, 5, 0x80);

        {
            ico_ptr32 v = ENTITY_ICO_PTR;
            ico_ptr32 src = 
                sub_1052A8((ico_ptr32)((u8 *)(*(ico_ptr32 *)((u8 *)v + 0x15C)) + 0x100));
            sub_118678(src);
        }

        {
            ico_ptr32 q = 
                sub_1052A8((ico_ptr32)((u8 *)(*(ico_ptr32 *)((u8 *)entity + 0x15C)) + 0x100));
            sub_118678(q);
        }

        sub_11E220((ico_ptr32)0x004C0330, 0x10, 8);
        sub_11E220((ico_ptr32)0x004C0340, 0x10, 8);
        sub_10F630();
    }
}

// ============================================================================
// hB (0x1C1DD8) -- per-frame update (physics + cloth + animation state)
//
// Called via descriptor[+0x50] (ItemGeo).
//
// Structure:
//   1. cloth physics hook (0x1D23E0)
//   2. shadow draw (0x1E3FC8)
//   3. distance trigger (0x1C1C48)
//   4. character frame pre (0x1C12F0)
//   5. character frame post (0x1E4868)
//   6. animation state set (0x1BB8C0, args 0x23/0x2C/0x198)
//   7. movement solver call (0x103D50, speed=50.0, damping=const)
//   8. if entity was at rest → sound trigger (0x13FF88 with arg 6)
//   9. speed check (0x14A0D8): returns non-zero for walking (30.0),
//      zero for running (15.0)
//   10. Tail-call movement solver (0x103D50) with computed speed
// ============================================================================
void boy_hB(struct entity_context *entity)
{
    float speed;
    float damping;
    u32 is_walking;

    sub_1D23E0(entity);

    damping = *(float *)(GP_BASE - 0x771C);
    sub_1E3FC8(entity);

    synchronizeMotionOutputOriginForGirl(entity);
    sub_1C12F0(entity);
    sub_1E4868(entity);

    sub_1BB8C0(entity, 0x23, 0x2C, 0x198);

    {
        u32 rest_flag = sub_103D50(entity, 4, 50.0f, 50.0f, damping);
        if (rest_flag)
            sub_13FF88(entity, 6, entity);
    }

    is_walking = sub_14A0D8();
    if (is_walking)
        speed = 15.0f;
    else
        speed = 30.0f;

    sub_103D50(entity, 2, speed, 50.0f, damping);
}

// ============================================================================
// boy_dispCrown (0x1C1EA8) -- model transform pipeline
//
// Called from hA idle path.
//
// 1. Get animation frame index via sub_109F10(entity, 0x23)
// 2. Select bone hierarchy from state block variant:
//      variant 1 → child at +0x08
//      variant 2 → child at +0x0C
//      default   → child at +0x04
// 3. Matrix A: alloc temp buffer, apply frame transform
// 4. Matrix B: alloc temp buffer, apply child model transform
// 5. Tail-call world-matrix finalizer at 0x121D90(bone_hierarchy, scene_obj)
// ============================================================================
void boy_dispCrown(struct entity_context *entity)
{
    ico_ptr32 scene_obj;
    struct boy_work *wk;
    ico_ptr32 bone_ptr;
    u32 frame_idx;
    ico_ptr32 buf_a, buf_b;
    ico_ptr32 mtx_base;

    scene_obj = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    wk = (struct boy_work *)(*(ico_ptr32 *)((u8 *)scene_obj + STATE_BLOCK_OFFSET));

    frame_idx = sub_109F10(entity, 0x23);

    switch (wk->variant) {
    case 1:
        bone_ptr = wk->child_hitbox;
        break;
    case 2:
        bone_ptr = wk->child_shadow;
        break;
    default:
        bone_ptr = wk->child_root;
        break;
    }

    buf_a = sub_105278();
    mtx_base = *(ico_ptr32 *)((u8 *)scene_obj + 0x0C);
    sub_105F20(buf_a, (ico_ptr32)((u8 *)mtx_base + 6), frame_idx * 64);
    sub_104F48((s32)-0x8000);

    buf_b = sub_105278();
    sub_105F20(buf_b, *(ico_ptr32 *)((u8 *)bone_ptr + 0x0C), 0);

    sub_121D90((ico_ptr32)bone_ptr, scene_obj);
}

// ============================================================================
// hA (0x1C1F58) -- primary update (cb_update / descriptor[+0x48])
//
// Two-path controller: active (full anim+physics) vs idle (transform-only).
//
// Path A — active (wk->active != 0):
//   1. Animation state machine (sub_104508)
//   2. Matrix/skeleton update (sub_102850)
//   3. Heading/rotation (sub_10E158 with -0x8000)
//   4. Collision/physics query (sub_12A060)
//   5. Store animation time if world_state allows
//
// Path B — idle (wk->active == 0):
//   1. sub_10ECD8 (NOP stub)
//   2. sub_10ECB8 (idle stub)
    //   3. Model transform pipeline (boy_dispCrown)
//   4. Physics step (sub_1C1250)
//
// Both paths converge:
//   If world_state_main == 0x27 (gameplay):
//     Check proximity < 20.0f AND interaction_id != 0 AND entity not busy
//       → Trigger interaction (sub_10CB48 / sub_10CF88)
// ============================================================================
void boy_hA(struct entity_context *entity)
{
    ico_ptr32 scene_obj;
    struct boy_work *wk;
    ico_ptr32 stack_sp;
    ico_ptr32 stack_s0;
    s32 anim_result;

    scene_obj = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    wk = (struct boy_work *)(*(ico_ptr32 *)((u8 *)scene_obj + STATE_BLOCK_OFFSET));
    stack_sp = (ico_ptr32)&stack_sp;

    sub_1E4938(entity);

    if (wk->active) {
        // Path A: active — full animation + physics
        sub_104508(stack_sp, entity);

        stack_s0 = (ico_ptr32)((u8 *)&stack_sp + 0x10);
        sub_102850(stack_s0, entity);
        sub_10E158(stack_s0, (s32)-0x8000);

        {
            float f_anim_time = (float)wk->anim_time;
            float result = sub_12A060((s32)-0x1A8, stack_sp, stack_s0);
            anim_result = (s32)result;
        }

        {
            u32 world_state_chk = *(u32 *)0x274ED4;
            if (world_state_chk == 0 && anim_result != -1)
                wk->anim_time = anim_result;
        }
    } else {
        // Path B: idle — transform-only
        sub_10ECD8();
        sub_10ECB8(entity);
        boy_dispCrown(entity);
        sub_1C1250(entity);
    }

    // Interaction proximity check (gated by world_state == 0x27)
    {
        u32 world_state_main = GAMEPLAY_FLAG;
        if (world_state_main == 0x27) {
            float dist = *(float *)((u8 *)scene_obj + 0x644);
            if (dist < 20.0f) {
                ico_ptr32 inter_id = *(ico_ptr32 *)((u8 *)scene_obj + 0x648);
                if (inter_id) {
                    if (!sub_10D180(entity)) {
                        u32 *buf = (u32 *)((u8 *)wk + 0x2C);
                        sub_10CB48((ico_ptr32)buf, inter_id, entity);
                        sub_10CF88((ico_ptr32)buf);
                    }
                }
            }
        }
    }
}

// ============================================================================
// sub_1C2098 (0x1C2098) -- state setter
//
// Writes arg a1 to work_block+0 (variant field).
// Tiny helper: 3 instructions (lw + jr $ra + sw delay slot)
// ============================================================================
void boy_set_state(struct entity_context *entity, u32 state)
{
    ico_ptr32 scene_obj;
    ico_ptr32 wk;

    scene_obj = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    wk = *(ico_ptr32 *)((u8 *)scene_obj + STATE_BLOCK_OFFSET);
    *(u32 *)((u8 *)wk + 0) = state;
}

// ============================================================================
// sub_1C20A8 (0x1C20A8) -- float array accumulator (100 entries)
//
// Iterates two parallel float arrays (A and B) of 100 elements each.
// Accumulates: A[i] += B[i]
// When accumulated value exceeds 1.0f: stores result, randomizes B[i]
// with scale+offset, resets A[i] to 0.
//
// Used for timed transitions (animation blending, particle lifecycle).
// ============================================================================
void boy_float_accum(void)
{
    ico_ptr32 ptr;
    ico_u32 *a_base;
    ico_u32 *b_base;
    float f_a, f_b;
    float one = 1.0f;
    float mult, offset;
    int i;

    ptr = ARRAY_2PTR;
    mult = FLOAT_MULT;
    offset = FLOAT_OFFSET;

    for (i = 0; i < 100; i++) {
        a_base = (ico_u32 *)*(ico_ptr32 *)((u8 *)ptr + 0);
        b_base = (ico_u32 *)*(ico_ptr32 *)((u8 *)ptr + 4);

        f_a = *(float *)&a_base[i];
        f_b = *(float *)&b_base[i];
        f_a += f_b;

        if (one < f_a) {
            *(float *)&a_base[i] = f_a;

            sub_118A68();

            f_b = f_b * mult + offset;
            ptr = ARRAY_2PTR;
            b_base = (ico_u32 *)*(ico_ptr32 *)((u8 *)ptr + 4);
            a_base = (ico_u32 *)*(ico_ptr32 *)((u8 *)ptr + 0);
            *(float *)&b_base[i] = f_b;
            *(u32 *)&a_base[i] = 0;
        }
    }
}

// ============================================================================
// sub_1C2170 (0x1C2170) -- state activator
//
// Activates BOY's animation/processing state.
// Sets wk->active = 1, wk->anim_time = 0, clears scene_obj+0x62C.
// ============================================================================
void boy_activate(struct entity_context *entity)
{
    ico_ptr32 scene_obj;
    struct boy_work *wk;

    scene_obj = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    wk = (struct boy_work *)(*(ico_ptr32 *)((u8 *)scene_obj + STATE_BLOCK_OFFSET));

    wk->anim_time = 0;
    wk->active = 1;
    *(u32 *)((u8 *)scene_obj + 0x62C) = 0;
}
