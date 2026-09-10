#include "engine/Ps2oMesh.h"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
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

// Builds a two-strip PS2O proving the Rev.156 family finding: the p2 wall
// family uses the exact same canonical rule as p1. The only per-strip
// attribute that varies is u16[0] (0 = long tri-strips a==b, 1 = short
// quads/fans a!=b per Rev.142) -- but u16[0] is NOT a type discriminator;
// both decode through header [N,0xFFFF x7] + records [1,0,a,s,m,b,u,f]
// with m=u16[4] UV and f=u16[7] material (Rev.151 rule).
std::vector<uint8_t> makeTwoStripBox() {
    constexpr uint32_t kPosCount = 6;
    constexpr uint32_t kUvCount = 4;
    constexpr uint32_t kFaceOff = 0x20 + kPosCount * 16 + kUvCount * 16 + 16;
    std::vector<uint8_t> b(kFaceOff + 2 * (16 + 4 * 16) + 16, 0);
    std::memcpy(b.data(), "PS2O", 4);
    putU32(b.data() + 4, 0);
    putU32(b.data() + 8, 1);
    std::memcpy(b.data() + 0x18, "SUM\0", 4);
    putU32(b.data() + 0x1c, 0x01020304);

    uint8_t* p = b.data() + 0x20;
    const float positions[6][3] = {
        { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f }, { 2.0f, 0.0f, 0.0f }, { 2.0f, 1.0f, 0.0f },
    };
    for (int v = 0; v < 6; ++v) {
        p = putFloat(p, positions[v][0]);
        p = putFloat(p, positions[v][1]);
        p = putFloat(p, positions[v][2]);
        p = putFloat(p, 1.0f);
    }
    const float uv[4][2] = {
        { 0.00f, 0.00f }, { 0.25f, 0.00f }, { 0.25f, 0.25f }, { 0.00f, 0.25f },
    };
    uint8_t* q = p;
    for (int e = 0; e < 4; ++e) {
        q = putFloat(q, uv[e][0]);
        q = putFloat(q, uv[e][1]);
        q = putFloat(q, 0.0f);
        q = putFloat(q, 0.0f);
    }
    q = putFloat(q, 0.0f); // UV-array terminator
    q = putFloat(q, 0.0f);
    q = putFloat(q, 1.0f);
    q = putFloat(q, 0.0f);
    assert(static_cast<size_t>(q - b.data()) == kFaceOff);

    // Strip A: u16[0]=0 (type 0x00 long). Strip B: u16[0]=1 (type 0x01 short).
    const uint16_t flags[2] = { 0, 1 };
    const uint16_t mats[2] = { 1, 5 };
    uint8_t* r = b.data() + kFaceOff;
    for (int s = 0; s < 2; ++s) {
        r = putU16(r, 4);
        for (int k = 1; k < 8; ++k) r = putU16(r, 0xFFFF);
        for (uint16_t k = 0; k < 4; ++k) {
            r = putU16(r, flags[s]);       // u16[0] per-strip flag
            r = putU16(r, 0x0000);
            r = putU16(r, k + (uint16_t)(s * 2)); // u16[2] position index
            r = putU16(r, 0x0000);         // u16[3] spine id
            r = putU16(r, k);              // u16[4] UV index
            r = putU16(r, k + (uint16_t)(s * 2)); // u16[5] mirror of a
            r = putU16(r, 0x0000);
            r = putU16(r, mats[s]);        // u16[7] material
        }
    }
    putU32(b.data() + 4, static_cast<uint32_t>(b.size() - 16));
    return b;
}

