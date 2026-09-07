#include "engine/Ps2oMesh.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

using namespace ico::engine;

namespace {

uint8_t* putU32(uint8_t* p, uint32_t v) {
    p[0] = v & 0xFF;
    p[1] = (v >> 8) & 0xFF;
    p[2] = (v >> 16) & 0xFF;
    p[3] = (v >> 24) & 0xFF;
    return p + 4;
}

uint8_t* putU16(uint8_t* p, uint16_t v) {
    p[0] = v & 0xFF;
    p[1] = (v >> 8) & 0xFF;
    return p + 2;
}

uint8_t* putFloat(uint8_t* p, float f) {
    uint32_t bits;
    __builtin_memcpy(&bits, &f, 4);
    return putU32(p, bits);
}

std::vector<uint8_t> makeBox() {
    // PS2O header: magic, payload=size-16, submeshes=1, "SUM\0", then 4 vertices.
    std::vector<uint8_t> b;
    b.resize(0x20 + 4 * 16 + 4 * 16 + 6 * 2 + 8); // header+verts+faces+term+guard
    std::memcpy(b.data(), "PS2O", 4);
    putU32(b.data() + 4, 0); // patched later
    putU32(b.data() + 8, 1);
    std::memcpy(b.data() + 0x18, "SUM\0", 4);
    putU32(b.data() + 0x1c, 0x01020304);

    uint8_t* p = b.data() + 0x20;
    const float positions[4][3] = {
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
    };
    for (int v = 0; v < 4; ++v) {
        p = putFloat(p, positions[v][0]);
        p = putFloat(p, positions[v][1]);
        p = putFloat(p, positions[v][2]);
        p = putFloat(p, 1.0f);
    }
    const size_t posEnd = static_cast<size_t>(p - b.data());

    // Face data: a type-0x00 triangle strip framed by 0xFFFF records.
    // spine: [0,1,2,3] -> triangles (0,1,2) (1,2,3)
    const uint16_t kFrame[8]  = { 0xFFFF, 0x00, 0x00, 0, 0x0001, 0, 0, 0 };
    const uint16_t kCont[8]   = { 0x0000, 0x00, 0x00, 0, 0x0001, 0, 0, 0 };
    auto emitRecord = [&](uint16_t c, uint16_t a) {
        uint16_t r[8] = { c, 0x00, 0x00, a, 0x0001, a, 0x00, 0x00 };
        for (uint16_t w : r) p = putU16(p, w);
        (void)kFrame; (void)kCont;
    };
    (void)kFrame; (void)kCont;
    emitRecord(0xFFFF, 0);
    emitRecord(0x0000, 1);
    emitRecord(0x0000, 2);
    emitRecord(0x0000, 3);
    // terminator run
    for (int i = 0; i < 6; ++i) p = putU16(p, 0xFFFF);

    putU32(b.data() + 4, static_cast<uint32_t>(b.size() - 16));
    (void)posEnd;
    return b;
}

} // namespace

int main() {
    std::vector<uint8_t> box = makeBox();

    Ps2oMesh mesh;
    const bool ok = loadPs2oMesh(box.data(), box.size(), mesh);
    assert(ok);
    assert(mesh.vertexCount == 4);
    assert(mesh.subMeshCount == 1);
    assert(mesh.triangles.size() == 6); // (0,1,2) (1,2,3)
    assert(mesh.triangles[0] == 0);
    assert(mesh.triangles[1] == 1);
    assert(mesh.triangles[2] == 2);
    assert(mesh.triangles[3] == 1);
    assert(mesh.triangles[4] == 2);
    assert(mesh.triangles[5] == 3);
    assert(mesh.positions.size() == 12); // 4 verts x 3 floats

    // Position round-trip.
    assert(mesh.positions[0 * 3 + 0] == 0.0f);
    assert(mesh.positions[1 * 3 + 1] == 0.0f);
    assert(mesh.positions[3 * 3 + 2] == 0.0f);

    // Reject non-PS2O.
    Ps2oMesh bad;
    assert(!loadPs2oMesh(reinterpret_cast<const uint8_t*>("XXXX"), 4, bad));

    std::fprintf(stderr, "ps2o_mesh_test: OK (%u verts, %u tris)\n",
                 mesh.vertexCount, static_cast<unsigned>(mesh.triangles.size() / 3));
    return 0;
}