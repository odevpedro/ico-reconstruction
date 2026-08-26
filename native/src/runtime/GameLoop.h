#pragma once

#include "ps2/Ps2Types.h"
#include <functional>

class GameLoop {
public:
    using UpdateCallback = std::function<bool(u32 frame)>;

    GameLoop();
    ~GameLoop();

    bool initialize(u32 targetFPS = 60);
    void shutdown();

    void setUpdateCallback(UpdateCallback callback);
    void run(u32 maxFrames = 0);
    void stop();

    u32 getCurrentFrame() const;
    bool isRunning() const;

private:
    UpdateCallback m_updateCallback;
    u32 m_currentFrame;
    bool m_running;
    bool m_initialized;
};
