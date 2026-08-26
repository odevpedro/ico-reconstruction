#include "game/IsysGObj.h"
#include "runtime/Logger.h"

IsysGObj::IsysGObj()
    : m_globalMask(0)
    , m_globalCounter(0)
    , m_initialized(false) {
    std::memset(m_headTable, 0, sizeof(m_headTable));
    std::memset(m_tailTable, 0, sizeof(m_tailTable));
    std::memset(m_dlTable, 0, sizeof(m_dlTable));
    std::memset(m_dlTailTable, 0, sizeof(m_dlTailTable));
}

IsysGObj::~IsysGObj() {
    shutdown();
}

bool IsysGObj::initialize() {
    if (m_initialized) {
        return true;
    }

    // Zero head table (8 entries)
    for (u32 i = 0; i < DL_HEAD_TAIL_COUNT; i++) {
        m_headTable[i] = 0;
    }

    // Zero tail table (8 entries)
    for (u32 i = 0; i < DL_HEAD_TAIL_COUNT; i++) {
        m_tailTable[i] = 0;
    }

    // Zero DL callback table (32 entries)
    for (u32 i = 0; i < DL_SLOT_COUNT; i++) {
        m_dlTable[i] = 0;
    }

    // Zero DL tail table (32 entries)
    for (u32 i = 0; i < DL_SLOT_COUNT; i++) {
        m_dlTailTable[i] = 0;
    }

    // Zero global mask and counter
    m_globalMask = 0;
    m_globalCounter = 0;

    m_initialized = true;
    Logger::info("isysgobj", "isysGObj initialized: head/tail=%u, dl=%u slots",
                 DL_HEAD_TAIL_COUNT, DL_SLOT_COUNT);
    return true;
}

void IsysGObj::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_initialized = false;
    Logger::info("isysgobj", "isysGObj shutdown");
}

bool IsysGObj::isInitialized() const {
    return m_initialized;
}
