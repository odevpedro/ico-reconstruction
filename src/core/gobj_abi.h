#ifndef ICO_GOBJ_ABI_H
#define ICO_GOBJ_ABI_H

#include <stddef.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
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
void ico_semantic_isysGObjKindTableRemove(IcoGObjSemanticPool *pool,
                                          IcoGObj *gobj);
void ico_semantic_isysGObjLinkObjDL(IcoGObjSemanticPool *pool,
                                    IcoGObj *gobj, ico_ptr32 callback,
                                    u8 type_id, u32 sort_key, u32 type_bits);
void ico_semantic_isysGObjLinkCameraDL(IcoGObjSemanticPool *pool,
                                       IcoGObj *gobj, ico_ptr32 callback,
                                       u8 type_id, u32 sort_key,
                                       u32 type_bits);
void ico_semantic_isysGObjActiveLink(void);

#ifdef __cplusplus
}
#endif

#endif /* ICO_GOBJ_ABI_H */
