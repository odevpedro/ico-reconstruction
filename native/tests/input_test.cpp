#include "platform/Input.h"

#include <cassert>
#include <cstdio>

// Input wiring contract (Rev.147): platform-neutral Input class is fed by the
// X11 glue via setKeyState (NativeKey codes). update() runs at the TOP of
// each frame and snapshots the previous frame's key state into prev; the
// platform pump then mutates keys. Edge queries (pressed/released) compare
// the pumped state against that snapshot.
int main()
{
    Input input;
    assert(!input.isKeyDown(KeyW));

    assert(input.initialize());

    // --- frame 1: pump delivers W down ---
    input.update();                  // top of frame: prev = keys (all 0)
    input.setKeyState(KeyW, true);   // platform event
    assert(input.isKeyDown(KeyW));
    assert(input.isKeyPressed(KeyW));  // 0->1 transition, edge fires
    assert(!input.isKeyReleased(KeyW));

    // --- frame 2: W held (no new pump event) ---
    input.update();                  // prev = keys (W down)
    assert(input.isKeyDown(KeyW));
    assert(!input.isKeyPressed(KeyW));  // held, no edge
    assert(!input.isKeyReleased(KeyW));

    // --- frame 3: W released ---
    input.update();                  // prev = keys (W still down)
    input.setKeyState(KeyW, false);  // platform event
    assert(!input.isKeyDown(KeyW));
    assert(input.isKeyReleased(KeyW));  // 1->0 transition, edge fires
    assert(!input.isKeyPressed(KeyW));

    // --- frame 4: W still released, edges clear ---
    input.update();
    assert(!input.isKeyDown(KeyW));
    assert(!input.isKeyReleased(KeyW));

    // Independent keys do not interfere.
    input.update();
    input.setKeyState(KeyA, true);
    input.setKeyState(KeyD, true);
    assert(input.isKeyDown(KeyA) && input.isKeyDown(KeyD));
    assert(!input.isKeyDown(KeyW));

    // Out-of-range codes are rejected.
    input.setKeyState(9999, true);
    assert(!input.isKeyDown(9999));

    // Gamepad stubs remain inert (P1 boundary: no pad plumbing yet).
    assert(!input.isGamepadButtonPressed(0));
    assert(input.getGamepadAxis(0) == 0.0f);

    input.shutdown();
    assert(!input.isKeyDown(KeyW));

    std::printf("input_test: OK\n");
    return 0;
}