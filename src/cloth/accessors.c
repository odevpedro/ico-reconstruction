#include "structs.h"

/* 0x001D3D70 (16B) — EXACT match
 * Returns the variant_id from the cloth payload.
 */
int cloth_get_variant(struct cloth_context *ctx) {
    ico_ptr32 entity = *(ico_ptr32*)((char*)ctx + 0x15C);
    ico_ptr32 payload = *(ico_ptr32*)((char*)entity + 0x800);
    return *(int*)((char*)payload + 4);
}

/* 0x001D3D80 (24B) — EXACT match
 * Returns 1 if field_00 is zero (payload inactive?).
 */
int cloth_payload_field0_is_zero(struct cloth_context *ctx) {
    ico_ptr32 entity = *(ico_ptr32*)((char*)ctx + 0x15C);
    ico_ptr32 payload = *(ico_ptr32*)((char*)entity + 0x800);
    return *(ico_u32*)((char*)payload + 0) < 1;
}

/* 0x001D3D98 (24B) — EXACT match
 * Returns 1 if state_id == 2.
 */
int cloth_payload_state_is_two(struct cloth_context *ctx) {
    ico_ptr32 entity = *(ico_ptr32*)((char*)ctx + 0x15C);
    ico_ptr32 payload = *(ico_ptr32*)((char*)entity + 0x800);
    return (*(ico_u32*)((char*)payload + 0x48) ^ 2) < 1;
}
