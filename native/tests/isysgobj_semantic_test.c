#include "core/gobj_abi.h"

#include <assert.h>
#include <math.h>
#include <string.h>

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

/* getEnemyDefLife hook capture (see the access-chain block in main). */
static ico_ptr32 s_gdl[3][3];
static int s_gdl_calls;

static ico_ptr32 gdl_hook(ico_ptr32 a0, ico_ptr32 a1, ico_ptr32 a2)
{
    assert(s_gdl_calls + 1 <= 3);
    s_gdl[s_gdl_calls][0] = a0;
    s_gdl[s_gdl_calls][1] = a1;
    s_gdl[s_gdl_calls][2] = a2;
    ++s_gdl_calls;
    return 0;
}

/* GirlForceFieldGeo hook capture: block_a→record[0], block_b→record[1],
   block_c/final→record[2]. The final output hook overrides record[2]. */
static ico_ptr32 s_gff[3][3];
static int s_gff_calls;

static ico_ptr32 gff_block0(ico_ptr32 a0, ico_ptr32 a1, ico_ptr32 a2)
{
    s_gff[0][0] = a0; s_gff[0][1] = a1; s_gff[0][2] = a2;
    ++s_gff_calls;
    return 1;
}
static ico_ptr32 gff_block1(ico_ptr32 a0, ico_ptr32 a1, ico_ptr32 a2)
{
    s_gff[1][0] = a0; s_gff[1][1] = a1; s_gff[1][2] = a2;
    ++s_gff_calls;
    return 0;
}
static ico_ptr32 gff_block2(ico_ptr32 a0, ico_ptr32 a1, ico_ptr32 a2)
{
    s_gff[2][0] = a0; s_gff[2][1] = a1; s_gff[2][2] = a2;
    ++s_gff_calls;
    return 1;
}

/* fn_14A100 idx lookup returning index 1 (Rev.166). */
static ico_ptr32 sec_test_lookup_index_1(ico_ptr32 a0, ico_ptr32 a1,
                                         ico_ptr32 a2)
{
    (void)a0; (void)a1; (void)a2;
    return (ico_ptr32)1u;
}

/* actEnemyFlagOnDead flag_send capture (Rev.167). */
static u32 s_efd_args[3];
static int s_efd_calls;

static ico_ptr32 efd_flag_send(ico_ptr32 a0, ico_ptr32 a1, ico_ptr32 a2)
{
    (void)a1; (void)a2;
    s_efd_args[0] = a0;
    ++s_efd_calls;
    return 0;
}

/* Write a host-width pointer into a byte buffer via a temporary (avoids the
   memcpy(&array, n) semantic trap where memcpy copies CONTENTS not address). */
#define STORE_PTR(dst, addr) \
    do { void *_tmp = (addr); memcpy((dst), &_tmp, sizeof(void *)); } while (0)
#define STORE_FLOAT(dst, f) do { \
    float _f = (f); memcpy((dst), &_f, sizeof(float)); } while (0)
#define STORE_U32(dst, v) do { \
    u32 _v = (v); memcpy((dst), &_v, sizeof(u32)); } while (0)
