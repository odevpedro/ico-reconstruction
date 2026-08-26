#include "platform/IopStub.h"
#include "runtime/Logger.h"

IopStub::IopStub()
    : m_initialized(false) {
}

IopStub::~IopStub() {
    shutdown();
}

bool IopStub::initialize() {
    if (m_initialized) {
        return true;
    }

    m_initialized = true;
    Logger::info("iop", "IOP stub initialized");
    return true;
}

void IopStub::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_handlers.clear();
    m_initialized = false;
    Logger::info("iop", "IOP stub shutdown");
}

u32 IopStub::call(u32 functionId, u32 arg0, u32 arg1, u32 arg2, u32 arg3) {
    if (!m_initialized) return 0;

    for (auto& [id, handler] : m_handlers) {
        if (id == functionId) {
            return handler(arg0, arg1, arg2, arg3);
        }
    }

    Logger::warn("iop", "call: unknown function 0x%08X", functionId);
    return 0;
}

void IopStub::registerHandler(u32 functionId, IopHandler handler) {
    if (!m_initialized) return;

    m_handlers.emplace_back(functionId, std::move(handler));
    Logger::debug("iop", "Registered handler 0x%08X", functionId);
}
