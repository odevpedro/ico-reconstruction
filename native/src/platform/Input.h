#pragma once

#include "ps2/Ps2Types.h"

class Input {
public:
    Input();
    ~Input();

    bool initialize();
    void shutdown();
    void update();

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
