#include "replay/ReplayFile.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

namespace ico::replay {

namespace {

bool startsWith(const std::string& s, const char* prefix) {
    const std::size_t n = std::strlen(prefix);
    return s.size() >= n && s.compare(0, n, prefix) == 0;
}

bool trimAndComment(std::string& line) {
    // Strip trailing / leading whitespace; a leading '#' is a comment.
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' '))
        line.pop_back();
    std::size_t first = 0;
    while (first < line.size() && std::isspace(static_cast<unsigned char>(line[first])))
        ++first;
    line = line.substr(first);
    if (line.empty() || line[0] == '#') {
        line.clear();
        return true;
    }
    return false;
}

bool parseU32Hex(const std::string& s, uint32_t& out) {
    if (s.empty()) return false;
    std::size_t i = 0;
    std::size_t baseOffset = (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) ? 2 : 0;
    unsigned long v = 0;
    for (i = baseOffset; i < s.size(); ++i) {
        const char c = s[i];
        int d = -1;
        if (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
        if (d < 0) return false;
        if (v > 0x0FFFFFFFull) return false;
        v = (v << 4) | static_cast<unsigned long>(d);
    }
    if (i == baseOffset) return false;
    out = static_cast<uint32_t>(v);
    return true;
}

bool parseU64(const std::string& s, uint64_t& out) {
    if (s.empty()) return false;
    uint64_t v = 0;
    for (char c : s) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        v = v * 10 + static_cast<uint64_t>(c - '0');
    }
    out = v;
    return true;
}

bool parseFrameAndPayload(const std::string& field, uint32_t& frame) {
    if (field.size() < 2 || field[0] != 'F') return false;
    std::string num = field.substr(1);
    if (num.empty() || num[0] < '0' || num[0] > '9') return false;
    uint64_t f = 0;
    for (char c : num) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        f = f * 10 + static_cast<uint64_t>(c - '0');
    }
    if (f > 0xFFFFFFFFull) return false;
    frame = static_cast<uint32_t>(f);
    return true;
}

// Frame field: "F<frame>"; payload "P<hex8>" is parsed separately per token.
bool parsePadToken(const std::string& token, uint32_t& pad) {
    if (token.size() < 2 || (token[0] != 'P' && token[0] != 'p')) return false;
    const std::string hex = token.substr(1);
    if (hex.empty() || hex.size() > 8) return false;
    uint32_t v = 0;
    for (char c : hex) {
        int d = -1;
        if (c >= '0' && c <= '9') d = c - '0';
        else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
        if (d < 0) return false;
        v = (v << 4) | static_cast<uint32_t>(d);
    }
    pad = v;
    return true;
}

}  // namespace

