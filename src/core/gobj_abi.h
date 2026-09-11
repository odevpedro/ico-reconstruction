#ifndef ICO_GOBJ_ABI_H
#define ICO_GOBJ_ABI_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t  s32;

/* A pointer stored in the original EE ABI is always one 32-bit word. */
typedef u32 ico_ptr32;

enum {
    ICO_GOBJ_SIZE = 0x174,
    ICO_PROCESS_NODE_SIZE = 0x94,
    ICO_GOBJ_PRIMARY_LIST_COUNT = 8,
    ICO_GOBJ_DL_LIST_COUNT = 8,
    ICO_GOBJ_DL_MASK_BITS = 32,
    ICO_GOBJ_TYPE_TABLE_ENTRIES = 0x44
};

enum {
    ICO_GOBJ_PRIMARY_HEADS_ADDRESS = 0x00281A70,
    ICO_GOBJ_PRIMARY_TAILS_ADDRESS = 0x00281A90,
    ICO_GOBJ_DL_HEADS_ADDRESS = 0x00281AB0,
    ICO_GOBJ_DL_TAILS_ADDRESS = 0x00281AD0
};

typedef struct IcoGObj {
    ico_ptr32 self;                 /* +0x000: zero marks a free pool slot */
    s32       unknown_004;          /* +0x004: initialized to -1 */
    s32       unknown_008;          /* +0x008: initialized to -1 */
    u32       type;                 /* +0x00C: kind-table selector */
    ico_ptr32 next;                 /* +0x010: primary-list next */
    ico_ptr32 prev;                 /* +0x014: primary-list previous */
    u8        list_id;              /* +0x018: primary-list index */
    u8        unknown_019[3];       /* +0x019..+0x01B */
    u32       sort_key;             /* +0x01C: ordered-list comparison key */
    u8        unknown_020[8];       /* +0x020..+0x027 */
    ico_ptr32 user_data;            /* +0x028: add argument; also jalr'd by _iosOmMain */
    ico_ptr32 process_head;         /* +0x02C: attached ProcessNode head */
    ico_ptr32 process_tail;         /* +0x030: attached ProcessNode tail */
    ico_ptr32 dl_next;              /* +0x034: DL-list next */
    ico_ptr32 dl_prev;              /* +0x038: DL-list previous */
    ico_ptr32 type_next;            /* +0x03C: kind-table chain next */
    u8        dl_list_id;           /* +0x040: DL-list index */
    u8        unknown_041[3];       /* +0x041..+0x043 */
    u32       dl_sort_key;          /* +0x044: DL ordered-list comparison key */
    ico_ptr32 callback;             /* +0x048: DL callback */
    u32       slot_mask;            /* +0x04C: per-object DL participation mask */
    u32       type_mask;            /* +0x050: callback filtering mask */
    u8        unknown_054[4];       /* +0x054..+0x057 */
    u32       unknown_058;          /* +0x058: cleared by isysGObjAdd */
    u8        unknown_05c[0x100];   /* +0x05C..+0x15B */
    u32       state_15c;            /* +0x15C: cleared by alloc and add */
    u32       unknown_160;          /* +0x160 */
    u32       unknown_164;          /* +0x164: cleared when a slot is selected */
    u32       unknown_168;          /* +0x168 */
    u32       state_16c;            /* +0x16C: dispatch gate */
    u32       state_170;            /* +0x170: cleared when a slot is selected */
} IcoGObj;

typedef struct IcoProcessNode {
    ico_ptr32 self;                 /* +0x00: zero marks a free pool slot */
    ico_ptr32 parent;               /* +0x04: owning GObj */
    ico_ptr32 next;                 /* +0x08: process-list next */
    ico_ptr32 prev;                 /* +0x0C: process-list previous */
    u32       type_mask;            /* +0x10: callback/thread mode */
    u32       priority;             /* +0x14: sort key and dispatch selector */
    u32       active;               /* +0x18: zero skips dispatch */
    ico_ptr32 callback;             /* +0x1C: callback pointer */
    u32       unknown_020;          /* +0x20 */
    u8        tcb_area[0x70];       /* +0x24..+0x93: embedded ios/thread.c state */
} IcoProcessNode;

typedef struct IcoGObjPointerTable8 {
    ico_ptr32 entries[8];
} IcoGObjPointerTable8;

