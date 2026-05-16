#include "structs.h"

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
