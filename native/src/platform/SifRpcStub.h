#pragma once

#include "ps2/Ps2Types.h"
#include <functional>
#include <vector>

using SifRpcCallback = std::function<u32(const u8* data, u32 size)>;

class SifRpcStub {
public:
    SifRpcStub();
    ~SifRpcStub();

    bool initialize();
    void shutdown();

    u32 callRpc(u32 serviceId, const u8* data, u32 size);
    void registerService(u32 serviceId, SifRpcCallback callback);

private:
    bool m_initialized;
    std::vector<std::pair<u32, SifRpcCallback>> m_services;
};
