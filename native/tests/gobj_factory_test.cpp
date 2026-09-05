#include "engine/GObjFactory.h"

#include <cassert>
#include <cstring>

namespace {

using ico::engine::GObj;
using ico::engine::GObjFactory;
using ico::engine::GObjFactoryDescriptor;
using ico::engine::kNullGObjHandle;

IcoGObj g_slots[8];
IcoGObjSemanticPool g_pool;
GObjFactory g_factory;

struct ProcAddCapture {
    int count = 0;
    ico_ptr32 a0[8];
    ico_ptr32 a1[8];
    ico_ptr32 a2[8];
    ico_ptr32 a3[8];
    ico_ptr32 t0[8];
    ico_ptr32 t1[8];
};
ProcAddCapture g_cap;

/* isysGObjProcAdd_ hook (sister_callback_reg forward):
   proc_add(a0, a0, a1, a2 & 0xff, a3, 0x1800). */
ico_ptr32 procAddHook(ico_ptr32 a0, ico_ptr32 a1, ico_ptr32 a2,
                      ico_ptr32 a3, ico_ptr32 t0, ico_ptr32 t1) {
    const int c = g_cap.count;
    if (c < 12) {
        g_cap.a0[c] = a0;
        g_cap.a1[c] = a1;
        g_cap.a2[c] = a2;
        g_cap.a3[c] = a3;
        g_cap.t0[c] = t0;
        g_cap.t1[c] = t1;
    }
    ++g_cap.count;
    return 0;
}

int g_wrapCount = 0;
ico_ptr32 g_wrapA0 = 0;
ico_ptr32 g_wrapA1 = 0;
ico_ptr32 g_wrapA2 = 0;
ico_ptr32 g_wrapA3 = 0;
ico_ptr32 g_wrapT0 = 0;

/* isysGObjProcAdd_Wrapper (0x13F7A8) hook: (gobj, trigger, 0, 0x13,
   0x1800). */
ico_ptr32 procAddWrapHook(ico_ptr32 a0, ico_ptr32 a1, ico_ptr32 a2,
                          ico_ptr32 a3, ico_ptr32 t0) {
    ++g_wrapCount;
    g_wrapA0 = a0;
    g_wrapA1 = a1;
    g_wrapA2 = a2;
    g_wrapA3 = a3;
    g_wrapT0 = t0;
    return 0;
}

}  // namespace

