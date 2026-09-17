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

bool locateReplay(const char* name, std::string& path) {
    const std::string rel = std::string("../native/replays/") + name;
    const std::string rel2 = std::string("native/replays/") + name;
    const std::string rel3 = std::string("../../native/replays/") + name;
    const std::string rel4 = std::string("../replays/") + name;
    for (const std::string& c : {rel, rel2, rel3, rel4}) {
        std::ifstream f(c);
        if (f.good()) {
            path = c;
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
    CHECK(locateReplay("sample-sala-0x0F.replay", samplePath));

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

    /* Multi-room fixture: scene swap mid-replay (0x0F -> 0x2B on frame 5). */
    std::string multiPath;
    CHECK(locateReplay("sample-sala-0x0F-to-0x2B.replay", multiPath));
    ReplayData multi;
    CHECK(ReplayFile::parse(multiPath, multi, err));
    std::vector<std::string> mlines1 =
        runReplayDeterministic(multi, ReplayRunnerConfig{multiPath, "/tmp/replay_multi1.txt"}, err);
    CHECK(!mlines1.empty() && err.empty());
    std::vector<std::string> mlines2 =
        runReplayDeterministic(multi, ReplayRunnerConfig{multiPath, "/tmp/replay_multi2.txt"}, err);
    CHECK(!mlines2.empty() && err.empty());
    CHECK(mlines1.size() == 12);
    CHECK(mlines1 == mlines2);  // scene swap is reproducible

    auto sceneField = [](const std::string& l) {
        const std::size_t p = l.find("scene=");
        return l.substr(p + 6, l.find(' ', p) - (p + 6));
    };
    CHECK(sceneField(mlines1[0]) == "15");
    CHECK(sceneField(mlines1[4]) == "15");
    CHECK(sceneField(mlines1[5]) == "43");   // E5 consumed → st02a resident
    CHECK(sceneField(mlines1[11]) == "43");

    std::printf("replay_runner_test: OK (12 frames, deterministic, sensitive)\n");
    return 0;
}