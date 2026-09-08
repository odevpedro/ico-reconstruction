#include "engine/ClipBridge.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ico::engine {

namespace {

constexpr float kFloorNormalY = 0.5f;   // |ny| > this  -> floor sample
constexpr float kWallNormalY = 0.25f;   // |ny| <= this -> wall sample (near-vertical only)
constexpr float kEpsilon = 1e-4f;

// Barycentric point-in-triangle test projected onto the XZ plane.
bool pointInTriXZ(float px, float pz,
                  float x0, float z0, float x1, float z1, float x2, float z2) {
    const float d0x = x1 - x0, d0z = z1 - z0;
    const float d1x = x2 - x0, d1z = z2 - z0;
    const float dpx = px - x0, dpz = pz - z0;
    const float denom = d0x * d1z - d1x * d0z;
    if (std::fabs(denom) < kEpsilon) return false;
    const float inv = 1.0f / denom;
    const float v = (dpx * d1z - d1x * dpz) * inv;
    const float w = (d0x * dpz - dpx * d0z) * inv;
    const float u = 1.0f - v - w;
    const float eps = 1e-3f;
    return u >= -eps && v >= -eps && w >= -eps;
}

}  // namespace

ClipBridge::~ClipBridge() = default;

bool ClipBridge::buildFromMesh(const float* positions, u32 vertexCount,
                               const uint32_t* triangles, u32 triangleCount) {
    shutdown();
    if (!positions || vertexCount == 0 || !triangles || triangleCount == 0) {
        return false;
    }

    // Tight XZ bounds over every referenced triangle vertex.
    float xs = std::numeric_limits<float>::max();
    float xe = -std::numeric_limits<float>::max();
    float zs = std::numeric_limits<float>::max();
    float ze = -std::numeric_limits<float>::max();
    for (u32 t = 0; t < triangleCount; ++t) {
        const u32 i0 = triangles[t * 3 + 0];
        const u32 i1 = triangles[t * 3 + 1];
        const u32 i2 = triangles[t * 3 + 2];
        if (i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount) {
            continue;
        }
        for (const u32 i : {i0, i1, i2}) {
            const float px = positions[i * 3 + 0];
            const float pz = positions[i * 3 + 2];
            xs = std::min(xs, px);
            xe = std::max(xe, px);
            zs = std::min(zs, pz);
            ze = std::max(ze, pz);
        }
    }
    if (!(xe > xs) || !(ze > zs)) {
        return false;
    }
    minX_ = xs;
    maxX_ = xe;
    minZ_ = zs;
    maxZ_ = ze;

    // Tune cell size so the room resolves into a bounded grid. Room span is
    // ~1700x2000 world units; 50-unit cells give ~34x40 = ~1400 cells.
    const float kDefaultCell = 50.0f;
    cellSize_ = kDefaultCell;
    gridW = static_cast<u32>(std::ceil((maxX_ - minX_) / cellSize_)) + 1;
    gridH = static_cast<u32>(std::ceil((maxZ_ - minZ_) / cellSize_)) + 1;
    heightCells.assign(static_cast<std::size_t>(gridW) * gridH,
                       HeightCell{std::numeric_limits<float>::lowest(), 0});
    blockedCells.assign(static_cast<std::size_t>(gridW) * gridH,
                        BlockedCell{false});
    blockedCount = 0;

    for (u32 t = 0; t < triangleCount; ++t) {
        const u32 i0 = triangles[t * 3 + 0];
        const u32 i1 = triangles[t * 3 + 1];
        const u32 i2 = triangles[t * 3 + 2];
        if (i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount) {
            continue;
        }
        const float x0 = positions[i0 * 3 + 0];
        const float y0 = positions[i0 * 3 + 1];
        const float z0 = positions[i0 * 3 + 2];
        const float x1 = positions[i1 * 3 + 0];
        const float y1 = positions[i1 * 3 + 1];
        const float z1 = positions[i1 * 3 + 2];
        const float x2 = positions[i2 * 3 + 0];
        const float y2 = positions[i2 * 3 + 1];
        const float z2 = positions[i2 * 3 + 2];
        rasterizeTriangle(x0, y0, z0, x1, y1, z1, x2, y2, z2);
    }

    // Second pass: walls. Every wall triangle is rasterized twice so cells
    // whose walk surface is far below the wall's base (sub-floor structure)
    // are not blocked. Reuse rasterizeTriangle: it only blocks cells, so
    // calling it again with the same data is harmless for floors (max
    // keeps monotonic).
    // (Floors and walls were already classified in pass one; the height
    // grid is complete here, so wall blocking uses the walk band directly.)
    for (u32 t = 0; t < triangleCount; ++t) {
        const u32 i0 = triangles[t * 3 + 0];
        const u32 i1 = triangles[t * 3 + 1];
        const u32 i2 = triangles[t * 3 + 2];
        if (i0 >= vertexCount || i1 >= vertexCount || i2 >= vertexCount) {
            continue;
        }
        const float x0 = positions[i0 * 3 + 0];
        const float y0 = positions[i0 * 3 + 1];
        const float z0 = positions[i0 * 3 + 2];
        const float x1 = positions[i1 * 3 + 0];
        const float y1 = positions[i1 * 3 + 1];
        const float z1 = positions[i1 * 3 + 2];
        const float x2 = positions[i2 * 3 + 0];
        const float y2 = positions[i2 * 3 + 1];
        const float z2 = positions[i2 * 3 + 2];
        rasterizeWall(x0, y0, z0, x1, y1, z1, x2, y2, z2);
    }

    initialized = true;
    return true;
}

