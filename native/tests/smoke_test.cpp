#include "runtime/IcoRuntime.h"
#include "runtime/Logger.h"
#include <cstdio>
#include <cassert>

int main() {
    std::fprintf(stderr, "ICO Native Runtime Smoke Test\n");

    Logger::setLevel(Logger::Level::Info);

    IcoRuntime runtime;
    bool ok = runtime.initialize();
    assert(ok);
    assert(runtime.isInitialized());

    assert(runtime.getMemory().isInitialized());
    assert(runtime.getFileSystem().getBasePath() == ".");

    u32 testAddr = 0x00100000;
    assert(runtime.getMemory().write32(testAddr, 0x12345678));
    u32 readVal = 0;
    assert(runtime.getMemory().read32(testAddr, readVal));
    assert(readVal == 0x12345678);

    runtime.getGameLoop().setUpdateCallback([&runtime](u32 frame) -> bool {
        Logger::info("smoke", "[frame %u] tick", frame);
        return true;
    });

    runtime.getGameLoop().run(3);
    assert(runtime.getGameLoop().getCurrentFrame() == 3);

    runtime.shutdown();
    assert(!runtime.isInitialized());

    std::fprintf(stderr, "[shutdown] clean exit\n");
    std::fprintf(stderr, "All tests passed.\n");
    return 0;
}
