#include "engine/Ps2oMesh.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <cmath>

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

    // Face data. Strip format (byte-exact, validated by Rev.151 metrics):
    //
    //   Each strip is a 16-byte = 8 x u16 header: [N, 0xFFFF x7] where
    //   N = number of record rows that follow this header. N is a literal
    //   record count, not a type code (4636 headers carry 3 records,
    //   2352 carry 4, 142 carry 5, ... ; count x N reproduces the observed
    //   record total exactly).
    //
    //   Records are 16 bytes = 8 x u16 each:
    //     u16[0] = 1 (0 on a minority of strip types)        [flag]
    //     u16[1] = 0
    //     u16[2] = a  vertex position index                  (max 7792 in p1)
    //     u16[3] = s  stream / spine id (const per strip,    (max 6083, 5349
    //              NOT material / NOT ordinal-in-file)        distinct in p1)
    //     u16[4] = m  UV index into the UV array             (max 13069 =
    //              NUV-1: EXACT full coverage 13070/13070)
    //     u16[5] = b  mirror of a (== a on most records)
    //     u16[6] = const per strip (10 distinct values 0..9; 2:2154 & 3:168
    //              coincidentally match the legacy tex=1/tex=2 counts but
    //              cannot be the material index: 10 > 7 names)
    //     u16[7] = f  material index (EXACTLY 7 distinct values 0..6 in p1,
    //              matching the 7 embedded material names)
    //
    //   Each strip of N records emits N-2 triangles in cascade strip order:
    //   records (0,1,2)(1,2,3)...(N-3,N-2,N-1). Vertex positions come from
    //   `a` and UVs directly from `m` (no per-material UV base offset
    //   heuristic: the UV index IS the per-vertex record field). This is the
    //   native counterpart of the PS2 gif_DrawStripF triangle-strip prim.
    //
    //   The face region ends at the first 16-byte row that is not a valid
    //   [N, ffff x7] header (in p1, exactly at 0x101580, right before the
    //   OBJH dispatch tags at 0x101620). The walker starts scanning from
    //   the end of the UV array for the first valid header.

    const size_t uvEnd = kPositionsOffset + (size_t)nv * kVertexStrideBytes
                       + mesh.uvs.size() * 4; // 2 floats per entry
    size_t i = (uvEnd + kVertexStrideBytes - 1) & ~(size_t)(kVertexStrideBytes - 1);
    auto isStripHeader = [&](size_t at) -> bool {
        if (at + kVertexStrideBytes > size) return false;
        const uint16_t n0 = rd16(data + at);
        if (n0 < 2 || n0 > 64) return false;
        for (int k = 1; k < 8; ++k)
            if (rd16(data + at + 2 * k) != 0xFFFF) return false;
        return true;
    };
    while (i + kVertexStrideBytes <= size && !isStripHeader(i)) i += kVertexStrideBytes;
    if (i + kVertexStrideBytes > size) {
        mesh.valid = false;
        return false;
    }

    // Two code paths below: strips consumed as UVB-spine (strip topology kept
    // so a native path can render GL_TRIANGLE_STRIP with N spine verts) and
    // a flat indexed list of cascade triangles.
    while (i + kVertexStrideBytes <= size && isStripHeader(i)) {
        const uint16_t n = rd16(data + i);
        const size_t recStart = i + kVertexStrideBytes;
        if (recStart + (size_t)n * kVertexStrideBytes > size) break;

        const size_t uvLimit = mesh.uvs.size() / 2;
        const uint16_t stripMat = rd16(data + recStart + 14); // u16[7] f

        // Guard: every record of a real strip is a non-header row; bail on
        // anything that looks like a next header or out-of-bounds position.
        bool bad = false;
        for (size_t r = 0; r < n; ++r) {
            const uint16_t a = rd16(data + recStart + r * kVertexStrideBytes + 4);
            if (a >= nv) { bad = true; break; }
        }
        if (bad) break;

        // Keep the strip spine (a) with per-vertex UVs from the same record.
        Ps2oStrip strip;
        strip.material = stripMat;
        strip.spine.reserve(n);
        strip.uvs.reserve(n * 2);
        for (size_t r = 0; r < n; ++r) {
            const size_t o = recStart + r * kVertexStrideBytes;
            const uint16_t a = rd16(data + o + 4);
            const uint16_t mm = rd16(data + o + 8);
            strip.spine.push_back(a);
            if (mm < uvLimit) {
                strip.uvs.push_back(mesh.uvs[mm * 2 + 0]);
                strip.uvs.push_back(mesh.uvs[mm * 2 + 1]);
            } else {
                strip.uvs.push_back(0.0f);
                strip.uvs.push_back(0.0f);
            }
        }

        // Flat cascade triangles: (r0,r1,r2)(r1,r2,r3)... N-2 triangles,
        // dropping degenerate (equal consecutive index) inner triangles.
        for (size_t r = 0; r + 2 < n; ++r) {
            const uint16_t s0 = strip.spine[r];
            const uint16_t s1 = strip.spine[r + 1];
            const uint16_t s2 = strip.spine[r + 2];
            if (s0 == s1 || s1 == s2) continue;
            const size_t base = mesh.triangles.size();
            mesh.triangles.push_back(s0);
            mesh.triangles.push_back(s1);
            mesh.triangles.push_back(s2);
            mesh.triMaterials.push_back(stripMat);
            // Flat UVs mirror the strip path: vertex k of the cascade span
            // (r+k) reads the record's own m (u16[4]).
            mesh.triVertUVs.reserve(mesh.triVertUVs.size() + 6);
            for (size_t k = 0; k < 3; ++k) {
                const size_t o = recStart + (r + k) * kVertexStrideBytes;
                const uint16_t mm = rd16(data + o + 8);
                if (mm < uvLimit) {
                    mesh.triVertUVs.push_back(mesh.uvs[mm * 2 + 0]);
                    mesh.triVertUVs.push_back(mesh.uvs[mm * 2 + 1]);
                } else {
                    mesh.triVertUVs.push_back(0.0f);
                    mesh.triVertUVs.push_back(0.0f);
                }
            }
            (void)base;
        }
        if (strip.spine.size() >= 3) mesh.strips.push_back(std::move(strip));

        i = recStart + (size_t)n * kVertexStrideBytes;
    }

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