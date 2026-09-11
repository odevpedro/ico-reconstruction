#include "gobj_abi.h"

#include <stdint.h>
#include <string.h>

static ico_ptr32 gobj_handle(const IcoGObjSemanticPool *pool,
                             const IcoGObj *gobj)
{
    return (ico_ptr32)((u32)(gobj - pool->slots) + 1u);
}

static IcoGObj *gobj_from_handle(IcoGObjSemanticPool *pool, ico_ptr32 handle)
{
    if (handle == 0 || handle > pool->capacity) {
        return NULL;
    }
    return &pool->slots[handle - 1u];
}

/*
 * Semantic reconstruction of isysGObjDlInit (0x0013F2C8).
 * Probable original source: gobj.c.
 * Ground truth: src/core/asm/isysGObjDlInit.s.
 * Confirmed: clears eight heads at 0x281AB0 and eight tails at 0x281AD0.
 * Inferred: the names "DL head" and "DL tail" follow the link helpers that
 * use GObj+0x34/+0x38. This C is semantic, not currently byte-exact.
 */
void ico_semantic_isysGObjDlInit(IcoGObjSemanticPool *pool)
{
    memset(&pool->dl_heads, 0, sizeof(pool->dl_heads));
    memset(&pool->dl_tails, 0, sizeof(pool->dl_tails));
}

/*
 * Semantic reconstruction of isysGObjAlloc (0x0013E4D0).
 * Confirmed original source class string: gobj.c.
 * Ground truth: src/core/asm/isysGObjAlloc.s.
 * Confirmed: contiguous 0x174-byte slots; self=0, +0x15C=0 and the words at
 * +0x04/+0x08 are -1. Clearing other bytes is a deterministic host-model
 * policy, not behavior claimed for the original allocator.
 */
void ico_semantic_isysGObjAlloc(IcoGObjSemanticPool *pool,
                                IcoGObj *storage,
                                u32 count)
{
    u32 i;

    pool->slots = storage;
    pool->capacity = count;
    memset(&pool->primary_heads, 0, sizeof(pool->primary_heads));
    memset(&pool->primary_tails, 0, sizeof(pool->primary_tails));
    ico_semantic_isysGObjDlInit(pool);

    for (i = 0; i < count; ++i) {
        memset(&storage[i], 0, sizeof(storage[i]));
        storage[i].unknown_004 = -1;
        storage[i].unknown_008 = -1;
        storage[i].state_15c = 0;
    }
}

/*
 * Semantic reconstruction of the ordered insertion at 0x0013DFF0.
 * Probable original source: gobj.c.
 * Ground truth: the 0x0013DFF0 body retained in
 * src/core/asm/isysGObjRemoveAll.s.
 * Confirmed: per-list heads 0x281A70, tails 0x281A90, next at +0x10,
 * previous at +0x14, list id at +0x18 and ascending sort key at +0x1C.
 */
static void insert_primary_sorted(IcoGObjSemanticPool *pool,
                                  IcoGObj *gobj,
                                  u8 list_id,
                                  u32 sort_key)
{
    ico_ptr32 handle = gobj_handle(pool, gobj);
    ico_ptr32 current_handle = pool->primary_heads.entries[list_id];
    IcoGObj *current;

    gobj->list_id = list_id;
    gobj->sort_key = sort_key;
    gobj->next = 0;
    gobj->prev = 0;

    if (current_handle == 0) {
        pool->primary_heads.entries[list_id] = handle;
        pool->primary_tails.entries[list_id] = handle;
        return;
    }

    current = gobj_from_handle(pool, current_handle);
    if (sort_key < current->sort_key) {
        gobj->next = current_handle;
        current->prev = handle;
        pool->primary_heads.entries[list_id] = handle;
        return;
    }

    for (;;) {
        IcoGObj *next = gobj_from_handle(pool, current->next);
        if (next == NULL || sort_key < next->sort_key) {
            gobj->prev = gobj_handle(pool, current);
            gobj->next = current->next;
            current->next = handle;
            if (next != NULL) {
                next->prev = handle;
            } else {
                pool->primary_tails.entries[list_id] = handle;
            }
            return;
        }
        current = next;
    }
}

/*
 * Semantic reconstruction of the head-insertion variant at 0x0013E0C0.
 * Probable original source: gobj.c.
 * Ground truth: the 0x0013E0C0 body retained in
 * src/core/asm/isysGObjRemoveAll.s and the wrapper in
 * src/core/asm/isysGObjAddHead.s.
 * Confirmed: same head/tail tables and linkage fields as the ordered variant.
 */
static void insert_primary_head(IcoGObjSemanticPool *pool,
                                IcoGObj *gobj,
                                u8 list_id,
                                u32 sort_key)
{
    ico_ptr32 handle = gobj_handle(pool, gobj);
    ico_ptr32 old_head = pool->primary_heads.entries[list_id];
    IcoGObj *next = gobj_from_handle(pool, old_head);

    gobj->list_id = list_id;
    gobj->sort_key = sort_key;
    gobj->prev = 0;
    gobj->next = old_head;
    pool->primary_heads.entries[list_id] = handle;

    if (next != NULL) {
        next->prev = handle;
    } else {
        pool->primary_tails.entries[list_id] = handle;
    }
}

static IcoGObj *first_free_slot(IcoGObjSemanticPool *pool)
{
    u32 i;
    for (i = 0; i < pool->capacity; ++i) {
        if (pool->slots[i].self == 0) {
            return &pool->slots[i];
        }
    }
    return NULL;
}

static void initialize_added_slot(IcoGObjSemanticPool *pool,
                                  IcoGObj *gobj,
                                  ico_ptr32 user_data)
{
    gobj->unknown_164 = 0;
    gobj->state_170 = 0;
    gobj->user_data = user_data;
    gobj->self = gobj_handle(pool, gobj);
    gobj->state_15c = 0;
    gobj->unknown_004 = -1;
    gobj->unknown_008 = -1;
    gobj->process_head = 0;
    gobj->process_tail = 0;
    gobj->unknown_058 = 0;
    gobj->type = 0;
}

/*
 * Semantic reconstruction of isysGObjAdd (0x0013E8D8).
 * Confirmed original source class string: gobj.c.
 * Ground truth: src/core/asm/isysGObjAdd.s.
 * Confirmed: first-free scan, slot initialization and ordered primary-list
 * insertion. Inferred: host handles replace original EE pointers.
 */
IcoGObj *ico_semantic_isysGObjAdd(IcoGObjSemanticPool *pool,
                                  ico_ptr32 user_data,
                                  u8 list_id,
                                  u32 sort_key)
{
    IcoGObj *gobj;
    if (pool == NULL || pool->slots == NULL || list_id >= ICO_GOBJ_PRIMARY_LIST_COUNT) {
        return NULL;
    }

    gobj = first_free_slot(pool);
    if (gobj == NULL) {
        return NULL;
    }

    initialize_added_slot(pool, gobj, user_data);
    insert_primary_sorted(pool, gobj, list_id, sort_key);
    return gobj;
}

