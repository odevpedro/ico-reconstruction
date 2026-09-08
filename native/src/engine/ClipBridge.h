#pragma once

#include "ps2/Ps2Types.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ico::engine {

// ClipBridge — semantic native counterpart of the PS2 `_Clip` walkable-space
// contract, built from the room geometry that the native renderer already
// decodes from the .p2o (Ps2oMesh). It is NOT a byte-exact port: `_Clip`
// (0x00166E10) is catalogued in rev097 as a collision/clip routine inside
// DispCollisionPC (4 `_clipW*` callbacks) but has no .s or C source in the
// tree. This bridge reconstructs the minimal floor/wall behaviour the player
// placeholder needs, fed by the same floor-preview facts that drove Rev.148:
//
//   - the room floor is NOT a single plane (Y from ~-1700 up to ~330 across
//     tiers); floor support must be derived from the mesh triangles.
//   - walls render as ordinary triangles; the player must not pass through
//     the near-vertical surface samples of the room.
//
// The bridge consumes the decoded triangle list (positions + indices as
// produced by loadPs2oMesh) and builds two flat data structures over the
// room's XZ bounds:
//
//   heightCells  — 2D grid of max floor heights (from up-facing sample
//     heights), so ground contact resolves to the top surface actually
//     under the character.
//   blockedCells — 2D occupancy of wall samples (triangles whose normal is
//     near-horizontal), so the character cannot walk through a wall face.
//
// Movement is axis-separated: it tries the X axis then the Z axis with the
// AABB half-extent, so walking into a wall slides along it instead of
// sticking, and ground Y snaps to the heightfield under the new position
// (with a stepHeight clamp so the character cannot teleport onto a ledge).
class ClipBridge {
public:
    ClipBridge() = default;
    ~ClipBridge();
    ClipBridge(const ClipBridge&) = delete;
    ClipBridge& operator=(const ClipBridge&) = delete;

    // Builds height + blocked grids from a decoded mesh triangle list.
    //   positions  : x,y,z per vertex, 3 floats per vertex (mesh.positions)
    //   vertexCount: number of vertices
    //   triangles  : 3 indices per triangle (mesh.triangles)
    //   triangleCount: number of triangles
    // Returns false on empty/invalid input.
    bool buildFromMesh(const float* positions, u32 vertexCount,
                       const uint32_t* triangles, u32 triangleCount);

    // Clears all grids; runtime stops resolving support. Safe on idle bridge.
    void shutdown();
    bool isInitialized() const { return initialized; }

    // Removes blocked cells (wall samples). Bridges for open rooms disable
    // wall blocking when the room has no wall samples or when the caller
    // wants a floor-only walk.
    void setWallBlocking(bool enabled) { wallBlocking_ = enabled; }
    bool wallBlocking() const { return wallBlocking_; }

    // World XZ bounds of the built grid.
    float minX() const { return minX_; }
    float maxX() const { return maxX_; }
    float minZ() const { return minZ_; }
    float maxZ() const { return maxZ_; }
    float cellSize() const { return cellSize_; }

    // Highest floor height at (x,z). Returns false (with outY valid) when the
    // point is inside walls/outside the room (no walkable support). outY holds
    // floor height and is set to the heightfield sample even on false so the
    // renderer can visually place objects on the nearest surface.
    bool floorHeightAt(float x, float z, float& outY) const;

    // Attempts a move from (x,y,z) by (dx,dz). Axis-separated; snaps y to the
    // floor under the new position. halfExtent = AABB half-size in x/z ground
    // plane; stepHeight = max climbable delta the character is allowed to
    // snap up (prevents ledge/step teleports). Wall samples block the move.
    // Returns true when any axis made progress (position updated).
    bool move(float& x, float& y, float& z, float dx, float dz,
              float halfExtent, float stepHeight) const;

    // Grid dimensions (debug/tests).
    u32 gridWidth() const { return gridW; }
    u32 gridHeight() const { return gridH; }
    u32 blockedCellCount() const { return blockedCount; }

    // True when a wall blocks the given world point (debug/inspection).
    bool isBlocked(float x, float z) const {
        if (!initialized) return false;
        const int ci = cellIndexOf(x, z);
        if (ci < 0) return false;
        return blockedCells[static_cast<std::size_t>(ci)].blocked;
    }

private:
    struct HeightCell {
        // Highest floor Y observed over this cell from up-facing samples.
        float height;
        // Sample basis (for floorHeightAt to look at neighbors). Stored as
        // the count of up-facing samples observed; >0 means a floor exists.
        u16 samples;
    };
    struct BlockedCell {
        bool blocked;
    };

    // Samples all triangles into height/blocked cells.
    void rasterizeTriangle(float x0, float y0, float z0,
                           float x1, float y1, float z1,
                           float x2, float y2, float z2);
    void rasterizeWall(float x0, float y0, float z0,
                       float x1, float y1, float z1,
                       float x2, float y2, float z2);
    int cellIndexOf(float x, float z) const;
    bool cellInBounds(float x, float z) const;

    bool initialized = false;
    bool wallBlocking_ = true;
    float minX_ = 0.0f, maxX_ = 0.0f, minZ_ = 0.0f, maxZ_ = 0.0f;
    float cellSize_ = 0.0f;
    u32 gridW = 0, gridH = 0;
    u32 blockedCount = 0;

    std::vector<HeightCell> heightCells;
    std::vector<BlockedCell> blockedCells;
};

}  // namespace ico::engine