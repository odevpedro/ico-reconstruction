#include "engine/GObjEntityAllocator.h"

#include <cassert>
#include <cstring>

namespace {

using ico::engine::GObjEntityAllocator;

constexpr ico_ptr32 kArenaEeBase = 0x00100000;
constexpr std::size_t kArenaSize = 0x10000;

alignas(16) u8 g_arena[kArenaSize];
ico_ptr32 g_heapCursor = 0;

/* host-side helpers for building / inspecting the arena */
void store32(u8* p, u32 v) {
    std::memcpy(p, &v, sizeof(v));
}
u32 load32(const u8* p) {
    u32 v = 0;
    std::memcpy(&v, p, sizeof(v));
    return v;
}
u8* host(ico_ptr32 ee) {
    assert(ee >= kArenaEeBase && ee < kArenaEeBase + kArenaSize);
    return g_arena + (ee - kArenaEeBase);
}

ico_ptr32 g_eeListBase = 0;

struct HeapCall {
    ico_ptr32 context = 0;
    u32 size = 0;
    ico_ptr32 tag = 0;
    u32 line = 0;
};
HeapCall g_heapCalls[8];
int g_heapCount = 0;

/* Returns a fresh EE address inside the arena and bumps a cursor. */
ico_ptr32 heapAllocHook(ico_ptr32 context, u32 size, ico_ptr32 tag,
                        u32 line) {
    if (g_heapCount < 8) {
        g_heapCalls[g_heapCount].context = context;
        g_heapCalls[g_heapCount].size = size;
        g_heapCalls[g_heapCount].tag = tag;
        g_heapCalls[g_heapCount].line = line;
    }
    ++g_heapCount;
    const ico_ptr32 r = g_heapCursor;
    g_heapCursor += (size + 0xFu) & ~0xFu;
    return r;
}

u8* translateHook(ico_ptr32 address) {
    if (address < kArenaEeBase || address >= kArenaEeBase + kArenaSize) {
        return nullptr;
    }
    return host(address);
}

ico_ptr32 g_debugMsgs[8];
int g_debugCount = 0;
void debugPrintHook(ico_ptr32 message) {
    if (g_debugCount < 8) {
        g_debugMsgs[g_debugCount] = message;
    }
    ++g_debugCount;
}

int g_typeInitCalls = 0;
int g_condInitCalls = 0;
int g_slotInitCalls = 0;
int g_listInitCalls = 0;
u8* g_typeInitBlock = nullptr;
ico_ptr32 g_typeInitValue = 0;
u8* g_condInitBlock = nullptr;
ico_ptr32 g_condInitValue = 0;
u8* g_slotInitBlock = nullptr;
ico_ptr32 g_slotInitValue = 0;
u8* g_listInitBlock = nullptr;

void typeInitHook(u8* block, ico_ptr32 value) {
    ++g_typeInitCalls;
    g_typeInitBlock = block;
    g_typeInitValue = value;
}
void condInitHook(u8* block, ico_ptr32 value) {
    ++g_condInitCalls;
    g_condInitBlock = block;
    g_condInitValue = value;
}
void slotInitHook(u8* block, ico_ptr32 value) {
    ++g_slotInitCalls;
    g_slotInitBlock = block;
    g_slotInitValue = value;
}
void listInitHook(u8* block, ico_ptr32) {
    ++g_listInitCalls;
    g_listInitBlock = block;
}

void resetArena() {
    std::memset(g_arena, 0, sizeof(g_arena));
    g_heapCursor = kArenaEeBase + 0x1000;   /* block at 0x101000 */
    g_heapCount = 0;
    g_debugCount = 0;
    g_typeInitCalls = 0;
    g_condInitCalls = 0;
    g_slotInitCalls = 0;
    g_listInitCalls = 0;
}

constexpr ico_ptr32 kBlockEe = 0x00101000;
constexpr ico_ptr32 kSlotTableEe = 0x00101850;

void buildTwoEntryList() {
    /* records at stride 0x40: link at +0x00, id at +0x04; -1 terminates */
    g_eeListBase = kArenaEeBase + 0x400;
    u8* list = host(g_eeListBase);
    store32(list + 0x00 * 0x40 + 0x00, 0x1);
    store32(list + 0x00 * 0x40 + 0x04, 4);
    store32(list + 0x01 * 0x40 + 0x00, 0x2);
    store32(list + 0x01 * 0x40 + 0x04, 7);
    store32(list + 0x02 * 0x40 + 0x00, 0xFFFFFFFFu);
    store32(list + 0x02 * 0x40 + 0x04, 0xDEADBEEF);
}

void buildEmptyList() {
    g_eeListBase = kArenaEeBase + 0x400;
    u8* list = host(g_eeListBase);
    store32(list + 0x00 * 0x40 + 0x00, 0xFFFFFFFFu);  /* link = sentinel */
}

}  // namespace

