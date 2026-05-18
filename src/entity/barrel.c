// Near-structural decompilation for BARREL (descriptor index 5)
// ALSO shared with ROPE (descriptor index 8) — same init_fn, hA, hB
// (hC differs: BARREL=0x1D27A8, ROPE=0x1D3B28)
//
// This is historically significant:
//   0x1D3A30 = BARREL cb_routine2, originally misidentified as "ROPE callback"
//   during Rev.001-036. Corrected in Rev.039 via ICO-decomp cross-reference
//   (clothAnimation.c range). It is a physics constraint solver for
//   cloth/rope/barrel interactions (src/item.c based on debug string).
//
// Files confirmed via assertion strings:
//   - src/item.c                    (hC line 0x1B2=434)
//   - src/fieldCollision.c          (init_fn line 0x215=533)
//
// Based on MIPS64 disassembly (ee-gcc 2.9-991111-01, -O2)
// ELF: SCUS_971.13, single PT_LOAD segment (VA 0x100000 -> file 0x1000)

#include "../types.h"
#include "structs.h"

// ============================================================================
// Forward declarations
// ============================================================================

// Allocator
ico_ptr32 sub_13A0F8(ico_ptr32 heap, u32 size, ico_ptr32 tag, u32 line);

// Entity iteration
ico_ptr32 sub_13ECA8(void);
ico_ptr32 sub_13ECF8(void);
ico_ptr32 sub_13EB50(u32 type);
ico_ptr32 sub_13EBE0(ico_ptr32 entity);

// Entity core
ico_ptr32 sub_1AE6F8(u32 type);
void sub_1A6E28(ico_ptr32 debug_str, u32 line);
void sub_1AD748(ico_ptr32 debug_str, u32 line);
void sub_1AD768(ico_ptr32 debug_str, u32 line);
void sub_263FF0(ico_ptr32, u32, ico_ptr32);
void sub_203B78(ico_ptr32, u32);
void sub_1651C8(struct entity_context *);
void sub_19F530(struct entity_context *);

// Transform / matrix helpers
void sub_104508(ico_ptr32 dst, struct entity_context *entity);
void sub_102850(ico_ptr32 buf, struct entity_context *entity);
void sub_102870(struct entity_context *entity, ico_ptr32 transform);
void sub_105F00(ico_ptr32 dst, ico_ptr32 src);
void sub_10D530(struct entity_context *entity);
void sub_10D838(ico_ptr32 dst, ico_ptr32 src);
void sub_10DDB8(ico_ptr32 dst, ico_ptr32 src_a, ico_ptr32 src_b);
void sub_10ECB8(struct entity_context *entity);
void sub_10E158(ico_ptr32 dst, s32 heading);
float sub_10EC08(float x, float y);
float sub_106048(ico_ptr32 a, ico_ptr32 b);

// Child entity
ico_ptr32 sub_1B7FE8(u32 a0, u32 a1, ico_ptr32 a2, u32 a3,
                     ico_ptr32 t0, ico_ptr32 t1, ico_ptr32 t2, ico_ptr32 t3,
                     ico_ptr32 entity, u32 zero);
void sub_19F4E8(struct entity_context *child, ico_ptr32 stack_frame);
void sub_1F2240(struct entity_context *child, u32 flag);
void sub_243BD8(ico_ptr32 dst, ico_ptr32 src);

// Physics / collision
void sub_1D29B8(struct entity_context *entity);
void sub_1D2BF0(struct entity_context *entity);
void sub_1D2738(struct entity_context *entity, ico_ptr32 child_state);
void sub_1D12A8(struct entity_context *entity, u32 arg);
void sub_1D12D8(struct entity_context *entity);
void sub_1D0FA8(struct entity_context *entity, u32 arg, u32 zero);
void sub_1D1158(u32 arg);
void sub_1C05A8(struct entity_context *child);
void sub_102858(struct entity_context *entity);
void sub_1185D0(ico_ptr32, ico_ptr32, ico_ptr32);
void sub_1182F0(ico_ptr32, ico_ptr32);
void sub_1EBC10(ico_ptr32 model_id, ico_ptr32 buf, ico_ptr32 transform);

