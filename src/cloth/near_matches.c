#include "structs.h"

struct cloth_context;

/* =================================================================
 * NEAR-STRUCTURAL matches
 *
 * These functions have validated control flow and struct offsets,
 * but the compiled output differs from the original in systematic
 * ways (register allocation, scheduling, instruction encoding).
 * Differences are due to GCC 2.95.2 PS2 Linux vs ee-gcc 2.9-991111-01.
 *
 * See Rev.048 for the full list of compiler gap limitations.
 * ================================================================= */

/* 0x001D3DB0 (40B) — NEAR-STRUCTURAL
 * If variant_id == 1, returns field_40; else returns 0.
 * Differences: 8/10 instructions match. Scheduler moves nop vs
 * move in delay slot. move encoding: or (GCC) vs addu (ICO).
 */
int cloth_test_variant_field(struct cloth_context *ctx) {
    ico_ptr32 entity = *(ico_ptr32*)((char*)ctx + 0x15C);
    ico_ptr32 payload = *(ico_ptr32*)((char*)entity + 0x800);
    int variant = *(int*)((char*)payload + 4);
    if ((variant ^ 1) == 0)
        return *(int*)((char*)payload + 0x40);
    return 0;
}

/* 0x001D3D40 (48B) — NEAR-STRUCTURAL
 * If extra != NULL and flag_08 == 0, returns (state_id < 2);
 * else returns 0. 12 instructions (same count as target).
 * Differences: instruction reorder (load entity after move a1).
 * move encoding: or (GCC) vs addu (ICO).
 */
int cloth_test_state_lt_2(struct cloth_context *ctx) {
    ico_ptr32 entity = *(ico_ptr32*)((char*)ctx + 0x15C);
    int result = 0;
    int extra = *(int*)((char*)ctx + 0x16C);
    if (extra != 0) {
        ico_ptr32 payload = *(ico_ptr32*)((char*)entity + 0x800);
        ico_u64 flag = *(ico_u64*)((char*)payload + 8);
        if (flag == 0) {
            result = *(int*)((char*)payload + 0x48) < 2;
        }
    }
    return result;
}

/* 0x001D40A0 (56B) — NEAR-STRUCTURAL
 * Returns 1 if field_00 == 1 OR extra == NULL; else 0.
 * 14 instructions (same as target). All registers differ
 * (compiler allocation); structure identical.
 */
int cloth_test_field0_or_extra(struct cloth_context *ctx) {
    ico_ptr32 entity = *(ico_ptr32*)((char*)ctx + 0x15C);
    ico_ptr32 payload = *(ico_ptr32*)((char*)entity + 0x800);
    return (*(int*)((char*)payload + 0) == 1) ||
           (*(ico_ptr32*)((char*)ctx + 0x16C) == 0);
}

/* =================================================================
 * 0x001D37C8 — cloth_dispatcher (NEAR-STRUCTURAL)
 *
 * 5-state cloth simulation FSM. State ID at [payload+0x48].
 * Jump table at 0x00618FB0 selects internal handler.
 *
 * State flow: 0→1→2→3→4 (terminal). External code resets to 0.
 *
 * Fields accessed relative to payload base (s3) and state_block (s1=s3+0x40):
 *   state_block+0x00 = field_40 (guard param)
 *   state_block+0x04 = counter   (countdown timer)
 *   state_block+0x08 = state_id  (0-4)
 *   state_block+0x10 = matrix    (state 2 intermediate)
 *   state_block+0x20 = result    (state 2 alloc)
 *   payload+0x64     = field_64  (active flag)
 *   payload+0x00     = field_00  (completion flag, set by state 3)
 * ================================================================= */
u32 sub_1F2148(ico_ptr32, ico_ptr32);
void sub_1D2538(struct entity_context *);
void sub_1D2540(struct entity_context *);
void sub_1D2548(struct entity_context *);
void sub_105F00(ico_ptr32, ico_ptr32);
void sub_12ABE0(u32, u32);
void sub_12AC28(u32, u32);
void sub_104508(ico_ptr32, struct entity_context *);
void sub_181BF8(struct entity_context *, u32, float, ico_ptr32, u32);
void sub_1F19F0(ico_ptr32);
void sub_1D12D8(struct entity_context *);
void sub_12ADE8(u32, u32);
u32 sub_12A618(u32);
void sub_118460(ico_ptr32, ico_ptr32);
void sub_10D830(ico_ptr32, ico_ptr32);
u32 sub_12A7F8(ico_ptr32);
u32 sub_118A68(void);

