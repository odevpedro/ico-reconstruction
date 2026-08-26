#pragma once

#include "ps2/Ps2Types.h"
#include "ps2/Ps2Memory.h"
#include "platform/Input.h"
#include "platform/FileSystem.h"
#include "platform/Timing.h"
#include "platform/AudioStub.h"
#include "platform/RenderStub.h"
#include "platform/SifRpcStub.h"
#include "platform/IopStub.h"
#include "runtime/GameLoop.h"
#include "game/IsysGObj.h"
#include "game/KanbanSceneLoader.h"

class IcoRuntime {
public:
    IcoRuntime();
    ~IcoRuntime();

    bool initialize();
    void shutdown();

    bool isInitialized() const;

    Ps2Memory& getMemory();
    Input& getInput();
    FileSystem& getFileSystem();
    Timing& getTiming();
    AudioStub& getAudio();
    RenderStub& getRender();
    SifRpcStub& getSifRpc();
    IopStub& getIop();
    GameLoop& getGameLoop();
    IsysGObj& getIsysGObj();
    KanbanSceneLoader& getSceneLoader();

private:
    bool m_initialized;

    Ps2Memory m_memory;
    Input m_input;
    FileSystem m_fileSystem;
    Timing m_timing;
    AudioStub m_audio;
    RenderStub m_render;
    SifRpcStub m_sifRpc;
    IopStub m_iop;
    GameLoop m_gameLoop;
    IsysGObj m_isysGObj;
    KanbanSceneLoader m_sceneLoader;
};