// Dispatch / state machine
void sub_1D37C8(struct entity_context *entity);

// Misc
void sub_1C1250(struct entity_context *);
void sub_1E3FC8(struct entity_context *);
void sub_1E4798(struct entity_context *, u32, u32, u32, ico_ptr32, ico_ptr32);
void sub_2641D8(ico_ptr32 dst, u32 val, u32 size);
void sub_181BF8(struct entity_context *entity, u32 type, ico_ptr32 pos, u32 arg);
void sub_1683C8(ico_ptr32 ptr);

// ============================================================================
// Template data for state_block initialization
// Copied from 0x4C46B0 (0x80 bytes) + 0x4C4730 (0x10 bytes) = 0x90 total
//
// Layout (0x90 bytes, mostly zero-initialized):
//   [0x00] = variant        (dispatcher state index, set by cb_routine2)
//   [0x04] = rope_segments  (from entry param_30, default 1)
//   [0x08] = active_flag    (0=inactive, 1=active)
//   [0x0C] = hA_active      (set to 1 in hA body)
//   [0x10] = attached       (set to 1 in sub_1D2738)
//   [0x14] = external_arg   (passed through from hA)
//   [0x18] = identity_scale (1.0f)
//   [0x48] = max_segments   (0x12C = 300)
// ============================================================================
#define BARREL_TEMPLATE_BASE  0x4C46B0
#define BARREL_TEMPLATE_COPY  0x80

// ============================================================================
// init_fn (0x166028) — shared BARREL+ROPE
//
// Collects collision-dependent game objects (BARREL/ROPE) into a global
// array. Iterates all scene objects; those matching type=1 and having
// valid physics state are stored at gp_var array.
//
// Called from descriptor init_fn slot (+0x00). File: src/fieldCollision.c
// (assertion string at 0x5591F0: "TOO MANY COLLISION DEPEND GOBJS").
// ============================================================================
void barrel_init(void)
{
    ico_ptr32 entity;
    ico_ptr32 entity_state;
    u32 count;
    ico_ptr32 *array;

    count = 0;
    *(u32 *)((u8 *)0 + -0x6528) = 0;           // gp-relative counter

    entity = sub_13ECA8();                       // first scene object
    while (entity != 0) {
        entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);

        if (entity_state != 0) {
            if (*(ico_ptr32 *)((u8 *)entity_state + 0x70) != 0) {
                if (*(ico_ptr32 *)((u8 *)entity + 0x16C) != 0) {
                    if (*(u32 *)((u8 *)entity + 4) == 1) {
                        if ((s32)*(u32 *)((u8 *)entity + 8) >= 0) {
                            if (*(ico_ptr32 *)((u8 *)entity_state + 0x74) != 0) {
                                array = (ico_ptr32 *)(0x6AAC80 + count * 4);
                                count = count + 1;
                                *array = entity;
                            }
                        }
                    }
                }
            }
        }

        entity = sub_13ECF8();
    }

    if (count >= 0x100) {
        sub_1AD748((ico_ptr32)0x5591F0, 0x215); // assert: "src/fieldCollision.c" line 533
        sub_263FF0((ico_ptr32)0x5591F0, 0x215, (ico_ptr32)0x6323D0);
    }
}

// ============================================================================
// hA stubs (0x1D2540, 0x1D2548) — descriptor +0x48
//
// BARREL hA at 0x1D2540 is a tail-call to the common animation state setter.
// 0x1D2548 is a variant with a different animation ID.
// These are NOT separate functions — they are alternate entry points sharing
// the sub_1D12A8 body.
// ============================================================================
void barrel_hA(struct entity_context *entity)
{
    // Tail-calls sub_1D12A8(entity, 0x31)
    sub_1D12A8(entity, 0x31);
}

void barrel_hA_alt(struct entity_context *entity)
{
    // Tail-calls sub_1D12A8(entity, 0x32)
    sub_1D12A8(entity, 0x32);
}

