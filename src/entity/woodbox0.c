// Near-structural decompilation for WOODBOX0 (descriptor index 17)
// Based on MIPS64 disassembly (ee-gcc 2.9-991111-01, -O2)
// ELF: SCUS_971.13, single PT_LOAD segment (VA 0x100000 -> file 0x1000)
//
// Functions (Rev.088, 2026-05-18):
//   hC  (0x1C00C0) -- per-instance constructor  via descriptor [+0x58]; ~288 insns
//   hB  (0x1C0538) -- per-frame update           via descriptor [+0x50]; ~45 insns
//   hA  (0x1C05D0) -- conditional handler        via descriptor [+0x48]; ~40 insns
//   cb_collision (0x1C11C0) -- collision callback set at scene_obj[+0x7EC]
//
// Key observations (Rev.088):
//   WOODBOX0 descriptor entry at 0x2A385C (index 17, not index 3 as previously
//   believed). The hA/hB/hC addresses from the ELF descriptor [+0x48/+0x50/+0x58]
//   are 0x1C05D0/0x1C0538/0x1C00C0 respectively.
//
//   The previous mapping (hA=0x1D1400, hB=0x1CF288, hC=0x1D15B0) belongs to
//   GIRL (Yorda) at descriptor index 2 — NOT WOODBOX0.
//
//   Uses Group B behavior_fn (0x23D660 — shared physics props behavior).
//   Work struct is 0x190 bytes, heap-allocated in hC (tag 0x7AC).
//   Full 0x190-byte work struct template-copied from ROM 0x4B5560.
//
//   hB increments 31-frame counter; on wrap, tail-calls sub_1AE460.
//   hA checks flags, world state; clears destruct_state at work+0x138.

#include "../types.h"
#include "structs.h"

// ============================================================================
// Forward declarations
// ============================================================================

// Allocator
ico_ptr32 sub_13A0F8(ico_ptr32 heap, u32 size, ico_ptr32 tag, u32 line);

// Entity core
ico_ptr32 sub_1B7FE8(struct entity_context *);
void sub_1BDA70(struct entity_context *, ico_ptr32);
void sub_1BDC58(struct entity_context *, ico_ptr32);
void sub_1BD408(struct entity_context *, ico_ptr32, u32);
void sub_1BCC18(struct entity_context *, ico_ptr32);

// Physics / collision
void sub_1BD668(struct entity_context *, u32 flags);
void sub_1C11C0(struct entity_context *);  // collision callback

// Animation / transform
void sub_1BF2C8(ico_ptr32 work);
void sub_102858(ico_ptr32 work);
void sub_1AE460(ico_ptr32 ptr, u32 param);

// Audio
void sub_10ECD8(void);
void sub_10ECB8(struct entity_context *);

// World state / child
void sub_10BBB0(ico_ptr32 child);
ico_ptr32 sub_19F310(u32 type, ico_ptr32 initializer);

// GP-relative globals
#define GP_BASE         0x00633D14
#define HEAP_PTR        (*(ico_ptr32 *)(GP_BASE - 0x68E0))
#define WORLD_STATE     (*(u32 *)(GP_BASE - 0x4ED4))

// ============================================================================
// Struct: private work area per WOODBOX0 instance
// Size: 0x190 (400 bytes), heap-allocated in hC, tag 0x7AC
// Stored at: scene_obj[0x800]
// Template-copied from ROM 0x4B5560
// ============================================================================
struct woodbox0_work {
    u32          counter;           // +0x000: per-frame counter (mod 0x1F)
    ico_u64      template_quads[3]; // +0x004: template data (4 quads)
    float        scale_x;           // +0x020: scale X
    float        scale_y;           // +0x024: scale Y
    float        scale_z;           // +0x028: scale Z
    float        scale_factor;      // +0x02C: from scene_obj[+0x70]
    u8           pad_030[0x28];     // +0x030 to +0x057
    u32          flags;             // +0x058: from scene_obj[+0x30]
    u32          action;            // +0x05C: from counter-based lookup
    u8           pad_060[0xD4];     // +0x060 to +0x133
    float        destruct_time;     // +0x134: from world state + 0x34
    u32          destruct_state;    // +0x138: cleared in hA
    u8           pad_13C[0x24];     // +0x13C to +0x15F
    ico_ptr32    child_entity;      // +0x160: child entity pointer
    u8           pad_164[0x1C];     // +0x164 to +0x17F
    ico_ptr32    scene_obj_ptr;     // +0x180: cached scene object
};

// Scene_obj offsets used by WOODBOX0:
// +0x800 : woodbox0_work* (this alloc)
// +0x30  : flags (copied to work[+0x58])
// +0x70  : scale_factor / type ID
// +0x7EC : collision callback (set to 0x1C11C0)
// +0x840 : animation data pointer

