#include "replay/ReplayFile.h"
#include "replay/ReplayRunner.h"
#include "replay/StateDigest.h"

#define CHECK(c)                                                             \
    do {                                                                     \
        if (!(c)) {                                                          \
            std::fprintf(stderr, "CHECK FAILED %s:%d: %s\n", __FILE__,      \
                         __LINE__, #c);                                      \
            std::abort();                                                    \
        }                                                                    \
    } while (0)

#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using ico::replay::ReplayData;
using ico::replay::ReplayFile;
using ico::replay::ReplayRunnerConfig;
using ico::replay::runReplayDeterministic;

namespace {

bool locateSample(std::string& path) {
    const char* candidates[] = {
        "../native/replays/sample-sala-0x0F.replay",
        "native/replays/sample-sala-0x0F.replay",
        "../../native/replays/sample-sala-0x0F.replay",
        "../replays/sample-sala-0x0F.replay",
        nullptr,
    };
    for (int i = 0; candidates[i] != nullptr; ++i) {
        std::ifstream f(candidates[i]);
        if (f.good()) {
            path = candidates[i];
            return true;
        }
    }
    return false;
}

std::string slurp(const char* path) {
    std::ifstream f(path, std::ios::binary);
    std::string s((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return s;
}

ReplayData withPadAt(ReplayData d, uint32_t frame, uint32_t pad) {
    for (auto& p : d.pads)
        if (p.frame == frame) p.pad = pad;
    return d;
}

}  // namespace

int main() {
    std::string samplePath;
    CHECK(locateSample(samplePath));

    ReplayData data;
    std::string err;
    CHECK(ReplayFile::parse(samplePath, data, err));

    /* Run the same replay twice → byte-identical capture (reproducible). */
    const std::string cap1 = "/tmp/replay_cap1.txt";
    const std::string cap2 = "/tmp/replay_cap2.txt";
    std::vector<std::string> lines1 =
        runReplayDeterministic(data, ReplayRunnerConfig{samplePath, cap1}, err);
    CHECK(!lines1.empty() && err.empty());
    std::vector<std::string> lines2 =
        runReplayDeterministic(data, ReplayRunnerConfig{samplePath, cap2}, err);
    CHECK(!lines2.empty() && err.empty());
    CHECK(lines1.size() == 12);
    CHECK(lines1 == lines2);
    CHECK(slurp(cap1.c_str()) == slurp(cap2.c_str()));
    CHECK(lines1[0].rfind("D0 ", 0) == 0);
    CHECK(lines1[11].rfind("D11 ", 0) == 0);

    /* Movement sensitivity: flipping frame 3's pad bit changes the digest. */
    ReplayData turned = withPadAt(data, 3, 0x2u);
    std::vector<std::string> turnedLines =
        runReplayDeterministic(turned, ReplayRunnerConfig{samplePath, "/tmp/replay_cap3.txt"}, err);
    CHECK(!turnedLines.empty() && err.empty());
    CHECK(turnedLines[3] != lines1[3]);
    CHECK(turnedLines[0] == lines1[0]);  // no input yet on frame 0

    std::printf("replay_runner_test: OK (12 frames, deterministic, sensitive)\n");
    return 0;
}