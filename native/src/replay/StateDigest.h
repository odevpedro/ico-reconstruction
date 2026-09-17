#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace ico::replay {

// FNV-1a 64-bit non-cryptographic hash (offset basis 0xcbf29ce484222325,
// prime 0x100000001b3) over a little-endian canonical byte stream. The
// STATE DIGEST uses it so a PCSX2-side capture and the native runner can
// produce byte-identical digests for the same logical state, independent of
// host endianness.
inline constexpr uint64_t kFnv1a64OffsetBasis = 0xcbf29ce484222325ull;
inline constexpr uint64_t kFnv1a64Prime = 0x100000001b3ull;

inline uint64_t fnv1a64Step(uint64_t hash, unsigned char byte) {
    hash ^= byte;
    hash *= kFnv1a64Prime;
    return hash;
}

// Canonical little-endian serializer. Append fields in a FIXED order; the
// digest depends on the exact (tag, value) sequence, never on host layout.
class DigestBuilder {
public:
    explicit DigestBuilder(uint64_t seed = kFnv1a64OffsetBasis) : m_hash(seed) {}

    DigestBuilder& u32(const char tag[2], uint32_t value);
    DigestBuilder& s32(const char tag[2], int32_t value);
    DigestBuilder& f32(const char tag[2], float value);

    uint64_t next() const { return m_hash; }

private:
    void feedLE(uint64_t value, unsigned bytes);
    uint64_t m_hash;
};

// Canonical STATE DIGEST window v1. Both the instrumented PS2 capture and the
// native ReplayRunner shrink game state to these fields; text form carries the
// tags so tools/replay_diff.py can classify drift even when hashes collide on
// rotation.
struct StateDigestFrame {
    uint32_t frame = 0;
    uint32_t scene = 0;       // currentSceneId
    uint32_t gobj = 0;        // active host GObj count
    uint32_t active = 0;      // semantic: 1 when gameplay motion active
    float bx = 0.0f, by = 0.0f, bz = 0.0f;  // boy world position (PS2 cm)
    int32_t mx = 0, mz = 0;   // pad-derived desired delta consumed this frame
};

uint64_t computeFrameDigest(const StateDigestFrame& frame);

// One digest line emitted by a capture: "D<frame> <hex16> <tags...>".
// Used verbatim by the native runner and (later) the PCSX2 recorder so
// tools/replay_diff.py consumes one identical format from both sides.
std::string formatDigestLine(const StateDigestFrame& frame);

}  // namespace ico::replay