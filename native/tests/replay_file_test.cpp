#include "replay/ReplayFile.h"

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
#include <string>

using ico::replay::ReplayData;
using ico::replay::ReplayFile;

namespace {

// Replay directory relative to the CTest build dir; try several layouts so the
// test does not depend on one workspace shape.
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

}  // namespace

int main() {
    std::string samplePath;
    CHECK(locateSample(samplePath));

    ReplayData parsed;
    std::string err;
    CHECK(ReplayFile::parse(samplePath, parsed, err));
    CHECK(parsed.header.version == 1);
    CHECK(parsed.header.worldStateInit == 0x0Fu);
    CHECK(parsed.pads.size() == 12);
    CHECK(parsed.events.size() == 1);
    CHECK(parsed.events[0].worldState == 0x0Fu);
    CHECK(parsed.events[0].sceneId == 0x0Fu);
    /* Pads sorted and preserved; frame 3 = +x bit. */
    CHECK(parsed.pads[3].frame == 3);
    CHECK(parsed.pads[3].pad == 0x1u);
    CHECK(ReplayFile::padForFrame(parsed, 3) == 0x1u);
    CHECK(ReplayFile::padForFrame(parsed, 0) == 0x0u);
    CHECK(ReplayFile::padForFrame(parsed, 200) == 0x0u);  // gap → 0
    CHECK(ReplayFile::estimatedFrameCount(parsed) == 12);

    /* Round-trip through the writer. */
    const std::string tmp = "/tmp/replay_rt_test.replay";
    CHECK(ReplayFile::save(tmp, parsed, err));
    ReplayData back;
    CHECK(ReplayFile::parse(tmp, back, err));
    CHECK(back.header.version == parsed.header.version);
    CHECK(back.header.worldStateInit == parsed.header.worldStateInit);
    CHECK(back.pads.size() == parsed.pads.size());
    CHECK(back.events.size() == parsed.events.size());
    for (std::size_t i = 0; i < back.pads.size(); ++i) {
        CHECK(back.pads[i].frame == parsed.pads[i].frame);
        CHECK(back.pads[i].pad == parsed.pads[i].pad);
    }

    /* Malformed input must fail loudly. */
    const std::string bad = "/tmp/replay_bad_test.replay";
    {
        std::ofstream f(bad);
        f << "VERSION 1\nF0 Pzz\n";  // non-hex pad
    }
    CHECK(!ReplayFile::parse(bad, back, err) && !err.empty());
    {
        std::ofstream f(bad);
        f << "VERSION 1\nBOGUS 5\n";  // unknown keyword
    }
    CHECK(!ReplayFile::parse(bad, back, err));
    {
        std::ofstream f(bad);
        f << "VERSION 99\n";  // unsupported format
    }
    CHECK(!ReplayFile::parse(bad, back, err));
    {
        std::ofstream f(bad);
        f << "VERSION 1\nE0 WORLD zz 0x0F\n";  // bad ws hex in event
    }
    CHECK(!ReplayFile::parse(bad, back, err));

    std::printf("replay_file_test: OK\n");
    return 0;
}