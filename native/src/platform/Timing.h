#pragma once

#include "ps2/Ps2Types.h"
#include <chrono>

class Timing {
public:
    Timing();
    ~Timing();

    bool initialize(u32 targetFPS = 60);
    void shutdown();

    void beginFrame();
    void endFrame();

    bool isFrameReady() const;
    u32 getFrameCount() const;
    f64 getDeltaTime() const;
    f64 getFrameRate() const;
    u64 getVBlankCount() const;

    void setTargetFPS(u32 fps);
    u32 getTargetFPS() const;

    static u64 getTicks();
    static f64 getSeconds();

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    TimePoint m_startTime;
    TimePoint m_frameStart;
    TimePoint m_frameEnd;

    u32 m_targetFPS;
    u32 m_frameCount;
    u64 m_vBlankCount;
    f64 m_deltaTime;
    f64 m_frameRate;
    bool m_initialized;
};
