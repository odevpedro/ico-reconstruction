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

}  // namespace

int main() {
    std::string samplePath;
    CHECK(locateReplay("sample-sala-0x0F.replay", samplePath));

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

    /* World events are addressable per frame (base fixture: E0 WORLD 0x0F). */
    const ico::replay::ReplayWorldEvent* e0 = ReplayFile::eventAtFrame(parsed, 0);
    CHECK(e0 != nullptr);
    CHECK(e0->worldState == 0x0Fu);
    CHECK(e0->sceneId == 0x0Fu);
    CHECK(ReplayFile::eventAtFrame(parsed, 1) == nullptr);
    CHECK(ReplayFile::eventAtFrame(parsed, 40) == nullptr);

    /* Multi-room fixture: event at frame 5 → scene swap mid-replay. */
    std::string multiPath;
    CHECK(locateReplay("sample-sala-0x0F-to-0x2B.replay", multiPath));
    ReplayData multi;
    CHECK(ReplayFile::parse(multiPath, multi, err));
    CHECK(multi.header.worldStateInit == 0x0Fu);
    CHECK(multi.pads.size() == 12);
    CHECK(multi.events.size() == 1);
    CHECK(multi.events[0].frame == 5);
    CHECK(ReplayFile::estimatedFrameCount(multi) == 12);
    const ico::replay::ReplayWorldEvent* e5 = ReplayFile::eventAtFrame(multi, 5);
    CHECK(e5 != nullptr);
    CHECK(e5->worldState == 0x2Bu);
    CHECK(e5->sceneId == 0x2Bu);
    CHECK(ReplayFile::eventAtFrame(multi, 4) == nullptr);

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