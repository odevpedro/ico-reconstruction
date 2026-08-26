#pragma once

#include "ps2/Ps2Types.h"
#include <cstddef>
#include <vector>

class Ps2Memory {
public:
    Ps2Memory();
    ~Ps2Memory();

    bool initialize(size_t heapSize = 0x01F00000);
    void shutdown();

    u8* translate(ico_ptr32 address);
    bool read8(ico_ptr32 address, u8& out);
    bool read16(ico_ptr32 address, u16& out);
    bool read32(ico_ptr32 address, u32& out);
    bool read64(ico_ptr32 address, u64& out);

    bool write8(ico_ptr32 address, u8 value);
    bool write16(ico_ptr32 address, u16 value);
    bool write32(ico_ptr32 address, u32 value);
    bool write64(ico_ptr32 address, u64 value);

    bool isInitialized() const;
    size_t getHeapSize() const;

private:
    std::vector<u8> m_heap;
    ico_ptr32 m_heapBase;
    bool m_initialized;
};
