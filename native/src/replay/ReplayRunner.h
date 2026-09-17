#pragma once

#include <string>
#include <vector>
#include "replay/ReplayFile.h"
#include "replay/StateDigest.h"

namespace ico::replay {

// Deterministic headless replay runner (GL-free). Consumes a ReplayFile
// through BoyController + verified scene tables and emits per-frame digest
// lines. Used both as ico_native --replay and as the core of the CTest
// reproducibility proof.
struct ReplayRunnerConfig {
    std::string replayPath;
    std::string capturePath; // written; empty → in-memory only (used by tests)
};

// Returns a vector of digest lines (one per frame, starting at frame 0).
// On error fills `error` and returns empty.
std::vector<std::string> runReplayDeterministic(
    const ReplayData& data,
    ReplayRunnerConfig config,
    std::string& error);

}  // namespace ico::replay