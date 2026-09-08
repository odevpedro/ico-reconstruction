#pragma once

#include "ps2/Ps2Types.h"

// Portable native key codes used by Input::isKeyDown/isKeyPressed. The glue
// layer (main.cpp's X11 event pump) translates platform-specific keysyms into
// these codes via setKeyState, so Input itself stays platform-neutral.
enum NativeKey : u32 {
    KeyW = 0,
    KeyA,
    KeyS,
    KeyD,
    KeyArrowUp,
    KeyArrowDown,
    KeyArrowLeft,
    KeyArrowRight,
    NativeKeyCount
};

class Input {
public:
    Input();
    ~Input();

    bool initialize();
    void shutdown();
    void update();

    // Feeds one translated key code with its down/released state. Called by
    // the platform glue between update() calls; update() snapshots current
    // state into m_prevKeys for edge detection.
    void setKeyState(u32 key, bool down);

    bool isKeyDown(u32 key) const;
    bool isKeyPressed(u32 key) const;
    bool isKeyReleased(u32 key) const;

    bool isGamepadButtonPressed(u32 button) const;
    f32 getGamepadAxis(u32 axis) const;

private:
    bool m_initialized;
    bool m_keys[256];
    bool m_prevKeys[256];
};
