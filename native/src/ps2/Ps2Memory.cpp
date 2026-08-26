#include "ps2/Ps2Memory.h"
#include "runtime/Logger.h"
#include <cstring>

Ps2Memory::Ps2Memory()
    : m_heapBase(PS2_HEAP_START)
    , m_initialized(false) {
}

Ps2Memory::~Ps2Memory() {
    shutdown();
}

bool Ps2Memory::initialize(size_t heapSize) {
    if (m_initialized) {
        return true;
    }

    m_heap.resize(heapSize, 0);
    m_initialized = true;

    Logger::info("ps2", "PS2 memory initialized: %zu bytes at 0x%08X",
                 heapSize, m_heapBase);
    return true;
}

void Ps2Memory::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_heap.clear();
    m_initialized = false;
    Logger::info("ps2", "PS2 memory shutdown");
}

u8* Ps2Memory::translate(ico_ptr32 address) {
    if (!m_initialized) {
        return nullptr;
    }

    if (address >= m_heapBase && address < m_heapBase + m_heap.size()) {
        return &m_heap[address - m_heapBase];
    }

    return nullptr;
}

bool Ps2Memory::read8(ico_ptr32 address, u8& out) {
    u8* ptr = translate(address);
    if (!ptr) return false;
    out = *ptr;
    return true;
}

bool Ps2Memory::read16(ico_ptr32 address, u16& out) {
    u8* ptr = translate(address);
    if (!ptr) return false;
    std::memcpy(&out, ptr, sizeof(u16));
    return true;
}

bool Ps2Memory::read32(ico_ptr32 address, u32& out) {
    u8* ptr = translate(address);
    if (!ptr) return false;
    std::memcpy(&out, ptr, sizeof(u32));
    return true;
}

bool Ps2Memory::read64(ico_ptr32 address, u64& out) {
    u8* ptr = translate(address);
    if (!ptr) return false;
    std::memcpy(&out, ptr, sizeof(u64));
    return true;
}

bool Ps2Memory::write8(ico_ptr32 address, u8 value) {
    u8* ptr = translate(address);
    if (!ptr) return false;
    *ptr = value;
    return true;
}

bool Ps2Memory::write16(ico_ptr32 address, u16 value) {
    u8* ptr = translate(address);
    if (!ptr) return false;
    std::memcpy(ptr, &value, sizeof(u16));
    return true;
}

bool Ps2Memory::write32(ico_ptr32 address, u32 value) {
    u8* ptr = translate(address);
    if (!ptr) return false;
    std::memcpy(ptr, &value, sizeof(u32));
    return true;
}

bool Ps2Memory::write64(ico_ptr32 address, u64 value) {
    u8* ptr = translate(address);
    if (!ptr) return false;
    std::memcpy(ptr, &value, sizeof(u64));
    return true;
}

bool Ps2Memory::isInitialized() const {
    return m_initialized;
}

size_t Ps2Memory::getHeapSize() const {
    return m_heap.size();
}