/* Same evidence as isysGObjAdd, using its confirmed head-insertion variant. */
IcoGObj *ico_semantic_isysGObjAddHead(IcoGObjSemanticPool *pool,
                                      ico_ptr32 user_data,
                                      u8 list_id,
                                      u32 sort_key)
{
    IcoGObj *gobj;
    if (pool == NULL || pool->slots == NULL || list_id >= ICO_GOBJ_PRIMARY_LIST_COUNT) {
        return NULL;
    }

    gobj = first_free_slot(pool);
    if (gobj == NULL) {
        return NULL;
    }

    initialize_added_slot(pool, gobj, user_data);
    insert_primary_head(pool, gobj, list_id, sort_key);
    return gobj;
}

/*
 * Semantic reconstruction of the primary-list unlink body at 0x0013DDF8.
 * Probable original source: gobj.c.
 * Ground truth: 0x0013DDF8 is the first call issued by isysGObjRemove,
 * isysGObjMove*, isysGObjRemoveAll and isysGObjAddAfterGObj.
 * Confirmed: removes the gobj from its primary list (head/tail repair) and
 * DOES NOT clear self; the caller clears self afterwards when recycling.
 * Inferred: reuses the same link fields (+0x10 next / +0x14 prev / +0x18 list_id).
 */
static void unlink_primary(IcoGObjSemanticPool *pool, IcoGObj *gobj)
{
    IcoGObj *next;
    IcoGObj *prev;
    u8 list_id;

    list_id = gobj->list_id;
    if (list_id >= ICO_GOBJ_PRIMARY_LIST_COUNT) {
        return;
    }

    next = gobj_from_handle(pool, gobj->next);
    prev = gobj_from_handle(pool, gobj->prev);

    if (prev != NULL) {
        prev->next = gobj->next;
    } else {
        pool->primary_heads.entries[list_id] = gobj->next;
    }

    if (next != NULL) {
        next->prev = gobj->prev;
    } else {
        pool->primary_tails.entries[list_id] = gobj->prev;
    }

    gobj->next = 0;
    gobj->prev = 0;
}

/*
 * Semantic reconstruction of the kind-table (type) unlink used by
 * isysGObjKindTableRemove (0x0013E728) and isysGObjRemove.
 * Probable original source: gobj.c.
 * Ground truth: isysGObjRemove.s walks the 0x6A93D0[type] chain via +0x3C and
 * unlinks; isysGObjKindTableAdd.s calls 0x0013E728 when moving types.
 * Confirmed: type stored at +0x0C is the table index; +0x3C is the type chain.
 */
static ico_ptr32 kind_head_for(const IcoGObjSemanticPool *pool, u32 type)
{
    if (type == 0 || type >= ICO_GOBJ_TYPE_TABLE_ENTRIES) {
        return 0;
    }
    return pool->kind_heads.entries[type];
}

static void kind_table_remove(IcoGObjSemanticPool *pool, IcoGObj *gobj)
{
    ico_ptr32 head;
    IcoGObj *cur;
    IcoGObj *prev;

    if (gobj == NULL || gobj->type == 0 ||
        gobj->type >= ICO_GOBJ_TYPE_TABLE_ENTRIES) {
        return;
    }

    head = pool->kind_heads.entries[gobj->type];
    cur = gobj_from_handle(pool, head);
    prev = NULL;

    while (cur != NULL) {
        if (cur == gobj) {
            break;
        }
        prev = cur;
        cur = gobj_from_handle(pool, cur->type_next);
    }

    if (cur == NULL) {
        return;
    }

    if (prev != NULL) {
        prev->type_next = gobj->type_next;
    } else {
        pool->kind_heads.entries[gobj->type] = gobj->type_next;
    }
    gobj->type_next = 0;
}

/*
 * Semantic reconstruction of isysGObjRemove (0x0013E548).
 * Probable original source: gobj.c.
 * Ground truth: src/core/asm/isysGObjRemove.s.
 * Confirmed sequence: (1) remove from the 0x6A93D0 kind-table when the type is
 * in [1, 0x44); (2) unlink from the primary list via 0x0013DDF8; (3) clear
 * self (+0x00) marking the slot free; (4) walk the attached-process list from
 * +0x2C calling 0x0013F6B8 (process removal) for each node. Inferred: the
 * semantic model emulates the process teardown loop but the internals of
 * 0x0013F6B8 are out of scope here.
 */
void ico_semantic_isysGObjRemove(IcoGObjSemanticPool *pool, IcoGObj *gobj)
{
    ico_ptr32 process;
    u32 type;

    if (pool == NULL || gobj == NULL || gobj->self == 0) {
        return;
    }

    type = gobj->type;
    if (type >= 1 && type < ICO_GOBJ_TYPE_TABLE_ENTRIES) {
        kind_table_remove(pool, gobj);
    }

    unlink_primary(pool, gobj);
    gobj->self = 0;
    gobj->unknown_004 = -1;
    gobj->unknown_008 = -1;

    process = gobj->process_head;
    while (process != 0) {
        /* 0x0013F6B8 removes one attached process node. Not modeled here. */
        process = gobj->process_head;
        gobj->process_head = 0;
        gobj->process_tail = 0;
    }
}

/*
 * Semantic reconstruction of isysGObjAddAfterGObj (0x0013E258 region).
 * Probable original source: gobj.c.
 * Ground truth: src/core/asm/isysGObjAddAfterGObj.s.
 * Confirmed: allocates the first free slot, initializes user_data (+0x28),
 * self (+0x00), +0x04/+0x08 = -1, +0x15C/+0x2C/+0x30/+0x58 = 0, then inserts
 * immediately AFTER the reference GObj in the reference's primary list
 * (list_id/sort_key copied from the reference; tail updated when at the end).
 * Inferred: procedure-model handles replace original EE pointers.
 */
IcoGObj *ico_semantic_isysGObjAddAfterGObj(IcoGObjSemanticPool *pool,
                                           ico_ptr32 user_data,
                                           IcoGObj *ref)
{
    ico_ptr32 handle;
    IcoGObj *gobj;
    IcoGObj *next;
    u8 list_id;

    if (pool == NULL || pool->slots == NULL || ref == NULL || ref->self == 0) {
        return NULL;
    }

    gobj = first_free_slot(pool);
    if (gobj == NULL) {
        return NULL;
    }

    handle = gobj_handle(pool, gobj);
    gobj->user_data = user_data;
    gobj->self = handle;
    gobj->unknown_004 = -1;
    gobj->unknown_008 = -1;
    gobj->state_15c = 0;
    gobj->process_head = 0;
    gobj->process_tail = 0;
    gobj->unknown_058 = 0;
    gobj->unknown_164 = 0;
    gobj->state_170 = 0;

    list_id = ref->list_id;
    gobj->list_id = list_id;
    gobj->prev = ref->self;
    next = gobj_from_handle(pool, ref->next);
    gobj->next = ref->next;
    gobj->sort_key = ref->sort_key;
    ref->next = handle;
    if (next != NULL) {
        next->prev = handle;
    } else {
        pool->primary_tails.entries[list_id] = handle;
    }

    return gobj;
}

