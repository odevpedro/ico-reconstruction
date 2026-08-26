cmake_minimum_required(VERSION 3.14)
project(ico_native VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source files
set(ICO_NATIVE_SOURCES
    src/main.cpp
    src/runtime/IcoRuntime.cpp
    src/runtime/GameLoop.cpp
    src/runtime/Logger.cpp
    src/platform/Platform.cpp
    src/platform/Input.cpp
    src/platform/FileSystem.cpp
    src/platform/Timing.cpp
    src/platform/AudioStub.cpp
    src/platform/RenderStub.cpp
    src/platform/SifRpcStub.cpp
    src/platform/IopStub.cpp
    src/ps2/Ps2Memory.cpp
)

# Main executable
add_executable(ico_native ${ICO_NATIVE_SOURCES})
target_include_directories(ico_native PRIVATE src)

# Smoke test
add_executable(smoke_test tests/smoke_test.cpp
    src/runtime/Logger.cpp
    src/runtime/GameLoop.cpp
    src/runtime/IcoRuntime.cpp
    src/platform/Platform.cpp
    src/platform/Input.cpp
    src/platform/FileSystem.cpp
    src/platform/Timing.cpp
    src/platform/AudioStub.cpp
    src/platform/RenderStub.cpp
    src/platform/SifRpcStub.cpp
    src/platform/IopStub.cpp
    src/ps2/Ps2Memory.cpp
)
target_include_directories(smoke_test PRIVATE src)

enable_testing()
add_test(NAME smoke COMMAND smoke_test)
