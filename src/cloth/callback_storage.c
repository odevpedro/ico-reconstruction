#include "structs.h"

/* =================================================================
 * ASM-HOLD — node_callback_storage (0x0013F3F0)
 *
 * 576 bytes (0x240). Stack frame: 144 bytes (0x90).
 * Linked-list callback storage with stride 0x94.
 * Max 3 linear nodes scanned before creating a new one.
 * Two alloc calls to 0x1A6E28 (node + body).
 * Fallback: [entity+0x1C] = data directly.
 *
 * Complexity too high for NEAR-STRUCTURAL classification:
 * - Linked list traversal with forward/backward links
 * - Dual alloc path with t1-controlled size
 * - Two-call pattern from 0x13F7A8 (main + sister at obj+0x10)
 * - Fallback with direct store at +0x1C
 * ================================================================= */

/* =================================================================
 * ASM-HOLD — callback_registration (0x0013F7A8)
 *
 * 44 bytes. Thin wrapper:
 *   a2 = a1 (original data pointer preserved)
 *   a1 = a3 (callback type = 0x13 for cloth)
 *   jal 0x13F3F0(a0=obj, a1=type, a2=data, a3=&spill)
 *   jal 0x13F3F0(a0=obj+0x10, a1=type, a2=data, a3=&spill)
 *
 * Registers the same callback on BOTH main and sister storage.
 * ================================================================= */

/* =================================================================
 * ASM-HOLD — callback_system_reg (0x0013F7D8)
 *
 * 36 bytes. Context-free registration:
 *   a0 = 0x194 (hardcoded system obj)
 *   a1 = 0 (no context ptr)
 *   t1 = 0x1800 (fixed buffer size)
 *   jal 0x13F3F0(a0=0x194, a1=0, a2=NULL, a3=&spill)
 *
 * Only ONE call (no sister call). Used for system-level callbacks.
 * ================================================================= */
