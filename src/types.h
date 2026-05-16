#ifndef ICO_TYPES_H
#define ICO_TYPES_H

/* ICO internal pointer: 32-bit pointers stored as int in structs.
 * Confirmed by 8 C scratch tests (Rev.048). Using int generates lw/sw
 * which matches the target. void* would generate ld (64-bit, wrong).
 */
typedef int ico_ptr32;

/* ICO uses unsigned int for state_id and flag fields.
 * Confirmed: original binary uses lw for all word loads,
 * never lwu. The cast via ico_ptr32 handles this.
 */
typedef unsigned int ico_u32;
typedef signed int ico_s32;
typedef unsigned long long ico_u64;

#endif /* ICO_TYPES_H */