/*
 * Semantic reconstruction of isysGObjAddBeforeGObj (0x0013E258 region).
 * Probable original source: gobj.c.
 * Ground truth: src/core/asm/isysGObjAddBeforeGObj.s (mirror of the After
 * variant; inserts immediately BEFORE the reference and repairs the head).
 * Inferred: procedure-model handles.
 */
IcoGObj *ico_semantic_isysGObjAddBeforeGObj(IcoGObjSemanticPool *pool,
                                            ico_ptr32 user_data,
                                            IcoGObj *ref)
{
    ico_ptr32 handle;
    IcoGObj *gobj;
    IcoGObj *prev;
    u8 list_id;

    if (pool == NULL || pool->slots == NULL || ref == NULL || ref->self == 0) {
        return NULL;
    }

    gobj = first_free_slot(pool);
    if (gobj == NULL) {
        return NULL;
    }

    handle = gobj_handle(pool, gobj);
    gobj->user_data = user_data;
    gobj->self = handle;
    gobj->unknown_004 = -1;
    gobj->unknown_008 = -1;
    gobj->state_15c = 0;
    gobj->process_head = 0;
    gobj->process_tail = 0;
    gobj->unknown_058 = 0;
    gobj->unknown_164 = 0;
    gobj->state_170 = 0;

    list_id = ref->list_id;
    gobj->list_id = list_id;
    prev = gobj_from_handle(pool, ref->prev);
    gobj->prev = ref->prev;
    gobj->next = ref->self;
    gobj->sort_key = ref->sort_key;
    ref->prev = handle;
    if (prev != NULL) {
        prev->next = handle;
    } else {
        pool->primary_heads.entries[list_id] = handle;
    }

    return gobj;
}

/*
 * Semantic reconstruction of isysGObjMove (0x0013DDF8 tail) and the Before/
 * After variants (0x0013E8A8/0x0013E820). Probable original source: gobj.c.
 * Ground truth: src/core/asm/isysGObjMove.s (thin wrapper: unlink via
 * 0x0013DDF8 then ordered insert 0x0013DFF0; a duplicated tail-merged variant
 * targets the head-insert 0x0013E0C0), isysGObjMoveBeforeGObj.s,
 * isysGObjMoveAfterGObj.s.
 * Confirmed: all three first unlink from the current primary list, then relink
 * relative to a reference gobj (or to a new list/sort-key for isysGObjMove).
 */
void ico_semantic_isysGObjMove(IcoGObjSemanticPool *pool, IcoGObj *gobj,
                               u8 list_id, u32 sort_key)
{
    if (pool == NULL || gobj == NULL || gobj->self == 0) {
        return;
    }
    unlink_primary(pool, gobj);
    insert_primary_sorted(pool, gobj, list_id, sort_key);
}

void ico_semantic_isysGObjMoveBeforeGObj(IcoGObjSemanticPool *pool,
                                         IcoGObj *gobj, IcoGObj *ref)
{
    ico_ptr32 handle;
    IcoGObj *prev;
    u8 list_id;

    if (pool == NULL || gobj == NULL || ref == NULL || gobj->self == 0) {
        return;
    }

    handle = gobj_handle(pool, gobj);
    unlink_primary(pool, gobj);

    list_id = ref->list_id;
    gobj->list_id = list_id;
    prev = gobj_from_handle(pool, ref->prev);
    gobj->next = ref->self;
    gobj->prev = ref->prev;
    gobj->sort_key = ref->sort_key;
    ref->prev = handle;
    if (prev != NULL) {
        prev->next = handle;
    } else {
        pool->primary_heads.entries[list_id] = handle;
    }
}

void ico_semantic_isysGObjMoveAfterGObj(IcoGObjSemanticPool *pool,
                                        IcoGObj *gobj, IcoGObj *ref)
{
    ico_ptr32 handle;
    IcoGObj *next;
    u8 list_id;

    if (pool == NULL || gobj == NULL || ref == NULL || gobj->self == 0) {
        return;
    }

    handle = gobj_handle(pool, gobj);
    unlink_primary(pool, gobj);

    list_id = ref->list_id;
    gobj->list_id = list_id;
    next = gobj_from_handle(pool, ref->next);
    gobj->prev = ref->self;
    gobj->next = ref->next;
    gobj->sort_key = ref->sort_key;
    ref->next = handle;
    if (next != NULL) {
        next->prev = handle;
    } else {
        pool->primary_tails.entries[list_id] = handle;
    }
}

/*
 * Semantic reconstruction of isysGObjKindTableAdd (0x0013E648).
 * Probable original source: gobj.c.
 * Ground truth: src/core/asm/isysGObjKindTableAdd.s.
 * Confirmed: when the gp-0x6730 "table disabled" flag is set, only type
 * (+0x0C) is written and no list work happens. Otherwise it scans the current
 * type chain (via 0x0013EB50 head / 0x0013EBE0 next) for the gobj, removes it
 * if present (0x0013E728), stores the new type, and for type < 0x44 appends
 * the gobj to the 0x6A93D0[type] tail.
 */
void ico_semantic_isysGObjKindTableAdd(IcoGObjSemanticPool *pool,
                                       IcoGObj *gobj, u32 type)
{
    ico_ptr32 head;
    ico_ptr32 handle;
    IcoGObj *cur;
    IcoGObj *tail;
    u32 old_type;

    if (gobj == NULL) {
        return;
    }

    if (pool->kind_table_disabled) {
        gobj->type = type;
        return;
    }

    old_type = gobj->type;
    head = kind_head_for(pool, old_type);
    cur = gobj_from_handle(pool, head);
    while (cur != NULL) {
        if (cur == gobj) {
            kind_table_remove(pool, gobj);
            break;
        }
        cur = gobj_from_handle(pool, cur->type_next);
    }

    gobj->type = type;

    if (type >= ICO_GOBJ_TYPE_TABLE_ENTRIES) {
        if (type != 0) {
            gobj->type_next = 0;
        }
        return;
    }

    handle = gobj_handle(pool, gobj);
    tail = gobj_from_handle(pool, pool->kind_heads.entries[type]);
    if (tail == NULL) {
        pool->kind_heads.entries[type] = handle;
        gobj->type_next = 0;
        return;
    }
    while (gobj_from_handle(pool, tail->type_next) != NULL) {
        tail = gobj_from_handle(pool, tail->type_next);
    }
    tail->type_next = handle;
    gobj->type_next = 0;
}

/*
 * Semantic reconstruction of isysGObjKindTableRemove (0x0013E728).
 * Probable original source: gobj.c.
 * Ground truth: src/core/asm/isysGObjKindTableRemove.s (used by
 * isysGObjKindTableAdd and isysGObjRemove; emits an assert stub on a bad walk).
 * Confirmed: unlinks the gobj from the 0x6A93D0 chain for its current type.
 */
void ico_semantic_isysGObjKindTableRemove(IcoGObjSemanticPool *pool,
                                          IcoGObj *gobj)
{
    if (pool == NULL) {
        return;
    }
    kind_table_remove(pool, gobj);
}

/*
 * Semantic reconstruction of isysGObjActiveLink (0x0013EC40, 8 bytes,
 * byte-exact). Ground truth: src/core/asm/isysGObjActiveLink.s.
 * Confirmed: `jr $31; move $v0, $a1` -- pure accessor that returns its
 * second argument unchanged.
 */
