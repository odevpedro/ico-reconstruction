#include "engine/Ps2oMesh.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <cmath>
#include <unordered_map>

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
                    // a terminator (<0x20, normally NUL) in the file. The
                    // prefix must be a real name marker at the START of the
                    // printable run whose preceding file byte is a table
                    // delimiter (NUL, or the 0x80 record padding of the
                    // material-name table, stride 0x90). A `?`/`>` sitting
                    // mid-run (binary float bytes 0x3F/0x3E inside "t?f7k6")
                    // is NOT a name marker.
                    bool pref = false;
                    if (k == 0) {
                        // A bare word is a material name only directly after a
                        // table delimiter (NUL or the 0x80 record padding) or
                        // at file start — never after arbitrary float bytes.
                        const unsigned char pre = (i == 0) ? 0u : data[i - 1];
                        pref = (pre == 0x00 || pre == 0x80);
                    } else if (k == 1 && (run[0] == '?' || run[0] == '>')) {
                        const unsigned char pre = (i == 0) ? 0u : data[i - 1];
                        pref = (pre == 0x00 || pre == 0x80);
                    }
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

// Position of every "OBJH" dispatch/tag record in the file. A single OBJH
// cluster at the very END is the room/static-mesh family (p1/p2: dispatch tags
// after the face region, e.g. p1 at 0x101620/0x103d20/0x1040b0 of 0x1041c0).
// Multiple OBJH scattered THROUGHOUT the file delimit per-bone submeshes of a
// CHARACTER family (.p2c): each OBJH owns a 26-row x 16-B submesh table and is
// preceded by a data region (positions + UVs + material-name table + strips).
std::vector<size_t> findObjs(const uint8_t* data, size_t size) {
    std::vector<size_t> pos;
    static const uint8_t kObjs[4] = { 'O', 'B', 'J', 'H' };
    for (size_t i = 0; i + 4 <= size; ++i)
        if (std::memcmp(data + i, kObjs, 4) == 0) pos.push_back(i);
    return pos;
}

// Returns true when data[at] starts a [a-z0-9A-Z_]{4,40} word that is
// NUL-terminated and contains at least one ASCII letter (so pure bone/float
// suffixes are rejected); lenOut = word length on success.
bool isNameAt(const uint8_t* data, size_t size, size_t at, size_t& lenOut) {
    if (at >= size) return false;
    auto isWord = [](unsigned char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') || c == '_';
    };
    if (!isWord(data[at])) return false;
    size_t j = at, letters = 0;
    while (j < size && isWord(data[j])) {
        if ((data[j] >= 'a' && data[j] <= 'z') || (data[j] >= 'A' && data[j] <= 'Z'))
            ++letters;
        ++j;
    }
    const size_t len = j - at;
    if (len < 4 || len > 40 || letters == 0) return false;
    if (j < size && data[j] != 0) return false;
    lenOut = len;
    return true;
}

