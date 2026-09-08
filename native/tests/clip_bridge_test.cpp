#include "engine/ClipBridge.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <vector>

namespace {
constexpr float kEps = 0.01f;
}  // namespace

// Builds a synthetic room:
//   - main floor: a large up-facing surface at z in [-500, 500],
//     x in [-500, 500], y = 0
//   - a south wall: near-vertical quad at z = 500 (blocks movement)
//   - an elevated tier: smaller floor at x in [100, 300], z in [100, 300],
//     y = 100 (reachable only with a stepHeight >= 100, i.e. not reachable
//     with the default step)
//   - a raised obstacle pedestal: a near-vertical ring around (0,0) to prove
//     that points inside a blocked cell report no support.
void makeTestRoom(ico::engine::ClipBridge& bridge) {
    std::vector<float> pos;
    std::vector<uint32_t> tris;
    auto pushTri = [&](float x0, float y0, float z0,
                       float x1, float y1, float z1,
                       float x2, float y2, float z2) {
        const uint32_t b = static_cast<uint32_t>(pos.size() / 3);
        pos.insert(pos.end(),
                   {x0, y0, z0, x1, y1, z1, x2, y2, z2});
        tris.insert(tris.end(), {b, b + 1, b + 2});
    };
    // -- Main floor (two triangles, up-facing: normal +Y) --
    pushTri(-500, 0,  500,   500, 0,  500,   500, 0, -500);
    pushTri(-500, 0,  500,   500, 0, -500,  -500, 0, -500);
    // -- South wall at z=500 (vertical quad: two triangles) --
    pushTri(-500, 0, 500,   500, 0, 500,   500, 200, 500);
    pushTri(-500, 0, 500,   500, 200, 500,  -500, 200, 500);
    // -- Elevated tier (two triangles, up-facing: normal +Y) --
    pushTri(100, 100, 300,   300, 100, 300,   300, 100, 100);
    pushTri(100, 100, 300,   300, 100, 100,   100, 100, 100);
    // -- Pedestal walls around (0,0): a 40x40 near-vertical box --
    const float bx0 = -20, bx1 = 20, bz0 = -20, bz1 = 20, bh = 60;
    pushTri(bx0, 0, bz0, bx1, 0, bz0, bx1, bh, bz0);  // north face
    pushTri(bx0, 0, bz0, bx1, bh, bz0, bx0, bh, bz0);
    pushTri(bx1, 0, bz1, bx0, 0, bz1, bx0, bh, bz1);  // south face
    pushTri(bx1, 0, bz1, bx0, bh, bz1, bx1, bh, bz1);
    pushTri(bx0, 0, bz1, bx0, 0, bz0, bx0, bh, bz0);  // west face
    pushTri(bx0, 0, bz1, bx0, bh, bz0, bx0, bh, bz1);
    pushTri(bx1, 0, bz0, bx1, 0, bz1, bx1, bh, bz1);  // east face
    pushTri(bx1, 0, bz0, bx1, bh, bz1, bx1, bh, bz0);

    const bool ok = bridge.buildFromMesh(pos.data(),
                                         static_cast<u32>(pos.size() / 3),
                                         tris.data(),
                                         static_cast<u32>(tris.size() / 3));
    assert(ok);
    assert(bridge.isInitialized());
}

