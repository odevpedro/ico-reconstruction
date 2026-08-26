#include "runtime/GameLoop.h"
#include "runtime/Logger.h"
#include "platform/Timing.h"

GameLoop::GameLoop()
    : m_currentFrame(0)
    , m_running(false)
    , m_initialized(false) {
}

GameLoop::~GameLoop() {
    shutdown();
}

bool GameLoop::initialize(u32 targetFPS) {
    if (m_initialized) {
        return true;
    }

    m_initialized = true;
    Logger::info("gameloop", "GameLoop initialized (target=%u FPS)", targetFPS);
    return true;
}

void GameLoop::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_running = false;
    m_initialized = false;
    Logger::info("gameloop", "GameLoop shutdown");
}

void GameLoop::setUpdateCallback(UpdateCallback callback) {
    m_updateCallback = std::move(callback);
}

void GameLoop::run(u32 maxFrames) {
    if (!m_initialized) {
        Logger::error("gameloop", "Cannot run: not initialized");
        return;
    }

    m_running = true;
    m_currentFrame = 0;

    Logger::info("gameloop", "GameLoop started (maxFrames=%u)", maxFrames);

    while (m_running) {
        if (maxFrames > 0 && m_currentFrame >= maxFrames) {
            break;
        }

        m_currentFrame++;

        if (m_updateCallback) {
            bool shouldContinue = m_updateCallback(m_currentFrame);
            if (!shouldContinue) {
                Logger::info("gameloop", "Update callback requested stop at frame %u", m_currentFrame);
                break;
            }
        }

        Timing::getTicks();
    }

    m_running = false;
    Logger::info("gameloop", "GameLoop ended at frame %u", m_currentFrame);
}

void GameLoop::stop() {
    m_running = false;
}

u32 GameLoop::getCurrentFrame() const {
    return m_currentFrame;
}

bool GameLoop::isRunning() const {
    return m_running;
}
