#include "engine/Ps2oMesh.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <cmath>
#include <limits>

namespace ico::engine {

namespace {

// Vertex position stride in bytes (4 floats: x, y, z, w).
constexpr uint32_t kVertexStrideBytes = 16;
constexpr uint32_t kPositionsOffset = 0x20;

inline uint16_t rd16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

// Per-material UV base offset, discovered in Rev.143 follow-up.
// UV index for the k-th vertex within a strip of material f = k + offset[f].
// Computed automatically by minimizing UV edge spread per material.
void computeMaterialUVOffsets(const std::vector<uint16_t>& materials,
                              const std::vector<float>& uvs,
                              const std::vector<uint32_t>& triangles,
                              const std::vector<uint16_t>& triMaterials,
                              std::vector<float>& triVertUVs) {
    // Group triangles by material to find best offset per material.
    // For each material, sample strips and find the UV base that
    // minimizes the mean edge spread of UV coordinates.
    const int kSampleTris = 50;
    const int kSearchStep = 4;

    // Find unique materials
    int maxMat = 0;
    for (auto m : triMaterials) if (m > maxMat) maxMat = m;
    maxMat++;

    std::vector<int> bestOffset(maxMat + 1, 0);

    for (int f = 0; f <= maxMat; ++f) {
        // Collect triangles with this material
        std::vector<int> matTris;
        for (size_t t = 0; t < triMaterials.size(); ++t) {
            if (triMaterials[t] == (uint16_t)f) matTris.push_back((int)t);
        }
        if (matTris.empty()) continue;

        float bestSpread = std::numeric_limits<float>::max();
        int bestOff = 0;

        for (int off = 0; off < (int)uvs.size() / 2; off += kSearchStep) {
            float totSpread = 0.0f;
            int cnt = 0;
            int sampleLimit = std::min((int)matTris.size(), kSampleTris);
            for (int si = 0; si < sampleLimit; ++si) {
                int t = matTris[si];
                const uint32_t* tri = &triangles[t * 3];
                for (int k = 0; k < 3; ++k) {
                    int idx = off + k;
                    if (idx * 2 + 1 >= (int)uvs.size()) break;
                    // Get UV for this vertex
                    float u0 = uvs[idx * 2];
                    float v0 = uvs[idx * 2 + 1];
                    int next = off + ((k + 1) % 3);
                    if (next * 2 + 1 >= (int)uvs.size()) break;
                    float u1 = uvs[next * 2];
                    float v1 = uvs[next * 2 + 1];
                    float spd = std::hypot(u0 - u1, v0 - v1);
                    totSpread += spd;
                    cnt++;
                }
            }
            if (cnt > 0 && totSpread / cnt < bestSpread) {
                bestSpread = totSpread / cnt;
                bestOff = off;
            }
        }
        bestOffset[f] = bestOff;
    }

    // Now compute triVertUVs for every triangle vertex using the offsets
    triVertUVs.resize(triangles.size() * 2);
    for (size_t t = 0; t < triMaterials.size(); ++t) {
        int f = triMaterials[t];
        int off = (f <= maxMat) ? bestOffset[f] : 0;
        const uint32_t* tri = &triangles[t * 3];
        for (int k = 0; k < 3; ++k) {
            int idx = off + k;
            size_t outIdx = (t * 3 + k) * 2;
            if (idx * 2 + 1 < (int)uvs.size()) {
                triVertUVs[outIdx] = uvs[idx * 2];
                triVertUVs[outIdx + 1] = uvs[idx * 2 + 1];
            } else {
                triVertUVs[outIdx] = 0.0f;
                triVertUVs[outIdx + 1] = 0.0f;
            }
        }
    }
}

} // namespace

bool loadPs2oMesh(const uint8_t* data, size_t size, Ps2oMesh& mesh) {
    if (!data || size < 0x20) return false;
    if (std::memcmp(data, "PS2O", 4) != 0) return false;

    const uint32_t payload = rd16(data + 4) | (static_cast<uint32_t>(rd16(data + 6)) << 16);
    const uint32_t subMeshes = rd16(data + 8) | (static_cast<uint32_t>(rd16(data + 10)) << 16);
    mesh.subMeshCount = subMeshes;

    // Vertex positions: contiguous 16-byte records of (x,y,z,1.0).
    // The array ends at the first record whose w is not 1.0.
    uint32_t v = 0;
    while (kPositionsOffset + (v + 1) * kVertexStrideBytes <= size) {
        const uint8_t* p = data + kPositionsOffset + v * kVertexStrideBytes;
        const uint32_t wBits = rd16(p + 12) | (static_cast<uint32_t>(rd16(p + 14)) << 16);
        if (wBits != 0x3F800000u) break; // 1.0f in IEEE-754
        const size_t base = mesh.positions.size();
        mesh.positions.resize(base + 3);
        std::memcpy(&mesh.positions[base], p, 3 * sizeof(float));
        ++v;
    }
    mesh.vertexCount = v;
    if (v == 0) {
        mesh.valid = false;
        return false;
    }

    const uint32_t nv = mesh.vertexCount;

    // UV array (Rev.142 follow-up, p1 only): immediately after the positions
    // array (which ends at the first non-1.0 w), entries are 4 floats LE
    // (u, v, 0, 0) at 16 bytes each, packed until the shading/color region.
    // We store only the (u,v) pair per entry. Geometry parsing below does NOT
    // depend on this; it is a best-effort read for UV-index validation.
    {
        const uint32_t uvOff = kPositionsOffset + nv * kVertexStrideBytes;
        const uint32_t maxUvCount = 20000; // generous cap for the full p1 range
        mesh.uvs.clear();
        for (uint32_t e = 0; e < maxUvCount; ++e) {
            const size_t o = uvOff + static_cast<size_t>(e) * kVertexStrideBytes;
            if (o + 15 >= size) break;
            const uint32_t w3 = rd16(data + o + 12) |
                                (static_cast<uint32_t>(rd16(data + o + 14)) << 16);
            const uint32_t w4 = rd16(data + o + 8) |
                                (static_cast<uint32_t>(rd16(data + o + 10)) << 16);
            // A UV entry must have the trailing two floats equal to 0.0f.
            if (w3 != 0u || w4 != 0u) break;
            const size_t base = mesh.uvs.size();
            mesh.uvs.resize(base + 2);
            std::memcpy(&mesh.uvs[base], data + o, 2 * sizeof(float));
        }
    }

    // Face data. Record layout is 16 bytes = 8 x u16: [c, t, 0, a, m, b, s, f].
    //   - c == 0xFFFF marks the first record of a primitive (frame head).
    //   - c != 0xFFFF on a continuation record is a sub-stream id (0, 1, 2, ...).
    //   - t is the primitive type; t > 1 is a terminator record ([0, t, 0xFFFF...]).
    //
    // Each frame primitive is a triangle strip whose spine is the u16[3] (`a`)
    // column of consecutive records. Consecutive a values [v0, v1, v2, ...]
    // form triangles (v0,v1,v2)(v1,v2,v3)(...). This matches the validated
    // Rev.142 spine-x decode (room p1: 15,161 tris / max idx 7792). Records
    // whose a and b agree (a == b) are the explicit-mirror form; records with
    // a != b still contribute their a value to the spine. Degenerate
    // (equal-index) inner triangles are dropped.

    size_t i = kPositionsOffset;
    while (i + 15 < size) {
        const uint16_t c0 = rd16(data + i);
        const uint16_t t0 = rd16(data + i + 2);
        if (c0 != 0xFFFF) { i += 2; continue; } // drift to next frame head
        if (t0 > 0x01)    { i += 2; continue; } // false head / terminator while hunting

        const uint16_t a0 = rd16(data + i + 6);
        const uint16_t b0 = rd16(data + i + 10);
        if (a0 >= nv || b0 >= nv) { i += 2; continue; } // false head while hunting

        // Face data = triangle strips. Each strip is a 16-byte record header
        // (see Rev.142): [c, t, 0, a, m, b, s, f] where c == 0xFFFF marks the
        // strip head and c == 0 on continuations. The strip spine is the
        // u16[3] (`a`) column of consecutive records: consecutive a values
        // [v0, v1, v2, ...] form triangles (v0,v1,v2)(v1,v2,v3)(...). Many
        // records mirror the index into u16[5] (`b' == a), but that mirror is
        // not guaranteed across all objects, so we decode from `a` only and
        // strip degenerate (equal-index) triangles.
        size_t j = i + kVertexStrideBytes;
        bool hitTerminator = false;
        std::vector<uint16_t> spine;
        uint16_t stripMat = rd16(data + i + 14); // f: material/partition (u16[7])
        spine.push_back(a0);
        // Continuation records contribute one spine vertex each.
        while (j + 15 < size) {
            const uint16_t c2 = rd16(data + j);
            const uint16_t t2 = rd16(data + j + 2);
            if (c2 == 0xFFFF && j != i) break;  // next frame head
            if (t2 > 0x01) { hitTerminator = true; break; } // terminator
            const uint16_t a2 = rd16(data + j + 6);
            const uint16_t b2 = rd16(data + j + 10);
            if (a2 >= nv || b2 >= nv) break;
            spine.push_back(a2);
            j += kVertexStrideBytes;
        }
        // spine = [v0, v1, v2, ...] : triangles (v0,v1,v2)(v1,v2,v3)(...),
        // dropping degenerate strips.
        for (size_t k = 0; k + 2 < spine.size(); ++k) {
            const uint16_t s0 = spine[k];
            const uint16_t s1 = spine[k + 1];
            const uint16_t s2 = spine[k + 2];
            if (s0 == s1 || s1 == s2) continue; // strip degenerate
            mesh.triangles.push_back(s0);
            mesh.triangles.push_back(s1);
            mesh.triangles.push_back(s2);
            mesh.triMaterials.push_back(stripMat);
        }

        // Advance outer loop past this strip.
        if (hitTerminator) {
            i = j + kVertexStrideBytes; // skip the 16-byte terminator record
        } else {
            i = j; // j is at the next frame head (c == 0xFFFF) or EOB
        }
    }

    // Compute per-triangle-vertex UV coordinates using the
    // material-specific base offset (UV = k + offset[f]).
    computeMaterialUVOffsets(mesh.triMaterials, mesh.uvs,
                             mesh.triangles, mesh.triMaterials,
                             mesh.triVertUVs);

    mesh.valid = true;
    return true;
}

bool loadPs2oMeshFromFile(const char* path, Ps2oMesh& mesh) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    f.seekg(0, std::ios::end);
    const std::streamoff sz = f.tellg();
    f.seekg(0, std::ios::beg);
    if (sz <= 0) return false;
    std::vector<uint8_t> buf(static_cast<size_t>(sz));
    f.read(reinterpret_cast<char*>(buf.data()), sz);
    return loadPs2oMesh(buf.data(), buf.size(), mesh);
}

} // namespace ico::engine