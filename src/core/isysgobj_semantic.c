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
 * Semantic reconstruction of primary-list removal used by isysGObjRemove
 * (0x0013E548) and slot recycling.
 * Probable original source: gobj.c.
 * Ground truth: src/core/asm/isysGObjRemove.s and the unlink body retained in
 * src/core/asm/isysGObjInit.s.
 * Confirmed: adjacent links and head/tail repair, then self=0 marks the slot
 * free. Inferred scope: attached-process and kind-table teardown remain outside
 * this deliberately small semantic core.
 */
void ico_semantic_isysGObjRemove(IcoGObjSemanticPool *pool, IcoGObj *gobj)
{
    IcoGObj *next;
    IcoGObj *prev;
    u8 list_id;

    if (pool == NULL || gobj == NULL || gobj->self == 0) {
        return;
    }

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

    gobj->self = 0;
    gobj->next = 0;
    gobj->prev = 0;
    gobj->unknown_004 = -1;
    gobj->unknown_008 = -1;
}