/* Type table (0x6A93D0): one head per type, chained via IcoGObj.type_next. */
typedef struct IcoGObjTypeHeadTable {
    ico_ptr32 entries[ICO_GOBJ_TYPE_TABLE_ENTRIES];
} IcoGObjTypeHeadTable;

#if defined(__cplusplus)
#define ICO_STATIC_ASSERT(condition, message) static_assert((condition), message)
#else
#define ICO_STATIC_ASSERT(condition, message) _Static_assert((condition), message)
#endif

ICO_STATIC_ASSERT(sizeof(ico_ptr32) == 4, "ico_ptr32 must be 32 bits");
ICO_STATIC_ASSERT(sizeof(IcoGObj) == 0x174, "IcoGObj size must be 0x174");
ICO_STATIC_ASSERT(sizeof(IcoProcessNode) == 0x94,
                  "IcoProcessNode size must be 0x94");

ICO_STATIC_ASSERT(offsetof(IcoGObj, self) == 0x00, "IcoGObj.self offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, type) == 0x0C, "IcoGObj.type offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, next) == 0x10, "IcoGObj.next offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, prev) == 0x14, "IcoGObj.prev offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, list_id) == 0x18, "IcoGObj.list_id offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, user_data) == 0x28, "IcoGObj.user_data offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, process_head) == 0x2C,
                  "IcoGObj.process_head offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, dl_next) == 0x34, "IcoGObj.dl_next offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, dl_prev) == 0x38, "IcoGObj.dl_prev offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, type_next) == 0x3C,
                  "IcoGObj.type_next offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, callback) == 0x48,
                  "IcoGObj.callback offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, slot_mask) == 0x4C,
                  "IcoGObj.slot_mask offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, type_mask) == 0x50,
                  "IcoGObj.type_mask offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, state_15c) == 0x15C,
                  "IcoGObj.state_15c offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, state_16c) == 0x16C,
                  "IcoGObj.state_16c offset");
ICO_STATIC_ASSERT(offsetof(IcoGObj, state_170) == 0x170,
                  "IcoGObj.state_170 offset");

ICO_STATIC_ASSERT(offsetof(IcoProcessNode, self) == 0x00,
                  "IcoProcessNode.self offset");
ICO_STATIC_ASSERT(offsetof(IcoProcessNode, parent) == 0x04,
                  "IcoProcessNode.parent offset");
ICO_STATIC_ASSERT(offsetof(IcoProcessNode, next) == 0x08,
                  "IcoProcessNode.next offset");
ICO_STATIC_ASSERT(offsetof(IcoProcessNode, prev) == 0x0C,
                  "IcoProcessNode.prev offset");
ICO_STATIC_ASSERT(offsetof(IcoProcessNode, type_mask) == 0x10,
                  "IcoProcessNode.type_mask offset");
ICO_STATIC_ASSERT(offsetof(IcoProcessNode, priority) == 0x14,
                  "IcoProcessNode.priority offset");
ICO_STATIC_ASSERT(offsetof(IcoProcessNode, active) == 0x18,
                  "IcoProcessNode.active offset");
ICO_STATIC_ASSERT(offsetof(IcoProcessNode, callback) == 0x1C,
                  "IcoProcessNode.callback offset");
ICO_STATIC_ASSERT(offsetof(IcoProcessNode, tcb_area) == 0x24,
                  "IcoProcessNode.tcb_area offset");

#undef ICO_STATIC_ASSERT

/*
 * Host-side semantic model. Handles are slot_index + 1, preserving zero as
 * the original ABI's null pointer without storing 64-bit host pointers in the
 * 32-bit layout above.
 */
typedef struct IcoGObjSemanticPool {
    IcoGObj *slots;
    u32 capacity;
    IcoGObjPointerTable8 primary_heads;
    IcoGObjPointerTable8 primary_tails;
    IcoGObjPointerTable8 dl_heads;
    IcoGObjPointerTable8 dl_tails;
    IcoGObjTypeHeadTable kind_heads;
    u32 kind_table_disabled;   /* mirrors gp-0x6730: high when table is off */
} IcoGObjSemanticPool;

/* Process-node storage for the host semantic model. Handles are slot+1,
 * matching IcoGObjSemanticPool; a ProcessNode's parent (+0x04), next (+0x08)
 * and prev (+0x0C) are handles into the corresponding pool. */
typedef struct IcoProcessNodeSemanticPool {
    IcoProcessNode *slots;
    u32 capacity;
} IcoProcessNodeSemanticPool;