u32 ico_semantic_isysGObjActiveLink(u32 value)
{
    return value;
}

/*
 * Semantic reconstruction of isysGObjActiveDlLink (0x00141160, 8 bytes,
 * byte-exact). Ground truth: src/core/asm/isysGObjActiveDlLink.s.
 * Confirmed: `jr $31; nop` -- a literal no-op; returns whatever the caller
 * left in $v0 (typically the matching DL link, hence the Ghidra "returns a0"
 * remark when a0 is also v0).
 */
void ico_semantic_isysGObjActiveDlLink(void)
{
}

/*
 * Semantic reconstruction of isysGObjProcPause (0x0013F808, 8 bytes,
 * byte-exact). Ground truth: src/core/asm/isysGObjProcPause.s.
 * Confirmed: `jr $31; sw $zero, 0x18($a0)` -- clears the ProcessNode active
 * flag (+0x18). A paused process is skipped by iosOm dispatch. This is the
 * per-process counterpart of isysGObjProcPauseAll / isysGObjProcPausePtr.
 */
void ico_semantic_isysGObjProcPause(IcoProcessNode *process)
{
    if (process != NULL) {
        process->active = 0;
    }
}

/*
 * Ordered DL-list insertion helper, reconstructing the DL body that the
 * Link* functions tail into (0x0013EE60 for isysGObjLinkObjDL and the
 * camera-DL counterpart reached by isysGObjLinkCameraDL's 0x001FC048 call
 * pair). Probable original source: gobj.c.
 * Ground truth: the DL insertion body retained in
 * src/core/asm/isysGObjAddHead.s (fields +0x34/+0x38 dl next/prev,
 * +0x40 dl_list_id, +0x44 dl_sort_key; heads 0x281AB0 / tails 0x281AD0).
 * Confirmed fields; the helper is a semantic model, not byte-exact.
 */
static void insert_dl_sorted(IcoGObjSemanticPool *pool,
                             IcoGObj *gobj,
                             u8 dl_list_id,
                             u32 dl_sort_key)
{
    ico_ptr32 handle;
    ico_ptr32 head;
    IcoGObj *cur;

    handle = gobj_handle(pool, gobj);
    if (dl_list_id >= ICO_GOBJ_DL_LIST_COUNT) {
        return;
    }

    head = pool->dl_heads.entries[dl_list_id];
    cur = gobj_from_handle(pool, head);

    gobj->dl_list_id = dl_list_id;
    gobj->dl_sort_key = dl_sort_key;
    gobj->dl_prev = 0;

    if (cur == NULL) {
        gobj->dl_next = 0;
        pool->dl_heads.entries[dl_list_id] = handle;
        pool->dl_tails.entries[dl_list_id] = handle;
        return;
    }

    if (dl_sort_key < cur->dl_sort_key) {
        gobj->dl_next = head;
        cur->dl_prev = handle;
        pool->dl_heads.entries[dl_list_id] = handle;
        return;
    }

    for (;;) {
        IcoGObj *next = gobj_from_handle(pool, cur->dl_next);
        if (next == NULL || dl_sort_key < next->dl_sort_key) {
            gobj->dl_prev = gobj_handle(pool, cur);
            gobj->dl_next = cur->dl_next;
            cur->dl_next = handle;
            if (next != NULL) {
                next->dl_prev = handle;
            } else {
                pool->dl_tails.entries[dl_list_id] = handle;
            }
            return;
        }
        cur = next;
    }
}

/*
 * Semantic reconstruction of isysGObjLinkObjDL (0x0013F130).
 * Probable original source: gobj.c.
 * Ground truth: src/core/asm/isysGObjLinkObjDL.s.
 * Confirmed: stores callback at +0x48 and type_bits at +0x50, then calls the
 * DL ordered-insert helper (0x0013EE60) with the type byte and sort key.
 * Inferred: the helper is modeled by insert_dl_sorted above.
 */
void ico_semantic_isysGObjLinkObjDL(IcoGObjSemanticPool *pool,
                                    IcoGObj *gobj, ico_ptr32 callback,
                                    u8 type_id, u32 sort_key, u32 type_bits)
{
    if (gobj == NULL) {
        return;
    }
    gobj->callback = callback;
    gobj->type_mask = type_bits;
    if (pool != NULL) {
        insert_dl_sorted(pool, gobj, type_id, sort_key);
    }
}

/*
 * Semantic reconstruction of isysGObjLinkCameraDL (0x0013EF88 region).
 * Probable original source: gobj.c.
 * Ground truth: src/core/asm/isysGObjLinkCameraDL.s.
 * Confirmed: stores callback at +0x48, slot_mask at +0x4C and type_bits at
 * +0x50, then calls the camera-DL insert helper (0x001FC048). The camera list
 * is separate from the object DL list; the helper is not modeled here beyond
 * the confirmed field stores.
 */
void ico_semantic_isysGObjLinkCameraDL(IcoGObjSemanticPool *pool,
                                       IcoGObj *gobj, ico_ptr32 callback,
                                       u8 type_id, u32 sort_key,
                                       u32 type_bits)
{
    if (gobj == NULL) {
        return;
    }
    gobj->callback = callback;
    gobj->slot_mask = type_id;
    gobj->type_mask = type_bits;
    (void)pool;
    (void)sort_key;
}

/* Semantic reconstruction of processPoll's direct (non-NULL) path.
 * Ground truth: src/core/asm/processPoll.s at 0x0013D8A0. */
u32 ico_semantic_processPoll(const IcoProcessNode *process)
{
    return process == NULL ? 0 : process->active;
}

/* 0x22 selects isysGObjProcRemove in _iosOmMain's thread path. */
int ico_semantic_processNeedsRemoval(const IcoProcessNode *process)
{
    return ico_semantic_processPoll(process) == 0x22;
}

static IcoProcessNode *process_from_handle(
    IcoProcessNodeSemanticPool *pool, ico_ptr32 handle)
{
    if (pool == NULL || pool->slots == NULL ||
        handle == 0 || handle > pool->capacity) {
        return NULL;
    }
    return &pool->slots[handle - 1u];
}

/*
 * Semantic reconstruction of isysGObjProcRemoveUnlink (0x0013F638, 0x80 B).
 * Probable original source: gobj.c (process teardown helper called by
 * isysGObjProcRemove at 0x0013F6B8).
 * Ground truth: src/core/asm/isysGObjProcRemoveUnlink.s.
 * Confirmed byte semantics:
 *   process == NULL -> assert path (DebugPrint 0x557B48 via 0x1A6E28);
 *   prev = process->prev (+0x0C), next = process->next (+0x08);
 *   if (prev) prev->next = next;  if (next) next->prev = prev;
 *   parent = process->parent (+0x04);
 *   if (parent->process_head == process) parent->process_head = next;
 *   if (parent->process_tail == process) parent->process_tail = prev.
 * The unlink deliberately does NOT clear process->next/process->prev/self;
 * the caller releases the node afterwards. Inferred: host handles replace
 * the original EE pointers; the NULL path is a documented early-out instead
 * of the assert trip.
 */