void ClipBridge::shutdown() {
    heightCells.clear();
    blockedCells.clear();
    heightCells.shrink_to_fit();
    blockedCells.shrink_to_fit();
    gridW = gridH = 0;
    blockedCount = 0;
    cellSize_ = 0.0f;
    minX_ = maxX_ = minZ_ = maxZ_ = 0.0f;
    initialized = false;
}

int ClipBridge::cellIndexOf(float x, float z) const {
    const int cx = static_cast<int>(std::floor((x - minX_) / cellSize_));
    const int cz = static_cast<int>(std::floor((z - minZ_) / cellSize_));
    if (cx < 0 || cz < 0 || static_cast<u32>(cx) >= gridW ||
        static_cast<u32>(cz) >= gridH) {
        return -1;
    }
    return cz * static_cast<int>(gridW) + cx;
}

bool ClipBridge::cellInBounds(float x, float z) const {
    return cellIndexOf(x, z) >= 0;
}

void ClipBridge::rasterizeTriangle(float x0, float y0, float z0,
                                   float x1, float y1, float z1,
                                   float x2, float y2, float z2) {
    // Normal via cross product; determines floor classification.
    const float ux = x1 - x0, uy = y1 - y0, uz = z1 - z0;
    const float vx = x2 - x0, vy = y2 - y0, vz = z2 - z0;
    const float nx = uy * vz - uz * vy;
    const float ny = uz * vx - ux * vz;
    const float nz = ux * vy - uy * vx;
    const float norm = std::sqrt(nx * nx + ny * ny + nz * nz);
    if (norm < kEpsilon) {
        return;  // degenerate
    }
    const float nyUnit = ny / norm;

    if (nyUnit <= kFloorNormalY) {
        return;  // not an up-facing walk surface; handled by rasterizeWall
    }

    // Cell AAB box of the triangle's XZ projection.
    const float tx0 = std::min({x0, x1, x2});
    const float tx1 = std::max({x0, x1, x2});
    const float tz0 = std::min({z0, z1, z2});
    const float tz1 = std::max({z0, z1, z2});

    const int c0x = std::max(0, static_cast<int>(std::floor((tx0 - minX_) / cellSize_)));
    const int c1x = std::min(static_cast<int>(gridW) - 1,
                             static_cast<int>(std::floor((tx1 - minX_) / cellSize_)));
    const int c0z = std::max(0, static_cast<int>(std::floor((tz0 - minZ_) / cellSize_)));
    const int c1z = std::min(static_cast<int>(gridH) - 1,
                             static_cast<int>(std::floor((tz1 - minZ_) / cellSize_)));

    // Plane coefficients for height lookup on up-facing floors.
    // n . p = d  ->  y = (d - nx*x - nz*z) / ny
    const float d = nx * x0 + ny * y0 + nz * z0;

    // Sample points per cell for floor rasterization. A real room's floor is
    // many triangles sharing edges; a single center probe misses most cells,
    // so probe a coarse sub-grid and keep the highest plane height seen.
    constexpr int kFloorSamples = 7;  // 7x7 sub-samples per cell

    for (int cz = c0z; cz <= c1z; ++cz) {
        for (int cx = c0x; cx <= c1x; ++cx) {
            const std::size_t cell =
                static_cast<std::size_t>(cz) * gridW + static_cast<std::size_t>(cx);
            bool hit = false;
            float best = std::numeric_limits<float>::lowest();
            const float xBase = minX_ + static_cast<float>(cx) * cellSize_;
            const float zBase = minZ_ + static_cast<float>(cz) * cellSize_;
            for (int sx = 0; sx < kFloorSamples; ++sx) {
                for (int sz = 0; sz < kFloorSamples; ++sz) {
                    const float px = xBase + (static_cast<float>(sx) + 0.5f) *
                                                cellSize_ / kFloorSamples;
                    const float pz = zBase + (static_cast<float>(sz) + 0.5f) *
                                                cellSize_ / kFloorSamples;
                    if (!pointInTriXZ(px, pz, x0, z0, x1, z1, x2, z2)) {
                        continue;
                    }
                    if (std::fabs(ny) > kEpsilon) {
                        const float yAt = (d - nx * px - nz * pz) / ny;
                        best = std::max(best, yAt);
                        hit = true;
                    }
                }
            }
            if (hit) {
                HeightCell& hc = heightCells[cell];
                hc.samples = static_cast<u16>(hc.samples + 1);
                hc.height = std::max(hc.height, best);
            }
        }
    }
}