int main() {
    ico::engine::ClipBridge bridge;
    makeTestRoom(bridge);

    /* Floor height at the main floor resolves across the whole walkable
       zone. */
    float h = -999.0f;
    assert(bridge.floorHeightAt(-100, -100, h));
    assert(std::fabs(h - 0.0f) < kEps);
    assert(bridge.floorHeightAt(400, 400, h));
    assert(std::fabs(h) < kEps);

    /* Outside the room (y beyond the north wall / z beyond) -> no support. */
    (void)bridge.minX();
    assert(!bridge.floorHeightAt(-700, 0, h));
    assert(!bridge.floorHeightAt(0, -700, h));

    /* Inside the pedestal (blocked cell) -> no support, even though the
       main floor continues beneath it. */
    assert(!bridge.floorHeightAt(0, 0, h));

    /* Elevated tier provides floor support at y=100. */
    assert(bridge.floorHeightAt(200, 200, h));
    assert(std::fabs(h - 100.0f) < kEps);

    /* ---- Movement: flat walk west along the main floor ---- */
    float x = -100, y = 0, z = 0;
    assert(bridge.move(x, y, z, -100, 0, 10.0f, 45.0f));
    assert(std::fabs(x + 200) < kEps && std::fabs(z) < kEps);
    assert(std::fabs(y - 0.0f) < kEps);

    /* ---- Walk north toward the wall: the wall stops the character, but
       axis-separated movement keeps progress on the unblocked axis. The
       character stops 1 cell (50 units) short of the wall band. ---- */
    x = 0; y = 0; z = 400;
    assert(bridge.move(x, y, z, 0, 200, 10.0f, 45.0f));
    assert(std::fabs(z - 450.0f) < kEps);
    assert(std::fabs(y - 0.0f) < kEps);

    /* Repeated pushes creep cell-by-cell up to the wall band, then stop
       making progress (the half-extent footprint enters the blocked cell). */
    assert(bridge.move(x, y, z, 0, 60, 10.0f, 45.0f));
    assert(std::fabs(z - 480.0f) < kEps);          // pushed to band edge
    assert(!bridge.move(x, y, z, 0, 60, 10.0f, 45.0f));  // now blocked
    assert(std::fabs(z - 480.0f) < kEps);

    /* ---- The pedestal blocks approach from the north: the character
       cannot walk into the raised obstacle. ---- */
    x = 0; y = 0; z = -40;
    assert(!bridge.move(x, y, z, 0, 40, 10.0f, 45.0f));
    assert(std::fabs(z + 40.0f) < kEps);
    assert(std::fabs(y - 0.0f) < kEps);

    /* ---- Diagonal into the wall slides: the X axis advances while the
       blocked Z axis clamps to the wall band. ---- */
    x = 0; y = 0; z = 400;
    assert(bridge.move(x, y, z, 40, 200, 10.0f, 45.0f));
    assert(std::fabs(x - 40.0f) < kEps);
    assert(std::fabs(z - 450.0f) < kEps);

    /* ---- Step height prohibits the tier climb when too low. ---- */
    x = 250; y = 0; z = 90;
    assert(!bridge.move(x, y, z, 0, 20, 10.0f, 45.0f));
    assert(std::fabs(z - 90.0f) < kEps);

    /* ---- With a big stepHeight the character can step up onto the tier ---- */
    x = 250; y = 0; z = 90;
    assert(bridge.move(x, y, z, 0, 130, 10.0f, 110.0f));
    assert(std::fabs(z - 220.0f) < kEps);
    assert(std::fabs(y - 100.0f) < kEps);

    /* ---- Wall blocking can be disabled for open rooms; the wall cell then
       reports floor support. ---- */
    bridge.setWallBlocking(false);
    assert(bridge.floorHeightAt(0, 0, h));  // pedestal now ignored too
    (void)h;
    bridge.setWallBlocking(true);
    assert(!bridge.floorHeightAt(0, 0, h));

    /* ---- Repeated move walking into a wall returns false with zero
       progress (blocked). ---- */
    x = 0; y = 0; z = 495;
    assert(!bridge.move(x, y, z, 0, 40, 10.0f, 45.0f));
    assert(std::fabs(z - 495.0f) < kEps);

    /* Empty/uninitialized bridge: all queries fail safely. */
    ico::engine::ClipBridge empty;
    assert(!empty.isInitialized());
    assert(!empty.floorHeightAt(0, 0, h));
    assert(!empty.move(x, y, z, 1, 0, 10.0f, 45.0f));
    empty.shutdown();
    assert(!empty.isInitialized());

    /* Rebuild after shutdown works. */
    bridge.shutdown();
    assert(!bridge.isInitialized());
    assert(!bridge.floorHeightAt(0, 0, h));
    makeTestRoom(bridge);
    assert(bridge.isInitialized());
    assert(bridge.floorHeightAt(200, 200, h));

    std::fprintf(stderr, "clip_bridge_test: OK (%ux%u grid, %u blocked cells)\n",
                 bridge.gridWidth(), bridge.gridHeight(),
                 bridge.blockedCellCount());
    return 0;
}