// ============================================================================
// fn_1D2550 (0x1D2550) — hA body / state block updater
//
// Called with (entity, arg). If entity is NULL, performs debug init.
// Otherwise:
//   1. Sets state_block[0x0C] = 1 (active flag)
//   2. Clears state_block[0x08]
//   3. Stores arg to state_block[0x14]
//   4. Clears entity_state[0x74]
//   5. Updates model transform via sub_10D530
//   6. If state_block[4] == 1, loads transform from 0x276140
//   7. Copies transforms to stack and updates state_block + 0x20
//      (position/rotation data)
// ============================================================================
void fn_1D2550(struct entity_context *entity, u32 arg)
{
    ico_ptr32 entity_state;
    u32 *state_block;
    u8 buf_sp[0x40];   // 0x70 - 0x30 for saved regs
    u8 buf_s0[0x10];

    if (entity == 0) {
        sub_1A6E28((ico_ptr32)0x618F40, 0x158);  // debug init
        sub_1AD768((ico_ptr32)0x618F68, 0x158);  // assert: "src/item.c" line 344
        sub_263FF0((ico_ptr32)0x618F68, 0x158, (ico_ptr32)0x633420);
        return;
    }

    entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));

    state_block[3] = 1;           // +0x0C: active flag
    state_block[2] = 0;           // +0x08: clear
    state_block[5] = arg;         // +0x14: external parameter

    *(u32 *)((u8 *)entity_state + 0x74) = 0;

    sub_10D530(entity);            // update model transform

    if ((state_block[1] ^ 1) == 0) {   // state_id == 1
        sub_102870(entity, (ico_ptr32)0x276140);  // load default transform
    }

    sub_102850((ico_ptr32)buf_sp, entity);         // copy entity transform
    sub_102850((ico_ptr32)buf_s0, (struct entity_context *)arg); // copy arg as transform

    sub_10D838((ico_ptr32)buf_s0, (ico_ptr32)buf_s0); // process transform

    sub_10DDB8((ico_ptr32)(state_block + 8),        // state_block + 0x20
               (ico_ptr32)buf_s0,
               (ico_ptr32)buf_sp);
}

// ============================================================================
// sub_1D2650 (0x1D2650) — child update / initializer
//
// If arg is non-zero: updates two sub-transforms and calls sub_1683C8.
// If the resulting model ID at +0x88 is non-zero, tail-calls sub_104360
// to finalize the child transform.
// ============================================================================
void sub_1D2650(struct entity_context *child, u32 arg)
{
    ico_ptr32 ptr;

    if (arg == 0) {
        return;
    }

    ptr = (ico_ptr32)0x4C4760;

    sub_104508(ptr, child);                   // copy matrix from child
    sub_104508((ico_ptr32)((u8 *)ptr + 0x10), child);
    sub_1683C8(ptr);                          // process

    if (*(u32 *)((u8 *)ptr + 0x88) != 0) {
        sub_104508((ico_ptr32)((u8 *)ptr + 0x20), child); // final transform copy
        // Tail-call: sub_104360(child, ptr + 0x20)
    }

    return;
}

// ============================================================================
// sub_1D2738 (0x1D2738) — attach callback helper
//
// Marks state_block as attached (sets +0x10 = 1, +0x08 = 1)
// and calls sub_1D2650 to update child transforms.
// ============================================================================
void sub_1D2738(struct entity_context *entity, ico_ptr32 child_state)
{
    u32 *state_block;
    ico_ptr32 entity_state;

    entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));

    sub_1D2650(entity, state_block[5]);   // state_block[0x14]

    state_block[3] = 0;                   // +0x0C: clear active
    state_block[4] = 1;                   // +0x10: attached flag
    state_block[2] = 1;                   // +0x08: active
}