// Character-family loader (multi-OBJH .p2c, e.g. boymodel.p2c). Each OBJH
// region is an independent bone-local submesh sharing ONE bind/T-pose space:
//   region k start = (k==0) ? 0x20 : objh[k-1] + 0x10 + 26*16
//   region k end   = objh[k] - 0x10
// Within a region: positions (16-B x,y,z,1.0 until w != 1.0), then UVs
// (16-B u,v,0,0), then per-submesh material-name records (stride 0x90, name at
// record+8, first record header 0xFF 0xFF 0xFF 0x80), then a skinning block,
// then the face region. Faces follow the SAME canonical p1 rule (Rev.151):
// header [N, 0xFFFF x7]; records [flag,0,a,a,m,0xFFFF,0,f] with a = u16[2]
// vertex index, m = u16[4] UV index, f = u16[7] LOCAL material index.
// Vertices are shared: the array holds 2x the used count (used mesh half then
// unused bone rest half), and faces only reference the first half. Concatenated
// output rebases vertex/UV indices and remaps f through the region name tables
// to one deduplicated global list, so the result renders dock to the p1 path.
bool loadCharacterMesh(const uint8_t* data, size_t size, Ps2oMesh& mesh,
                       const std::vector<size_t>& objh) {
    mesh.positions.clear();
    mesh.uvs.clear();
    mesh.triangles.clear();
    mesh.strips.clear();
    mesh.triMaterials.clear();
    mesh.triVertUVs.clear();
    mesh.materialNames.clear();
    mesh.subMeshCount = 0;
    mesh.vertexCount = 0;
    mesh.valid = false;

    std::vector<std::string> globalNames;
    auto globalIndexOf = [&](const std::string& n) -> uint16_t {
        for (size_t i = 0; i < globalNames.size(); ++i)
            if (globalNames[i] == n) return static_cast<uint16_t>(i);
        globalNames.push_back(n);
        return static_cast<uint16_t>(globalNames.size() - 1);
    };

    auto isStripHeader = [&](size_t at, size_t end) -> bool {
        if (at + kVertexStrideBytes > end) return false;
        const uint16_t n0 = rd16(data + at);
        if (n0 < 2 || n0 > 64) return false;
        for (int k = 1; k < 8; ++k)
            if (rd16(data + at + 2 * k) != 0xFFFF) return false;
        return true;
    };

    for (size_t k = 0; k < objh.size(); ++k) {
        const size_t s = (k == 0) ? kPositionsOffset : objh[k - 1] + 0x10 + 26 * 16;
        const size_t e = objh[k] - 0x10;
        if (s >= e || e > size) return false;

        // Positions.
        const size_t baseVert = mesh.positions.size() / 3;
        size_t o = s, v = 0;
        while (o + kVertexStrideBytes <= e) {
            const uint32_t wBits = rd16(data + o + 12) |
                                   (static_cast<uint32_t>(rd16(data + o + 14)) << 16);
            if (wBits != 0x3F800000u) break;
            const size_t b = mesh.positions.size();
            mesh.positions.resize(b + 3);
            std::memcpy(&mesh.positions[b], data + o, 3 * sizeof(float));
            ++v;
            o += kVertexStrideBytes;
        }
        const uint32_t nvLocal = static_cast<uint32_t>(v);
        // Skip empty/auxiliary regions instead of failing the whole mesh.
        // Rev.171: st02a_p1.p2o / st02a_p3.p2o are room-family files that the
        // auto-detect routes to the character loader; among their scattered
        // OBJH submeshes several carry NO position data (pure material-name
        // records such as "47a_block3"/"sabi01", or 2-D footprints). Those
        // regions contribute nothing to the scene and must not abort the load.
        if (nvLocal == 0) continue;

        // UVs.
        const size_t baseUV = mesh.uvs.size() / 2;
        while (o + kVertexStrideBytes <= e) {
            const uint32_t w3 = rd16(data + o + 12) |
                                (static_cast<uint32_t>(rd16(data + o + 14)) << 16);
            const uint32_t w4 = rd16(data + o + 8) |
                                (static_cast<uint32_t>(rd16(data + o + 10)) << 16);
            if (w3 != 0u || w4 != 0u) break;
            const size_t b = mesh.uvs.size();
            mesh.uvs.resize(b + 2);
            std::memcpy(&mesh.uvs[b], data + o, 2 * sizeof(float));
            o += kVertexStrideBytes;
        }

        // Per-submesh material-name table: first record starts with the
        // 0..FF 0..FF 0..FF 0x80 signature and a valid name 8 bytes in;
        // subsequent records follow at stride 0x90 while their +8 slot holds
        // a valid NUL-terminated name. Scan all candidates and keep the first
        // whose stride walk yields at least one name (defensive against a
        // signature-ish byte combo inside the skinning block).
        std::vector<std::string> regionNames;
        for (size_t x = s; x + 16 <= e; ++x) {
            if ((data[x] & 0xF0u) != 0xF0u) continue;
            size_t len = 0;
            if (!isNameAt(data, size, x + 8, len)) continue;
            std::vector<std::string> candidate;
            for (size_t t = x; t + 16 <= e; t += 0x90) {
                size_t l2 = 0;
                if (!isNameAt(data, size, t + 8, l2)) break;
                candidate.emplace_back(
                    reinterpret_cast<const char*>(data + t + 8), l2);
            }
            if (!candidate.empty()) {
                regionNames = std::move(candidate);
                break;
            }
        }
        std::vector<uint16_t> localToGlobal;
        localToGlobal.reserve(regionNames.size());
        for (const auto& nm : regionNames) localToGlobal.push_back(globalIndexOf(nm));

        // Face region: forward-scan past the skinning block to the first
        // [N, 0xFFFF x7] strip header, then parse strips within this region.
        size_t i = (o + kVertexStrideBytes - 1) & ~(size_t)(kVertexStrideBytes - 1);
        while (i + kVertexStrideBytes <= e && !isStripHeader(i, e)) i += kVertexStrideBytes;
        // A region may legitimately lack face data (metadata-only submesh);
        // skip it rather than aborting the whole mesh load.
        if (i + kVertexStrideBytes > e) continue;

        const size_t uvLimit = mesh.uvs.size() / 2;
        while (i + kVertexStrideBytes <= e && isStripHeader(i, e)) {
            const uint16_t n = rd16(data + i);
            const size_t recStart = i + kVertexStrideBytes;
            if (recStart + (size_t)n * kVertexStrideBytes > e) break;
            bool bad = false;
            for (size_t r = 0; r < n; ++r)
                if (rd16(data + recStart + r * kVertexStrideBytes + 4) >= nvLocal) {
                    bad = true; break;
                }
            if (bad) break;

            const uint16_t localF = rd16(data + recStart + 14);
            const uint16_t stripMat =
                (localF < localToGlobal.size()) ? localToGlobal[localF] : 0;

            Ps2oStrip strip;
            strip.material = stripMat;
            strip.spine.reserve(n);
            strip.uvs.reserve(n * 2);
            for (size_t r = 0; r < n; ++r) {
                const size_t ro = recStart + r * kVertexStrideBytes;
                const uint16_t a = rd16(data + ro + 4);
                const uint16_t mm = rd16(data + ro + 8);
                strip.spine.push_back(static_cast<uint16_t>(baseVert + a));
                const size_t gUV = baseUV + mm;
                if (gUV < uvLimit) {
                    strip.uvs.push_back(mesh.uvs[gUV * 2 + 0]);
                    strip.uvs.push_back(mesh.uvs[gUV * 2 + 1]);
                } else {
                    strip.uvs.push_back(0.0f);
                    strip.uvs.push_back(0.0f);
                }
            }
            for (size_t r = 0; r + 2 < n; ++r) {
                const uint16_t s0 = strip.spine[r];
                const uint16_t s1 = strip.spine[r + 1];
                const uint16_t s2 = strip.spine[r + 2];
                if (s0 == s1 || s1 == s2) continue;
                mesh.triangles.push_back(s0);
                mesh.triangles.push_back(s1);
                mesh.triangles.push_back(s2);
                mesh.triMaterials.push_back(stripMat);
                mesh.triVertUVs.reserve(mesh.triVertUVs.size() + 6);
                for (size_t kk = 0; kk < 3; ++kk) {
                    const size_t ro = recStart + (r + kk) * kVertexStrideBytes;
                    const uint16_t mm = rd16(data + ro + 8);
                    const size_t gUV = baseUV + mm;
                    if (gUV < uvLimit) {
                        mesh.triVertUVs.push_back(mesh.uvs[gUV * 2 + 0]);
                        mesh.triVertUVs.push_back(mesh.uvs[gUV * 2 + 1]);
                    } else {
                        mesh.triVertUVs.push_back(0.0f);
                        mesh.triVertUVs.push_back(0.0f);
                    }
                }
            }
            if (strip.spine.size() >= 3) mesh.strips.push_back(std::move(strip));
            i = recStart + (size_t)n * kVertexStrideBytes;
        }
        mesh.subMeshCount = static_cast<uint32_t>(k + 1);
    }

    mesh.vertexCount = static_cast<uint32_t>(mesh.positions.size() / 3);
    mesh.materialNames = std::move(globalNames);
    mesh.valid = !mesh.strips.empty();
    return mesh.valid;
}

} // namespace

