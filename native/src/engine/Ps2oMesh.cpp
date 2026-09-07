#include "engine/Ps2oMesh.h"

#include <cctype>
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

// Extracts the ordered material texture names embedded in a PS2O file.
// The material-name table is a series of records (stride 0x90 in p1) of the
// form "<prefix><name>\0[\...path...]" where <prefix> is `?` (0x3F) or `>`
// (0x3E), or the name may sit on a NUL boundary (subsequent torch records).
// Two variants occur:
//   1) path-style  — "...\texture\name\0"  (p1/p2/brdg/door/windows)
//   2) name-only   — "?name\0<zeros>"      (torches)
// Unified rule (validated against the byte-level tables on all 9 st00a
// pieces): for each printable-ASCII run, take the trailing word
// [a-z0-9_]{4,40} containing at least one letter (digits may lead), and
// accept it iff (a) the run contains a backslash (path-style tail), or
// (b) the word is immediately preceded by `?`/`>` inside the run, or (c) the
// word starts the run on a NUL boundary of the file (name-only torch
// records, where the *preceding* byte is NUL). Words that only match the
// suffix pattern but sit inside binary floats (preceding byte != NUL) are
// rejected. Dedup preserving file order == material index f order.
void extractMaterialNames(const uint8_t* data, size_t size,
                          std::vector<std::string>& out) {
    out.clear();
    std::vector<std::string> names;
    size_t i = 0;
    while (i < size) {
        if (data[i] < 0x20 || data[i] > 0x7e) { ++i; continue; }
        size_t j = i;
        while (j < size && data[j] >= 0x20 && data[j] <= 0x7e) ++j;
        std::string run(reinterpret_cast<const char*>(data + i), j - i);
        std::string last;
        bool accept = false;
        if (run.find('\\') != std::string::npos) {
            // Path-style: the component after the last backslash names texture.
            last = run.substr(run.rfind('\\') + 1);
            accept = true;
        } else {
            // Name-only: find a [?|>]word or word on a NUL-preceded boundary.
            const size_t rl = run.size();
            size_t k = 0;
            while (k < rl) {
                const bool letter = (run[k] >= 'a' && run[k] <= 'z');
                const bool digit = (run[k] >= '0' && run[k] <= '9');
                if (!letter && !digit) { ++k; continue; }
                size_t k2 = k + 1;
                while (k2 < rl) {
                    const char c2 = run[k2];
                    const bool od = (c2 >= '0' && c2 <= '9');
                    const bool ou = (c2 == '_');
                    const bool ol = (c2 >= 'a' && c2 <= 'z');
                    if (!(od || ou || ol)) break;
                    ++k2;
                }
                if (k2 == rl) {
                    // Word runs to the end of the printable run => followed by
                    // a terminator (<0x20, normally NUL) in the file.
                    const bool pref = (k == 0) ? (i == 0 || data[i - 1] == 0x00)
                                               : (run[k - 1] == '?' || run[k - 1] == '>');
                    if (pref) { last = run.substr(k); accept = true; }
                }
                k = k2;
            }
        }
        if (accept) {
            bool ok = last.size() >= 4 && last.size() <= 40;
            bool hasLetter = false;
            for (char c : last) {
                const bool d = (c >= '0' && c <= '9');
                const bool u = (c == '_');
                const bool l = (c >= 'a' && c <= 'z');
                if (!(d || u || l)) { ok = false; break; }
                if (l) hasLetter = true;
            }
            if (ok && hasLetter) {
                // Reject truncated aliases from the OBJH submesh dispatch
                // table: a candidate that is a suffix (>=4 chars) of an
                // already-accepted material name is a copy/alias fragment
                // (e.g. "uchi2" tail of "wall_fuchi2" in p1's OBJH region).
                bool isAlias = false;
                for (const auto& n : names) {
                    if (n.size() >= 4 && last.size() >= 4 &&
                        n.size() > last.size() &&
                        n.compare(n.size() - last.size(), last.size(), last) == 0) {
                        isAlias = true;
                        break;
                    }
                }
                if (!isAlias) {
                    bool dup = false;
                    for (const auto& n : names) if (n == last) { dup = true; break; }
                    if (!dup) names.push_back(last);
                }
            }
        }
        i = j;
    }
    out = std::move(names);
}

// Per-material UV base offset, discovered in Rev.143 follow-up.
// The UV array has dense interleaving (not grouped by material
// in blocks). UV index = base + k where base is the material-specific
// base offset and k is the triangle vertex index (0..2).
// The base offsets are found by sampling and minimizing UV edge
// spread per material group.
void computeMaterialUVOffsets(const std::vector<uint16_t>& materials,
                              const std::vector<float>& uvs,
                              const std::vector<uint32_t>& triangles,
                              const std::vector<uint16_t>& triMaterials,
                              std::vector<float>& triVertUVs) {
    // Find unique materials
    int maxMat = 0;
    for (auto m : triMaterials) if (m > maxMat) maxMat = m;
    maxMat++;

    std::vector<int> bestOffset(maxMat, 0);

    // For each material, find the best UV base offset by minimizing
    // mean edge spread of sampled triangles.
    for (int f = 0; f < maxMat; ++f) {
        // Collect triangle indices with this material
        std::vector<int> matTris;
        for (size_t t = 0; t < triMaterials.size(); ++t) {
            if (triMaterials[t] == (uint16_t)f) matTris.push_back((int)t);
        }
        if (matTris.empty()) continue;

        float bestSpread = std::numeric_limits<float>::max();
        int bestOff = 0;

        // Search all possible offsets
        int maxOff = std::min((int)uvs.size() / 2 - 3, 20000);
        for (int off = 0; off < maxOff; ++off) {
            float totSpread = 0.0f;
            int cnt = 0;
            // Sample up to 200 triangles for this material
            int sampleLimit = std::min((int)matTris.size(), 200);
            for (int si = 0; si < sampleLimit; ++si) {
                int t = matTris[si];
                const uint32_t* tri = &triangles[t * 3];
                float spd[3];
                for (int k = 0; k < 3; ++k) {
                    int idx = off + k;
                    if (idx * 2 + 1 >= (int)uvs.size()) { spd[k] = 0; continue; }
                    float u0 = uvs[idx * 2];
                    float v0 = uvs[idx * 2 + 1];
                    int next = off + ((k + 1) % 3);
                    if (next * 2 + 1 >= (int)uvs.size()) { spd[k] = 0; continue; }
                    float u1 = uvs[next * 2];
                    float v1 = uvs[next * 2 + 1];
                    spd[k] = std::hypot(u0 - u1, v0 - v1);
                    totSpread += spd[k];
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

    // Build triVertUVs using the best offsets per material
    triVertUVs.resize(triangles.size() * 2);
    for (size_t t = 0; t < triMaterials.size(); ++t) {
        int f = triMaterials[t];
        int off = (f < maxMat) ? bestOffset[f] : 0;
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

    extractMaterialNames(data, size, mesh.materialNames);

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