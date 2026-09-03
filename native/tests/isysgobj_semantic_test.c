#include "core/gobj_abi.h"

#include <assert.h>

int main(void)
{
    IcoGObj storage[8];
    IcoGObjSemanticPool pool = {0};
    IcoGObj *first;
    IcoGObj *second;
    IcoGObj *relative;

    ico_semantic_isysGObjAlloc(&pool, storage, 8);
    first = ico_semantic_isysGObjAdd(&pool, 0x1000, 2, 20);
    second = ico_semantic_isysGObjAdd(&pool, 0x2000, 2, 30);
    assert(first != 0 && second != 0);

    relative = ico_semantic_isysGObjAddBeforeGObj(&pool, 0x3000, second);
    assert(relative != 0);
    assert(pool.primary_heads.entries[2] == first->self);
    assert(first->next == relative->self);
    assert(relative->next == second->self);
    assert(second->prev == relative->self);

    ico_semantic_isysGObjMoveAfterGObj(&pool, first, second);
    assert(pool.primary_heads.entries[2] == relative->self);
    assert(relative->next == second->self);
    assert(second->next == first->self);
    assert(pool.primary_tails.entries[2] == first->self);

    ico_semantic_isysGObjKindTableAdd(&pool, relative, 5);
    ico_semantic_isysGObjKindTableAdd(&pool, second, 5);
    assert(pool.kind_heads.entries[5] == relative->self);
    assert(relative->type_next == second->self);

    ico_semantic_isysGObjLinkObjDL(&pool, relative, 0x4000, 3, 10, 0x80);
    ico_semantic_isysGObjLinkObjDL(&pool, second, 0x5000, 3, 20, 0x80);
    assert(pool.dl_heads.entries[3] == relative->self);
    assert(relative->dl_next == second->self);

    ico_semantic_isysGObjRemove(&pool, relative);
    assert(relative->self == 0);
    assert(pool.kind_heads.entries[5] == second->self);
    assert(pool.primary_heads.entries[2] == second->self);
    return 0;
}
