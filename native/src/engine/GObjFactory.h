#pragma once

#include "core/gobj_abi.h"
#include "engine/GObj.h"

#include <cstddef>
#include <vector>

namespace ico::engine {

/*
 * GObjFactory — semantic bridge for the two scene-object factories
 * CreateGObj (USA 0x00240D40, 0x160) and CreateGObj_v (USA 0x00240EA0,
 * 0x128).
 *
 * Ground truth: src/core/asm/CreateGObj.s and src/core/asm/CreateGObj_v.s
 * (both byte-exact against the USA ELF, Rev.129).
 *
 * Confirmed step sequence (identical in both):
 *   1. isysGObjAdd(user_data, 0, 0)                 (0x13E8D8)
 *   2. factory words: +0x0C=-1, +0x08=-1, +0x16C=1, +0x164=0, +0x04=1
 *   3. registry: table[0x712CC0 + counter] = gobj; counter++ (gp-0x4D58)
 *   4. 3x sister_callback_reg (0x13F778):
 *        (gobj, cbA, 1, 0x16)  (gobj, cbB, 1, 0x17)  (gobj, cbC, 1, 0x18)
 *      each forwarding to isysGObjProcAdd_(gobj, gobj, cb, 1, type, 0x1800)
 *   5. isysGObjLinkObjDL(gobj, dl_callback, type_id=0,
 *                        sort_key, type_bits=0xFFFFFFFF)   (0x13F130)
 *   6. if trigger: isysGObjProcAdd_Wrapper(gobj, trigger, 0, 0x13, 0x1800)
 *      (0x13F7A8) — host hook, no host semantic for the callee
 *
 * CreateGObj additionally: +0x08 = a2, then
 *   isysGObjKindTableAdd(gobj, a1)              (0x13E648)
 * CreateGObj_v does NOT write +0x08 and does NOT touch the kind table.
 *
 * The registry counter (gp-0x4D58) and table (0x712CC0) are documented
 * constants; on the host they are mirrored by a growable handle vector.
 *
 * This is a semantic bridge: host-side handles replace the PS2 absolute
 * addresses. It is not claimed byte-exact.
 */

/* Confirmed CreateGObj descriptor accesses (Rev.129). Stride is the 0x64
   entity descriptor; only the directly referenced words are named. */
struct GObjFactoryDescriptor {
    u8        reserved_000[0x40];  /* +0x000..+0x03F */
    ico_ptr32 trigger;             /* +0x040 */
    u8        reserved_044[0x04];  /* +0x044..+0x047 */
    ico_ptr32 dl_callback;         /* +0x048 */
    ico_ptr32 cb_c;                /* +0x04C (sister type 0x18) */
    ico_ptr32 cb_b;                /* +0x050 (sister type 0x17) */
    u8        reserved_054[0x08];  /* +0x054..+0x05B */
    ico_ptr32 cb_a;                /* +0x05C (sister type 0x16) */
    ico_ptr32 user_data;           /* +0x060 (isysGObjAdd arg) */
};

static_assert(sizeof(GObjFactoryDescriptor) == 0x64,
              "CreateGObj descriptor must keep the 0x64 stride");
static_assert(offsetof(GObjFactoryDescriptor, trigger) == 0x40,
              "CreateGObj descriptor trigger offset");
static_assert(offsetof(GObjFactoryDescriptor, dl_callback) == 0x48,
              "CreateGObj descriptor dl_callback offset");
static_assert(offsetof(GObjFactoryDescriptor, cb_c) == 0x4C,
              "CreateGObj descriptor cb_c offset");
static_assert(offsetof(GObjFactoryDescriptor, cb_b) == 0x50,
              "CreateGObj descriptor cb_b offset");
static_assert(offsetof(GObjFactoryDescriptor, cb_a) == 0x5C,
              "CreateGObj descriptor cb_a offset");
static_assert(offsetof(GObjFactoryDescriptor, user_data) == 0x60,
              "CreateGObj descriptor user_data offset");

class GObjFactory {
public:
    /* isysGObjProcAdd_ hook (no host semantic; Deferred to the port). */
    using ProcAddFn = IcoSemanticProcAddFn;
    /* isysGObjProcAdd_Wrapper (0x13F7A8) hook. Confirmed argument order:
       (gobj, callback, a2=0, a3=0x13, t0=0x1800). */
    using ProcAddWrapFn = ico_ptr32 (*)(ico_ptr32 a0, ico_ptr32 a1,
                                        ico_ptr32 a2, ico_ptr32 a3,
                                        ico_ptr32 t0);

    /* Original constants, documented for archaeology. */
    static constexpr ico_ptr32 kRegistryTableAddress = 0x00712CC0;
    static constexpr s32       kRegistryCounterGpOffset = -0x4D58;

    bool initialize(IcoGObjSemanticPool* pool, ProcAddFn procAdd,
                    ProcAddWrapFn procAddWrap);
    void shutdown();
    bool isInitialized() const;

    /* CreateGObj (0x240D40): descriptor form. a1 -> kind-table type,
       a2 -> final +0x08 write, a3 -> isysGObjLinkObjDL sort key,
       t0 -> gate for loading the +0x40 trigger word. */
    GObj* createGObj(const GObjFactoryDescriptor& desc,
                     ico_ptr32 a1, ico_ptr32 a2, ico_ptr32 a3, ico_ptr32 t0);

    /* CreateGObj_v (0x240EA0): register form. No +0x08 write, no kind
       table add. trigger serves as both gate and wrapper callback. */
    GObj* createGObjV(ico_ptr32 userData,
                      ico_ptr32 cbA, ico_ptr32 cbB, ico_ptr32 cbC,
                      ico_ptr32 dlCallback, ico_ptr32 dlSortKey,
                      ico_ptr32 trigger);

    std::size_t registeredCount() const;
    GObjHandle registeredHandle(std::size_t index) const;

private:
    void writeFactoryWords(GObj& gobj);
    void registerGObj(GObj& gobj);
    void sisterRegisters(GObj& gobj, ico_ptr32 cbA, ico_ptr32 cbB,
                         ico_ptr32 cbC);

    IcoGObjSemanticPool* m_pool = nullptr;
    ProcAddFn m_procAdd = nullptr;
    ProcAddWrapFn m_procAddWrap = nullptr;
    std::vector<GObjHandle> m_registry;
    std::size_t m_counter = 0;
    bool m_initialized = false;
};

}  // namespace ico::engine