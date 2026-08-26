#pragma once

#include "ps2/Ps2Types.h"
#include <cstring>

// Dispatch table slot count
constexpr u32 DL_SLOT_COUNT = 32;

// Head/tail table entry count
constexpr u32 DL_HEAD_TAIL_COUNT = 8;

// GObj structure (stride 0x174)
struct GObj {
    u8 padding[0x174];
};

// Process node / TCB (stride 0x94)
struct ProcessNode {
    u8 padding[0x94];
};

// isysGObj system state
class IsysGObj {
public:
    IsysGObj();
    ~IsysGObj();

    bool initialize();
    void shutdown();

    // Head table: 8 entries at 0x281A70
    u32 m_headTable[DL_HEAD_TAIL_COUNT];

    // Tail table: 8 entries at 0x281A90
    u32 m_tailTable[DL_HEAD_TAIL_COUNT];

    // DL callback table: 32 entries at 0x281AB0
    u32 m_dlTable[DL_SLOT_COUNT];

    // DL tail table: 32 entries at 0x281AD0
    u32 m_dlTailTable[DL_SLOT_COUNT];

    // Global mask bits at gp-0x6724
    u32 m_globalMask;

    // Global counter at gp-0x6720
    u32 m_globalCounter;

    bool isInitialized() const;

private:
    bool m_initialized;
};
