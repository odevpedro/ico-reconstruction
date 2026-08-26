#pragma once

#include "ps2/Ps2Types.h"
#include <functional>
#include <vector>

using IopHandler = std::function<u32(u32, u32, u32, u32)>;

class IopStub {
public:
    IopStub();
    ~IopStub();

    bool initialize();
    void shutdown();

    u32 call(u32 functionId, u32 arg0 = 0, u32 arg1 = 0, u32 arg2 = 0, u32 arg3 = 0);
    void registerHandler(u32 functionId, IopHandler handler);

private:
    bool m_initialized;
    std::vector<std::pair<u32, IopHandler>> m_handlers;
};