int main() {
    std::memset(&g_pool, 0, sizeof(g_pool));
    ico_semantic_isysGObjAlloc(&g_pool, g_slots, 8);

    /* Initialize rejects a bad pool or missing hooks. */
    assert(!g_factory.initialize(nullptr, procAddHook, procAddWrapHook));
    assert(!g_factory.initialize(&g_pool, nullptr, procAddWrapHook));
    assert(!g_factory.initialize(&g_pool, procAddHook, nullptr));
    assert(!g_factory.isInitialized());

    assert(g_factory.initialize(&g_pool, procAddHook, procAddWrapHook));
    assert(g_factory.isInitialized());

    /* --- CreateGObj (0x240D40), full path --- */
    GObjFactoryDescriptor desc{};
    desc.trigger = 0x1000;
    desc.dl_callback = 0xAAAA;
    desc.cb_c = 0x33;
    desc.cb_b = 0x22;
    desc.cb_a = 0x11;
    desc.user_data = 0x777;

    GObj* g = g_factory.createGObj(desc, /*a1=*/0x5, /*a2=*/0x300,
                                   /*a3=*/0x8000, /*t0=*/1);
    assert(g != nullptr);

    /* Factory words: +0x0C=-1, +0x08=-1, +0x16C=1, +0x164=0, +0x04=1. */
    assert(g->user_data == 0x777);
    assert(g->unknown_004 == 1);
    assert(g->unknown_008 == 0x300);      /* later +0x08 = a2 */
    assert(g->state_16c == 1);
    assert(g->unknown_164 == 0);

    /* DL link: callback at +0x48, mask 0xFFFFFFFF, list 0, sort key a3. */
    assert(g->callback == 0xAAAA);
    assert(g->slot_mask == 0);
    assert(g->type_mask == 0xFFFFFFFFu);
    assert(g->dl_list_id == 0);
    assert(g->dl_sort_key == 0x8000);
    assert(g_pool.dl_heads.entries[0] == 1);

    /* Kind table: type = a1 (0x5), linked as the head of chain 5. */
    assert(g->type == 0x5);
    assert(g->type_next == 0);
    assert(g_pool.kind_heads.entries[5] == 1);

    /* Primary list: isysGObjAdd placed it on list 0. */
    assert(g_pool.primary_heads.entries[0] == 1);
    assert(g_pool.primary_tails.entries[0] == 1);

    /* ABI handle: slot 0 + 1. */
    assert(g->self == 1);

    /* Registry: table[0x712CC0 + counter] mirrored on the host. */
    assert(g_factory.registeredCount() == 1);
    assert(g_factory.registeredHandle(0) == 1);
    assert(g_factory.registeredHandle(1) == kNullGObjHandle);

    /* Three sister registrations with type ids 0x16/0x17/0x18. */
    assert(g_cap.count == 3);
    assert(g_cap.a0[0] == 1 && g_cap.a1[0] == 1);
    assert(g_cap.a2[0] == 0x11 && g_cap.a3[0] == 1);
    assert(g_cap.t0[0] == 0x16 && g_cap.t1[0] == 0x1800);
    assert(g_cap.a2[1] == 0x22 && g_cap.t0[1] == 0x17);
    assert(g_cap.a2[2] == 0x33 && g_cap.t0[2] == 0x18);

    /* Trigger wrap fired once with the confirmed constant tail. */
    assert(g_wrapCount == 1);
    assert(g_wrapA0 == 1);
    assert(g_wrapA1 == 0x1000);
    assert(g_wrapA2 == 0);
    assert(g_wrapA3 == 0x13);
    assert(g_wrapT0 == 0x1800);

    /* --- CreateGObj gate off (t0 == 0): trigger is not even loaded --- */
    GObjFactoryDescriptor desc2{};
    desc2.trigger = 0x9999;
    desc2.user_data = 0x778;

    GObj* g2 = g_factory.createGObj(desc2, /*a1=*/0x0, /*a2=*/0x0,
                                    /*a3=*/0x0, /*t0=*/0);
    assert(g2 != nullptr);
    assert(g2->user_data == 0x778);
    assert(g2->unknown_008 == 0);         /* a2 = 0 */
    assert(g2->type == 0);                /* a1 = 0, linked to chain 0 */
    assert(g_pool.kind_heads.entries[0] == 2);
    assert(g_wrapCount == 1);             /* gate off: no wrap call */
    assert(g2->self == 2);

    /* --- CreateGObj with a zero trigger word --- */
    GObjFactoryDescriptor desc3{};
    desc3.trigger = 0;
    desc3.user_data = 0x779;

    GObj* g3 = g_factory.createGObj(desc3, /*a1=*/0x1, /*a2=*/0x0,
                                    /*a3=*/0x0, /*t0=*/1);
    assert(g3 != nullptr);
    assert(g3->type == 1);
    assert(g_pool.kind_heads.entries[1] == 3);
    assert(g_wrapCount == 1);             /* trigger word zero: no wrap */

    /* --- CreateGObj_v (0x240EA0), full path --- */
    const int wrapBefore = g_wrapCount;
    GObj* gv = g_factory.createGObjV(0x901, /*cbA=*/0xA1, /*cbB=*/0xA2,
                                     /*cbC=*/0xA3, /*dlCallback=*/0xB1,
                                     /*dlSortKey=*/0x8800, /*trigger=*/0x1234);
    assert(gv != nullptr);
    assert(gv->user_data == 0x901);
    assert(gv->unknown_004 == 1);
    assert(gv->unknown_008 == -1);        /* _v does NOT write +0x08 */
    assert(gv->type == 0xFFFFFFFFu);      /* _v does NOT touch the kind table */
    assert(gv->state_16c == 1);
    assert(gv->callback == 0xB1);
    assert(gv->type_mask == 0xFFFFFFFFu);
    assert(gv->dl_sort_key == 0x8800);
    assert(gv->dl_list_id == 0);
    assert(gv->self == 4);
    assert(g_factory.registeredCount() == 4);
    assert(g_factory.registeredHandle(3) == 4);
    assert(g_wrapCount == wrapBefore + 1);
    assert(g_wrapA0 == 4);
    assert(g_wrapA1 == 0x1234);           /* trigger doubles as wrapper cb */
    assert(g_wrapA3 == 0x13 && g_wrapT0 == 0x1800);

    /* --- CreateGObj_v with a zero trigger --- */
    const int wrapsAfterV = g_wrapCount;
    GObj* gv2 = g_factory.createGObjV(0x902, 0, 0, 0, 0xBE, 0x44, 0);
    assert(gv2 != nullptr);
    assert(gv2->unknown_008 == -1);
    assert(gv2->type == 0xFFFFFFFFu);
    assert(gv2->callback == 0xBE);
    assert(gv2->dl_sort_key == 0x44);
    assert(g_wrapCount == wrapsAfterV);   /* trigger zero: no wrap */

    /* Twelve sisters registered in total (3 per object x 4 objects). */
    assert(g_cap.count == 12);

    /* --- Shutdown makes the factory inert --- */
    g_factory.shutdown();
    assert(!g_factory.isInitialized());
    assert(g_factory.registeredCount() == 0);
    assert(g_factory.createGObj(desc, 1, 1, 1, 1) == nullptr);
    assert(g_factory.createGObjV(0, 0, 0, 0, 0, 0, 0) == nullptr);

    return 0;
}