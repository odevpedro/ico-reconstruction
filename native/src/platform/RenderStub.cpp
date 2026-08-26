#include "platform/RenderStub.h"
#include "runtime/Logger.h"

RenderStub::RenderStub()
    : m_initialized(false)
    , m_width(0)
    , m_height(0) {
}

RenderStub::~RenderStub() {
    shutdown();
}

bool RenderStub::initialize(u32 width, u32 height) {
    if (m_initialized) {
        return true;
    }

    m_width = width;
    m_height = height;
    m_initialized = true;

    Logger::info("render", "Render stub initialized: %ux%u", m_width, m_height);
    return true;
}

void RenderStub::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_initialized = false;
    Logger::info("render", "Render stub shutdown");
}

void RenderStub::beginFrame() {
    if (!m_initialized) return;
}

void RenderStub::endFrame() {
    if (!m_initialized) return;
}

void RenderStub::clear() {
    if (!m_initialized) return;
}

u32 RenderStub::getWidth() const {
    return m_width;
}

u32 RenderStub::getHeight() const {
    return m_height;
}