static u32 load_u32(const void *src)
{
    u32 _v = 0;
    memcpy(&_v, src, sizeof(u32));
    return _v;
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

    /* getEnemyDefLife (0x001C11C0): access chain
         root->self(+0) -> self+0x15c = work, work+0x800 = sched;
         sched+0x20 = type (u32), work+0x134 = float life.
       Flat buffers + STORE_PTR keep the PS2 byte offsets exact; pointer cells
       store host-width pointers (documented adaptation). Confirmed:
       type != 5 -> return 0 with no side effects; type == 5 -> life += 0.5f
       and three hook calls. */
    {
        u8 root_b[0x170];
        u8 self_b[0x170];
        u8 work_b[0x820];
        u8 sched_b[0xa0];
        ico_ptr32 root = (ico_ptr32)(uintptr_t)root_b;
        ico_ptr32 self = (ico_ptr32)(uintptr_t)self_b;
        ico_ptr32 work = (ico_ptr32)(uintptr_t)work_b;
        ico_ptr32 sched = (ico_ptr32)(uintptr_t)sched_b;

        memset(root_b, 0, sizeof(root_b));
        memset(self_b, 0, sizeof(self_b));
        memset(work_b, 0, sizeof(work_b));
        memset(sched_b, 0, sizeof(sched_b));
        STORE_PTR(root_b + 0x00, self_b);
        STORE_PTR(root_b + 0x15c, work_b);
        STORE_PTR(self_b + 0x15c, work_b);
        STORE_PTR(work_b + 0x800, sched_b);
        STORE_FLOAT(work_b + 0x134, 2.0f);
        STORE_U32(sched_b + 0x20, 5);

        s_gdl_calls = 0;

        {
            int r = ico_semantic_getEnemyDefLife(root_b, gdl_hook, gdl_hook,
                                                 gdl_hook);
            float life;
            memcpy(&life, work_b + 0x134, sizeof(life));
            assert(r == 1);
            assert(s_gdl_calls == 3);
            assert(life == 2.5f);
            /* prelude: a0 = &scratch[0x10] (nonzero), a1 = self */
            assert(s_gdl[0][0] != 0);
            assert(s_gdl[0][1] == self);
            /* stage: a0 = &scratch[0], a1 = *(root+0x15c) + 0xa0 */
            assert(s_gdl[1][0] != 0);
            assert(s_gdl[1][1] == work + 0xa0);
            /* sched_own: a0 = a1 = sched + 0xd0, a2 = &scratch */
            assert(s_gdl[2][0] == sched + 0xd0);
            assert(s_gdl[2][1] == sched + 0xd0);
            assert(s_gdl[2][2] != 0);
        }

        /* non-matching type: return 0, no hooks, no life write */
        s_gdl_calls = 0;
        STORE_U32(sched_b + 0x20, 6);
        STORE_FLOAT(work_b + 0x134, 3.0f);
        assert(ico_semantic_getEnemyDefLife(root_b, gdl_hook, gdl_hook,
                                            gdl_hook) == 0);
        assert(s_gdl_calls == 0);
        {
            float life;
            memcpy(&life, work_b + 0x134, sizeof(life));
            assert(life == 3.0f);
        }

        /* explicit NULL hooks on the type==5 path are allowed */
        STORE_U32(sched_b + 0x20, 5);
        assert(ico_semantic_getEnemyDefLife(root_b, 0, 0, 0) == 1);
        {
            float life;
            memcpy(&life, work_b + 0x134, sizeof(life));
            assert(life == 3.5f);
        }
    }

    /* ── HoldRope tests ──────────────────────────────────────────────── */
    {
        /* Setup: gp-0x53A4 = spring cell, gp-0x53B0 = entity cell
           (a pointer cell that holds the entity pointer) → entity → work */
        u8 spring_b[4];
        u8 entity_cell_b[sizeof(void *)];
        u8 entity_b[0x170];
        u8 work_b[0x100];
        float spring_val;
        u16 work_f0;
        float work_f4;

        memset(spring_b, 0, sizeof(spring_b));
        memset(entity_cell_b, 0, sizeof(entity_cell_b));
        memset(entity_b, 0, sizeof(entity_b));
        memset(work_b, 0, sizeof(work_b));
        STORE_PTR(entity_cell_b, entity_b);   /* gp-0x53B0 → entity */
        STORE_PTR(entity_b + 0x15c, work_b);  /* entity→work */
        STORE_FLOAT(spring_b, 0.5f);          /* initial spring */

        /* Test 1: no flags set → work_f0=0, work_f4=1.0, spring unchanged */
        ico_semantic_holdRope(spring_b, entity_cell_b, 0, 0,
                              0, 0, 0, 0, 0);
        memcpy(&work_f4, work_b + 0xF4, sizeof(float));
        memcpy(&work_f0, work_b + 0xF0, sizeof(u16));
        memcpy(&spring_val, spring_b, sizeof(float));
        assert(work_f4 == 1.0f);
        assert(work_f0 == 0);
        assert(spring_val == 0.5f);  /* no bit 3 → spring unchanged */

        /* Test 2: bit 3 (0x08) set → spring = 1.0 - (input_111 / 255.0)
           input_111 = 127 → 127/255 ≈ 0.498 → spring ≈ 0.502 */
        STORE_FLOAT(spring_b, 0.0f);
        ico_semantic_holdRope(spring_b, entity_cell_b, 0x08, 0,
                              127, 0, 0, 0, 0);
        memcpy(&spring_val, spring_b, sizeof(float));
        assert(spring_val > 0.50f && spring_val < 0.51f);

        /* Test 3: bit 1 (0x02) set → work_f4 = 1.0 - (input_113 * 0.0078125)
           input_113 = 64 → 64 * 0.0078125 = 0.5 → work_f4 = 0.5 */
        STORE_FLOAT(spring_b, 0.0f);
        ico_semantic_holdRope(spring_b, entity_cell_b, 0, 0x02,
                              0, 64, 0, 0, 0);
        memcpy(&work_f4, work_b + 0xF4, sizeof(float));
        assert(work_f4 == 0.5f);

        /* Test 4: bit 1 NOT set → work_f4 = 1.0 */
        STORE_FLOAT(work_b + 0xF4, 0.0f);  /* clear prior */
        ico_semantic_holdRope(spring_b, entity_cell_b, 0, 0,
                              0, 0, 0, 0, 0);
        memcpy(&work_f4, work_b + 0xF4, sizeof(float));
        assert(work_f4 == 1.0f);

        /* Test 5: bit 0x8000 → work_f0 = (input_109 / 255.0) * 8192
           input_109 = 255 → 1.0 * 8192 = 8192 */
        ico_semantic_holdRope(spring_b, entity_cell_b, 0x8000, 0,
                              0, 0, 255, 0, 0);
        memcpy(&work_f0, work_b + 0xF0, sizeof(u16));
        assert(work_f0 == 8192);

        /* Test 6: bit 0x2000 → work_f0 = (input_108 / 255.0) * (-8192)
           input_108 = 128 → 0.501961 * (-8192) ≈ -4112 */
        ico_semantic_holdRope(spring_b, entity_cell_b, 0x2000, 0,
                              0, 0, 0, 128, 0);
        memcpy(&work_f0, work_b + 0xF0, sizeof(u16));
        /* u16 wraps: -4112 → 0xEFE0 = 61424 */
        assert(work_f0 == 61424);

        /* Test 7: bit 0x8000 takes priority over 0x2000 (both set) */
        ico_semantic_holdRope(spring_b, entity_cell_b, 0x8000 | 0x2000, 0,
                              0, 0, 128, 0, 0);
        memcpy(&work_f0, work_b + 0xF0, sizeof(u16));
        assert(work_f0 == 4112);  /* 128/255 * 8192 = 4112 */

        /* Test 8: NULL entity cell → no crash */
        ico_semantic_holdRope(spring_b, 0, 0, 0, 0, 0, 0, 0, 0);

        /* Test 9: NULL spring_addr → no crash */
        ico_semantic_holdRope(0, entity_cell_b, 0, 0, 0, 0, 0, 0, 0);
    }

    /* ── subEnemyCollision tests ─────────────────────────────────────── */
    {
        u8 entity_b[0x170];
        u8 work_b[0x500];
        int r;

        memset(entity_b, 0, sizeof(entity_b));
        memset(work_b, 0, sizeof(work_b));
        STORE_PTR(entity_b + 0x15c, work_b);

        /* Test 1: mask bit 0 clear → returns 0, no hooks called */
        r = ico_semantic_subEnemyCollision(entity_b, 0x00, 0,
                                            0, 0, 0, 0, 0);
        assert(r == 0);

        /* Test 2: mask bit 0 set, NULL hooks → returns 1, no crash */
        r = ico_semantic_subEnemyCollision(entity_b, 0x01, 0,
                                            0, 0, 0, 0, 0);
        assert(r == 1);

        /* Test 3: NULL entity → returns 0 */
        r = ico_semantic_subEnemyCollision(0, 0x01, 0,
                                            0, 0, 0, 0, 0);
        assert(r == 0);

        /* Test 4: entity with NULL work → returns 0 */
        {
            u8 bad_entity[0x170] = {0};
            r = ico_semantic_subEnemyCollision(bad_entity, 0x01, 0,
                                                0, 0, 0, 0, 0);
            assert(r == 0);
        }
    }

    /* ── GirlForceFieldGeo tests ────────────────────────────────────── */
    {
        /* Confirmed pipeline: int_part=(int)f12, frac=f12-int,
           result = 1.0 - frac, then jal 0x243AA8 (hooks in order). */

        /* Test 1: f12 = 3.7 → frac=0.7 → result = 0.3 */
        {
            float out = ico_semantic_girlForceFieldGeo(
                3.7f, 0x1111, 0x2222, 0x3333, 0, 0, 0, 0);
            assert(fabsf(out - 0.3f) < 1e-5f);
        }

        /* Test 2: f12 = 1.0 → frac=0.0 → result = 1.0 */
        {
            float out = ico_semantic_girlForceFieldGeo(
                1.0f, 0, 0, 0, 0, 0, 0, 0);
            assert(fabsf(out - 1.0f) < 1e-6f);
        }

        /* Test 3: f12 = 0.0 → frac=0.0 → result = 1.0 */
        {
            float out = ico_semantic_girlForceFieldGeo(
                0.0f, 0, 0, 0, 0, 0, 0, 0);
            assert(fabsf(out - 1.0f) < 1e-6f);
        }

        /* Test 4: negative f12 = -2.5 → (int)=-2, frac=-0.5,
           result = 1.0 - (-0.5) = 1.5 */
        {
            float out = ico_semantic_girlForceFieldGeo(
                -2.5f, 0, 0, 0, 0, 0, 0, 0);
            assert(fabsf(out - 1.5f) < 1e-6f);
        }

        /* Test 5: hooks called in order; final_output receives out_a0/a1/a2.
           block_* return 1/0/1 (unused by the model — branches are host-side). */
        memset(s_gff, 0, sizeof(s_gff));
        s_gff_calls = 0;
        {
            float out = ico_semantic_girlForceFieldGeo(
                2.25f, 0xAAAA, 0xBBBB, 0xCCCC,
                gff_block0, gff_block1, gff_block2, gff_block2);
            assert(fabsf(out - 0.75f) < 1e-6f);
            assert(s_gff_calls == 4);      /* 3 blocks + 1 final */
            assert(s_gff[0][0] == 0xCCCC); /* block_* receives out_a2 */
            assert(s_gff[2][0] == 0xAAAA); /* final receives out_a0 */
            assert(s_gff[2][1] == 0xBBBB);
            assert(s_gff[2][2] == 0xCCCC);
        }

        /* Test 6: NULL hooks on every slot — base rounding still returns */
        {
            float out = ico_semantic_girlForceFieldGeo(
                7.5f, 0, 0, 0, 0, 0, 0, 0);
            assert(fabsf(out - 0.5f) < 1e-6f);
        }
    }

    /* ── subEnemyCollision delegables tests (Rev.166) ─────────────────── */
    {
        u8 entity_b[0x170];
        u8 work_b[0x510];
        u8 target_b[0x500];
        u8 preamble[0x800];
        u32 counter_b[2];

        memset(entity_b, 0, sizeof(entity_b));
        memset(work_b, 0, sizeof(work_b));
        memset(target_b, 0, sizeof(target_b));
        memset(preamble, 0, sizeof(preamble));
        counter_b[0] = 0;
        counter_b[1] = 60;

        /* Test 1: fn_14A100 setup — idx 0, base = *(work+0xC), angle floats
           from +0x30/0x34/0x38 land in dst[0..3) */
        {
            float angles[3] = {1.5f, 2.5f, 3.5f};
            u8 dst[12];
            memset(dst, 0, sizeof(dst));
            memset(preamble, 0, sizeof(preamble));
            memcpy(preamble + 0x30, angles, sizeof(angles));
            STORE_PTR(preamble + 0x0C, preamble);   /* base points at table */
            STORE_PTR(entity_b + 0x15c, work_b);
            STORE_PTR(work_b + 0x0c, preamble);
            ico_semantic_fun14A100(dst, entity_b, 0x2C, 0);
            assert(*(float *)(dst + 0) == 1.5f);
            assert(*(float *)(dst + 4) == 2.5f);
            assert(*(float *)(dst + 8) == 3.5f);
        }

        /* Test 2: fn_14A100 with an idx lookup hook → base advances idx<<6 */
        {
            float angles[3] = {9.0f, 8.0f, 7.0f};
            IcoSemanticTriFn idx_lookup = sec_test_lookup_index_1;
            u8 dst2[12];
            u8 idx_entity[0x170];
            memset(dst2, 0, sizeof(dst2));
            memset(idx_entity, 0, sizeof(idx_entity));
            STORE_PTR(idx_entity + 0x15c, work_b);
            STORE_PTR(work_b + 0x0c, preamble);
            memcpy(preamble + 0x70, angles, sizeof(angles));
            ico_semantic_fun14A100(dst2, idx_entity, 0x33, idx_lookup);
            assert(*(float *)(dst2 + 0) == 9.0f);
            assert(*(float *)(dst2 + 4) == 8.0f);
            assert(*(float *)(dst2 + 8) == 7.0f);
        }

        /* Test 3: fn_15BCC8 — incoming outside {0xA8,0xAD} stays */
        {
            STORE_PTR(entity_b + 0x164, target_b);
            STORE_U32(entity_b + 0x0C, 1);
            assert(ico_semantic_fun15BCC8(entity_b, 0x9D) == 0x9D);
        }

        /* Test 4: fn_15BCC8 — bit29 set on both u64 halves → 0xA9 */
        {
            u64 f1 = (u64)1 << 29;
            u64 f2 = (u64)1 << 29;
            memcpy(target_b + 0x470, &f1, sizeof(f1));
            memcpy(target_b + 0x480, &f2, sizeof(f2));
            assert(ico_semantic_fun15BCC8(entity_b, 0xA8) == 0xA9);
        }

        /* Test 5: fn_15BCC8 — only bit27 pair set → 0xAA */
        {
            u64 f1 = (u64)1 << 27;
            u64 f2 = (u64)1 << 27;
            memset(target_b + 0x470, 0, 0x20);
            memcpy(target_b + 0x470, &f1, sizeof(f1));
            memcpy(target_b + 0x480, &f2, sizeof(f2));
            assert(ico_semantic_fun15BCC8(entity_b, 0xAD) == 0xAA);
        }

        /* Test 6: fn_15BCC8 — single bit29 (not both) → incoming unchanged */
        {
            u64 f1 = (u64)1 << 29;
            u64 f2 = 0;
            memset(target_b + 0x470, 0, 0x20);
            memcpy(target_b + 0x470, &f1, sizeof(f1));
            memcpy(target_b + 0x480, &f2, sizeof(f2));
            assert(ico_semantic_fun15BCC8(entity_b, 0xA8) == 0xA8);
        }

        /* Test 7: fn_15BCC8 — state != 1 → unchanged even with bits set */
        {
            STORE_U32(entity_b + 0x0C, 0);
            assert(ico_semantic_fun15BCC8(entity_b, 0xA8) == 0xA8);
            STORE_U32(entity_b + 0x0C, 1);
        }

        /* Test 8: fn_203AA0 — divisor 0 → trap sentinel 0 */
        {
            u32 c[2] = {0, 0};
            assert(ico_semantic_fun203AA0(1, c) == 0);
        }

        /* Test 9: fn_203AA0 — count=30, divisor=60 → (60-30)/60/60 = 0 →
           frame_count!=0 clamps to 1; frame_count==0 returns 0 (infinite) */
        {
            u32 c[2] = {30, 60};
            assert(ico_semantic_fun203AA0(1, c) == 1);
            assert(ico_semantic_fun203AA0(0, c) == 0);
        }

        /* Test 10: fn_203AA0 — count=0, divisor=60 → 60/60/60 = 0 →
           frame_count 1 → 1; larger frame_count still 1 (a0 unused in math) */
        {
            u32 c[2] = {0, 60};
            assert(ico_semantic_fun203AA0(160, c) == 1);
        }

        /* Test 11: fn_203AA0 — count=0, divisor=1 → 60/1/60 = 1 → spin 1 */
        {
            u32 c[2] = {0, 1};
            assert(ico_semantic_fun203AA0(1, c) == 1);
            assert(ico_semantic_fun203AA0(0, c) == 1);
        }

        /* Test 12: subEnemyCollision NULL-hook path now uses built-ins and
           the verified -=5.0f on scratch_b[4]; active gate returns 1 */
        {
            int r = ico_semantic_subEnemyCollision(entity_b, 0x01, 0,
                                                     0, 0, 0, 0, 0);
            assert(r == 1);
        }

        /* Test 13: AP1JumpReq clears bit0 (>a1 = -2, 64-bit and) of count u64
           entries, stride 0x40 (base + i*0x40), leaves other bits intact */
        {
            u8 area[0x40 * 3];
            void *base = area;
            memset(area, 0, sizeof(area));
            STORE_U32(area + 0x00, 0xFFFFFFFF);
            STORE_U32(area + 0x04, 0xFFFFFFFF);
            STORE_U32(area + 0x40, 0xFFFFFFFE);
            STORE_U32(area + 0x44, 0xFFFFFFFF);
            STORE_U32(area + 0x80, 0x00000007);
            STORE_U32(area + 0x84, 0x00000000);
            ico_semantic_AP1JumpReq(base, 3);
            assert(load_u32(area + 0x00) == 0xFFFFFFFE);
            assert(load_u32(area + 0x04) == 0xFFFFFFFF);
            assert(load_u32(area + 0x40) == 0xFFFFFFFE);
            assert(load_u32(area + 0x44) == 0xFFFFFFFF);
            assert(load_u32(area + 0x80) == 0x00000006);
            assert(load_u32(area + 0x84) == 0x00000000);
            ico_semantic_AP1JumpReq(NULL, 7);   /* NULL-guard */
        }

/* Test 14: actSt04bEne1Chk busy/empty slot + sink dispatch + cell
           stores (0x164 → m, 0x12C busy, 0x130 payload) */
        {
            u8 entity_buf[0x170];
            u8 m[0x140];
            STORE_PTR(entity_buf + 0x164, m);
            STORE_U32(m + 0x12c, 0u);           /* slot free */
            s_efd_args[0] = 0; s_efd_calls = 0;
            assert(ico_semantic_actSt04bEne1Chk(entity_buf, 0x1B, 0x2C,
                                                efd_flag_send) == 1);
            assert(s_efd_calls == 1);
            assert(s_efd_args[0] == (u32)(uintptr_t)entity_buf);
            assert(load_u32(m + 0x130) == 0x2C);
            assert(load_u32(m + 0x12c) == (u32)(uintptr_t)entity_buf);

            STORE_U32(m + 0x12c, 1u);           /* slot busy */
            s_efd_calls = 0;
            assert(ico_semantic_actSt04bEne1Chk(entity_buf, 0x1B, 0x2C,
                                                efd_flag_send) == 0);
            assert(s_efd_calls == 0);
            assert(ico_semantic_actSt04bEne1Chk(NULL, 0x1B, 0x2C,
                                                efd_flag_send) == 0);
        }

        /* Test 15: actEnemyFlagOnDead fires flag_send(0x5588C0) then the
           built-in fn_203AA0 delay (count=160 → 1 frame, a0==0 path) */
        {
            u32 c[2] = {160, 60};
            s_efd_calls = 0; s_efd_args[0] = 0;
            ico_semantic_actEnemyFlagOnDead(efd_flag_send, c);
            assert(s_efd_calls == 1);
            assert(s_efd_args[0] == 0x5588C0u);
            s_efd_calls = 0;
            ico_semantic_actEnemyFlagOnDead(NULL, c);   /* hook-less path */
            assert(s_efd_calls == 0);
        }
    }

    return 0;
}