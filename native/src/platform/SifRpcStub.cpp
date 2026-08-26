#include "platform/SifRpcStub.h"
#include "runtime/Logger.h"

SifRpcStub::SifRpcStub()
    : m_initialized(false) {
}

SifRpcStub::~SifRpcStub() {
    shutdown();
}

bool SifRpcStub::initialize() {
    if (m_initialized) {
        return true;
    }

    m_initialized = true;
    Logger::info("sif", "SIF RPC stub initialized");
    return true;
}

void SifRpcStub::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_services.clear();
    m_initialized = false;
    Logger::info("sif", "SIF RPC stub shutdown");
}

u32 SifRpcStub::callRpc(u32 serviceId, const u8* data, u32 size) {
    if (!m_initialized) return 0;

    for (auto& [id, callback] : m_services) {
        if (id == serviceId) {
            return callback(data, size);
        }
    }

    Logger::warn("sif", "callRpc: unknown service 0x%08X", serviceId);
    return 0;
}

void SifRpcStub::registerService(u32 serviceId, SifRpcCallback callback) {
    if (!m_initialized) return;

    m_services.emplace_back(serviceId, std::move(callback));
    Logger::debug("sif", "Registered service 0x%08X", serviceId);
}
