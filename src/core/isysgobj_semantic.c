#include "gobj_abi.h"

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
 * Semantic reconstruction of isysGObjActiveLink (0x0013FA30 region).
 * Ground truth: src/core/asm/isysGObjActiveLink.s consists of a single
 * `jr $31` -- a no-op stub. Confirmed: the function returns immediately and
 * performs no observable work. It is preserved to document the empty symbol.
 */
void ico_semantic_isysGObjActiveLink(void)
{
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

/*
 * NOTES ON THE DISPATCH CORE (iosOmExeEachGObj / iosOmCreateDL / _iosOmMain)
 *
 * These three functions invoke per-GObj / per-DL-entry callbacks that the
 * canonical ABI stores as 32-bit ee pointers (IcoGObj.callback +0x48, field
 * `callback`). The semantic pool deliberately keeps those fields 32-bit to
 * mirror the EE ABI, so they cannot carry 64-bit host function pointers.
 * Correctly invoking them therefore requires the native engine's pool (which
 * stores real host pointers), not this semantic model.
 *
 * Confirmed structure (from the byte-exact .s; recorded as documentation, not
 * re-implemented here to avoid a half-runnable callback path):
 *
 *  iosOmExeEachGObj (0x0013FD10):
 *    walks primary-list head 0x281A70[slot] via +0x10 (next), invoking
 *    cb(gobj, arg) on each element. -> pool.primary_heads[slot] via next.
 *
 *  iosOmCreateDL (0x0013FC00):
 *    walks a GObj chain linked via +0x34 (dl_next), head from gp-0x671C;
 *    mask gp-0x6724 gates slots; slot 0 also fires the GObj's own +0x48
 *    callback when mask bit 0 is set and callback is non-null; for each of
 *    32 slots, if mask bit(slot) and GObj slot_mask (+0x4C) bit(slot) are
 *    set, walks DL head 0x281AB0[slot] via +0x34, invoking +0x48 of each DL
 *    entry whose +0x16C is non-zero and whose type_mask (+0x50) ANDs
 *    non-zero with the GObj's type_mask.
 *
 *  _iosOmMain (0x0013F9D0):
 *    Pass 1 walks mask-enabled slots (0..7) of gp-0x6724 over primary head
 *    0x281A70[slot]; each GObj whose +0x16C and +0x170 are non-zero has its
 *    +0x28 user_data called as a callback. Pass 2 iterates the same mask
 *    over a type-node chain rooted at +0x2C, dispatching process nodes for
 *    types 0x13..0x1B via helpers 0x13D8A0 / 0x13D928 / 0x13F6B8 and the
 *    gp-0x6710 transient.
 *
 * The native engine already implements this dispatch with real pointers
 * (dispatchActiveLists / dispatchTypeSlots / dispatchAll, see Rev.105 /
 * native-port). This semantic core intentionally leaves the three functions
 * as documentation instead of shipping callbacks through 32-bit handles.
 */