int ico_semantic_isysGObjProcRemoveUnlink(IcoGObjSemanticPool *pool,
                                          IcoProcessNodeSemanticPool *proc_pool,
                                          IcoProcessNode *process)
{
    IcoProcessNode *prev;
    IcoProcessNode *next;
    IcoGObj *parent;

    if (process == NULL) {
        return 0;
    }

    prev = process_from_handle(proc_pool, process->prev);
    next = process_from_handle(proc_pool, process->next);
    parent = (pool != NULL) ? gobj_from_handle(pool, process->parent) : NULL;

    if (prev != NULL) {
        prev->next = process->next;
    }
    if (next != NULL) {
        next->prev = process->prev;
    }

    if (parent != NULL && parent->self != 0) {
        if (parent->process_head == process->self) {
            parent->process_head = process->next;
        }
        if (parent->process_tail == process->self) {
            parent->process_tail = process->prev;
        }
    }

    return 1;
}

/*
 * Semantic reconstruction of sister_callback_reg (0x0013F778, 0x30 B).
 * Probable original source: gobj.c.
 * Ground truth: src/core/asm/sister_callback_reg.s.
 * Confirmed byte semantics: forwards to isysGObjProcAdd_ (0x0013F3F0) as
 *   isysGObjProcAdd_(a0, a0, a1, a2 & 0xff, a3, 0x1800)
 * with t1 fixed to the constant 0x1800. isysGObjProcAdd_ has no host
 * semantic yet, so the actual registration is delegated to the hook; this
 * function only reproduces the confirmed argument shuffle and fixed t1.
 */
ico_ptr32 ico_semantic_sisterCallbackReg(IcoSemanticProcAddFn proc_add,
                                         ico_ptr32 a0, ico_ptr32 a1,
                                         ico_ptr32 a2, ico_ptr32 a3)
{
    if (proc_add == NULL) {
        return 0;
    }
    return proc_add(a0, a0, a1, a2 & 0xff, a3, 0x1800);
}

/*
 * Semantic reconstruction of getEnemyDefLife (0x001C11C0, 0x90 bytes).
 * Probable original source: enemy(s).c. See gobj_abi.h for the confirmed
 * access chain and hook contract. The type gate is 5; the float at
 * work+0x134 is incremented by 0.5f (lui 0x3f00 + add.s), not doubled.
 */
/* Host pointer widths differ from the original 32-bit EE ABI. The memory
   cells in this model keep the PS2 byte offsets (0x15c / 0x800 / 0x20 /
   0x134) but store host-width pointers, so a truncated 32-bit round-trip
   can never reach a bogus low address. */

static u32 gdl_ld_u32(const void *address)
{
    u32 value;
    memcpy(&value, address, sizeof(value));
    return value;
}

static void gdl_st_u32(void *address, u32 value)
{
    memcpy(address, &value, sizeof(value));
}

static float gdl_ld_float(const void *address)
{
    float value;
    memcpy(&value, address, sizeof(value));
    return value;
}

static void gdl_st_float(void *address, float value)
{
    memcpy(address, &value, sizeof(value));
}

static const void *const *gdl_cell(const void *address)
{
    return (const void *const *)address;
}

static void *gdl_cell_w(void *address)
{
    return address;
}

/* ── HoldRope helpers ────────────────────────────────────────────────── */

static u16 hr_ld_u16(const void *address)
{
    u16 value;
    memcpy(&value, address, sizeof(value));
    return value;
}

static void hr_st_u16(void *address, u16 value)
{
    memcpy(address, &value, sizeof(value));
}

static float hr_ld_float(const void *address)
{
    float value;
    memcpy(&value, address, sizeof(value));
    return value;
}

static void hr_st_float(void *address, float value)
{
    memcpy(address, &value, sizeof(value));
}

/* ── subEnemyCollision helpers ───────────────────────────────────────── */

static u32 sec_ld_u32(const void *address)
{
    u32 value;
    memcpy(&value, address, sizeof(value));
    return value;
}

static void *sec_cell(const void *address)
{
    return *(void *const *)address;
}

static void sec_st_float(void *address, float value)
{
    memcpy(address, &value, sizeof(value));
}

static void sec_st_u32(void *address, u32 value)
{
    memcpy(address, &value, sizeof(value));
}

static u64 sec_ld_u64(const void *address)
{
    u64 value;
    memcpy(&value, address, sizeof(value));
    return value;
}

int ico_semantic_getEnemyDefLife(const void *root, IcoSemanticTriFn prelude,
                                 IcoSemanticTriFn stage,
                                 IcoSemanticTriFn sched_own)
{
    const void *self = *gdl_cell((const u8 *)root + 0x00);
    const void *work = *gdl_cell((const u8 *)self + 0x15c);
    const void *sched = *gdl_cell((const u8 *)work + 0x800);
    u8 scratch[0x50];
    ico_ptr32 scratch_addr;

    if (gdl_ld_u32((const u8 *)sched + 0x20) != 5u) {
        return 0;
    }

    gdl_st_float(gdl_cell_w((u8 *)work + 0x134),
                 gdl_ld_float((const u8 *)work + 0x134) + 0.5f);
    memset(scratch, 0, sizeof(scratch));
    scratch_addr = (ico_ptr32)(uintptr_t)scratch;

    if (prelude != NULL) {
        (void)prelude(scratch_addr + 0x10, (ico_ptr32)(uintptr_t)self, 0);
    }
    if (stage != NULL) {
        const void *work_via_root = *gdl_cell((const u8 *)root + 0x15c);
        (void)stage(scratch_addr,
                    (ico_ptr32)((uintptr_t)work_via_root + 0xa0u), 0);
    }
    if (sched_own != NULL) {
        ico_ptr32 sched_addr = (ico_ptr32)(uintptr_t)sched;
        (void)sched_own(sched_addr + 0xd0, sched_addr + 0xd0, scratch_addr);
    }
    return 1;
}

/*
 * Semantic reconstruction of HoldRope (0x001E59A0, 0x154 bytes).
 * Probable original source: rope/chain physics handler.
 * Ground truth: src/entity/asm/HoldRope.s (byte-exact).
 *
 * Access chain (confirmed from disassembly):
 *   gp-0x53A4  = spring coefficient (float, processed via hook)
 *   gp-0x53B0  = entity cell (holds the entity pointer) →
 *                *(entity+0x15c) = work GObj
 *   *(s1+0x5250) = player_flags_1 (bitmask)
 *   *(s2+0x5250) = player_flags_2 (bitmask)
 *   sp+0x108..0x113 = input bytes (stack scratch from prior computation)
 *
 * Core semantic behavior (flag→value transformation):
 *   1. process_spring(gp-0x53A4)                           jal 0x1E4980
 *   2. if (player_flags_1 & 0x08):
 *        spring = 1.0 - (input_111 / 255.0)               div.s + sub.s
 *        process_spring(spring)                            jal 0x1E4980
 *   3. if (player_flags_2 & 0x02):
 *        work->+0xF4 = 1.0 - (input_113 * 0.0078125)     mul.s + sub.s
 *      else:
 *        work->+0xF4 = 1.0f                               lui 0x3F80
 *   4. if (player_flags_1 & 0x8000):
 *        work->+0xF0 = (input_109 / 255.0) * 8192.0       div.s + mul.s + cvt.w.s
 *      elif (player_flags_1 & 0x2000):
 *        work->+0xF0 = (input_108 / 255.0) * (-8192.0)    div.s + mul.s + cvt.w.s
 *      else:
 *        work->+0xF0 = 0                                  sw $zero
 *
 * Float constants from the disassembly:
 *   0x437F0000 = 255.0f
 *   0x3F800000 = 1.0f
 *   0x3C000000 = 0.0078125f  (1/128)
 *   0x46000000 = 8192.0f
 *   0xC6000000 = -8192.0f
 *
 * The three hook calls (process_spring ×2, read_state ×1) are delegated;
 * NULL hooks are allowed (skips the call). The semantic function does NOT
 * model the bc1f entry or the 0x24E578 call — those are caller-specific
 * integration points.
 */
