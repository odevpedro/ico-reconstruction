#include "replay/StateDigest.h"

#define CHECK(c)                                                             \
    do {                                                                     \
        if (!(c)) {                                                          \
            std::fprintf(stderr, "CHECK FAILED %s:%d: %s\n", __FILE__,      \
                         __LINE__, #c);                                      \
            std::abort();                                                    \
        }                                                                    \
    } while (0)

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

using ico::replay::DigestBuilder;
using ico::replay::StateDigestFrame;
using ico::replay::computeFrameDigest;
using ico::replay::formatDigestLine;

namespace {

// Manual little-endian reference mirroring DigestBuilder's byte layout for one
// known frame; pins the digest contract so a PCSX2-side producer cannot drift.
uint64_t referenceDigest(const StateDigestFrame& f) {
    uint64_t h = ico::replay::kFnv1a64OffsetBasis;
    auto feed = [&h](uint64_t v, unsigned bytes) {
        for (unsigned i = 0; i < bytes; ++i)
            h = ico::replay::fnv1a64Step(h, static_cast<unsigned char>((v >> (i * 8)) & 0xFFu));
    };
    feed(0, 4);     // version marker slot 1
    feed(1, 4);     // version marker slot 2
    feed(f.frame, 4);
    feed(f.scene, 4);
    feed(f.gobj, 4);
    feed(f.active, 4);
    uint32_t bx = 0, by = 0, bz = 0;
    std::memcpy(&bx, &f.bx, 4);
    std::memcpy(&by, &f.by, 4);
    std::memcpy(&bz, &f.bz, 4);
    feed(bx, 4);
    feed(by, 4);
    feed(bz, 4);
    feed(static_cast<uint32_t>(f.mx) & 0xFFFFFFFFu, 4);
    feed(static_cast<uint32_t>(f.mz) & 0xFFFFFFFFu, 4);
    return h;
}

}  // namespace

int main() {
    StateDigestFrame f;
    f.frame = 3;
    f.scene = 0x0Fu;
    f.gobj = 25;
    f.active = 1;
    f.bx = 10.5f;
    f.by = 0.0f;
    f.bz = 0.0f;
    f.mx = 25;
    f.mz = 0;

    /* Pinned contract: computeFrameDigest equals the manual LE reference. */
    CHECK(computeFrameDigest(f) == referenceDigest(f));

    /* Formatting round-trip: hex in the line equals the raw digest. */
    const std::string line = formatDigestLine(f);
    CHECK(line.rfind("D3 ", 0) == 0);
    char hex16[17];
    std::snprintf(hex16, sizeof(hex16), "%016llx",
                  static_cast<unsigned long long>(computeFrameDigest(f)));
    CHECK(std::string(hex16).size() == 16);
    CHECK(line.find(hex16) == 3);

    /* Stability: identical fields in another instance hash identically. */
    StateDigestFrame g = f;
    CHECK(computeFrameDigest(g) == computeFrameDigest(f));

    /* Sensitivity: one bit flips the digest. */
    StateDigestFrame h = f;
    h.bx += 0.0001f;
    CHECK(computeFrameDigest(h) != computeFrameDigest(f));
    h = f;
    h.mx = 30;
    CHECK(computeFrameDigest(h) != computeFrameDigest(f));
    h = f;
    h.gobj = 24;
    CHECK(computeFrameDigest(h) != computeFrameDigest(f));

    /* FNV golden vector: fnv1a64 of the single byte 0x61 ('a'). */
    uint64_t a = ico::replay::kFnv1a64OffsetBasis;
    a = ico::replay::fnv1a64Step(a, 0x61u);
    CHECK(a == 0xaf63dc4c8601ec8cull);

    std::printf("state_digest_test: OK\n");
    return 0;
}