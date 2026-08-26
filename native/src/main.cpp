#include "runtime/IcoRuntime.h"
#include "runtime/Logger.h"
#include <cstdio>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::fprintf(stderr, "ICO Native Runtime Prototype\n");

    Logger::setLevel(Logger::Level::Info);

    IcoRuntime runtime;
    if (!runtime.initialize()) {
        Logger::error("main", "Failed to initialize runtime");
        return 1;
    }

    runtime.getGameLoop().setUpdateCallback([&runtime](u32 frame) -> bool {
        Logger::info("main", "[frame %u] tick", frame);
        return true;
    });

    runtime.getGameLoop().run(3);

    runtime.shutdown();

    std::fprintf(stderr, "[shutdown] clean exit\n");
    return 0;
}
