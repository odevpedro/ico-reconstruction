#include "runtime/IcoRuntime.h"
#include "runtime/Logger.h"

IcoRuntime::IcoRuntime()
    : m_initialized(false) {
}

IcoRuntime::~IcoRuntime() {
    shutdown();
}

bool IcoRuntime::initialize() {
    if (m_initialized) {
        Logger::warn("runtime", "Already initialized");
        return true;
    }

    Logger::info("runtime", "Initializing ICO Native Runtime...");

    if (!m_memory.initialize()) {
        Logger::error("runtime", "Failed to initialize PS2 memory");
        return false;
    }

    if (!m_fileSystem.initialize()) {
        Logger::error("runtime", "Failed to initialize filesystem");
        return false;
    }

    if (!m_sifRpc.initialize()) {
        Logger::error("runtime", "Failed to initialize SIF RPC");
        return false;
    }

    if (!m_iop.initialize()) {
        Logger::error("runtime", "Failed to initialize IOP");
        return false;
    }

    if (!m_input.initialize()) {
        Logger::error("runtime", "Failed to initialize input");
        return false;
    }

    if (!m_audio.initialize()) {
        Logger::error("runtime", "Failed to initialize audio");
        return false;
    }

    if (!m_render.initialize()) {
        Logger::error("runtime", "Failed to initialize render");
        return false;
    }

    if (!m_timing.initialize()) {
        Logger::error("runtime", "Failed to initialize timing");
        return false;
    }

    if (!m_gameLoop.initialize()) {
        Logger::error("runtime", "Failed to initialize game loop");
        return false;
    }

    if (!m_isysGObj.initialize()) {
        Logger::error("runtime", "Failed to initialize isysGObj");
        return false;
    }

    m_initialized = true;
    Logger::info("runtime", "ICO Native Runtime initialized successfully");
    return true;
}

void IcoRuntime::shutdown() {
    if (!m_initialized) {
        return;
    }

    Logger::info("runtime", "Shutting down ICO Native Runtime...");

    m_isysGObj.shutdown();
    m_gameLoop.shutdown();
    m_timing.shutdown();
    m_render.shutdown();
    m_audio.shutdown();
    m_input.shutdown();
    m_iop.shutdown();
    m_sifRpc.shutdown();
    m_fileSystem.shutdown();
    m_memory.shutdown();

    m_initialized = false;
    Logger::info("runtime", "ICO Native Runtime shutdown complete");
}

bool IcoRuntime::isInitialized() const {
    return m_initialized;
}

Ps2Memory& IcoRuntime::getMemory() {
    return m_memory;
}

Input& IcoRuntime::getInput() {
    return m_input;
}

FileSystem& IcoRuntime::getFileSystem() {
    return m_fileSystem;
}

Timing& IcoRuntime::getTiming() {
    return m_timing;
}

AudioStub& IcoRuntime::getAudio() {
    return m_audio;
}

RenderStub& IcoRuntime::getRender() {
    return m_render;
}

SifRpcStub& IcoRuntime::getSifRpc() {
    return m_sifRpc;
}

IopStub& IcoRuntime::getIop() {
    return m_iop;
}

GameLoop& IcoRuntime::getGameLoop() {
    return m_gameLoop;
}

IsysGObj& IcoRuntime::getIsysGObj() {
    return m_isysGObj;
}