// ============================================================================
// hC BARREL (0x1D27A8) — constructor via descriptor +0x58
//
// Allocates 0x90-byte state_block, copies template from 0x4C46B0.
// Creates two child entities (woodbox/physics props) unless
// param_30 == 1 (single-segment rope mode).
//
// File: src/item.c, line 0x1B2 = 434.
// ============================================================================
ico_ptr32 barrel_hC(struct entity_context *entity, struct entry_record *entry)
{
    ico_ptr32 entity_state;
    u32 *state_block;
    ico_ptr32 alloc;
    ico_ptr32 heap;
    u32 copy_src;
    u32 copy_end;
    ico_ptr32 child;
    u32 a0_val, a1_val, a2_val, a3_val;
    u32 t0_val, t1_val, t2_val, t3_val;
    u32 seg_count;
    u32 stack_frame[11];   // sp+0x10 through sp+0x58

    heap = *(ico_ptr32 *)((u8 *)0 + -0x68E0);  // gp-relative heap pointer
    alloc = sub_13A0F8(heap, 0x90, (ico_ptr32)0x618F68, 0x1B2);  // "src/item.c" line 434
    entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);

    *(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET) = alloc;
    state_block = (u32 *)alloc;

    // Copy template from 0x4C46B0 (0x80 bytes = 8 qwords)
    copy_src = BARREL_TEMPLATE_BASE;
    copy_end = copy_src + BARREL_TEMPLATE_COPY;
    while (copy_src < copy_end) {
        *(u64 *)((u8 *)state_block + (copy_src - BARREL_TEMPLATE_BASE)) =
            *(u64 *)copy_src;
        copy_src += 8;
    }

    // Remaining 0x10 bytes from 0x4C4730 (all zeros)
    *(u64 *)((u8 *)state_block + 0x80) = *(u64 *)copy_src;
    *(u64 *)((u8 *)state_block + 0x88) = *(u64 *)(copy_src + 8);

    seg_count = (u32)(entry->param_30);
    state_block[1] = seg_count;              // +0x04: rope segment count
    *(u32 *)((u8 *)entity_state + 0x78) = 0;

    // Branch: if seg_count != 1, skip child creation (jump to epilogue)
    if (seg_count != 1) {
        goto no_children;
    }

    // --- Child entity creation ---
    a0_val = 10;
    a1_val = 69;
    a2_val = -1;
    a3_val = 1;
    t0_val = (u32)((u8 *)stack_frame);       // sp+0x10
    t1_val = -1;
    t2_val = 7;
    t3_val = 0;

    // Build stack frame
    stack_frame[0]  = *(u32 *)((u8 *)entry + 0x00);  // pos_x
    stack_frame[1]  = *(u32 *)((u8 *)entry + 0x04);  // pos_y
    stack_frame[2]  = *(u32 *)((u8 *)entry + 0x08);  // pos_z
    stack_frame[3]  = *(u32 *)((u8 *)entry + 0x0C);  // rot
    stack_frame[4]  = *(u32 *)((u8 *)entry + 0x10);  // scale_a
    stack_frame[5]  = *(u32 *)((u8 *)entry + 0x14);  // scale_b
    stack_frame[6]  = *(u32 *)((u8 *)entry + 0x18);  // unk_18
    stack_frame[7]  = *(u32 *)((u8 *)entry + 0x1C);  // unk_1C
    stack_frame[8]  = *(u32 *)((u8 *)entry + 0x20);  // unk_20
    stack_frame[9]  = *(u32 *)((u8 *)entry + 0x28);  // unk_28
    stack_frame[10] = *(u32 *)((u8 *)entry + 0x38);  // param_38

    // Overwrite frame[8] (at sp+0x40) with child type constant
    ((u32 *)stack_frame)[8] = 2;   // stack slot at base+0x30 = sp+0x40

    child = sub_1B7FE8(a0_val, a1_val, (ico_ptr32)a2_val, a3_val,
                       (ico_ptr32)t0_val, (ico_ptr32)t1_val,
                       (ico_ptr32)t2_val, (ico_ptr32)t3_val,
                       entity, 0);

    sub_1F2240(child, 1);
    sub_19F4E8(child, (ico_ptr32)stack_frame);

    // Copy child rotation data from template at 0x4C4750 ({0, -50, 0, 1})
    sub_105F00((ico_ptr32)((u8 *)(*(ico_ptr32 *)((u8 *)child + ENTITY_STATE_OFFSET)) + 0xA0),
               (ico_ptr32)0x4C4750);

    // Attach to physics group 10
    sub_203B78((ico_ptr32)(*(u32 *)((u8 *)entity + 0xC)), 10);  // entity type

    // Note: falls through to epilogue — there is more code after the branch target
    // (setting up a cylinder/sphere collision radius of 60.0 = 0x42700000)
    // This is shared with the no_children path.

