#pragma once

#include "ps2/Ps2Types.h"

class RenderStub {
public:
    RenderStub();
    ~RenderStub();

    bool initialize(u32 width = 640, u32 height = 480);
    void shutdown();

    void beginFrame();
    void endFrame();
    void clear();

    u32 getWidth() const;
    u32 getHeight() const;

private:
    bool m_initialized;
    u32 m_width;
    u32 m_height;
};
