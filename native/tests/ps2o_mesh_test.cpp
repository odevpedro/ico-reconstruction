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

static Ps2oMesh makeManualMesh(const std::vector<uint32_t>& tris,
                               std::size_t uvFloats) {
    Ps2oMesh m;
    m.positions = {0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0};
    m.triangles = tris;
    m.triVertUVs.assign(uvFloats, 0.0f);
    for (size_t k = 0; k < uvFloats; ++k) m.triVertUVs[k] = (float)(k + 1);
    m.triMaterials.assign(tris.size() / 3, 1);
    m.vertexCount = 4;
    m.valid = true;
    return m;
}

static void test_fill_strips_guards() {
    // Already has strip topology -> no synthesis.
    Ps2oMesh withStrips = makeManualMesh({0, 1, 2, 1, 2, 3}, 12);
    withStrips.strips.push_back(Ps2oStrip{});
    assert(synthesizeTriangleStrips(withStrips) == 0);

    // Invalid mesh / no triangles.
    Ps2oMesh invalid = makeManualMesh({0, 1, 2}, 6);
    invalid.valid = false;
    assert(synthesizeTriangleStrips(invalid) == 0);
    Ps2oMesh empty = makeManualMesh({}, 0);
    assert(synthesizeTriangleStrips(empty) == 0);

    // Vertex index beyond u16 spine range is refused.
    Ps2oMesh tooBig = makeManualMesh({0, 1, 0xFFFF}, 6);
    assert(synthesizeTriangleStrips(tooBig) == 0);
}

static void test_fill_strips_cascade() {
    // Two triangles sharing the (1,2) edge -> one strip, spine (0,1,2,3).
    Ps2oMesh m = makeManualMesh({0, 1, 2, 1, 2, 3}, 12);
    const std::size_t n = synthesizeTriangleStrips(m);
    assert(n == 1);
    assert(m.strips.size() == 1);
    const Ps2oStrip& st = m.strips[0];
    assert(st.spine.size() == 4);
    assert(st.spine[0] == 0 && st.spine[1] == 1 && st.spine[2] == 2 && st.spine[3] == 3);
    assert(st.material == 1);
    // UVs follow the per-vertex record of each triangle: tri0 (0,1,2) emits
    // triUV values 1..6 (floats 0-5); the forward extension takes the third
    // vertex (local idx 2) of tri1 (1,2,3) = floats 6+4, 6+5 -> values 11,12.
    assert(st.uvs.size() == 8);
    assert(st.uvs[0] == 1.0f);   // tri0 v0 u
    assert(st.uvs[1] == 2.0f);   // tri0 v0 v
    assert(st.uvs[4] == 5.0f);   // tri0 v2 u
    assert(st.uvs[6] == 11.0f);  // tri1 third vertex u
    assert(st.uvs[7] == 12.0f);  // tri1 third vertex v
}

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

    // Rev.153 strip synthesis (front 3 — unified batch).
    test_fill_strips_guards();
    test_fill_strips_cascade();

    std::fprintf(stderr, "ps2o_mesh_test: OK (%u verts, %u tris)\n",
                 mesh.vertexCount, static_cast<unsigned>(mesh.triangles.size() / 3));
    return 0;
}