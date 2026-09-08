#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ico::engine {

// Decoded PS2O (.p2o) geometry, as reverse-engineered in Rev.142.
//
// Header (little-endian):
//   +0x00  magic "PS2O"
//   +0x04  u32 payload_size - 16
//   +0x08  u32 sub-mesh count
//   +0x18  tag "SUM\0"
//   +0x1c  u32 (flags/checksum, semantics not yet confirmed)
// Vertex positions start at +0x20: 4 floats LE (x, y, z, 1.0), 16 bytes each.
// Face data is stored as 16-byte records [c, t, 0, a, m, b, s, f] framed by
// runs of 0xFFFF (c == 0xFFFF on the first record of a primitive, 0 on
// continuations; t > 1 is a terminator record). The strip spine is the u16[3]
// (`a`) column of consecutive records: consecutive a values [s0,s1,...]
// form triangles (s0,s1,s2)(s1,s2,s3)(...), dropping degenerate (equal-index)
// inner triangles. This matches the validated Rev.142/Rev.143 spine-x decode
// (room p1: 15,161 tris, max index 7792).
//
// NOTE: the p2 wall family (Rev.142) uses a different rule (type 0x00 long
// tri-strips a==b vs type 0x01 short quads/fans a!=b), so the p1 spine-a rule
// over-decodes p2. A per-file family discriminator is still to be resolved.

// One p2o frame primitive decoded as a triangle strip whose spine is the
// u16[3] (`a`) column of consecutive 16-byte records. Consecutive spine
// values [v0, v1, v2, ...] form triangles (v0,v1,v2)(v1,v2,v3)(...). With
// a GL_TRIANGLE_STRIP path, N spine vertices render N-2 triangles with no
// indexed duplication — the native counterpart of PS2 gif_DrawStripF
// (GIF prim 0xD), where adjacent triangles share an edge by construction.
struct Ps2oStrip {
    // Spine vertex indices in draw order. Degenerate (equal-index) inner
    // pairs are KEPT so the strip renders exactly what the record list says
    // (GL skips zero-area tris); the flat `triangles` list below drops them.
    std::vector<uint16_t> spine;
    // Material/partition index f (u16[7]) of the strip (constant per strip).
    uint16_t material = 0;
    // Per-spine-vertex UV coordinates (2 floats per spine vertex, same order
    // as spine). Computed as UVarray[k + offset[material]] with k the position
    // within the strip (Rev.143 "UV = k + offset[f]"). Empty when the file
    // carries no UV array, or when the strip has no vertices.
    std::vector<float> uvs;
};

struct Ps2oMesh {
    // One entry per vertex: {x, y, z}.
    std::vector<float> positions;
    // Texture coordinates (Raw, (u,v) pairs) extracted from the p2o UV array.
    // Populated independently of the geometry parse at the confirmed p1 UV
    // region (Rev.142 follow-up). Empty when the file's layout diverges.
    // Used ONLY for UV-index validation (--uv-test), never for geometry.
    std::vector<float> uvs;
    // Triangle indices (strips), 3 per triangle.
    std::vector<uint32_t> triangles;
    // Preserved strip topology (Rev.147): one entry per p2o frame primitive,
    // spine in record order. Lets a native batch render N-2 triangles from N
    // spine verts (GL_TRIANGLE_STRIP) instead of the [A,B,C,C] quad-group
    // flattening that doubles the GPU triangle count. Empty when the file
    // carries no strip faces (fall back to flat `triangles`).
    std::vector<Ps2oStrip> strips;
    // Material/partition index (f = u16[7]) per triangle, 1:1 with triangles
    // (one entry per emitted triangle; reliable for p1 where f is constant
    // within each strip). Empty if not available.
    std::vector<uint16_t> triMaterials;
    // Pre-computed UV coordinates per triangle vertex (2 floats each).
    // Computed as UVarray[k + offset[material]] for the k-th vertex within
    // each strip, using the material-specific base offset discovered in
    // Rev.143 follow-up (UV = k + offset[f]).
    std::vector<float> triVertUVs;
    // Material-specific UV base offsets into the UV array (index == material f,
    // value == offset[f]). Discovered heuristically in Rev.143 (best offset per
    // material minimizing UV edge spread). Used to build per-strip UVs and to
    // key native strip batches. Empty when no UV array was found.
    std::vector<int> materialUVBase;
    // Secondary face records (kept raw for p2 quad/fan semantics, not yet
    // populated by the spine-a decode).
    std::vector<uint16_t> quadRecords;
    // Ordered material texture names, one per material index `f`. Extracted
    // from the embedded material-name table (`name\0...\texture\<name>\0`
    // records whose last path component names the TM2 file). Empty when the
    // file carries no path-style material strings (e.g. 0str structures).
    // Index access: texture for `f` = materialNames[f] when f < size.
    std::vector<std::string> materialNames;
    uint32_t subMeshCount = 0;
    uint32_t vertexCount = 0;
    bool valid = false;
};

// Parses a PS2O file. Returns true and fills `mesh` when the header and a
// contiguous vertex position array (w == 1.0) are present.
bool loadPs2oMesh(const uint8_t* data, size_t size, Ps2oMesh& mesh);

// Convenience wrapper reading a whole file into memory.
bool loadPs2oMeshFromFile(const char* path, Ps2oMesh& mesh);

} // namespace ico::engine