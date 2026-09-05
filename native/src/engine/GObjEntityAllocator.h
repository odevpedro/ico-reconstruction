#pragma once

#include "core/gobj_abi.h"

#include <cstddef>

namespace ico::engine {

/*
 * GObjEntityAllocator — semantic bridge for AllocGObjEntity (USA
 * 0x0019F310, 0x850 bytes).
 *
 * Ground truth: src/core/asm/AllocGObjEntity.s (byte-exact against the USA
 * ELF, Rev.130).
 *
 * Confirmed byte sequence:
 *   1. heapAlloc(ctx gp-0x68CC, 0x850, tag 0x612620, line 0x1CE)
 *   2. template copy: linear copy of 0x850 bytes (loop of 66 x 0x20 chunks
 *      plus a final 2 x 8B pair)
 *   3. if type != 0x5EB: typeInit(block, type)            (0x1A27F8)
 *   4. DebugPrint(0x612640)                               (0x1A6E28 hook)
 *   5. if *(block+0x820) != 0 || *(block+0x8C) != 0:
 *        condInit(block, arg)                             (0x19EF58)
 *   6. DebugPrint(0x612658)
 *   7. slotInit(block, arg)                               (0x19E648), always
 *   8. DebugPrint(0x612678)
 *   9. if *(block+0x8C) != 0:
 *        listInit(block)                                  (0x19EF10)
 *        slotTable = heapAlloc(ctx gp-0x68E0, 0x35, tag 0x612620,
 *                              line 0x1B7)
 *        *(block+0x810) = slotTable
 *        for i in 0x00..0x34:
 *          listBase = *(block+0x8C)            (reloaded every iteration)
 *          if *(listBase) == -1: slot i = -1
 *          else walk the 0x40-stride chain:
 *            cursor = listBase + j*0x40 (j = 0, 1, ...)
 *            if *(cursor+0x04) == i: slot i = j, stop
 *            next = listBase + (j+1)*0x40
 *            if *(next) == -1: slot i = -1, stop
 *          *(slotTable + i) = byte
 *  10. DebugPrint(0x612698)
 *  returns block
 *
 * The EE pointer arithmetic (listBase, slotTable) runs through a translate
 * hook; the 0x850 template is copied from a host-supplied source pointer
 * (the original bytes at 0x2F23F0 are copyrighted and are never embedded in
 * this repository). Init hooks are optional and mirror the original's
 * indirect calls.
 *
 * This is a semantic bridge: host pointers replace the PS2 absolute
 * addresses. It is not claimed byte-exact.
 */
class GObjEntityAllocator {
public:
    using HeapAllocFn = ico_ptr32 (*)(ico_ptr32 context, u32 size,
                                      ico_ptr32 tag, u32 line);
    using TranslateFn = u8* (*)(ico_ptr32 address);
    using DebugPrintFn = void (*)(ico_ptr32 message);
    /* typeInit/condInit/slotInit carry a value; listInit ignores it. */
    using InitFn = void (*)(u8* block, ico_ptr32 value);

    /* Confirmed constants, documented for archaeology. */
    static constexpr u32       kBlockSize = 0x850;
    static constexpr u32       kSlotTableSize = 0x35;
    static constexpr u32       kSlotStride = 0x40;
    static constexpr ico_ptr32 kTag = 0x612620;
    static constexpr u32       kBlockAllocLine = 0x1CE;
    static constexpr u32       kSlotTableAllocLine = 0x1B7;
    static constexpr ico_ptr32 kBlockContextGpOffset = -0x68CC;
    static constexpr ico_ptr32 kSlotTableContextGpOffset = -0x68E0;
    /* Debug message addresses confirmed in the .s. */
    static constexpr ico_ptr32 kMsgBeforeTypeInit = 0x612640;
    static constexpr ico_ptr32 kMsgBeforeCondInit = 0x612658;
    static constexpr ico_ptr32 kMsgBeforeSlotInit = 0x612678;
    static constexpr ico_ptr32 kMsgAfterTableFill = 0x612698;

    bool initialize(HeapAllocFn heapAlloc, TranslateFn translate,
                    DebugPrintFn debugPrint, ico_ptr32 blockContext,
                    ico_ptr32 slotTableContext, u8* templateSource,
                    InitFn typeInit, InitFn condInit, InitFn slotInit,
                    InitFn listInit);
    void shutdown();
    bool isInitialized() const;

    /* Semantics of AllocGObjEntity(type, arg). Returns the host-resolved
       block offset pointer that replaces the original EE block address. */
    u8* allocEntity(ico_ptr32 type, ico_ptr32 arg);

private:
    void copyTemplate(u8* dst);
    void fillSlotTable(u8* block, ico_ptr32 slotTableAddress);

    HeapAllocFn m_heapAlloc = nullptr;
    TranslateFn m_translate = nullptr;
    DebugPrintFn m_debugPrint = nullptr;
    ico_ptr32 m_blockContext = 0;
    ico_ptr32 m_slotTableContext = 0;
    u8* m_templateSource = nullptr;
    InitFn m_typeInit = nullptr;
    InitFn m_condInit = nullptr;
    InitFn m_slotInit = nullptr;
    InitFn m_listInit = nullptr;
    bool m_initialized = false;
};

}  // namespace ico::engine