#ifdef __cplusplus
extern "C" {
#endif

void ico_semantic_isysGObjDlInit(IcoGObjSemanticPool *pool);
void ico_semantic_isysGObjAlloc(IcoGObjSemanticPool *pool,
                                IcoGObj *storage,
                                u32 count);
IcoGObj *ico_semantic_isysGObjAdd(IcoGObjSemanticPool *pool,
                                  ico_ptr32 user_data,
                                  u8 list_id,
                                  u32 sort_key);
IcoGObj *ico_semantic_isysGObjAddHead(IcoGObjSemanticPool *pool,
                                      ico_ptr32 user_data,
                                      u8 list_id,
                                      u32 sort_key);
void ico_semantic_isysGObjRemove(IcoGObjSemanticPool *pool, IcoGObj *gobj);
IcoGObj *ico_semantic_isysGObjAddAfterGObj(IcoGObjSemanticPool *pool,
                                           ico_ptr32 user_data,
                                           IcoGObj *ref);
IcoGObj *ico_semantic_isysGObjAddBeforeGObj(IcoGObjSemanticPool *pool,
                                            ico_ptr32 user_data,
                                            IcoGObj *ref);
void ico_semantic_isysGObjMove(IcoGObjSemanticPool *pool, IcoGObj *gobj,
                               u8 list_id, u32 sort_key);
void ico_semantic_isysGObjMoveBeforeGObj(IcoGObjSemanticPool *pool,
                                         IcoGObj *gobj, IcoGObj *ref);
void ico_semantic_isysGObjMoveAfterGObj(IcoGObjSemanticPool *pool,
                                        IcoGObj *gobj, IcoGObj *ref);
void ico_semantic_isysGObjKindTableAdd(IcoGObjSemanticPool *pool,
                                       IcoGObj *gobj, u32 type);
/*
 * processPoll (0x0013D8A0): the non-NULL path returns ProcessNode+0x18.
 * The original NULL path resolves the current thread through 0x6A6F30; the
 * host semantic model has no kernel thread table and returns zero for NULL.
 */
u32 ico_semantic_processPoll(const IcoProcessNode *process);
int ico_semantic_processNeedsRemoval(const IcoProcessNode *process);

void ico_semantic_isysGObjKindTableRemove(IcoGObjSemanticPool *pool,
                                          IcoGObj *gobj);
void ico_semantic_isysGObjLinkObjDL(IcoGObjSemanticPool *pool,
                                    IcoGObj *gobj, ico_ptr32 callback,
                                    u8 type_id, u32 sort_key, u32 type_bits);
void ico_semantic_isysGObjLinkCameraDL(IcoGObjSemanticPool *pool,
                                       IcoGObj *gobj, ico_ptr32 callback,
                                       u8 type_id, u32 sort_key,
                                       u32 type_bits);
u32 ico_semantic_isysGObjActiveLink(u32 value);
void ico_semantic_isysGObjActiveDlLink(void);
void ico_semantic_isysGObjProcPause(IcoProcessNode *process);

/*
 * isysGObjProcRemoveUnlink (0x0013F638, 0x80 bytes):
 * unlinks a ProcessNode from its parent GObj's process list. Confirmed byte
 * semantics: reads node->prev (+0x0C) / node->next (+0x08), relinks the
 * neighbors, then fixes parent->process_head (+0x2C) / parent->process_tail
 * (+0x30) when they point at the removed node. The original NULL path tripped
 * the assert hoist (DebugPrint 0x557B48); the host model returns 0. The
 * function itself does NOT clear node->next/prev/self — the caller
 * (isysGObjProcRemove, 0x0013F6B8) releases the node afterwards.
 */
int ico_semantic_isysGObjProcRemoveUnlink(IcoGObjSemanticPool *pool,
                                          IcoProcessNodeSemanticPool *proc_pool,
                                          IcoProcessNode *process);

/*
 * sister_callback_reg (0x0013F778, 0x30 bytes): thin forwarding shim into
 * isysGObjProcAdd_ (0x0013F3F0). Confirmed argument mapping:
 *   proc_add(a0, a0, a1, a2 & 0xff, a3, 0x1800)
 * t1 is a fixed constant (0x1800); isysGObjProcAdd_ has no host semantic
 * yet, so the call is delegated to the caller-supplied hook.
 */
typedef ico_ptr32 (*IcoSemanticProcAddFn)(ico_ptr32 a0, ico_ptr32 a1,
                                          ico_ptr32 a2, ico_ptr32 a3,
                                          ico_ptr32 t0, ico_ptr32 t1);
ico_ptr32 ico_semantic_sisterCallbackReg(IcoSemanticProcAddFn proc_add,
                                         ico_ptr32 a0, ico_ptr32 a1,
                                         ico_ptr32 a2, ico_ptr32 a3);

/*
 * getEnemyDefLife (0x001C11C0, 0x90 bytes, byte-exact):
 * ground truth src/entity/asm/GetEnemyDefLife.s. Confirmed access chain and
 * behavior:
 *   self = *(root + 0x00)
 *   work = *(self + 0x15c)
 *   sched = *(work + 0x800)
 *   if *(sched + 0x20) != 5  ->  return 0
 *   life = *(work + 0x134); life += 0.5f; *(work + 0x134) = life
 *   prelude : (addr &scratch[0x10], self, 0)         jal 0x104508
 *   stage   : (addr &scratch[0x00], work + 0xa0, 0)  jal 0x105F00
 *   sched_own: (sched + 0xd0, sched + 0xd0, &scratch) jal 0x243AD0
 *   return 1
 * The three targets have no host semantic yet; each is delegated to a hook
 * (NULL skips the call). The pointer cells keep the PS2 byte offsets but
 * store host-width pointers (a documented host adaptation; 32-bit truncation
 * cannot round-trip on 64-bit hosts). The scratch frame is allocated on the
 * host side and passed by address so hooks observe the same scaffolding.
 */
typedef ico_ptr32 (*IcoSemanticTriFn)(ico_ptr32 a0, ico_ptr32 a1,
                                       ico_ptr32 a2);
int ico_semantic_getEnemyDefLife(const void *root, IcoSemanticTriFn prelude,
                                  IcoSemanticTriFn stage,
                                  IcoSemanticTriFn sched_own);

/*
 * HoldRope (0x001E59A0, 0x154 bytes, byte-exact):
 * ground truth src/entity/asm/HoldRope.s. Rope/chain spring physics.
 * Confirmed access chain and behavior:
 *   spring_addr = gp-0x53A4 (float, processed via hook)
 *   entity_addr = gp-0x53B0 → *(entity+0x15c) = work GObj
 *   player_flags_1 = *(s1+0x5250), player_flags_2 = *(s2+0x5250)
 *   Flag branches:
 *     bit 3 (0x08): spring = 1.0 - (input_111 / 255.0)
 *     bit 1 (0x02): work->+0xF4 = 1.0 - (input_113 * 0.0078125)
 *     bit 0x8000:   work->+0xF0 = (input_109 / 255.0) * 8192
 *     bit 0x2000:   work->+0xF0 = (input_108 / 255.0) * (-8192)
 *   Hook: process_spring (jal 0x1E4980) — NULL skips.
 */
void ico_semantic_holdRope(void *spring_addr, void *entity_addr,
                            u32 player_flags_1, u32 player_flags_2,
                            u8 input_111, u8 input_113,
                            u8 input_109, u8 input_108,
                            IcoSemanticTriFn process_spring);

/*
 * subEnemyCollision (0x0015E2C8, 0xB8 bytes, byte-exact):
 * ground truth src/entity/asm/subEnemyCollision.s. Collision polling loop.
 * Confirmed access chain:
 *   entity → work = *(entity+0x15c)
 *   entity_list = *(work+0x4A0), stride 0x190
 *   entity_mask bits (bit 0 = active gate)
 *   Five hook calls delegated: setup_a, setup_b, collision_check,
 *   collision_response, counter_inc. Returns 1 if gate active, 0 if skipped.
 */
int ico_semantic_subEnemyCollision(const void *entity, u32 entity_mask,
                                    ico_ptr32 entity_list,
                                    IcoSemanticTriFn setup_a,
                                    IcoSemanticTriFn setup_b,
                                    IcoSemanticTriFn collision_check,
                                    IcoSemanticTriFn collision_response,
                                    IcoSemanticTriFn counter_inc);

/*
 * Delegables of subEnemyCollision (Rev.166). Byte-exact ground truth:
 *   src/core/asm/fn_14A100.s  (0x14A100, 0x74 B)
 *   src/core/asm/fn_15BCC8.s  (0x15BCC8, 0x7C B)
 *   src/core/asm/fn_203AA0.s  (0x203AA0, 0xA0 B)
 *
 * fn_14A100 = setup:
 *   idx  = lookup(entity, key)              (jal 0x109F10: a0=entity, a1=key)
 *   work = *(entity+0x15C)
 *   base = *(work+0xC) + (idx << 6)
 *   copies f32(+0x30/+0x34/+0x38) of base into dst[0..3)
 *
 * fn_15BCC8 = collision select (returns the decided message byte):
 *   requires *(entity+0xC) == 1
 *   a2 = *(entity+0x164); if bit29 of u64@a2+0x470 AND bit29 of u64@a2+0x480
 *   → 0xA9; else bit27 of both → 0xAA; else incoming. Original tail-jumps to
 *   0x13FF88 with the selected byte; this model returns the selection.
 *
 * fn_203AA0 = frame-delay so far (vblank counter 0x274EC0/0x274EC4):
 *   count=ld32(0x274EC0), divisor=ld32(0x274EC4)
 *   v = ((60 - count) / divisor) / 60  (integer div per ps2 MIPS)
 *   a0==0 ? (v? v : INFINITE) : (v ? v : 1)
 *   Trap (break 0,7) on divisor==0. Infinite wait returned as 0.
 */
void ico_semantic_fun14A100(void *dst, const void *entity, u32 key,
                            IcoSemanticTriFn lookup);
u32 ico_semantic_fun15BCC8(const void *entity, u32 incoming);
u32 ico_semantic_fun203AA0(u32 frame_count, const void *counters);

/*
 * Rev.167 — first batch of inventoried named functions (Rev.163) with new
 * byte-exact .s ground truth:
 *   src/entity/asm/actEnemyFlagOnDead.s  (0x15D5F0, 0x2C B)
 *   src/entity/asm/AP1JumpReq.s          (0x1AE3B0, 0x34 B)
 *   src/entity/asm/actSt04bEne1Chk.s     (0x203A10, 0x48 B)
 *
 * actEnemyFlagOnDead:
 *   jal 0x1A6E28 (a0 = 0x5588C0)       send enemy-dead flag
 *   jal 0x203AA0 (a0 = 0)              frame delay (built-in fn_203AA0)
 *   flag_send hook models 0x1A6E28 (NULL skips).
 *
 * AP1JumpReq:
 *   clears bit0 of count (0xB5) consecutive u64 at &0x4B3D10 stride 0x40:
 *   for i in 0..count-1: *(u64*)(base + i*0x40) &= -2   (a1=-2, 64-bit and)
 *
 * actSt04bEne1Chk:
 *   m = *(entity+0x164); if *(m+0x12C) != 0 → return 0 (slot busy)
 *   *(m+0x130) = a2 ; *(m+0x12C) = entity
 *   sink hook models the 0x13FF88 dispatch: sink(entity, a2, a1); return 1
 */
void ico_semantic_actEnemyFlagOnDead(IcoSemanticTriFn flag_send,
                                     const void *counters);
void ico_semantic_AP1JumpReq(void *round_base, u32 count);
int ico_semantic_actSt04bEne1Chk(const void *entity, u32 arg_b, u32 arg_c,
                                 IcoSemanticTriFn sink);

/*
 * GirlForceFieldGeo (0x001C3C90, 0x178 bytes, byte-exact):
 * ground truth src/entity/asm/GirlForceFieldGeo.s. Cloth force-field geometry.
 * Sub-range of SetGirlClothDispSwitch (0x001C3C38).
 *
 * CONFIRMED tail path (byte-identical): $f12 rounded via cvt.w.s, frac
 * subtracted from $f1=1.0 → result = 1.0 - frac; then jal 0x243AA8
 * (a0=s2, a1=model+s5, a2=model+s6). The threshold-gated blocks before the
 * tail are delegated to block_a/b/c hooks (each returns nonzero when its
 * force settled), and the 0x243AA8 call is delegated to final_output.
 * Returns the confirmed tail result.
 */
float ico_semantic_girlForceFieldGeo(float f12_input, ico_ptr32 out_a0,
                                     ico_ptr32 out_a1, ico_ptr32 out_a2,
                                     IcoSemanticTriFn block_a,
                                     IcoSemanticTriFn block_b,
                                     IcoSemanticTriFn block_c,
                                     IcoSemanticTriFn final_output);

#ifdef __cplusplus
}
#endif

#endif /* ICO_GOBJ_ABI_H */