void ClipBridge::rasterizeWall(float x0, float y0, float z0,
                               float x1, float y1, float z1,
                               float x2, float y2, float z2) {
    // Normal via cross product; determines wall classification.
    const float ux = x1 - x0, uy = y1 - y0, uz = z1 - z0;
    const float vx = x2 - x0, vy = y2 - y0, vz = z2 - z0;
    const float nx = uy * vz - uz * vy;
    const float ny = uz * vx - ux * vz;
    const float nz = ux * vy - uy * vx;
    const float norm = std::sqrt(nx * nx + ny * ny + nz * nz);
    if (norm < kEpsilon) {
        return;  // degenerate
    }
    const float nyUnit = ny / norm;
    const bool wall = std::fabs(nyUnit) <= kWallNormalY;
    if (!wall) {
        return;  // floors (up) and ceiling/undersides (down) never block
    }

    // Cell AAB box of the triangle's XZ projection.
    const float tx0 = std::min({x0, x1, x2});
    const float tx1 = std::max({x0, x1, x2});
    const float tz0 = std::min({z0, z1, z2});
    const float tz1 = std::max({z0, z1, z2});

    // Vertical band a character occupies walking on this level: from just
    // below the walk surface up to standing height. A wall only blocks a
    // cell when it actually crosses that band; vertical panels whose base
    // sits far under the floor (sub-floor structure) or whose top barely
    // skims it (friezes) are ignored.
    const float tyMin = std::min({y0, y1, y2});
    const float tyMax = std::max({y0, y1, y2});
    // A wall that pokes 1 cell above the floor is still a wall the character
    // must not walk inside; one that starts half a cell below the floor is
    // supporting slab. Use generous margins; the floor height below decides.
    const float kStandHeight = 220.0f;   // generous stand/capsule height
    const float kSlabMargin = 15.0f;     // ignore walls starting >15 below floor

    const int c0x = std::max(0, static_cast<int>(std::floor((tx0 - minX_) / cellSize_)));
    const int c1x = std::min(static_cast<int>(gridW) - 1,
                             static_cast<int>(std::floor((tx1 - minX_) / cellSize_)));
    const int c0z = std::max(0, static_cast<int>(std::floor((tz0 - minZ_) / cellSize_)));
    const int c1z = std::min(static_cast<int>(gridH) - 1,
                             static_cast<int>(std::floor((tz1 - minZ_) / cellSize_)));

    for (int cz = c0z; cz <= c1z; ++cz) {
        for (int cx = c0x; cx <= c1x; ++cx) {
            const std::size_t cell =
                static_cast<std::size_t>(cz) * gridW + static_cast<std::size_t>(cx);
            // Walls block only when the triangle actually passes through the
            // cell's inner core (not merely grazes a corner of its AABB).
            const float cellX = minX_ + static_cast<float>(cx) * cellSize_;
            const float cellZ = minZ_ + static_cast<float>(cz) * cellSize_;
            const float core0x = cellX + cellSize_ * 0.2f;
            const float core1x = cellX + cellSize_ * 0.8f;
            const float core0z = cellZ + cellSize_ * 0.2f;
            const float core1z = cellZ + cellSize_ * 0.8f;
            if (tx1 < core0x || tx0 > core1x || tz1 < core0z || tz0 > core1z) {
                continue;  // only grazing the corner: not a block
            }
            const HeightCell& hc = heightCells[cell];
            if (hc.samples > 0) {
                // Walkable cell: the wall must reach near the walk surface.
                const float floorY = hc.height;
                if (tyMax < floorY - kSlabMargin) {
                    continue;  // wall entirely below the floor: sub-floor slab
                }
                if (tyMin > floorY + kStandHeight) {
                    continue;  // wall hangs above stand height: decoration
                }
            }
            BlockedCell& bc = blockedCells[cell];
            if (!bc.blocked) {
                bc.blocked = true;
                ++blockedCount;
            }
        }
    }
}

