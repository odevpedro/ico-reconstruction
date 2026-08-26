#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace platform {

// Audio formats
enum class AudioFormat : uint8_t {
    Unknown,
    PCM8,
    PCM16,
    PCM24,
    PCM32,
    Float32
};

// Sound channels
enum class SoundChannel : uint8_t {
    Mono,
    Stereo,
    Quad,
    Surround51,
    Surround71
};

class Sound {
public:
    virtual ~Sound() = default;
    
    virtual uint32_t getDuration() const = 0;
    virtual uint32_t getSampleRate() const = 0;
    virtual AudioFormat getFormat() const = 0;
    virtual SoundChannel getChannels() const = 0;
    virtual size_t getSampleCount() const = 0;
    virtual size_t getDataSize() const = 0;
    
    virtual void play(float volume = 1.0f, float panning = 0.0f) = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;
    virtual bool isPlaying() const = 0;
    
    virtual void setVolume(float volume) = 0;
    virtual void setPanning(float panning) = 0;
    virtual void setPitch(float pitch) = 0;
    virtual void setLoop(bool loop) = 0;
    virtual void setPosition(uint32_t positionMs) = 0;
};

class Music {
public:
    virtual ~Music() = default;
    
    virtual void play(float volume = 1.0f) = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void stop() = 0;
    virtual bool isPlaying() const = 0;
    
    virtual void setVolume(float volume) = 0;
    virtual void setLoop(bool loop) = 0;
    virtual void setPosition(uint32_t positionMs) = 0;
    virtual uint32_t getPosition() const = 0;
    virtual uint32_t getDuration() const = 0;
};

class Audio {
public:
    virtual ~Audio() = default;
    
    virtual void initialize(int sampleRate = 44100, int bufferSize = 1024) = 0;
    virtual void shutdown() = 0;
    
    virtual void update() = 0;
    
    // Master volume
    virtual void setMasterVolume(float volume) = 0;
    virtual float getMasterVolume() const = 0;
    
    // Sound management
    virtual std::unique_ptr<Sound> loadSound(const std::string& path) = 0;
    virtual std::unique_ptr<Sound> loadSoundFromMemory(const void* data, size_t size) = 0;
    virtual void playSound(Sound* sound, float volume = 1.0f, float panning = 0.0f) = 0;
    
    // Music management
    virtual std::unique_ptr<Music> loadMusic(const std::string& path) = 0;
    virtual void playMusic(Music* music, float volume = 1.0f) = 0;
    virtual void stopMusic() = 0;
    
    // 3D audio
    virtual void setListenerPosition(float x, float y, float z) = 0;
    virtual void setListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                        float upX, float upY, float upZ) = 0;
    virtual void setSoundPosition(Sound* sound, float x, float y, float z) = 0;
};

std::unique_ptr<Audio> createAudio();

} // namespace platform