no_children:
    // Epilogue: set collision radius and return
    // ... (collision radius setup)
    return alloc;
}

// ============================================================================
// hC ROPE (0x1D3B28) — constructor via descriptor +0x58
//
// Simpler than BARREL hC: no allocation, no child creation.
// Checks variant early exit; otherwise dispatches to hA if state is
// idle and counter allows, or updates transforms via sub_10ECB8.
// ============================================================================
void rope_hC(struct entity_context *entity)
{
    u32 *state_block;
    ico_ptr32 entity_state;
    u32 variant;

    entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));

    variant = state_block[0];
    if (variant == 1) {
        return;                 // early exit: already active
    }

    if ((state_block[1] ^ 1) != 0) {     // state_id != 1
        goto update_transform;
    }

    // Counter-based dispatch (from state_block-like data at entity+0x40)
    {
        u32 *entity_data = (u32 *)((u8 *)entity + 0x40);
        u32 counter = entity_data[2];       // +0x48 from entity? sp+0x08 in function

        if (counter < 2) {
            if (counter == 1) {
                // Check world state flag
                if (*(u32 *)0x276ED4 != 0) {
                    if (entity_data[9] != 0) {   // entity+0x64
                        sub_1D12D8(entity);
                        entity_data[9] = 0;
                        goto call_hA;
                    }
                } else {
                    if (entity_data[9] == 0) {   // entity+0x64
                        sub_1D12D8(entity);
                    } else {
                        sub_1D2540(entity);      // call hA stub
                    }
                    entity_data[9] = counter;    // store counter
                }
            }
        }
    }

call_hA:
    sub_1D12A8(entity, 0x31);

update_transform:
    sub_10ECB8(entity);           // update entity transforms
}

// ============================================================================
// cb_routine2 (0x1D3A30) — BARREL physics constraint solver
// (Previously misidentified as "ROPE callback")
//
// This is the central update function for BARREL's dispatcher.
// It manages child entity interactions and dispatches to the
// 5-state jump table at 0x618FB0 for cloth/rope physics steps.
//
// Flow:
//   1. Load state_block, check variant == 1 (early exit)
//   2. If child attachment present, process child state
//   3. Update transforms via sub_102858
//   4. If state active (!variant check), dispatch to sub_1D37C8
//
// Called from descriptor +0x50 (hB slot) and from sub_1D2738 path.
// ============================================================================
void cb_routine2(struct entity_context *entity)
{
    ico_ptr32 entity_state;
    u32 *state_block;
    struct entity_context *child;
    u32 variant;
    u32 *child_state;
    u32 saved_flag;

    entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));

    variant = state_block[0];
    if (variant == 1) {
        return;                     // state already active, exit
    }

    // Child interaction
    if (state_block[3] != 0) {      // +0x0C: child attached?
        sub_1D29B8(entity);         // process child physics
    } else {
        if (state_block[2] == 0) {  // +0x08: no child flag
            sub_1D2BF0(entity);     // run child-less physics
        } else {
            // Child reference exists
            saved_flag = *(u32 *)((u8 *)entity_state + 0x74);
            *(u32 *)((u8 *)entity_state + 0x74) = 0;
            sub_1D2BF0(entity);     // enhanced physics with child
            if (saved_flag != 0) {
                *(u32 *)((u8 *)entity_state + 0x74) = 1;
            }
        }
    }

    // Check child entity interaction
    child = (struct entity_context *)state_block[0];   // re-read as child ptr
    if (child != 0) {
        if (*(u32 *)((u8 *)child + 0xC) == 0x11) {    // child type WOODBOX0
            if (sub_1C05A8(child) == 2) {              // check interaction state
                state_block[5] = 0;                    // clear external arg
                child_state = *(u32 *)((u8 *)child + ENTITY_STATE_OFFSET);
                sub_1D2738(entity, child_state + 0x130);
            }
        }
    }

    // Update entity transform
    sub_102858(entity);

    // Final state dispatch
    entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));

    if ((state_block[1] ^ 1) != 0) {     // if state_id != 1
        return;                           // not in active state
    }

    sub_1D37C8(entity);                   // dispatch to 5-state jump table
}

