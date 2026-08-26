#include "platform/AudioStub.h"
#include "runtime/Logger.h"

AudioStub::AudioStub()
    : m_initialized(false)
    , m_masterVolume(1.0f) {
}

AudioStub::~AudioStub() {
    shutdown();
}

bool AudioStub::initialize() {
    if (m_initialized) {
        return true;
    }

    m_initialized = true;
    Logger::info("audio", "Audio stub initialized");
    return true;
}

void AudioStub::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_initialized = false;
    Logger::info("audio", "Audio stub shutdown");
}

void AudioStub::update() {
    if (!m_initialized) {
        return;
    }
}

void AudioStub::playSound(u32 soundId, f32 volume) {
    if (!m_initialized) return;
    Logger::debug("audio", "playSound(id=%u, vol=%.2f)", soundId, volume);
}

void AudioStub::stopSound(u32 soundId) {
    if (!m_initialized) return;
    Logger::debug("audio", "stopSound(id=%u)", soundId);
}

void AudioStub::setMasterVolume(f32 volume) {
    if (!m_initialized) return;
    m_masterVolume = volume;
    Logger::debug("audio", "setMasterVolume(%.2f)", volume);
}
