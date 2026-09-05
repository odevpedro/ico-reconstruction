cmake_minimum_required(VERSION 3.14)
project(ico_native VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ICO_ENABLE_OPENGL "Enable OpenGL rendering backend (requires GL + X11)" OFF)

set(ICO_ENGINE_SOURCES
    src/game/KanbanSceneLoader.cpp
    src/game/GirlBrainSemantic.cpp
    src/engine/GObj.cpp
    src/engine/GObjPool.cpp
    src/engine/ProcessNodePool.cpp
    src/engine/IsysGObjRuntime.cpp
    src/engine/RenderBackend.cpp
    src/engine/Tm2Format.cpp
    src/engine/Tm2Converter.cpp
    src/engine/GifTag.cpp
    src/engine/GifCommandBuffer.cpp
    src/engine/GifCommandExecutor.cpp
    src/engine/GifPacket.cpp
    src/engine/WorldStateLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core/isysgobj_semantic.c
)

if(ICO_ENABLE_OPENGL)
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    list(APPEND ICO_ENGINE_SOURCES src/engine/OpenGLBackend.cpp)
endif()

add_library(ico_engine_core ${ICO_ENGINE_SOURCES})
target_include_directories(ico_engine_core PUBLIC
    src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

if(ICO_ENABLE_OPENGL)
    target_link_libraries(ico_engine_core PUBLIC OpenGL::GL X11::X11 ${CMAKE_DL_LIBS})
    target_compile_definitions(ico_engine_core PUBLIC ICO_HAS_OPENGL=1)
endif()

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

add_executable(isysgobj_semantic_test tests/isysgobj_semantic_test.c)
target_link_libraries(isysgobj_semantic_test PRIVATE ico_engine_core)

add_executable(isysgobj_semantic_runtime_test tests/isysgobj_semantic_runtime_test.cpp)
target_link_libraries(isysgobj_semantic_runtime_test PRIVATE ico_engine_core)

add_executable(process_node_pool_test tests/process_node_pool_test.cpp)
target_link_libraries(process_node_pool_test PRIVATE ico_engine_core)

add_executable(isysgobj_process_test tests/isysgobj_process_test.cpp)
target_link_libraries(isysgobj_process_test PRIVATE ico_engine_core)

add_executable(slot_dispatch_test tests/slot_dispatch_test.cpp)
target_link_libraries(slot_dispatch_test PRIVATE ico_engine_core)

add_executable(render_backend_test tests/render_backend_test.cpp)
target_link_libraries(render_backend_test PRIVATE ico_engine_core)

add_executable(tm2_test tests/tm2_test.cpp)
target_link_libraries(tm2_test PRIVATE ico_engine_core)

add_executable(gif_command_test tests/gif_command_test.cpp)
target_link_libraries(gif_command_test PRIVATE ico_engine_core)

add_executable(gif_executor_test tests/gif_executor_test.cpp)
target_link_libraries(gif_executor_test PRIVATE ico_engine_core)

add_executable(gif_packet_test tests/gif_packet_test.cpp)
target_link_libraries(gif_packet_test PRIVATE ico_engine_core)

add_executable(scene_loader_test tests/scene_loader_test.cpp)
target_link_libraries(scene_loader_test PRIVATE ico_engine_core)

add_executable(game_loop_scene_test tests/game_loop_scene_test.cpp
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
target_include_directories(game_loop_scene_test PRIVATE src)
target_link_libraries(game_loop_scene_test PRIVATE ico_engine_core)

add_executable(girlbrain_semantic_test tests/girlbrain_semantic_test.cpp)
target_link_libraries(girlbrain_semantic_test PRIVATE ico_engine_core)

add_executable(world_state_slot_test tests/world_state_slot_test.cpp)
target_link_libraries(world_state_slot_test PRIVATE ico_engine_core)

add_executable(world_state_loader_test tests/world_state_loader_test.cpp)
target_link_libraries(world_state_loader_test PRIVATE ico_engine_core)

enable_testing()
add_test(NAME smoke COMMAND smoke_test)
add_test(NAME gobj_pool COMMAND gobj_pool_test)
add_test(NAME isysgobj_runtime COMMAND isysgobj_runtime_test)
add_test(NAME isysgobj_semantic COMMAND isysgobj_semantic_test)
add_test(NAME isysgobj_semantic_runtime COMMAND isysgobj_semantic_runtime_test)
add_test(NAME process_node_pool COMMAND process_node_pool_test)
add_test(NAME isysgobj_process COMMAND isysgobj_process_test)
add_test(NAME slot_dispatch COMMAND slot_dispatch_test)
add_test(NAME render_backend COMMAND render_backend_test)
add_test(NAME tm2 COMMAND tm2_test)
add_test(NAME gif_command COMMAND gif_command_test)
add_test(NAME gif_executor COMMAND gif_executor_test)
add_test(NAME gif_packet COMMAND gif_packet_test)
add_test(NAME scene_loader COMMAND scene_loader_test)
add_test(NAME game_loop_scene COMMAND game_loop_scene_test)
add_test(NAME girlbrain_semantic COMMAND girlbrain_semantic_test)
add_test(NAME world_state_slot COMMAND world_state_slot_test)
add_test(NAME world_state_loader COMMAND world_state_loader_test)

if(ICO_ENABLE_OPENGL)
    add_executable(opengl_backend_test tests/opengl_backend_test.cpp)
    target_link_libraries(opengl_backend_test PRIVATE ico_engine_core)
    add_test(NAME opengl_backend COMMAND opengl_backend_test)
endif()