// ============================================================================
// hC (0x1C00C0) -- per-instance constructor
//
// Called from descriptor[+0x58] (cb_routine3). Returns woodbox0_work ptr.
//
// 1. Allocate 0x190-byte work struct (tag 0x7AC, string "6A")
// 2. Store at entity->entity_state[0x800]
// 3. Copy 0x190-byte template from ROM 0x4B5560 into work
// 4. Read scene_obj[+0x70] → scale_factor
// 5. Call sub_1B7FE8 → scene_obj_ptr (stored to work[+0x180])
// 6. Create child entity via sub_19F310
// 7. Set collision callback at scene_obj[+0x7EC] = 0x1C11C0
// 8. If flags at scene_obj[+0x30] != 0:
//      use alternate animation path
//    Else:
//      load animation from 0x1C0AEC
// 9. Call sub_1BDA70 (matrix/transform setup)
// 10. Call sub_1BDC58 (physics init)
// 11. Call sub_1BD408 (cloth assignment)
// 12. Call sub_1BCC18 (cloth init)
// 13. Load destruct_time from world state area
// 14. Return work pointer
// ============================================================================
ico_ptr32 woodbox0_hC(struct entity_context *entity, ico_ptr32 initializer)
{
    struct woodbox0_work *wk;
    ico_ptr32 scene_obj;
    u32 flags;
    u32 type_id;
    int i;

    scene_obj = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);

    wk = (struct woodbox0_work *)sub_13A0F8(
        HEAP_PTR, 0x190,
        (ico_ptr32)0x006186A0, 0x7AC);

    *(ico_ptr32 *)((u8 *)scene_obj + STATE_BLOCK_OFFSET) = (ico_ptr32)wk;

    // Copy 0x190-byte template from ROM
    {
        ico_u64 *src = (ico_u64 *)0x004B5560;
        ico_u64 *dst = (ico_u64 *)wk;
        for (i = 0; i < 0x190 / 8; i++)
            dst[i] = src[i];
    }

    type_id = (u32)(*(float *)((u8 *)scene_obj + 0x70));
    flags = *(u32 *)((u8 *)scene_obj + 0x30);

    // Overwrite work fields with scene data
    wk->scale_factor = (float)type_id;
    wk->flags = flags;
    wk->scene_obj_ptr = (ico_ptr32)sub_1B7FE8(entity);

    // Create child entity
    wk->child_entity = sub_19F310(type_id, initializer);

    // Set collision callback
    *(ico_ptr32 *)((u8 *)scene_obj + 0x7EC) = (ico_ptr32)0x1C11C0;

    // Animation data load (two paths based on flags)
    if (flags != 0) {
        // Alternate path: uses scene_obj[+0x840]
        // (animation data from alternate source)
    } else {
        // Main path: animation data from ROM 0x1C0AEC
    }

    // Common init chain
    sub_1BDA70(entity, (ico_ptr32)wk);
    sub_1BDC58(entity, (ico_ptr32)wk);
    sub_1BD408(entity, (ico_ptr32)wk, type_id);
    sub_1BCC18(entity, (ico_ptr32)wk);

    // Load destruct time from world-state-related data
    wk->destruct_time = *(float *)((u8 *)scene_obj + 0x34);

    return (ico_ptr32)wk;
}

// ============================================================================
// hB (0x1C0538) -- per-frame update handler
//
// Called via descriptor[+0x50] each frame.
//
// 1. Get work from entity->entity_state[0x800]
// 2. Increment counter at work[+0x00]; wrap at 0x1F (31)
// 3. Call sub_1BF2C8(work+0x20) — scale/animation update
// 4. Call sub_102858(work+0x20) — transform update
// 5. If counter wrapped (was >= 0x1F), tail-call sub_1AE460(work[+0x180], 0x14)
// 6. Otherwise return
// ============================================================================
void woodbox0_hB(struct entity_context *entity)
{
    struct woodbox0_work *wk;
    ico_ptr32 scene_obj;

    scene_obj = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    wk = *(struct woodbox0_work **)((u8 *)scene_obj + STATE_BLOCK_OFFSET);

    wk->counter++;
    if (wk->counter >= 0x1F)
        wk->counter = 0;

    sub_1BF2C8((ico_ptr32)&wk->scale_x);
    sub_102858((ico_ptr32)&wk->scale_x);

    if (wk->counter == 0)
        sub_1AE460(wk->scene_obj_ptr, 0x14);
}

// ============================================================================
// hA (0x1C05D0) -- conditional handler
//
// Called via descriptor[+0x48] per dispatch cycle.
//
// 1. Get work from entity->entity_state[0x800]
// 2. Call sub_10ECD8 (audio event)
// 3. Call sub_10ECB8(entity) (audio setup)
// 4. If work[+0x58] (flags) non-zero:
//      call sub_1BD668(entity, flags) — conditional physics update
// 5. Check WORLD_STATE (gp-0x4ED4)
//    If match: call sub_10BBB0(child_entity)
// 6. Clear work[+0x138] (destruct_state)
// 7. Return
// ============================================================================
void woodbox0_hA(struct entity_context *entity)
{
    struct woodbox0_work *wk;
    ico_ptr32 scene_obj;

    scene_obj = *(ico_ptr32 *)((u8 *)entity + ENTITY_STATE_OFFSET);
    wk = *(struct woodbox0_work **)((u8 *)scene_obj + STATE_BLOCK_OFFSET);

    sub_10ECD8();
    sub_10ECB8(entity);

    if (wk->flags != 0)
        sub_1BD668(entity, wk->flags);

    // Unknown condition on WORLD_STATE (gp-0x4ED4)
    // If match: sub_10BBB0(child_entity) called

    wk->destruct_state = 0;
}
