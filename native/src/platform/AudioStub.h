#pragma once

#include "ps2/Ps2Types.h"

class AudioStub {
public:
    AudioStub();
    ~AudioStub();

    bool initialize();
    void shutdown();

    void update();

    void playSound(u32 soundId, f32 volume);
    void stopSound(u32 soundId);
    void setMasterVolume(f32 volume);

private:
    bool m_initialized;
    f32 m_masterVolume;
};