void ico_semantic_holdRope(void *spring_addr,      /* gp-0x53A4 cell */
                           void *entity_cell,       /* gp-0x53B0 cell → entity */
                           u32 player_flags_1,      /* *(s1+0x5250) */
                           u32 player_flags_2,      /* *(s2+0x5250) */
                           u8 input_111,            /* sp+0x111 byte */
                           u8 input_113,            /* sp+0x113 byte */
                           u8 input_109,            /* sp+0x109 byte */
                           u8 input_108,            /* sp+0x108 byte */
                           IcoSemanticTriFn process_spring)
{
    void *entity;
    void *work;
    float spring;
    float byte_f;
    ico_ptr32 spring_value;

    if (spring_addr == NULL || entity_cell == NULL) {
        return;
    }

    /* Step 1: process existing spring coefficient. The original passes the
     * value in $f12 (lwc1 $f12, -0x53a4($gp)); the hook receives the bit
     * pattern of that float in a0. */
    spring = hr_ld_float(spring_addr);
    memcpy(&spring_value, &spring, sizeof(spring_value));
    if (process_spring != NULL) {
        (void)process_spring(spring_value, 0, 0);
    }

    /* Step 2: bit 3 of player_flags_1 — update spring */
    if (player_flags_1 & 0x08) {
        byte_f = (float)input_111 / 255.0f;
        spring = 1.0f - byte_f;
        hr_st_float(spring_addr, spring);
        memcpy(&spring_value, &spring, sizeof(spring_value));
        if (process_spring != NULL) {
            (void)process_spring(spring_value, 0, 0);
        }
    }

    /* Resolve entity → work GObj (gp-0x53B0 holds the entity pointer) */
    entity = sec_cell(entity_cell);
    if (entity == NULL) {
        return;
    }
    work = sec_cell((const u8 *)entity + 0x15c);
    if (work == NULL) {
        return;
    }

    /* Step 3: bit 1 of player_flags_2 — set work->+0xF4 */
    if (player_flags_2 & 0x02) {
        byte_f = (float)input_113;
        hr_st_float((u8 *)work + 0xF4, 1.0f - (byte_f * 0.0078125f));
    } else {
        hr_st_float((u8 *)work + 0xF4, 1.0f);
    }

    /* Step 4: bits 0x8000 / 0x2000 of player_flags_1 — set work->+0xF0 */
    if (player_flags_1 & 0x8000) {
        byte_f = (float)input_109 / 255.0f;
        hr_st_u16((u8 *)work + 0xF0, (u16)(int)(byte_f * 8192.0f));
    } else if (player_flags_1 & 0x2000) {
        byte_f = (float)input_108 / 255.0f;
        hr_st_u16((u8 *)work + 0xF0, (u16)(int)(byte_f * (-8192.0f)));
    } else {
        hr_st_u16((u8 *)work + 0xF0, 0);
    }
}

/*
 * Semantic reconstruction of subEnemyCollision (0x0015E2C8, 0xB8 bytes).
 * Probable original source: enemy collision polling loop.
 * Ground truth: src/entity/asm/subEnemyCollision.s (byte-exact).
 *
 * Access chain (confirmed from disassembly):
 *   a0 = entity
 *   *(entity+0x15c) = work GObj
 *   *(work+0x4A0) = entity_list_start
 *   entity stride = 0x190 (mult ac2, v1, 0x190)
 *   *(work + 0x565060 + 0x188) = entity_mask_bits (bit 0 = active)
 *     (0x565060 = GP-relative global structure base)
 *
 * Core semantic behavior (gate check + collision delegation):
 *   1. work = *(entity+0x15c)
 *   2. entity_list = *(work+0x4A0)
 *   3. mask = *(work + GLOBAL_OFFSET + 0x188)  — GP-relative, modeled as parameter
 *   4. if (mask & 1) == 0 → skip (return 0)
 *   5. setup_a(sp+0x10, entity, 0x2C)            jal 0x14A100
 *   6. setup_b(sp+0x20, entity, 0x33)            jal 0x14A100
 *   7. collision_result = collision_check(sp+0x10) jal 0x168538
 *   8. if (result != 0) → response(entity, 0x9D)  jal 0x15BCC8
 *   9. counter_inc(1)                             jal 0x203AA0
 *  10. loop back to step 1
 *
 * The semantic function models ONE iteration of the loop (the infinite
 * back-edge is the caller's responsibility in the original; here we
 * process a single entity). DIVERGENCE: in the original counter_inc fires
 * at the end of EVERY iteration (0x15E374, jal 0x203AA0), including the
 * gate-fail path; in this single-iteration model counter_inc runs only on
 * the gate-active path and the host wrapper is responsible for it on the
 * skipped (return 0) path. The five hook calls are delegated; NULL
 * hooks skip the call. The function returns 1 if the gate was active
 * (mask & 1), 0 if skipped.
 */