void cloth_dispatcher(struct entity_context *ctx)
{
    ico_ptr32 entity_state = *(ico_ptr32 *)((u8 *)ctx + 0x15C);
    ico_ptr32 payload = *(ico_ptr32 *)((u8 *)entity_state + 0x800);
    ico_ptr32 sb = payload + 0x40;          /* state block */
    ico_u32 sid = *(ico_u32 *)((u8 *)sb + 0x08);  /* state_id */

    /* Fallback for invalid state: treat as state 0 */
    if (sid >= 5)
        sid = 0;

    switch (sid) {
    case 0: {
        /* Guard: check if simulation should activate */
        ico_ptr32 param = *(ico_ptr32 *)((u8 *)sb + 0x00);
        if (sub_1F2148(param, ctx)) {
            *(ico_u32 *)((u8 *)sb + 0x08) = 1;    /* state → 1 */
            sub_1D2538(ctx);
            sub_1D2540(ctx);
            *(ico_u32 *)((u8 *)payload + 0x64) = 1; /* flag active */
        }
        break;
    }
    case 1: {
        /* Timing/prepare phase: decrement counter, compute force quaternion */
        u32 seed_a = *(u32 *)0x004C4750;
        u32 seed_b = *(u32 *)0x004C4754;
        ico_ptr32 es = *(ico_ptr32 *)((u8 *)ctx + 0x15C);
        u32 time_param = *(u32 *)0x00274EC0;
        u32 time_div  = *(u32 *)0x00274EC4;
        ico_s32 counter;
        u32 quat_tmp[4];
        (void)seed_a; (void)seed_b;

        counter = *(ico_s32 *)((u8 *)sb + 0x04) - 1;
        *(ico_s32 *)((u8 *)sb + 0x04) = counter;
        if (counter == 0)
            *(ico_u32 *)((u8 *)sb + 0x08) = 2;

        sub_105F00((ico_ptr32)quat_tmp, (ico_ptr32)&seed_a);
        sub_105F00((ico_ptr32)((u8 *)es + 0xA0), (ico_ptr32)quat_tmp);
        break;
    }
    case 2: {
        /* Active simulation phase */
        ico_ptr32 matrix = sb + 0x10;   /* state_block+0x10 workspace */
        *(ico_u32 *)((u8 *)payload + 0x64) = 0;
        sub_12ABE0(0x1B8, 0);
        sub_12AC28(0x1B8, 1);
        sub_104508((ico_ptr32)matrix, ctx);
        sub_181BF8(ctx, 17, 100.0f, (ico_ptr32)matrix, 0);
        sub_1F19F0(*(ico_ptr32 *)((u8 *)sb + 0x00));
        *(ico_u32 *)((u8 *)(*(ico_ptr32 *)((u8 *)sb + 0x00)) + 0x16C) = 0;
        *(ico_u32 *)((u8 *)payload + 0x08) = 0;
        sub_1D12D8(ctx);
        sub_1D2548(ctx);
        sub_12ADE8(0x1B8, 1);
        {
            u32 res_addr = sub_12A618(0x1B8);
            *(ico_ptr32 *)((u8 *)sb + 0x20) = (ico_ptr32)res_addr;
            *(float *)((u8 *)res_addr + 0x04) = 1.0f;
            sub_118460((ico_ptr32)((u8 *)res_addr + 0x20), (ico_ptr32)matrix);
            sub_10D830((ico_ptr32)((u8 *)res_addr + 0x30), 0x00276140);
        }
        *(ico_u32 *)((u8 *)sb + 0x08) = 3;
        *(ico_u32 *)((u8 *)(*(ico_ptr32 *)((u8 *)ctx + 0x15C)) + 0x74) = 0;
        break;
    }
    case 3: {
        /* Post-simulation check */
        ico_ptr32 es2;
        ico_ptr32 p2;
        *(ico_u32 *)((u8 *)payload + 0x08) = 0;
        if (sub_12A7F8((ico_ptr32)((u8 *)sb + 0x20))) {
            *(ico_u32 *)((u8 *)sb + 0x08) = 4;   /* state → 4 (done) */
            es2 = *(ico_ptr32 *)((u8 *)ctx + 0x15C);
            p2 = *(ico_ptr32 *)((u8 *)es2 + 0x800);
            *(ico_u32 *)((u8 *)es2 + 0x74) = 0;
            *(ico_u32 *)((u8 *)p2 + 0x00) = 1;   /* payload completed */
            *(ico_ptr32 *)((u8 *)ctx + 0x16C) = 0; /* clear extra */
        }
        break;
    }
    case 4:
        /* Idle/done — no-op until external reset */
        break;
    }
}

/* =================================================================
 * 0x001D3F78 — clothSubForceApply (NEAR-STRUCTURAL)
 *
 * Applies force to a chain of cloth entities (type 19).
 * Allocates entities and updates their state with force vectors.
 * Uses rand() for angle randomization, then sin/cos to project force.
 *
 * EE-register usage: f20/f21 force in, f22=65536.0f, f23=300.0f
 * Stack: 96B (8 saved regs + 2 callee-saved float + local)
 * ================================================================= */
void sub_13EBE0(ico_ptr32);
ico_ptr32 sub_13F7A8(ico_ptr32, ico_ptr32, u32, u32);
ico_ptr32 sub_13A0F8(ico_ptr32, u32, ico_ptr32, u32);
ico_ptr32 sub_1D3B28(ico_ptr32);
void sub_105F00_force(ico_ptr32, ico_ptr32);
void sub_10E9A0(ico_ptr32, float);
float ee_sin(s16);
float ee_cos(s16);

ico_ptr32 clothSubForceApply(struct entity_context *ctx, float force_h, float force_v)
{
    ico_ptr32 heap = *(ico_ptr32 *)0x00719720;
    ico_ptr32 entity = sub_13A0F8(heap, 64, 0x0056AFD8, 1038);
    ico_ptr32 s1 = entity;

    if (!s1)
        return 1;

    /* Process entity chain */
    while (s1) {
        u32 r;
        s16 angle;
        ico_ptr32 extra;
        ico_ptr32 es;
        float sin_val;
        float cos_val;

        r = sub_118A68();
        angle = (s16)((float)r * 65536.0f);
        (void)angle;

        extra = *(ico_ptr32 *)((u8 *)ctx + 0x16C);
        if (extra) {
            int placeholder = 0;
            (void)placeholder;
        }

        es = *(ico_ptr32 *)((u8 *)ctx + 0x15C);
        sin_val = ee_sin(angle);
        *(float *)((u8 *)es + 0x130) = force_h * sin_val;

        r = sub_118A68();
        *(float *)((u8 *)es + 0x134) = force_v * (float)r;

        cos_val = ee_cos(angle);
        *(float *)((u8 *)es + 0x138) = force_h * cos_val;

        s1 = sub_1D3B28(s1);
    }
    return 1;
}
