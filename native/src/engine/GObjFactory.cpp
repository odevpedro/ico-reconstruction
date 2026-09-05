#include "engine/GObjFactory.h"

namespace ico::engine {

bool GObjFactory::initialize(IcoGObjSemanticPool* pool, ProcAddFn procAdd,
                             ProcAddWrapFn procAddWrap) {
    if (pool == nullptr || procAdd == nullptr || procAddWrap == nullptr) {
        return false;
    }
    m_pool = pool;
    m_procAdd = procAdd;
    m_procAddWrap = procAddWrap;
    m_registry.clear();
    m_counter = 0;
    m_initialized = true;
    return true;
}

void GObjFactory::shutdown() {
    m_pool = nullptr;
    m_procAdd = nullptr;
    m_procAddWrap = nullptr;
    m_registry.clear();
    m_counter = 0;
    m_initialized = false;
}

bool GObjFactory::isInitialized() const {
    return m_initialized;
}

void GObjFactory::writeFactoryWords(GObj& gobj) {
    /* Confirmed write order from CreateGObj.s (Rev.129):
       +0x0C = -1, +0x08 = -1, +0x16C = 1, +0x164 = 0, +0x04 = 1. */
    gobj.type = 0xFFFFFFFFu;
    gobj.unknown_008 = -1;
    gobj.state_16c = 1;
    gobj.unknown_164 = 0;
    gobj.unknown_004 = 1;
}

void GObjFactory::registerGObj(GObj& gobj) {
    /* CreateGObj writes table[0x712CC0 + counter] = gobj and stores the
       incremented counter back to gp-0x4D58; the host registry mirrors that
       observable contract as a growable handle vector. */
    m_registry.push_back(gobj.self);
    ++m_counter;
}

void GObjFactory::sisterRegisters(GObj& gobj, ico_ptr32 cbA, ico_ptr32 cbB,
                                  ico_ptr32 cbC) {
    /* Three sister_callback_reg calls (0x13F778) with type ids 0x16/0x17/
       0x18; the gobj (a0) argument travels as the ABI handle. Each forwards
       to the proc_add hook as (gobj, gobj, cb, 1, type_id, 0x1800). */
    ico_semantic_sisterCallbackReg(m_procAdd, gobj.self, cbA, 1, 0x16);
    ico_semantic_sisterCallbackReg(m_procAdd, gobj.self, cbB, 1, 0x17);
    ico_semantic_sisterCallbackReg(m_procAdd, gobj.self, cbC, 1, 0x18);
}

GObj* GObjFactory::createGObj(const GObjFactoryDescriptor& desc,
                              ico_ptr32 a1, ico_ptr32 a2, ico_ptr32 a3,
                              ico_ptr32 t0) {
    if (!m_initialized) {
        return nullptr;
    }

    /* t0 gates the +0x40 trigger load; with the gate off the trigger stays
       the zero the function entered with (beqz in CreateGObj.s). */
    const ico_ptr32 trigger = (t0 != 0) ? desc.trigger : 0;

    GObj* gobj = ico_semantic_isysGObjAdd(m_pool, desc.user_data, 0, 0);
    if (gobj == nullptr) {
        return nullptr;
    }

    writeFactoryWords(*gobj);
    registerGObj(*gobj);
    sisterRegisters(*gobj, desc.cb_a, desc.cb_b, desc.cb_c);

    /* isysGObjLinkObjDL(gobj, +0x48 callback, 0, a3, 0xFFFFFFFF):
       a2=0 is the DL type id, a3 the sort key, t0=0xFFFFFFFF the mask. */
    ico_semantic_isysGObjLinkObjDL(m_pool, gobj, desc.dl_callback, 0, a3,
                                   0xFFFFFFFFu);

    if (trigger != 0) {
        /* isysGObjProcAdd_Wrapper (0x13F7A8): (gobj, trigger, 0, 0x13,
           0x1800). Host hook; the callee has no host semantic yet. */
        m_procAddWrap(gobj->self, trigger, 0, 0x13, 0x1800);
    }

    /* CreateGObj-only tail: +0x08 = a2, then KindTableAdd(gobj, a1). */
    gobj->unknown_008 = static_cast<s32>(a2);
    ico_semantic_isysGObjKindTableAdd(m_pool, gobj, a1);

    return gobj;
}

GObj* GObjFactory::createGObjV(ico_ptr32 userData, ico_ptr32 cbA,
                               ico_ptr32 cbB, ico_ptr32 cbC,
                               ico_ptr32 dlCallback, ico_ptr32 dlSortKey,
                               ico_ptr32 trigger) {
    if (!m_initialized) {
        return nullptr;
    }

    GObj* gobj = ico_semantic_isysGObjAdd(m_pool, userData, 0, 0);
    if (gobj == nullptr) {
        return nullptr;
    }

    writeFactoryWords(*gobj);
    registerGObj(*gobj);
    sisterRegisters(*gobj, cbA, cbB, cbC);

    /* isysGObjLinkObjDL(gobj, dlCallback, 0, dlSortKey, 0xFFFFFFFF). In the
       .s the DL callback travels in t1 and the sort key in t2. No +0x08
       write and no kind-table add for the _v form. */
    ico_semantic_isysGObjLinkObjDL(m_pool, gobj, dlCallback, 0, dlSortKey,
                                   0xFFFFFFFFu);

    if (trigger != 0) {
        /* CreateGObj_v reuses the same source register for the beqz gate
           and the wrapper callback (a1), mirroring the .s exactly. */
        m_procAddWrap(gobj->self, trigger, 0, 0x13, 0x1800);
    }

    return gobj;
}

std::size_t GObjFactory::registeredCount() const {
    return m_counter;
}

GObjHandle GObjFactory::registeredHandle(std::size_t index) const {
    if (index >= m_registry.size()) {
        return kNullGObjHandle;
    }
    return m_registry[index];
}

}  // namespace ico::engine