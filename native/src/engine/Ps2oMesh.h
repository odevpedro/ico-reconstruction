#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ico::engine {

// Decoded PS2O (.p2o) geometry, as reverse-engineered in Rev.142 and refined
// in Rev.151 (byte-level face/UV validation).
//
// Header (little-endian):
//   +0x00  magic "PS2O"
//   +0x04  u32 payload_size - 16
//   +0x08  u32 sub-mesh count
//   +0x18  tag "SUM\0"
//   +0x1c  u32 (flags/checksum, semantics not yet confirmed)
// Vertex positions start at +0x20: 4 floats LE (x, y, z, 1.0), 16 bytes each.
// Face data is stored as 16-byte = 8 x u16 records. A strip is a header row
// [N, 0xFFFF x7] where N = number of record rows that follow; each record row
// is [1, 0, a, s, m, b, u, f] with:
//   a = u16[2] vertex position index
//   s = u16[3] stream/spine id (const per strip; NOT material)
//   m = u16[4] UV index into the UV array (EXACT full coverage of NUV in p1:
//             max == NUV-1, so this column IS the per-vertex UV index)
//   f = u16[7] material index (EXACTLY 7 distinct values 0..6 in p1, matching
//             the 7 embedded material names)
// Each strip of N records emits N-2 triangles in cascade strip order
// (0,1,2)(1,2,3)...(N-3,N-2,N-1); vertex positions from `a`, UVs directly
// from `m`. Validated: 7877 strips / 15007 tris / mean UV edge spread 0.1645
// in the room piece, and count x N reproduces the record total exactly for
// every header value (4636x3, 2352x4, 142x5, ...).
//
// NOTE: the p2 wall family (Rev.142) uses a different rule (type 0x00 long
// tri-strips a==b vs type 0x01 short quads/fans a!=b), so the p1 cascade-a
// rule over-decodes p2. A per-file family discriminator is still to be
// resolved.

// One p2o frame primitive decoded as a triangle strip whose N records emit
// N-2 cascade triangles. Consecutive spine positions [v0, v1, v2, ...] form
// triangles (v0,v1,v2)(v1,v2,v3)(...). With a GL_TRIANGLE_STRIP path, N spine
// vertices render N-2 triangles with no indexed duplication — the native
// counterpart of PS2 gif_DrawStripF (GIF prim 0xD), where adjacent triangles
// share an edge by construction.
struct Ps2oStrip {
    // Spine vertex indices in draw order. Degenerate (equal-index) inner
    // pairs are KEPT so the strip renders exactly what the record list says
    // (GL skips zero-area tris); the flat `triangles` list below drops them.
    std::vector<uint16_t> spine;
    // Material/partition index f (u16[7]) of the strip (constant per strip;
    // validated: exactly 7 distinct values 0..6 in the room piece).
    uint16_t material = 0;
    // Per-spine-vertex UV coordinates (2 floats per spine vertex, same order
    // as spine) from the record's own UV index m (u16[4]). Out-of-range UV
    // indices default to (0,0). Empty when the file carries no UV array.
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
    // Computed directly from each triangle's own record UV index m (u16[4]),
    // in cascade strip order — no per-material UV base offset heuristic (the
    // old "UV = k + offset[f]" model was replaced in Rev.151 after byte-level
    // validation showed u16[4] independently covers all NUV UV entries).
    std::vector<float> triVertUVs;
    // Secondary face records (kept raw for p2 quad/fan semantics, not yet
    // populated by the cascade-a decode).
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

/* Greedy triangle→strip stripification (Rev.153, front 3 — unified batch).
   Fills mesh.strips from the flat `triangles`/`triVertUVs`/`triMaterials`
   lists when the file parsed no strip topology (e.g. the 0str structures and
   p2 wall family). Each emitted strip renders N-2 triangles from N spine verts
   via glMultiDrawArrays(GL_TRIANGLE_STRIP), so no [A,B,C,C] indexed
   duplication remains for strip-less pieces. Consecutive spine triangles share
   an edge by construction (greedy adjacency walk). Returns the number of
   strips built; 0 when the mesh already has strips, is invalid, has no
   triangles, or has a vertex index that does not fit the u16 spine. */
std::size_t synthesizeTriangleStrips(Ps2oMesh& mesh);

} // namespace ico::engine