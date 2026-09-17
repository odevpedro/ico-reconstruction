#include "replay/StateDigest.h"

#include <cstdio>
#include <cstring>

namespace ico::replay {

void DigestBuilder::feedLE(uint64_t value, unsigned bytes) {
    for (unsigned i = 0; i < bytes; ++i) {
        m_hash = fnv1a64Step(m_hash, static_cast<unsigned char>((value >> (i * 8)) & 0xFFu));
    }
}

DigestBuilder& DigestBuilder::u32(const char tag[2], uint32_t value) {
    (void)tag;  // v1: tags are informational in the text form, not hashed
    feedLE(value, 4);
    return *this;
}

DigestBuilder& DigestBuilder::s32(const char tag[2], int32_t value) {
    (void)tag;
    feedLE(static_cast<uint64_t>(value) & 0xFFFFFFFFull, 4);
    return *this;
}

DigestBuilder& DigestBuilder::f32(const char tag[2], float value) {
    (void)tag;
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    feedLE(bits, 4);
    return *this;
}

uint64_t computeFrameDigest(const StateDigestFrame& frame) {
    DigestBuilder b;
    b.u32("ws", 0).u32("sc", 1);            // reserved ext slot markers
    b.u32("sf", frame.frame)
        .u32("sc", frame.scene)
        .u32("go", frame.gobj)
        .u32("ac", frame.active)
        .f32("bx", frame.bx)
        .f32("by", frame.by)
        .f32("bz", frame.bz)
        .s32("mx", frame.mx)
        .s32("mz", frame.mz);
    return b.next();
}

std::string formatDigestLine(const StateDigestFrame& frame) {
    const uint64_t digest = computeFrameDigest(frame);
    char buf[192];
    std::snprintf(buf, sizeof(buf),
                  "D%u %016llx scene=%u gobj=%u active=%u boy=%.4f,%.4f,%.4f pad=%d,%d",
                  frame.frame,
                  static_cast<unsigned long long>(digest),
                  frame.scene, frame.gobj, frame.active,
                  static_cast<double>(frame.bx), static_cast<double>(frame.by),
                  static_cast<double>(frame.bz), frame.mx, frame.mz);
    return std::string(buf);
}

}  // namespace ico::replay