int ico_semantic_subEnemyCollision(const void *entity,
                                   u32 entity_mask,       /* gate: bit 0 */
                                   ico_ptr32 entity_list,  /* from *(work+0x4A0) */
                                   IcoSemanticTriFn setup_a,
                                   IcoSemanticTriFn setup_b,
                                   IcoSemanticTriFn collision_check,
                                   IcoSemanticTriFn collision_response,
                                   IcoSemanticTriFn counter_inc)
{
    void *work;
    ico_ptr32 entity_p;
    ico_ptr32 scratch_a;
    ico_ptr32 scratch_b;
    u8 scratch_area[0xB0];   /* covers sp+0x00..0xA4 slots of the 0x120 frame */

    (void)entity_list;

    if (entity == NULL) {
        return 0;
    }

    work = sec_cell((const u8 *)entity + 0x15c);
    if (work == NULL) {
        return 0;
    }

    /* Gate check: bit 0 of entity mask */
    if ((entity_mask & 1) == 0) {
        return 0;
    }

    entity_p = (ico_ptr32)(uintptr_t)entity;
    memset(scratch_area, 0, sizeof(scratch_area));
    scratch_a = (ico_ptr32)(uintptr_t)(scratch_area + 0x10);
    scratch_b = (ico_ptr32)(uintptr_t)(scratch_area + 0x20);

    /* setup_a: (&sp[0x10], entity, 0x2C) — built-in delegate fn_14A100.
     * The idx lookup (jal 0x109F10) is NULL here → idx 0 read. */
    if (setup_a != NULL) {
        (void)setup_a(scratch_a, entity_p, 0x2C);
    } else {
        ico_semantic_fun14A100(scratch_area + 0x10, entity, 0x2C, NULL);
    }

    /* setup_b: (&sp[0x20], entity, 0x33) — a1 stayed an entity throughout */
    if (setup_b != NULL) {
        (void)setup_b(scratch_b, entity_p, 0x33);
    } else {
        ico_semantic_fun14A100(scratch_area + 0x20, entity, 0x33, NULL);
    }

    /* Verified in subEnemyCollision.s between setup_b and collision_check:
     *   lwc1 $f0, 0x24($sp); sub.s $f0, $f0, $f20; swc1 $f0, 0x24($sp)
     * where $f20 = 0x40A00000 = 5.0f (mtc1 at entry). scratch_b[4] -= 5.0f. */
    sec_st_float((void *)(scratch_area + 0x24),
                 *(float *)(scratch_area + 0x24) - 5.0f);

    /* collision_check: (&sp[0x10]) → result. Registers at the call site:
     * a0=&sp[0x10], a1=entity, a2=0x33 (carried from setup_b). The original
     * reads the caller outcome from sp+0xA4 after the call; the host hook
     * returns it. */
    if (collision_check != NULL) {
        u32 result = collision_check(scratch_a, entity_p, 0x33);
        if (result != 0) {
            /* collision_response: (entity, 0x9D, 0) — built-in fn_15BCC8.
             * Incoming 0x9D is outside its select set {0xA8,0xAD}, so the
             * built-in validates *(entity+0xC)==1 and returns 0x9D. */
            if (collision_response != NULL) {
                (void)collision_response(entity_p, 0x9D, 0);
            } else {
                (void)ico_semantic_fun15BCC8(entity, 0x9D);
            }
        }
    }

    /* counter_inc: (1) — the original fires this on every completed pass */
    if (counter_inc != NULL) {
        (void)counter_inc(1, 0, 0);
    }

    return 1;
}

/* ── subEnemyCollision delegables (Rev.166) ─────────────────────────── */

/*
 * fn_14A100 (0x14A100, 0x74 B) — setup delegate used by subEnemyCollision at
 * call sites (&sp+0x10, key=0x2C) and (&sp+0x20, key=0x33). Confirmed from
 * src/core/asm/fn_14A100.s:
 *   move a0, s0; move a1, a2; jal 0x109F10   → v0 = lookup(entity, key)
 *   sll  v0, v0, 6                            → idx << 6  (idx stride 0x40)
 *   lw   a0, 0x15c(s0)                        → work = *(entity+0x15C)
 *   lw   v1, 0xc(a0); addu v1, v0, v1         → base = *(work+0xC) + idx*64
 *   lwc1 f0, 0x30(v1); swc1 f0, ($s1)         dst[0]  = f32(base+0x30)
 *   lwc1 f0, 0x34(v1); swc1 f0, 4($s1)        dst[4]  = f32(base+0x34)
 *   lwc1 f0, 0x38(v1); swc1 f0, 8($s1)        dst[8]  = f32(base+0x38)
 * Angle floats written from base+0x30/0x34/0x38 (no -1 adjustment).
 */
void ico_semantic_fun14A100(void *dst, const void *entity, u32 key,
                            IcoSemanticTriFn lookup)
{
    void *work;
    u8 *base;
    u32 idx;

    if (dst == NULL || entity == NULL) {
        return;
    }
    idx = (lookup != NULL)
              ? (u32)lookup((ico_ptr32)(uintptr_t)entity, key, 0)
              : 0u;
    work = sec_cell((const u8 *)entity + 0x15c);
    if (work == NULL) {
        return;
    }
    /* EE `lw $v1, 0x0c($a0)` — pointer cell modeled host-width. */
    base = sec_cell((const u8 *)work + 0x0c);
    if (base == (u8 *)0) {
        return;
    }
    base += (idx << 6);

    sec_st_float((u8 *)dst + 0,
                 *(float *)(base + 0x30));
    sec_st_float((u8 *)dst + 4,
                 *(float *)(base + 0x34));
    sec_st_float((u8 *)dst + 8,
                 *(float *)(base + 0x38));
}

/*
 * fn_15BCC8 (0x15BCC8, 0x7C B) — collision-select delegate. Confirmed from
 * src/core/asm/fn_15BCC8.s:
 *   only incoming a1 == 0xA8 or 0xAD enter the select (else tail with a1)
 *   a2 = *(entity+0x164) ; state = *(entity+0xC); state != 1 → tail
 *   f1 = u64 @a2+0x470 ; f2 = u64 @a2+0x480
 *   if b29(f1) && b29(f2)        → a1 = 0xA9   (dsrl32 0x1D; andi 1)
 *   else if b27(f1) && b27(f2)   → a1 = 0xAA   (movn $a1,$v1,b27($f2))
 *   else → a1 unchanged
 * The tail `j 0x13FF88` is the shared response sink; this model returns the
 * selected byte and leaves the sink to the host.
 */
u32 ico_semantic_fun15BCC8(const void *entity, u32 incoming)
{
    u32 state;
    const void *target;
    u64 f1;
    u64 f2;

    if (entity == NULL) {
        return incoming;
    }
    if (incoming != 0xA8u && incoming != 0xADu) {
        return incoming;
    }
    state = sec_ld_u32((const u8 *)entity + 0x0c);
    if (state != 1u) {
        return incoming;
    }
    target = sec_cell((const u8 *)entity + 0x164);
    if (target == NULL) {
        return incoming;
    }
    f1 = sec_ld_u64((const u8 *)target + 0x470);
    f2 = sec_ld_u64((const u8 *)target + 0x480);
    if (((f1 >> 29) & 1u) && ((f2 >> 29) & 1u)) {
        return 0xA9u;
    }
    if (((f1 >> 27) & 1u) && ((f2 >> 27) & 1u)) {
        return 0xAAu;
    }
    return incoming;
}

/*
 * fn_203AA0 (0x203AA0, 0xA0 B) — frame-delay delegate. Confirmed from
 * src/core/asm/fn_203AA0.s:
 *   v0 = lw 0x4EC0(0x27)          count  = *(0x274EC0)
 *   a1 = lw 4(v1)                 divisor= *(0x274EC4)
 *   v0 = 60 - count ; div v0,a1 ; div v0,60
 *   a0==0 → v1 = v0; else v1 = (v0 ? v0 : 1) (movz)
 *   v1>0  → spin: n=set v1; while(n) { jal 0x13D3F0; n-- }
 *   v1==0 (a0==0 && v0==0) → infinite jal 0x13D3F0 loop (never returns)
 *   a1(divisor)==0 → break 0,7 (trap)
 * Returns the yield count polynomial; 0 signals the infinite-wait/trap path.
 */
