#include "platform/Timing.h"
#include "runtime/Logger.h"
#include <thread>

Timing::Timing()
    : m_targetFPS(60)
    , m_frameCount(0)
    , m_vBlankCount(0)
    , m_deltaTime(0.0)
    , m_frameRate(0.0)
    , m_initialized(false) {
}

Timing::~Timing() {
    shutdown();
}

bool Timing::initialize(u32 targetFPS) {
    if (m_initialized) {
        return true;
    }

    m_targetFPS = targetFPS;
    m_startTime = Clock::now();
    m_frameStart = m_startTime;
    m_frameEnd = m_startTime;
    m_initialized = true;

    Logger::info("timing", "Timing initialized: target=%u FPS", m_targetFPS);
    return true;
}

void Timing::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_initialized = false;
    Logger::info("timing", "Timing shutdown");
}

void Timing::beginFrame() {
    m_frameStart = Clock::now();
}

void Timing::endFrame() {
    m_frameEnd = Clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        m_frameEnd - m_frameStart);
    m_deltaTime = duration.count() / 1000000.0;

    if (m_deltaTime > 0.0) {
        m_frameRate = 1.0 / m_deltaTime;
    }

    m_frameCount++;
    m_vBlankCount++;

    f64 targetFrameTime = 1.0 / m_targetFPS;
    if (m_deltaTime < targetFrameTime) {
        auto sleepTime = std::chrono::microseconds(
            static_cast<long long>((targetFrameTime - m_deltaTime) * 1000000.0));
        std::this_thread::sleep_for(sleepTime);
    }
}

bool Timing::isFrameReady() const {
    return m_initialized;
}

u32 Timing::getFrameCount() const {
    return m_frameCount;
}

f64 Timing::getDeltaTime() const {
    return m_deltaTime;
}

f64 Timing::getFrameRate() const {
    return m_frameRate;
}

u64 Timing::getVBlankCount() const {
    return m_vBlankCount;
}

void Timing::setTargetFPS(u32 fps) {
    m_targetFPS = fps;
}

u32 Timing::getTargetFPS() const {
    return m_targetFPS;
}

u64 Timing::getTicks() {
    auto now = Clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch());
    return static_cast<u64>(duration.count());
}

f64 Timing::getSeconds() {
    auto now = Clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch());
    return duration.count() / 1000000.0;
}