// ============================================================================
// fn_1D3BF0 (0x1D3BF0) — state sub-dispatcher (called from state blocks)
//
// Manages per-state physics iterations. If state is idle (state_id == 1),
// sets completion flag (+0x48 = 2). Otherwise:
//   1. Captures entity transform
//   2. Looks up model ID in table 0x4D4188 indexed by state_id
//   3. If model ID != 0x32F (815 sentinel), builds transform chain
//      and attaches model via sub_1EBC10
//   4. Calls sub_1D12A8(entity, 0x2A)
//   5. If state_id == 6, calls sub_181BF8 with radius 100.0
//      (barrier/destruction check)
//   6. Sets variant = 1 (state complete)
//
// Table at 0x4D4188 has 0x14-byte stride (5 fields):
//   field[0..4] = model_id, model_id2, model_id3, model_id4, flags
//   sentinel 0x32F = skip model processing
// ============================================================================
u32 fn_1D3BF0(struct entity_context *entity)
{
    ico_ptr32 entity_state;
    u32 *state_block;
    u32 state_id;
    u32 table_entry;
    void *model_id;
    u8 buf_transform[0x30];
    u8 buf_stk[0x10];

    entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));

    state_id = state_block[1];                // +0x04

    if ((state_id ^ 1) == 0) {                // state_id == 1 (idle)
        state_block[0x12] = 2;                // +0x48: completion flag
        return 0;
    }

    // Capture entity transform to stack
    sub_104508((ico_ptr32)buf_transform, entity);

    // Look up model ID from state table
    state_id = state_block[1];                // reload (may have changed)
    // Table at 0x4D4188, stride 0x14 (5 u32s), field[2] at +8
    {
        u32 tbl_idx = state_id * 5 + 2;
        u32 *model_table = (u32 *)0x4D4188;
        model_id = (void *)(u32)model_table[tbl_idx];
    }

    if ((u32)model_id != 0x32F) {             // 0x32F = no-model sentinel
        // Build transform with identity scale
        sub_2641D8((ico_ptr32)buf_stk, 0, 0x10);
        *(float *)((u8 *)buf_stk + 0xC) = 1.0f;

        sub_105F00((ico_ptr32)buf_transform, (ico_ptr32)0x276850); // copy rotation

        // Set position
        *(u32 *)((u8 *)buf_transform + 0x24) = 0;

        sub_1182F0((ico_ptr32)buf_transform, (ico_ptr32)buf_transform);

        // Calculate heading angle from position
        {
            float px = *(float *)((u8 *)buf_transform + 0x20);
            float pz = *(float *)((u8 *)buf_transform + 0x28);
            s32 heading = (s32)sub_10EC08(px, pz);
            sub_10E158((ico_ptr32)buf_stk, heading);
        }

        sub_1EBC10((ico_ptr32)model_id, (ico_ptr32)buf_transform, (ico_ptr32)buf_stk);
    }

    // Common exit path
    sub_1D12A8(entity, 0x2A);                  // update animation state

    entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));

    if (state_block[1] == 6) {                // destruction/barrier state
        sub_181BF8(entity, 0x11, (ico_ptr32)buf_transform, 0);
        entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
        state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));
    }

    *(u32 *)((u8 *)entity_state + 0x74) = 0;   // clear collision flag
    state_block[0] = 1;                         // variant = 1 (state complete)
    *(u32 *)((u8 *)entity + 0x16C) = 0;         // clear activity flag

    return 0;
}

