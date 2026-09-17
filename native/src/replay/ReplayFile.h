#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ico::replay {

// Replay container v1 (textual, versioned). One logical input trace plus a
// save-state/ISO fingerprint so a future PCSX2-side recorder and the native
// runner share one deterministic input contract.
//
// Line grammar (blank lines and '#' comments ignored):
//   VERSION <n>
//   ISO_SHA1 <40hex>          `-` when unknown
//   SAVE_STATE <64hex>        `-` when unknown
//   WORLD_STATE_INIT 0xNN
//   FRAME_REF <u64>           informational; logical frames always start at 0
//   F<frame> P<hex8>          pad word consumed on <frame>
//   E<frame> WORLD <0xNN> <sceneId>
//
// Pad word v1 (host-defined; the PS2 recorder maps real pad to this nibble):
//   bit 0 = +x, bit 1 = -x, bit 2 = +z, bit 3 = -z; magnitude 25 (walk tier).
inline constexpr uint32_t kReplayFormatVersion = 1;

struct ReplayHeader {
    uint32_t version = kReplayFormatVersion;
    std::string isoSha1 = "-";
    std::string saveState = "-";
    uint32_t worldStateInit = 0x0Fu;
    uint64_t frameRef = 0;
};

struct ReplayFramePad {
    uint32_t frame = 0;
    uint32_t pad = 0;
};

struct ReplayWorldEvent {
    uint32_t frame = 0;
    uint32_t worldState = 0;
    uint32_t sceneId = 0;
};

struct ReplayData {
    ReplayHeader header;
    std::vector<ReplayFramePad> pads;      // sorted by frame, gaps allowed (0)
    std::vector<ReplayWorldEvent> events;
};

class ReplayFile {
public:
    // Parses a .replay file. Returns false (with err filled) on malformed
    // input; unknown keywords are fatal so a future format bump cannot be
    // silently misread by a v1 reader.
    static bool parse(const std::string& path, ReplayData& out, std::string& err);

    // Convenience: pad for a given logical frame, 0 when absent.
    static uint32_t padForFrame(const ReplayData& data, uint32_t frame);

    // Serializes data back; used by tests to round-trip and to generate the
    // sample fixture.
    static bool save(const std::string& path, const ReplayData& data, std::string& err);

    // Human-readable size for logging.
    static std::size_t estimatedFrameCount(const ReplayData& data);
};

}  // namespace ico::replay