#pragma once

#include <cstdint>
#include <chrono>
#include <functional>

namespace platform {

class Timer {
public:
    virtual ~Timer() = default;
    
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void reset() = 0;
    
    virtual uint64_t getTicks() const = 0;
    virtual uint64_t getFrequency() const = 0;
    virtual double getSeconds() const = 0;
    virtual double getMilliseconds() const = 0;
};

class GameClock {
public:
    virtual ~GameClock() = default;
    
    virtual void initialize(uint32_t targetFPS = 60) = 0;
    virtual void shutdown() = 0;
    
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    
    virtual bool isFrameReady() = 0;
    virtual uint32_t getFrameCount() const = 0;
    virtual double getDeltaTime() const = 0;
    virtual double getFrameRate() const = 0;
    
    virtual void setTargetFPS(uint32_t fps) = 0;
    virtual uint32_t getTargetFPS() const = 0;
    
    virtual uint64_t getVBlankCount() const = 0;
};

class Stopwatch {
public:
    virtual ~Stopwatch() = default;
    
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void reset() = 0;
    
    virtual double getElapsedSeconds() const = 0;
    virtual double getElapsedMilliseconds() const = 0;
    virtual bool isRunning() const = 0;
};

std::unique_ptr<Timer> createTimer();
std::unique_ptr<GameClock> createGameClock();
std::unique_ptr<Stopwatch> createStopwatch();

} // namespace platform
