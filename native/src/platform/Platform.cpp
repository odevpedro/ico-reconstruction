#include "platform/Platform.h"
#include "runtime/Logger.h"

Platform::Platform()
    : m_initialized(false) {
}

Platform::~Platform() {
    shutdown();
}

bool Platform::initialize() {
    if (m_initialized) {
        return true;
    }

    m_initialized = true;
    Logger::info("platform", "Platform stub initialized");
    return true;
}

void Platform::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_initialized = false;
    Logger::info("platform", "Platform stub shutdown");
}