bool ReplayFile::parse(const std::string& path, ReplayData& out, std::string& err) {
    std::ifstream f(path);
    if (!f) {
        err = "cannot open " + path;
        return false;
    }
    ReplayData data;
    std::string line;
    std::size_t lineNo = 0;
    bool seenVersion = false;
    while (std::getline(f, line)) {
        ++lineNo;
        if (trimAndComment(line)) continue;
        std::istringstream ss(line);
        std::string kw;
        ss >> kw;
        if (kw == "VERSION") {
            uint64_t v = 0;
            std::string rest;
            ss >> rest;
            if (!parseU64(rest, v)) {
                err = "line " + std::to_string(lineNo) + ": bad VERSION";
                return false;
            }
            data.header.version = static_cast<uint32_t>(v);
            if (data.header.version != kReplayFormatVersion) {
                err = "line " + std::to_string(lineNo) + ": unsupported format version " +
                      std::to_string(v);
                return false;
            }
            seenVersion = true;
        } else if (kw == "ISO_SHA1") {
            std::string rest;
            ss >> rest;
            data.header.isoSha1 = (rest == "-") ? "-" : rest;
        } else if (kw == "SAVE_STATE") {
            std::string rest;
            ss >> rest;
            data.header.saveState = (rest == "-") ? "-" : rest;
        } else if (kw == "WORLD_STATE_INIT") {
            std::string rest;
            ss >> rest;
            if (!parseU32Hex(rest, data.header.worldStateInit)) {
                err = "line " + std::to_string(lineNo) + ": bad WORLD_STATE_INIT";
                return false;
            }
        } else if (kw == "FRAME_REF") {
            std::string rest;
            ss >> rest;
            if (!parseU64(rest, data.header.frameRef)) {
                err = "line " + std::to_string(lineNo) + ": bad FRAME_REF";
                return false;
            }
        } else if (kw[0] == 'F') {
            uint32_t frame = 0;
            if (!parseFrameAndPayload(kw, frame)) {
                err = "line " + std::to_string(lineNo) + ": bad frame token '" + kw + "'";
                return false;
            }
            uint32_t pad = 0;
            std::string tok;
            while (ss >> tok) {
                if (tok[0] == '#') break;
                if (!parsePadToken(tok, pad)) {
                    err = "line " + std::to_string(lineNo) + ": bad pad token '" + tok + "'";
                    return false;
                }
            }
            data.pads.push_back(ReplayFramePad{frame, pad});
        } else if (kw[0] == 'E') {
            // Event lines are one token: "E<frame> WORLD <ws hex> <scene hex>".
            uint32_t frame = 0;
            if (!parseFrameAndPayload("F" + kw.substr(1), frame)) {
                err = "line " + std::to_string(lineNo) + ": bad event frame token '" +
                      kw.substr(1) + "'";
                return false;
            }
            std::string rest;
            ss >> rest;  // WORLD
            if (rest != "WORLD") {
                err = "line " + std::to_string(lineNo) + ": expected WORLD";
                return false;
            }
            std::string wsTok, sceneTok;
            ss >> wsTok >> sceneTok;
            uint32_t ws = 0, scene = 0;
            if (!parseU32Hex(wsTok, ws) || !parseU32Hex(sceneTok, scene)) {
                err = "line " + std::to_string(lineNo) + ": bad WORLD event";
                return false;
            }
            data.events.push_back(ReplayWorldEvent{frame, ws, scene});
        } else {
            err = "line " + std::to_string(lineNo) + ": unknown keyword '" + kw + "'";
            return false;
        }
    }
    if (!seenVersion) {
        err = "missing VERSION line";
        return false;
    }
    out = std::move(data);
    return true;
}

uint32_t ReplayFile::padForFrame(const ReplayData& data, uint32_t frame) {
    for (const auto& p : data.pads) {
        if (p.frame == frame) return p.pad;
        if (p.frame > frame) break;
    }
    return 0;
}

bool ReplayFile::save(const std::string& path, const ReplayData& data, std::string& err) {
    std::ofstream f(path);
    if (!f) {
        err = "cannot write " + path;
        return false;
    }
    f << "VERSION " << data.header.version << "\n";
    f << "ISO_SHA1 " << data.header.isoSha1 << "\n";
    f << "SAVE_STATE " << data.header.saveState << "\n";
    f << "WORLD_STATE_INIT 0x" << std::hex << data.header.worldStateInit << std::dec << "\n";
    f << "FRAME_REF " << data.header.frameRef << "\n";
    for (const auto& p : data.pads)
        f << "F" << p.frame << " P" << std::hex << p.pad << std::dec << "\n";
    for (const auto& e : data.events)
        f << "E" << e.frame << " WORLD 0x" << std::hex << e.worldState << " " << e.sceneId
          << std::dec << "\n";
    return true;
}

std::size_t ReplayFile::estimatedFrameCount(const ReplayData& data) {
    std::size_t n = 0;
    for (const auto& p : data.pads) {
        if (p.frame >= n) n = static_cast<std::size_t>(p.frame) + 1;
    }
    return n;
}

}  // namespace ico::replay