int main() {
    /* --- Initialize rejects a missing hook or template --- */
    {
        GObjEntityAllocator a;
        u8 t[0x850] = {};
        assert(!a.initialize(nullptr, translateHook, debugPrintHook, 0xCC1,
                             0xCC2, t, typeInitHook, condInitHook,
                             slotInitHook, listInitHook));
        assert(!a.initialize(heapAllocHook, nullptr, debugPrintHook, 0xCC1,
                             0xCC2, t, typeInitHook, condInitHook,
                             slotInitHook, listInitHook));
        assert(!a.initialize(heapAllocHook, translateHook, debugPrintHook,
                             0xCC1, 0xCC2, nullptr, typeInitHook,
                             condInitHook, slotInitHook, listInitHook));
        assert(!a.isInitialized());
        assert(a.allocEntity(0x1, 0) == nullptr);
    }

    /* --- Full path: two-entry list, type init, cond init, slot fill --- */
    resetArena();
    buildTwoEntryList();

    u8 templ[0x850];
    std::memset(templ, 0xA5, sizeof(templ));
    store32(templ + 0x8C, g_eeListBase);
    store32(templ + 0x820, 0);                 /* flag cleared */
    store32(templ + 0x810, 0);                 /* placeholder, overwritten */

    GObjEntityAllocator alloc;
    assert(alloc.initialize(heapAllocHook, translateHook, debugPrintHook,
                            0xCC1, 0xCC2, templ, typeInitHook, condInitHook,
                            slotInitHook, listInitHook));
    assert(alloc.isInitialized());

    u8* block = alloc.allocEntity(/*type=*/0x2, /*arg=*/0xAABB);
    assert(block != nullptr);
    assert(block == host(kBlockEe));

    /* template copied verbatim */
    assert(std::memcmp(block, templ, sizeof(templ)) == 0);

    /* heap calls: block (0x850, line 0x1CE) then slot table (0x35,
       line 0x1B7); both with tag 0x612620 and the two GP contexts */
    assert(g_heapCount == 2);
    assert(g_heapCalls[0].context == 0xCC1);
    assert(g_heapCalls[0].size == 0x850);
    assert(g_heapCalls[0].tag == 0x612620);
    assert(g_heapCalls[0].line == 0x1CE);
    assert(g_heapCalls[1].context == 0xCC2);
    assert(g_heapCalls[1].size == 0x35);
    assert(g_heapCalls[1].tag == 0x612620);
    assert(g_heapCalls[1].line == 0x1B7);

    /* init hooks: type (0x2 != 0x5EB), cond (listBase != 0), slot (always),
       list */
    assert(g_typeInitCalls == 1 && g_typeInitBlock == block &&
           g_typeInitValue == 0x2);
    assert(g_condInitCalls == 1 && g_condInitBlock == block &&
           g_condInitValue == 0xAABB);
    assert(g_slotInitCalls == 1 && g_slotInitBlock == block &&
           g_slotInitValue == 0xAABB);
    assert(g_listInitCalls == 1 && g_listInitBlock == block);

    /* debug messages in the confirmed order */
    assert(g_debugCount == 4);
    assert(g_debugMsgs[0] == 0x612640);
    assert(g_debugMsgs[1] == 0x612658);
    assert(g_debugMsgs[2] == 0x612678);
    assert(g_debugMsgs[3] == 0x612698);

    /* slot table word stored at +0x810, second allocation address */
    assert(load32(block + 0x810) == kSlotTableEe);

    /* slot entries: id 4 -> slot 0, id 7 -> slot 1, rest 0xFF */
    const u8* table = host(kSlotTableEe);
    assert(table[4] == 0x00);
    assert(table[7] == 0x01);
    for (int i = 0; i < 0x35; ++i) {
        if (i != 4 && i != 7) {
            assert(table[i] == 0xFF);
        }
    }

    /* --- type 0x5EB skips the type init --- */
    resetArena();
    buildTwoEntryList();
    std::memset(templ, 0xA5, sizeof(templ));
    store32(templ + 0x8C, g_eeListBase);
    GObjEntityAllocator alloc2;
    assert(alloc2.initialize(heapAllocHook, translateHook, debugPrintHook,
                             0xCC1, 0xCC2, templ, typeInitHook, condInitHook,
                             slotInitHook, listInitHook));
    u8* block2 = alloc2.allocEntity(/*type=*/0x5EB, /*arg=*/0);
    assert(block2 != nullptr);
    assert(g_typeInitCalls == 0);
    assert(g_condInitCalls == 1);   /* listBase nonzero still gates it */
    assert(g_slotInitCalls == 1);
    assert(g_listInitCalls == 1);
    assert(g_heapCount == 2);

    /* --- Empty list: every slot is 0xFF --- */
    resetArena();
    buildEmptyList();
    std::memset(templ, 0xA5, sizeof(templ));
    store32(templ + 0x8C, g_eeListBase);
    GObjEntityAllocator alloc3;
    assert(alloc3.initialize(heapAllocHook, translateHook, debugPrintHook,
                             0xCC1, 0xCC2, templ, typeInitHook, condInitHook,
                             slotInitHook, listInitHook));
    u8* block3 = alloc3.allocEntity(0x2, 0);
    assert(block3 != nullptr);
    const u8* table3 = host(kSlotTableEe);
    for (int i = 0; i < 0x35; ++i) {
        assert(table3[i] == 0xFF);
    }

    /* --- No list (listBase 0) with flag 0: cond/list/slot-table skipped --- */
    resetArena();
    std::memset(templ, 0xA5, sizeof(templ));
    store32(templ + 0x8C, 0);
    store32(templ + 0x820, 0);
    store32(templ + 0x810, 0x12345678);
    GObjEntityAllocator alloc4;
    assert(alloc4.initialize(heapAllocHook, translateHook, debugPrintHook,
                             0xCC1, 0xCC2, templ, typeInitHook, condInitHook,
                             slotInitHook, listInitHook));
    u8* block4 = alloc4.allocEntity(0x2, 0);
    assert(block4 != nullptr);
    assert(g_condInitCalls == 0);
    assert(g_listInitCalls == 0);
    assert(g_heapCount == 1);                       /* block only */
    assert(load32(block4 + 0x810) == 0x12345678);   /* untouched */
    assert(g_debugCount == 4);                      /* final print still runs */

    /* --- Flag +0x820 nonzero alone gates the cond init --- */
    resetArena();
    std::memset(templ, 0xA5, sizeof(templ));
    store32(templ + 0x8C, 0);
    store32(templ + 0x820, 0x111);
    GObjEntityAllocator alloc5;
    assert(alloc5.initialize(heapAllocHook, translateHook, debugPrintHook,
                             0xCC1, 0xCC2, templ, typeInitHook, condInitHook,
                             slotInitHook, listInitHook));
    u8* block5 = alloc5.allocEntity(0x2, 0xABCD);
    assert(block5 != nullptr);
    assert(g_condInitCalls == 1 && g_condInitValue == 0xABCD);
    assert(g_listInitCalls == 0);
    assert(g_heapCount == 1);

    /* --- Shutdown makes the allocator inert --- */
    alloc.shutdown();
    assert(!alloc.isInitialized());
    assert(alloc.allocEntity(0x2, 0) == nullptr);

    return 0;
}