bool ClipBridge::floorHeightAt(float x, float z, float& outY) const {
    if (!initialized) {
        return false;
    }
    const int ci = cellIndexOf(x, z);
    if (ci < 0) {
        outY = std::numeric_limits<float>::lowest();
        return false;
    }
    const std::size_t idx = static_cast<std::size_t>(ci);
    const HeightCell& hc = heightCells[idx];
    if (hc.samples == 0) {
        outY = std::numeric_limits<float>::lowest();
        return false;
    }
    outY = hc.height;
    if (wallBlocking_ && blockedCells[idx].blocked) {
        return false;
    }
    return true;
}

bool ClipBridge::move(float& x, float& y, float& z, float dx, float dz,
                      float halfExtent, float stepHeight) const {
    if (!initialized || (dx == 0.0f && dz == 0.0f)) {
        return false;
    }
    if (stepHeight < 0.0f) {
        stepHeight = 0.0f;
    }

    // Footprint sample points: the box corners plus the center. A step is
    // only valid when every sample rests on a walkable surface not higher
    // than stepHeight above the current ground (prevents ledge/step snaps).
    auto standable = [&](float cx, float cz) -> bool {
        const float ext = std::max(halfExtent, 0.0f);
        const float corners[5][2] = {
            {cx, cz},
            {cx - ext, cz - ext},
            {cx + ext, cz - ext},
            {cx - ext, cz + ext},
            {cx + ext, cz + ext},
        };
        for (const auto& pt : corners) {
            float h = 0.0f;
            if (!floorHeightAt(pt[0], pt[1], h)) {
                return false;
            }
            if (h - y > stepHeight) {
                return false;
            }
        }
        return true;
    };

    // Sub-step the horizontal move so a large stride cannot tunnel through
    // a wall band thinner than the stride (walls rasterize to cell-thick
    // obstacles). Each sub-step re-probes the footprint.
    const float dist = std::max(std::fabs(dx), std::fabs(dz));
    u32 steps = 1;
    if (dist > cellSize_) {
        steps = static_cast<u32>(std::ceil(dist / cellSize_));
    }

    bool moved = false;
    const float sx = dx / static_cast<float>(steps);
    const float sz = dz / static_cast<float>(steps);
    const float startX = x, startZ = z;
    for (u32 s = 0; s < steps; ++s) {
        // X axis first: attempt the horizontal move along X.
        const float nx = x + sx;
        if (standable(nx, z)) {
            x = nx;
        }
        // Then Z axis, with Y snapped so the Z probe uses the updated x.
        const float nz = z + sz;
        if (standable(x, nz)) {
            z = nz;
        }
        if (x == startX && z == startZ) {
            break;  // blocked on both axes within this sub-step
        }
        // Snap Y to the floor under the new footprint center.
        float h = 0.0f;
        if (floorHeightAt(x, z, h)) {
            y = h;
        }
    }

    // If nothing could move at all, the position is untouched.
    return x != startX || z != startZ;
}

}  // namespace ico::engine