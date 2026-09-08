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

// Builds a minimal PS2O in the Rev.151 validated strip format:
// positions at +0x20, a UV array (u,v,0,0 entries) right after, then a
// single strip: header [N, 0xFFFF x7] with N=4 records, each record is
// 8 x u16 = [1, 0, a, s, m, b, u, f] where a = position index (u16[2]),
// m = UV index (u16[4]) and f = material index (u16[7]).
std::vector<uint8_t> makeBox() {
    constexpr uint32_t kPosCount = 4;   // 0x20 + 4*16 = 0x60
    constexpr uint32_t kUvCount  = 4;   // 4 * 16 = 0x40, ends at 0xA0
    constexpr uint32_t kFaceOff  = 0xB0;
    std::vector<uint8_t> b(kFaceOff + 16 /*header*/ + 4 * 16 /*records*/ + 16 /*guard*/, 0);
    std::memcpy(b.data(), "PS2O", 4);
    putU32(b.data() + 4, 0);                                 // payload, patched later
    putU32(b.data() + 8, 1);                               // sub-mesh count
    std::memcpy(b.data() + 0x18, "SUM\0", 4);              // "SUM\0" tag
    putU32(b.data() + 0x1c, 0x01020304);

    uint8_t* p = b.data() + 0x20;
    const float positions[4][3] = {
        { 0.0f, 0.0f, 0.0f },  // v0
        { 1.0f, 0.0f, 0.0f },  // v1
        { 1.0f, 1.0f, 0.0f },  // v2
        { 0.0f, 1.0f, 0.0f },  // v3
    };
    for (int v = 0; v < 4; ++v) {
        p = putFloat(p, positions[v][0]);
        p = putFloat(p, positions[v][1]);
        p = putFloat(p, positions[v][2]);
        p = putFloat(p, 1.0f);
    }

    // UV array: (u, v, 0, 0) per entry (trailing 0.0f marks array end).
    const float uv[4][2] = {
        { 0.00f, 0.00f },
        { 0.25f, 0.00f },
        { 0.25f, 0.25f },
        { 0.00f, 0.25f },
    };
    uint8_t* q = b.data() + 0x60;
    for (int e = 0; e < 4; ++e) {
        q = putFloat(q, uv[e][0]);
        q = putFloat(q, uv[e][1]);
        q = putFloat(q, 0.0f);
        q = putFloat(q, 0.0f);
    }
    // UV-array terminator (trailing float != 0 stops the UV scan).
    q = putFloat(q, 0.0f);
    q = putFloat(q, 0.0f);
    q = putFloat(q, 1.0f);
    q = putFloat(q, 0.0f);
    assert(static_cast<size_t>(q - b.data()) == kFaceOff);

    // One strip: header [4, 0xFFFF x7], then 4 records.
    uint8_t* h = b.data() + kFaceOff;
    h = putU16(h, 4);
    for (int k = 1; k < 8; ++k) h = putU16(h, 0xFFFF);
    assert(h == b.data() + kFaceOff + 16);

    const uint16_t stripMat = 3; // material f = u16[7], constant across the strip
    uint8_t* r = b.data() + kFaceOff + 16;
    for (uint16_t k = 0; k < 4; ++k) {
        // [1, 0, a=k, s=0, m=k, b=k, u=0, f=stripMat]
        r = putU16(r, 0x0001);
        r = putU16(r, 0x0000);
        r = putU16(r, k);            // u16[2] position index
        r = putU16(r, 0x0000);       // u16[3] stream/spine id
        r = putU16(r, k);            // u16[4] UV index
        r = putU16(r, k);            // u16[5] mirror of a
        r = putU16(r, 0x0000);       // u16[6] per-strip const (non-material)
        r = putU16(r, stripMat);     // u16[7] material index
    }

    putU32(b.data() + 4, static_cast<uint32_t>(b.size() - 16));
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

    // Rev.151 strip topology: one header [4, ffff x7] + 4 records decodes to
    // one strip with spine [0,1,2,3] (N spine verts => N-2 = 2 triangles).
    assert(mesh.strips.size() == 1);
    assert(mesh.strips[0].spine.size() == 4);
    assert(mesh.strips[0].spine[0] == 0);
    assert(mesh.strips[0].spine[1] == 1);
    assert(mesh.strips[0].spine[2] == 2);
    assert(mesh.strips[0].spine[3] == 3);
    assert(mesh.strips[0].material == 3); // u16[7]

    // Per-vertex UVs come straight from the record's m (u16[4]) -- no
    // per-material base-offset heuristic (Rev.151 correction).
    assert(mesh.strips[0].uvs.size() == 8);
    // Exact UV coordinates (uv index e -> uv[e]).
    assert(mesh.strips[0].uvs[0] == 0.00f && mesh.strips[0].uvs[1] == 0.00f);
    assert(mesh.strips[0].uvs[2] == 0.25f && mesh.strips[0].uvs[3] == 0.00f);
    assert(mesh.strips[0].uvs[4] == 0.25f && mesh.strips[0].uvs[5] == 0.25f);
    assert(mesh.strips[0].uvs[6] == 0.00f && mesh.strips[0].uvs[7] == 0.25f);

    // Flat triangle UVs mirror the strip path: tri (0,1,2) -> m 0,1,2;
    // tri (1,2,3) -> m 1,2,3.
    assert(mesh.triMaterials.size() == 2);
    assert(mesh.triMaterials[0] == 3 && mesh.triMaterials[1] == 3);
    assert(mesh.triVertUVs.size() == 12);
    const float e0[12] = { 0.00f,0.00f, 0.25f,0.00f, 0.25f,0.25f,
                           0.25f,0.00f, 0.25f,0.25f, 0.00f,0.25f };
    for (int k = 0; k < 12; ++k) {
        assert(mesh.triVertUVs[k] == e0[k]);
    }

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