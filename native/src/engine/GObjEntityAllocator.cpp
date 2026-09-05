#include "engine/GObjEntityAllocator.h"

#include <cstring>

namespace ico::engine {

namespace {

constexpr u32 kSentinelInverse = 0xFFFFFFFFu;

u32 read_u32(const u8* p) {
    u32 v = 0;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

void write_u32(u8* p, u32 v) {
    std::memcpy(p, &v, sizeof(v));
}

}  // namespace

bool GObjEntityAllocator::initialize(HeapAllocFn heapAlloc,
                                     TranslateFn translate,
                                     DebugPrintFn debugPrint,
                                     ico_ptr32 blockContext,
                                     ico_ptr32 slotTableContext,
                                     u8* templateSource, InitFn typeInit,
                                     InitFn condInit, InitFn slotInit,
                                     InitFn listInit) {
    if (heapAlloc == nullptr || translate == nullptr ||
        templateSource == nullptr) {
        return false;
    }
    m_heapAlloc = heapAlloc;
    m_translate = translate;
    m_debugPrint = debugPrint;
    m_blockContext = blockContext;
    m_slotTableContext = slotTableContext;
    m_templateSource = templateSource;
    m_typeInit = typeInit;
    m_condInit = condInit;
    m_slotInit = slotInit;
    m_listInit = listInit;
    m_initialized = true;
    return true;
}

void GObjEntityAllocator::shutdown() {
    m_heapAlloc = nullptr;
    m_translate = nullptr;
    m_debugPrint = nullptr;
    m_blockContext = 0;
    m_slotTableContext = 0;
    m_templateSource = nullptr;
    m_typeInit = nullptr;
    m_condInit = nullptr;
    m_slotInit = nullptr;
    m_listInit = nullptr;
    m_initialized = false;
}

bool GObjEntityAllocator::isInitialized() const {
    return m_initialized;
}

void GObjEntityAllocator::copyTemplate(u8* dst) {
    /* .s: linear 0x850-byte copy (66 x 0x20 loop chunks plus a final 2 x 8B
       pair). memcpy reproduces the same observable result. */
    std::memcpy(dst, m_templateSource, kBlockSize);
}

void GObjEntityAllocator::fillSlotTable(u8* block,
                                        ico_ptr32 slotTableAddress) {
    u8* slotTable = m_translate(slotTableAddress);
    if (slotTable == nullptr) {
        return;
    }
    for (u32 i = 0; i < kSlotTableSize; ++i) {
        const ico_ptr32 listBaseAddress = read_u32(block + 0x8C);
        s32 match = -1;
        if (listBaseAddress != 0) {
            u8* listBase = m_translate(listBaseAddress);
            if (listBase != nullptr &&
                read_u32(listBase + 0) != kSentinelInverse) {
                /* Walk the 0x40-stride chain. The -1 link word terminates
                   the scan; the last record's +0x04 id is never examined
                   (matches the .s inner loop exactly). */
                u8* cursor = listBase;
                for (u32 j = 0;; ++j) {
                    if (read_u32(cursor + 0x04) == i) {
                        match = static_cast<s32>(j);
                        break;
                    }
                    cursor = listBase + (j + 1u) * kSlotStride;
                    if (read_u32(cursor + 0) == kSentinelInverse) {
                        match = -1;
                        break;
                    }
                }
            }
        }
        /* The +0x810 slot-table word is reloaded for every entry. */
        const ico_ptr32 reloaded = read_u32(block + 0x810);
        u8* table = m_translate(reloaded);
        if (table != nullptr) {
            table[i] = static_cast<u8>(match & 0xFF);
        }
    }
}

u8* GObjEntityAllocator::allocEntity(ico_ptr32 type, ico_ptr32 arg) {
    if (!m_initialized) {
        return nullptr;
    }

    const ico_ptr32 blockAddress =
        m_heapAlloc(m_blockContext, kBlockSize, kTag, kBlockAllocLine);
    u8* block = m_translate(blockAddress);
    if (block == nullptr) {
        return nullptr;
    }

    copyTemplate(block);

    if (type != 0x5EB && m_typeInit != nullptr) {
        m_typeInit(block, type);
    }
    if (m_debugPrint != nullptr) {
        m_debugPrint(kMsgBeforeTypeInit);
    }

    const u32 flag_820 = read_u32(block + 0x820);
    const ico_ptr32 listBaseEarly = read_u32(block + 0x8C);
    if ((flag_820 != 0 || listBaseEarly != 0) && m_condInit != nullptr) {
        m_condInit(block, arg);
    }
    if (m_debugPrint != nullptr) {
        m_debugPrint(kMsgBeforeCondInit);
    }

    if (m_slotInit != nullptr) {
        m_slotInit(block, arg);
    }
    if (m_debugPrint != nullptr) {
        m_debugPrint(kMsgBeforeSlotInit);
    }

    const ico_ptr32 listBase = read_u32(block + 0x8C);
    if (listBase != 0) {
        if (m_listInit != nullptr) {
            m_listInit(block, 0);
        }
        const ico_ptr32 slotTableAddress =
            m_heapAlloc(m_slotTableContext, kSlotTableSize, kTag,
                        kSlotTableAllocLine);
        write_u32(block + 0x810, slotTableAddress);
        fillSlotTable(block, slotTableAddress);
    }

    if (m_debugPrint != nullptr) {
        m_debugPrint(kMsgAfterTableFill);
    }

    return block;
}

}  // namespace ico::engine