bool loadPs2oMesh(const uint8_t* data, size_t size, Ps2oMesh& mesh) {
    if (!data || size < 0x20) return false;
    if (std::memcmp(data, "PS2O", 4) != 0) return false;

    const uint32_t payload = rd16(data + 4) | (static_cast<uint32_t>(rd16(data + 6)) << 16);
    const uint32_t subMeshes = rd16(data + 8) | (static_cast<uint32_t>(rd16(data + 10)) << 16);
    mesh.subMeshCount = subMeshes;

    // Character-family auto-detect: multiple OBJH tags with the FIRST in the
    // first half of the file (delimiting submesh regions throughout), unlike
    // the room family whose OBJH tags cluster at the very end.
    const std::vector<size_t> objh = findObjs(data, size);
    if (objh.size() >= 2 && objh[0] < size / 2)
        return loadCharacterMesh(data, size, mesh, objh);

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

std::size_t synthesizeTriangleStrips(Ps2oMesh& mesh) {
    if (!mesh.valid) return 0;
    if (!mesh.strips.empty()) return 0;

    const uint32_t triCount = static_cast<uint32_t>(mesh.triangles.size() / 3);
    if (triCount == 0) return 0;
    const bool hasUV = mesh.triVertUVs.size() >= static_cast<uint32_t>(triCount) * 6;

    // The spine is u16; refuse meshes whose indices exceed that range.
    uint32_t maxIdx = 0;
    for (size_t t = 0; t < mesh.triangles.size(); ++t)
        if (mesh.triangles[t] > maxIdx) maxIdx = mesh.triangles[t];
    if (maxIdx > 0xFFFEu) return 0;

    // Adjacency: ordered vertex-pair key -> triangle indices.
    auto keyOf = [](uint32_t a, uint32_t b) -> uint64_t {
        const uint32_t lo = a < b ? a : b;
        const uint32_t hi = a < b ? b : a;
        return (static_cast<uint64_t>(hi) << 32) | lo;
    };
    std::unordered_map<uint64_t, std::vector<uint32_t>> edgeTriangles;
    edgeTriangles.reserve(mesh.triangles.size());
    for (uint32_t t = 0; t < triCount; ++t) {
        const uint32_t a = mesh.triangles[t * 3 + 0];
        const uint32_t b = mesh.triangles[t * 3 + 1];
        const uint32_t c = mesh.triangles[t * 3 + 2];
        edgeTriangles[keyOf(a, b)].push_back(t);
        edgeTriangles[keyOf(b, c)].push_back(t);
        edgeTriangles[keyOf(c, a)].push_back(t);
    }

    std::vector<uint8_t> used(static_cast<size_t>(triCount), 0);
    std::vector<Ps2oStrip> result;

    auto appendUV = [&](Ps2oStrip& st, uint32_t tri, uint32_t local) {
        st.uvs.push_back(mesh.triVertUVs[tri * 6 + local * 2 + 0]);
        st.uvs.push_back(mesh.triVertUVs[tri * 6 + local * 2 + 1]);
    };
    auto localIndexOf = [&](uint32_t tri, uint32_t vi) -> uint32_t {
        for (uint32_t k = 0; k < 3; ++k)
            if (mesh.triangles[tri * 3 + k] == vi) return k;
        return 0;
    };

    // Advance the spine by one cascade triangle sharing the (X,Y) edge.
    // Returns the triangle index+1 (0 = none) and the third vertex.
    constexpr uint32_t kInvalid = 0xFFFFFFFFu;
    auto nextEdgeTri = [&](uint32_t x, uint32_t y, uint32_t& third) -> uint32_t {
        if (x == y) return 0;
        const auto it = edgeTriangles.find(keyOf(x, y));
        if (it == edgeTriangles.end()) return 0;
        for (uint32_t t : it->second) {
            if (used[t]) continue;
            const uint32_t a = mesh.triangles[t * 3 + 0];
            const uint32_t b = mesh.triangles[t * 3 + 1];
            const uint32_t c = mesh.triangles[t * 3 + 2];
            bool hasX = false, hasY = false;
            third = kInvalid;
            for (uint32_t v : {a, b, c}) {
                if (v == x) hasX = true;
                else if (v == y) hasY = true;
                else third = v;
            }
            if (hasX && hasY && third != kInvalid) {
                used[t] = 1;
                return t + 1;
            }
        }
        return 0;
    };

    for (uint32_t s = 0; s < triCount; ++s) {
        if (used[s]) continue;
        used[s] = 1;

        const uint32_t a = mesh.triangles[s * 3 + 0];
        const uint32_t b = mesh.triangles[s * 3 + 1];
        const uint32_t c = mesh.triangles[s * 3 + 2];
        if (a == b || b == c || c == a) continue; // degenerate seed

        Ps2oStrip st;
        st.material = (s < mesh.triMaterials.size()) ? mesh.triMaterials[s] : 0;
        st.spine.push_back(static_cast<uint16_t>(a));
        st.spine.push_back(static_cast<uint16_t>(b));
        st.spine.push_back(static_cast<uint16_t>(c));
        if (hasUV) {
            appendUV(st, s, 0);
            appendUV(st, s, 1);
            appendUV(st, s, 2);
        }

        // Forward: extend the (last-1,last) spine edge.
        for (;;) {
            const uint32_t x = st.spine[st.spine.size() - 2];
            const uint32_t y = st.spine[st.spine.size() - 1];
            uint32_t third = kInvalid;
            const uint32_t t1 = nextEdgeTri(x, y, third);
            if (t1 == 0) break;
            st.spine.push_back(static_cast<uint16_t>(third));
            if (hasUV) appendUV(st, t1 - 1, localIndexOf(t1 - 1, third));
        }

        // Backward: prepend a vertex to the (0,1) spine edge.
        for (;;) {
            const uint32_t x = st.spine[0];
            const uint32_t y = st.spine[1];
            uint32_t third = kInvalid;
            const uint32_t t1 = nextEdgeTri(x, y, third);
            if (t1 == 0) break;
            st.spine.insert(st.spine.begin(), static_cast<uint16_t>(third));
            if (hasUV) {
                const uint32_t lo = localIndexOf(t1 - 1, third);
                st.uvs.insert(st.uvs.begin(), mesh.triVertUVs[(t1 - 1) * 6 + lo * 2 + 1]);
                st.uvs.insert(st.uvs.begin(), mesh.triVertUVs[(t1 - 1) * 6 + lo * 2 + 0]);
            }
        }

        if (st.spine.size() >= 3) result.push_back(std::move(st));
    }

    const std::size_t built = result.size();
    if (built != 0) mesh.strips = std::move(result);
    return built;
}

} // namespace ico::engine