u32 ico_semantic_fun203AA0(u32 frame_count, const void *counters)
{
    u32 count;
    u32 divisor;
    u32 v;

    if (counters == NULL) {
        return 0;
    }
    count = sec_ld_u32((const u8 *)counters + 0x00);
    divisor = sec_ld_u32((const u8 *)counters + 0x04);
    if (divisor == 0u) {
        return 0u;   /* beql+break 0,7 trap on divisor==0 */
    }
    /* signed div [$zero] to match ps2 `div` semantics */
    v = (u32)(((int)(60u - count) / (int)divisor) / 60);
    if (frame_count == 0u) {
        return (v != 0u) ? v : 0u;   /* 0 → infinite wait path */
    }
    return (v != 0u) ? v : 1u;
}

/* ── Rev.167 inventoried named functions ────────────────────────────── */

/*
 * actEnemyFlagOnDead (0x15D5F0, 0x2C B) — confirmed from
 * src/entity/asm/actEnemyFlagOnDead.s:
 *   jal 0x1A6E28  (a0 = 0x560000 - 0x7740 = 0x5588C0)   enemy-dead flag send
 *   jal 0x203AA0  (a0 = 0)                               frame delay (a0==0)
 * The flag-send target 0x1A6E28 is an unmodelled delegable; flag_send hook
 * receives the 0x5588C0 store address. The delay uses the fn_203AA0 built-in.
 */
void ico_semantic_actEnemyFlagOnDead(IcoSemanticTriFn flag_send,
                                     const void *counters)
{
    if (flag_send != NULL) {
        (void)flag_send(0x5588C0u, 0, 0);
    }
    (void)ico_semantic_fun203AA0(0u, counters);
}

/*
 * AP1JumpReq (0x1AE3B0, 0x34 B) — confirmed from
 * src/entity/asm/AP1JumpReq.s:
 *   base = 0x4B3D10 ; a1 = -2 (0xFFFF...FFFE) ; a0 = 0xB4..0 (0xB5 iters)
 *   loop: *(u64*)(base + i*0x40) &= -2      (64-bit `and`; clears bit0)
 * Clears the low bit of every AP1 round state entry.
 */
void ico_semantic_AP1JumpReq(void *round_base, u32 count)
{
    u32 i;
    void *p;

    if (round_base == NULL) {
        return;
    }
    for (i = 0; i < count; ++i) {
        p = (u8 *)round_base + (i * 0x40u);
        *(u64 *)p &= (u64)-2;
    }
}

/*
 * actSt04bEne1Chk (0x203A10, 0x48 B) — confirmed from
 * src/entity/asm/actSt04bEne1Chk.s:
 *   v1 = *(entity+0x164) ; v0 = *(v1+0x12C)
 *   v0 != 0 → return 0 (slot busy)
 *   *(v1+0x130) = a2 ; *(v1+0x12C) = entity
 *   jal 0x13FF88 (a0=entity, a1=old a2, a2=old a1)   sink (host hook)
 *   return 1
 * The sink 0x13FF88 is the same shared response dispatch that fn_15BCC8
 * tail-jumps to; modelled as an optional host hook.
 */
int ico_semantic_actSt04bEne1Chk(const void *entity, u32 arg_b, u32 arg_c,
                                 IcoSemanticTriFn sink)
{
    void *m;

    if (entity == NULL) {
        return 0;
    }
    m = sec_cell((const u8 *)entity + 0x164);
    if (m == NULL) {
        return 0;
    }
    if (sec_ld_u32((const u8 *)m + 0x12c) != 0u) {
        return 0;
    }
    sec_st_u32((u8 *)m + 0x130, arg_c);
    sec_st_u32((u8 *)m + 0x12c, (u32)(uintptr_t)entity);   /* 32-bit slot
        (EE `sw`): +0x12c and +0x130 are adjacent 4-byte fields, so an
        8-byte host cell at 0x12c would clobber +0x130. */
    if (sink != NULL) {
        (void)sink((ico_ptr32)(uintptr_t)entity, arg_c, arg_b);
    }
    return 1;
}

/*
 * Semantic reconstruction of GirlForceFieldGeo (0x001C3C90, 0x178 bytes).
 * Probable original source: cloth/force-field geometry computation.
 * Ground truth: src/entity/asm/GirlForceFieldGeo.s (byte-exact).
 *
 * This is a sub-range of SetGirlClothDispSwitch (0x001C3C38, 0x1D0 bytes).
 * Both functions share the same prologue/epilogue and register save frame;
 * GirlForceFieldGeo begins at the div.s instruction inside the parent and
 * handles the later threshold comparison blocks.
 *
 * Confirmed from the disassembly (byte-identical tail path):
 *   lui 0x3f80; mtc1 ...            $f1 = 1.0
 *   cvt.w.s $f0, $f12               $f0 = (int)f12          (round-to-zero)
 *   mfc1 $v0, $f0                   integer part
 *   mtc1 $v0, $f0; cvt.s.w $f0,$f0  back to float
 *   sub.s $f12, $f12, $f0           frac = f12 - (float)int
 *   sub.s $f12, $f1, $f12           $f12 = 1.0 - frac
 *   jal 0x243AA8 (a0=s2, a1=model+s5, a2=model+s6)
 *   jr $ra
 *
 * Unconfirmed / delegated to hooks: the threshold-gated blocks that lead to
 * this tail. Each block in the original follows the pattern
 *   compute_dist → compute_mag → c.olt (compare) → force_path
 * where compute_dist is one of 0x243AE8/0x243AA8/0x243AD0, compute_mag is
 * 0x106028, and the force path calls 0x117C20 and 0x244448. These calls are
 * delegated to three block hooks; each hook returns nonzero when its force
 * settled (>$fN), matching the branching the semantic cannot reproduce
 * without the real magnitude values.
 *
 * The returned value IS the confirmed tail result: 1.0 - frac($f12). The
 * final 0x243AA8 call is delegated to final_output(out_a0, out_a1, out_a2).
 * NOTE: block_a/b/c argument registers are UNVERIFIED (the unknown
 * subroutines use s1/s2/sp+0x30 etc.); passing out_a2 is a modeling choice.
 * NOTE: cvt.w.s rounding mode is unconfirmed (assumed truncation toward
 * zero, matching the (int) cast); the original may round per FCSR.
 */
float ico_semantic_girlForceFieldGeo(float f12_input,     /* value rounded */
                                     ico_ptr32 out_a0,    /* s2 context */
                                     ico_ptr32 out_a1,    /* model+s5 */
                                     ico_ptr32 out_a2,    /* model+s6 */
                                     IcoSemanticTriFn block_a,
                                     IcoSemanticTriFn block_b,
                                     IcoSemanticTriFn block_c,
                                     IcoSemanticTriFn final_output)
{
    int int_part;
    float frac;
    float result;

    if (block_a != NULL) {
        (void)block_a(out_a2, 0, 0);
    }
    if (block_b != NULL) {
        (void)block_b(out_a2, 0, 0);
    }
    if (block_c != NULL) {
        (void)block_c(out_a2, 0, 0);
    }

    int_part = (int)f12_input;               /* cvt.w.s (round-to-zero) */
    frac = f12_input - (float)int_part;      /* sub.s */
    result = 1.0f - frac;                    /* sub.s with $f1 = 1.0 */

    if (final_output != NULL) {
        (void)final_output(out_a0, out_a1, out_a2);   /* jal 0x243AA8 */
    }

    return result;
}
