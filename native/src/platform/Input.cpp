#include "platform/Input.h"
#include "runtime/Logger.h"
#include <cstring>

Input::Input()
    : m_initialized(false) {
    std::memset(m_keys, 0, sizeof(m_keys));
    std::memset(m_prevKeys, 0, sizeof(m_prevKeys));
}

Input::~Input() {
    shutdown();
}

bool Input::initialize() {
    if (m_initialized) {
        return true;
    }

    m_initialized = true;
    Logger::info("input", "Input stub initialized");
    return true;
}

void Input::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_initialized = false;
    Logger::info("input", "Input stub shutdown");
}

void Input::update() {
    if (!m_initialized) {
        return;
    }

    std::memcpy(m_prevKeys, m_keys, sizeof(m_keys));
}

bool Input::isKeyDown(u32 key) const {
    if (key >= 256) return false;
    return m_keys[key];
}

bool Input::isKeyPressed(u32 key) const {
    if (key >= 256) return false;
    return m_keys[key] && !m_prevKeys[key];
}

bool Input::isKeyReleased(u32 key) const {
    if (key >= 256) return false;
    return !m_keys[key] && m_prevKeys[key];
}

bool Input::isGamepadButtonPressed(u32 button) const {
    (void)button;
    return false;
}

f32 Input::getGamepadAxis(u32 axis) const {
    (void)axis;
    return 0.0f;
}