// ============================================================================
// fn_1D3D40 (0x1D3D40) — state check helper
//
// Returns 1 if:
//   - entity+0x16C is non-zero (activity flag set) AND
//   - state_block[2] (active flag at +0x08) is zero AND
//   - state_block[0x12] (state_index at +0x48) < 2
//
// Otherwise returns 0.
// ============================================================================
u32 fn_1D3D40(struct entity_context *entity)
{
    ico_ptr32 entity_state;
    u32 *state_block;
    u32 result;

    result = 0;
    entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);

    if (*(u32 *)((u8 *)entity + 0x16C) != 0) {
        state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));
        if (state_block[2] == 0) {                  // +0x08: active flag?
            if ((s32)state_block[0x12] < 2) {       // +0x48: state_index < 2
                result = 1;
            }
        }
    }

    return result;
}

// ============================================================================
// fn_1D3D70 (0x1D3D70) — state_id getter
//
// Returns state_block[1] (state_id at +0x04).
// ============================================================================
u32 fn_1D3D70(struct entity_context *entity)
{
    ico_ptr32 entity_state;
    u32 *state_block;

    entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));

    return state_block[1];
}

// ============================================================================
// fn_1D3D80 (0x1D3D80) — variant getter (with saturation)
//
// Returns state_block[0] (variant) saturated to 0/1.
// ============================================================================
u32 fn_1D3D80(struct entity_context *entity)
{
    ico_ptr32 entity_state;
    u32 *state_block;

    entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));

    return (state_block[0] < 1) ? 1 : 0;
}

// ============================================================================
// fn_1D3D98 (0x1D3D98) — state completion check
//
// Returns 1 if state_block[0x12] (state_index at +0x48) != 2 (not done).
// ============================================================================
u32 fn_1D3D98(struct entity_context *entity)
{
    ico_ptr32 entity_state;
    u32 *state_block;

    entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));

    return ((state_block[0x12] ^ 2) == 0) ? 0 : 1;
}

// ============================================================================
// fn_1D3DB0 (0x1D3DB0) — active state check
//
// If state_block[1] ^ 1 != 0 (state_id != 1), returns 0.
// Otherwise returns state_block[0x10] (field at +0x40).
// ============================================================================
u32 fn_1D3DB0(struct entity_context *entity)
{
    ico_ptr32 entity_state;
    u32 *state_block;

    entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));

    if ((state_block[1] ^ 1) != 0) {     // state_id != 1
        return 0;
    }

    return state_block[0x10];             // field at +0x40
}

// ============================================================================
// fn_1D3DD8 (0x1D3DD8) — entity iterator / barrier cleanup
//
// Iterates all entities of type 0x13 (19 = BARREL). For each entity that
// has the active flag set (entity+0x16C != 0), is not marked active
// (state_block+0x08 == 0), and has state_index < 2:
//   - Calls sub_19F530 to destroy/release the entity
//   - Clears state_block[0x74]
//   - Sets state_block+0x08 = 1 (mark as processed)
// Returns 1.
// ============================================================================
u32 fn_1D3DD8(void)
{
    struct entity_context *entity;
    ico_ptr32 entity_state;
    u32 *state_block;
    u32 should_process;

    entity = (struct entity_context *)sub_13EB50(0x13);  // first entity type 0x13 (BARREL)

    while (entity != 0) {
        should_process = 0;
        entity_state = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);

        if (*(u32 *)((u8 *)entity + 0x16C) != 0) {
            state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));
            if (state_block[2] == 0) {                  // +0x08: not already processed
                should_process = 1;
                if ((s32)state_block[0x12] >= 2) {      // +0x48: state_index >= 2
                    should_process = 0;                   // skip: state too far
                }
            }
        }

        if (should_process != 0) {
            state_block = (u32 *)(*(ico_ptr32 *)((u8 *)entity_state + STATE_BLOCK_OFFSET));
            sub_19F530(entity);                          // destroy/release entity
            *(u32 *)((u8 *)state_block + 0x74) = 0;
            state_block[2] = 1;                          // +0x08: mark processed
        }

        entity = (struct entity_context *)sub_13EBE0((ico_ptr32)entity); // next entity
    }

    return 1;
}
