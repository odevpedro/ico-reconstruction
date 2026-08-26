cmake_minimum_required(VERSION 3.14)
project(ico_native VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(ico_engine_core
    src/engine/GObj.cpp
    src/engine/GObjPool.cpp
    src/engine/IsysGObjRuntime.cpp
)
target_include_directories(ico_engine_core PUBLIC
    src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

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
target_link_libraries(ico_native PRIVATE ico_engine_core)

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
target_link_libraries(smoke_test PRIVATE ico_engine_core)

add_executable(gobj_pool_test tests/gobj_pool_test.cpp)
target_link_libraries(gobj_pool_test PRIVATE ico_engine_core)

add_executable(isysgobj_runtime_test tests/isysgobj_runtime_test.cpp)
target_link_libraries(isysgobj_runtime_test PRIVATE ico_engine_core)

enable_testing()
add_test(NAME smoke COMMAND smoke_test)
add_test(NAME gobj_pool COMMAND gobj_pool_test)
add_test(NAME isysgobj_runtime COMMAND isysgobj_runtime_test)
