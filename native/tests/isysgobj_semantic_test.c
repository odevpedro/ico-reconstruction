#include "core/gobj_abi.h"

#include <assert.h>

static ico_ptr32 s_sister_args[6];
static int s_sister_calls;

static ico_ptr32 capture_proc_add(ico_ptr32 a0, ico_ptr32 a1, ico_ptr32 a2,
                                  ico_ptr32 a3, ico_ptr32 t0, ico_ptr32 t1)
{
    s_sister_args[0] = a0;
    s_sister_args[1] = a1;
    s_sister_args[2] = a2;
    s_sister_args[3] = a3;
    s_sister_args[4] = t0;
    s_sister_args[5] = t1;
    ++s_sister_calls;
    return 0x7;
}

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

    /* processPoll / processNeedsRemoval direct paths */
    {
        IcoProcessNode process = {0};
        process.active = 0x22;
        assert(ico_semantic_processPoll(&process) == 0x22);
        assert(ico_semantic_processNeedsRemoval(&process));
        assert(ico_semantic_processPoll(0) == 0);
        assert(!ico_semantic_processNeedsRemoval(0));
    }

    /* isysGObjProcRemoveUnlink: relink of neighbours and parent head/tail
       repair for head, middle and tail removal. */
    {
        IcoGObj *parent = ico_semantic_isysGObjAdd(&pool, 0x6000, 1, 5);
        IcoProcessNode proc[3];
        IcoProcessNodeSemanticPool ppool = {0};
        int i;

        for (i = 0; i < 3; ++i) {
            proc[i].self = (ico_ptr32)(i + 1);
            proc[i].parent = parent->self;
        }
        proc[0].next = 2;
        proc[0].prev = 0;
        proc[1].next = 3;
        proc[1].prev = 1;
        proc[2].next = 0;
        proc[2].prev = 2;
        ppool.slots = proc;
        ppool.capacity = 3;
        parent->process_head = 1;
        parent->process_tail = 3;

        /* middle */
        assert(ico_semantic_isysGObjProcRemoveUnlink(&pool, &ppool,
                                                     &proc[1]) == 1);
        assert(proc[0].next == 3);
        assert(proc[2].prev == 1);
        assert(parent->process_head == 1);
        assert(parent->process_tail == 3);

        /* head */
        assert(ico_semantic_isysGObjProcRemoveUnlink(&pool, &ppool,
                                                     &proc[0]) == 1);
        assert(parent->process_head == 3);
        assert(proc[2].prev == 0);

        /* the unlink does NOT clear the node's own fields */
        assert(proc[0].self == 1);
        assert(proc[0].next == 3);

        /* tail */
        assert(ico_semantic_isysGObjProcRemoveUnlink(&pool, &ppool,
                                                     &proc[2]) == 1);
        assert(parent->process_head == 0);
        assert(parent->process_tail == 0);

        assert(ico_semantic_isysGObjProcRemoveUnlink(&pool, &ppool,
                                                     0) == 0);
    }

    /* sister_callback_reg: argument shuffle and the fixed t1 constant. */
    {
        ico_ptr32 r;

        s_sister_calls = 0;
        r = ico_semantic_sisterCallbackReg(capture_proc_add, 0xAA, 0xBB,
                                           0xCC, 0xDD);
        assert(r == 0x7);
        assert(s_sister_calls == 1);
        assert(s_sister_args[0] == 0xAA);
        assert(s_sister_args[1] == 0xAA);
        assert(s_sister_args[2] == 0xBB);
        assert(s_sister_args[3] == 0xCC);       /* 0xCC & 0xff */
        assert(s_sister_args[4] == 0xDD);
        assert(s_sister_args[5] == 0x1800);

        /* a2 is masked to a byte before forwarding */
        s_sister_calls = 0;
        (void)ico_semantic_sisterCallbackReg(capture_proc_add, 1, 2, 0x1FF,
                                             4);
        assert(s_sister_calls == 1);
        assert(s_sister_args[3] == 0xFF);

        /* a null hook short-circuits the forward */
        assert(ico_semantic_sisterCallbackReg(0, 1, 2, 3, 4) == 0);
    }

    return 0;
}