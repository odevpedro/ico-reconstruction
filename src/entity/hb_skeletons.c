#include "../types.h"
#include "../cloth/structs.h"

/* =================================================================
 * hB (update handler) skeleton models — Rev.060
 *
 * These are ASM-HOLD structural summaries, not compilable C models.
 * Complexity and size exceed NEAR-STRUCTURAL feasibility.
 * ================================================================= */

/* =================================================================
 * ENEMY1 hB (0x1CE3C0) — 142 insns, 112B stack
 *
 * State: counter at [s3+0x4C] (< 11 decrements). 3 AI modes.
 * Calls: 0x1654C8 (state get), 0x165540 (state action), 2-pass draw
 *
 * Pseudocode:
 *   if entity.flags & 1: counter = 0
 *   else if ++counter >= 11: return
 *   state_550 = 0, state_54C = 2, state_548 = 0
 *   mode = ai_state_get(entity)       // 0x1654C8
 *   if mode == 3: state_550 = 1
 *   shadow_draw(entity)               // 0x1E3FC8
 *   move_check(entity, f12, f13)      // 0x103F00
 *   action = ai_action(entity)        // 0x165540
 *   if action: draw_sprites(entity, s3[0x28])  // 0x1CF6C0
 *   counter = (counter + 1) % 10
 *   for pass in [0, 1]:
 *     child = get_child(entity, 37 - pass)
 *     draw_sprite_child(entity, child, brightness)
 *   return
 * ================================================================= */

/* =================================================================
 * BOY hB (0x1C1DD8) — 176 insns, 3 sub-functions
 *
 * Sub-fn A (main, 48B):
 *   cloth_update(entity)     // 0x1D23E0
 *   shadow_draw(entity)      // 0x1E3FC8
 *   child_update_A(entity)   // 0x1C1C48
 *   child_update_B(entity)   // 0x1C12F0
 *   seeker_update(entity)    // 0x1E4868
 *   state_check(entity)
 *   collision_4(entity, 50f, 50f)  // 0x103D50
 *   if hit: event(entity, 6)
 *   collision_2(entity, cond?30:50, 50f)  // tail call 0x103D50
 *
 * Sub-fn B (0x1C1EA8, 64B) — collision/transform:
 *   child = get_child(entity, 35)     // 0x109F10
 *   switch child[+0x00]:
 *     case 1: slot = child[+0x08]
 *     case 2: slot = child[+0x0C]
 *     default: slot = child[+0x04]
 *   matrix = entity[+0x15C]→payload[+0x0C] + idx * 0x40
 *   draw_collision(entity, matrix)
 *   collision_detect(entity)          // tail 0x121D90
 *
 * Sub-fn C (0x1C1F58, 96B) — head/weapon:
 *   if head_attached:
 *     transform_setup()
 *     collision_check(424)
 *     save [s2+0x14] = result
 *   else:
 *     recovery_update(entity)         // 0x10ECD8 + 0x10ECB8
 *     sub_fn_B(entity)
 *     child_draw(entity)
 *     if world_state == 39:
 *       if pos > 20.0 AND target != 0:
 *         if can_slope(target):
 *           slope_collision(entity, target)
 * ================================================================= */

/* =================================================================
 * GIRL hB (0x1D17F8) — 113 insns
 *
 * Similar to BOY hB but:
 *   - 2 cloth calls (0x1D23E0 + 0x1D14B8)
 *   - GIRL-specific AI (0x243AE8 + 0x243950)
 *   - Animation blend (0x174D78)
 *   - No children/seeker updates
 *   - Simpler body structure
 *
 * Pseudocode:
 *   cloth_update(entity)
 *   shadow_draw(entity)
 *   state_check(entity)
 *   collision_4(entity, ...)        // 0x103D50
 *   if hit: event(entity, 6)
 *   timing_check()                   // 0x14A0D8
 *   collision_4_repeat(...)
 *   if hit: event(entity, 6)
 *   animation_blend(entity)          // 0x174D78
 *   ai_A(entity), ai_B(entity)       // 0x243AE8, 0x243950
 *   entity_draw(entity)              // 0x105FE0
 *   cloth_cleanup(entity)            // 0x1D14B8
 *   return
 * ================================================================= */