static void test_two_strip_family_unified() {
    // Both flag 0x00 and flag 0x01 strips decode with the exact same Rev.151
    // canonical rule: header [N, 0xFFFF x7] + N records, one Ps2oStrip with
    // spine of N verts, N-2 cascade triangles (spine size 4 -> 2 triangles).
    // u16[0] is a per-strip attribute (Rev.142 type), NOT a decoder branch.
    std::vector<uint8_t> two = makeTwoStripBox();
    Ps2oMesh mesh;
    assert(loadPs2oMesh(two.data(), two.size(), mesh));
    assert(mesh.strips.size() == 2);

    const Ps2oStrip& stA = mesh.strips[0];
    const Ps2oStrip& stB = mesh.strips[1];
    assert(stA.spine.size() == 4);
    assert(stB.spine.size() == 4);
    // Same triangle topology for both flags.
    assert(stA.spine[0] == 0 && stB.spine[0] == 2);
    assert(stA.spine[1] == 1 && stB.spine[1] == 3);
    assert(stA.spine[3] == 3 && stB.spine[3] == 5);
    // Materials carried from u16[7], distinct per strip.
    assert(stA.material == 1);
    assert(stB.material == 5);
    // Per-vertex UVs from u16[4]=m for both families.
    assert(stA.uvs.size() == 8 && stB.uvs.size() == 8);
    assert(stA.uvs[0] == 0.00f && stA.uvs[7] == 0.25f);
    assert(stB.uvs[0] == 0.00f && stB.uvs[7] == 0.25f);
    // Flat triangles cascade-constructed for both: 2 tris/strip -> 12 indices.
    assert(mesh.triangles.size() == 12);
    assert(mesh.triangles[0] == 0 && mesh.triangles[2] == 2);   // strip A tri0
    assert(mesh.triangles[3] == 1 && mesh.triangles[5] == 3);   // strip A tri1
    assert(mesh.triangles[6] == 2 && mesh.triangles[8] == 4);   // strip B tri0
    assert(mesh.triangles[9] == 3 && mesh.triangles[11] == 5);  // strip B tri1
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

static void test_character_family() {
    // Character-family (.p2c) synthetic fixture: 2 OBJH-delimited submeshes
    // sharing one bind space. Each region carries positions (w==1.0), UVs
    // (u,v,0,0), a stride-0x90 material table (name at record+8, header
    // 0xFF 0xFF 0xFF 0x80) and then strips [N, 0xFFFF x7]. The forward header
    // scan must skip the material table and the skinning gap.
    std::vector<uint8_t> ch(0x570, 0);
    std::memcpy(ch.data(), "PS2O", 4);
    putU32(ch.data() + 8, 2);                 // subMeshCount = 2
    std::memcpy(ch.data() + 0x18, "SUM\0", 4);
    putU32(ch.data() + 0x1c, 0x01020304);

    auto putPositionsUVs = [](uint8_t* p) -> uint8_t* {
        const float pos[4][3] = { {0,0,0},{1,0,0},{1,1,0},{0,1,0} };
        for (int v = 0; v < 4; ++v) {
            p = putFloat(p, pos[v][0]); p = putFloat(p, pos[v][1]);
            p = putFloat(p, pos[v][2]); p = putFloat(p, 1.0f);
        }
        const float uv[4][2] = { {0,0},{0.25f,0},{0.25f,0.25f},{0,0.25f} };
        for (int e = 0; e < 4; ++e) {
            p = putFloat(p, uv[e][0]); p = putFloat(p, uv[e][1]);
            p = putFloat(p, 0.0f);       p = putFloat(p, 0.0f);
        }
        return p;
    };
    auto putMaterialTable = [](uint8_t* base, const char* const* names, int count) {
        for (int n = 0; n < count; ++n) {
            uint8_t* rec = base + (size_t)n * 0x90;
            rec[0] = 0xFF; rec[1] = 0xFF; rec[2] = 0xFF; rec[3] = 0x80;
            std::memcpy(rec + 8, names[n], std::strlen(names[n]));
        }
    };
    auto putStrip = [](uint8_t* f, uint16_t matF) {
        uint8_t* h = f;
        h = putU16(h, 4);
        for (int k = 1; k < 8; ++k) h = putU16(h, 0xFFFF);
        uint8_t* r = f + 16;
        for (uint16_t k = 0; k < 4; ++k) {
            r = putU16(r, 0x0001); r = putU16(r, 0x0000); r = putU16(r, k);
            r = putU16(r, 0x0000); r = putU16(r, k);      r = putU16(r, k);
            r = putU16(r, 0x0000); r = putU16(r, matF);
        }
    };

    // Region 0 @ 0x20..0x190 | table @ 0xA0, strip @ 0x140 -> OBJH0 @ 0x1A0.
    putPositionsUVs(ch.data() + 0x20);
    const char* names0[1] = { "b_mantle" };
    putMaterialTable(ch.data() + 0xA0, names0, 1);
    putStrip(ch.data() + 0x140, 0);
    std::memcpy(ch.data() + 0x1A0, "OBJH", 4);
    // 26 rows x 16 B submesh table @ 0x1B0..0x350 stays zero.

    // Region 1 @ 0x350..0x550 | table @ 0x3D0 (b_mantle shared + eye02),
    // strip @ 0x500 -> OBJH1 @ 0x560. Dedup must keep b_mantle global idx 0.
    putPositionsUVs(ch.data() + 0x350);
    const char* names1[2] = { "b_mantle", "eye02" };
    putMaterialTable(ch.data() + 0x3D0, names1, 2);
    putStrip(ch.data() + 0x500, 1);
    std::memcpy(ch.data() + 0x560, "OBJH", 4);

    putU32(ch.data() + 4, static_cast<uint32_t>(ch.size() - 16));

    Ps2oMesh mesh;
    assert(loadPs2oMesh(ch.data(), ch.size(), mesh));
    assert(mesh.valid);
    assert(mesh.subMeshCount == 2);
    assert(mesh.vertexCount == 8);               // 4 + 4 concatenated
    assert(mesh.positions.size() == 24);
    assert(mesh.uvs.size() == 16);               // 8 UV pairs
    assert(mesh.strips.size() == 2);
    assert(mesh.triangles.size() == 12);         // 4 tris x 3
    assert(mesh.materialNames.size() == 2);
    assert(mesh.materialNames[0] == "b_mantle");
    assert(mesh.materialNames[1] == "eye02");

    const Ps2oStrip& s0 = mesh.strips[0];
    const Ps2oStrip& s1 = mesh.strips[1];
    assert(s0.material == 0);
    assert(s1.material == 1);
    assert(s0.spine.size() == 4 && s1.spine.size() == 4);
    // Global vertex rebase across submeshes.
    assert(s0.spine[0] == 0 && s0.spine[3] == 3);
    assert(s1.spine[0] == 4 && s1.spine[3] == 7);
    // Flat cascade triangles carry the rebased indices.
    assert(mesh.triangles[0] == 0 && mesh.triangles[2] == 2);
    assert(mesh.triangles[3] == 1 && mesh.triangles[5] == 3);
    assert(mesh.triangles[6] == 4 && mesh.triangles[8] == 6);
    assert(mesh.triangles[9] == 5 && mesh.triangles[11] == 7);
    // UVs resolve from the rebased global m.
    assert(s0.uvs[0] == 0.00f && s0.uvs[1] == 0.00f);
    assert(s1.uvs[4] == 0.25f && s1.uvs[5] == 0.25f);
    assert(s1.uvs[6] == 0.00f && s1.uvs[7] == 0.25f);
}

static void test_real_boymodel(const char* const* candidates, int n) {
    const char* path = nullptr;
    for (int i = 0; i < n; ++i) {
        std::ifstream t(candidates[i], std::ios::binary);
        if (t.good()) { path = candidates[i]; break; }
    }
    if (!path) {
        std::fprintf(stderr, "boymodel.p2c not found; skipping real-asset check\n");
        return;
    }
    Ps2oMesh mesh;
    assert(loadPs2oMeshFromFile(path, mesh));
    const unsigned npos = static_cast<unsigned>(mesh.positions.size() / 3);
    const unsigned nuv = static_cast<unsigned>(mesh.uvs.size() / 2);
    const unsigned ntris = static_cast<unsigned>(mesh.triangles.size() / 3);
    std::fprintf(stderr,
                 "boymodel.p2c: %u verts, %u uvs, %u strips, %u submeshes, "
                 "%u tris, %zu mats\n",
                 npos, nuv, static_cast<unsigned>(mesh.strips.size()),
                 static_cast<unsigned>(mesh.subMeshCount), ntris,
                 mesh.materialNames.size());
    assert(mesh.valid);
    assert(mesh.subMeshCount == 5);
    assert(npos == 3210);
    assert(nuv == 1560);
    assert(mesh.strips.size() == 526);
    assert(mesh.materialNames.size() == 15);
    const char* expect[15] = {
        "b_mantle",      "test_small_ref", "eye02",  "b_arm",   "b_shoes",
        "b_suit",        "b_head_top",     "b_pants", "b_face2", "b_lashe2",
        "eye_inner",     "toothU01",       "toothD01", "tape_b", "tape_boro",
    };
    for (int i = 0; i < 15; ++i)
        assert(mesh.materialNames[i] == expect[i]);
    assert(ntris >= 2600 && ntris <= 2725);
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

    // Rev.156 p2 family: flag u16[0] is not a decoder discriminator.
    test_two_strip_family_unified();

    // Rev.161 character family: multi-OBJH submesh decode + global rebase +
    // local->global material remap.
    test_character_family();

    // Real character asset, validated against the p2c byte-level layout.
    const char* boyCandidates[] = {
        "assets/boy/boymodel.p2c",
        "native/assets/boy/boymodel.p2c",
        "../native/assets/boy/boymodel.p2c",
        "../assets/boy/boymodel.p2c",
    };
    test_real_boymodel(boyCandidates, 4);

    std::fprintf(stderr, "ps2o_mesh_test: OK (%u verts, %u tris)\n",
                 static_cast<unsigned>(mesh.vertexCount),
                 static_cast<unsigned>(mesh.triangles.size() / 3));
    return 0;
}