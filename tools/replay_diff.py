#!/usr/bin/env python3
"""replay_diff.py — deterministic capture diff for the native PORT oracle.

Aligns two STATE DIGEST captures (PCSX2-side and native-side) by frame and
classifies divergence:

    POSITION_DRIFT    boy position differs beyond --eps (float tolerance)
    WORLD_STATE_*     current scene id diverges
    GOBJ_MISMATCH     active GObj count diverges
    INPUT_DIVERGE     pad consumed this frame diverged
    ACTIVE_TOGGLE     semantic motion flag diverged
    PROBE_MISSING     a gold frame has no candidate digest line
    UNCLASSIFIED      digest differs but no field comparison matched

Line grammar is identical on both producers (native ReplayRunner and the
future PCSX2 recorder):  D<frame> <hex16> scene=.. gobj=.. active=.. boy=.. pad=..

Exit code is 0 only when every divergence is whitelisted (--allow-class or
--allow). Stdlib-only, so it also runs as a CI self-test without a build.
"""

import re
import sys

FRAME_RE = re.compile(r"^D(\d+)$")

ALL_CLASSES = (
    "OK",
    "POSITION_DRIFT",
    "WORLD_STATE_DIVERGE",
    "GOBJ_MISMATCH",
    "INPUT_DIVERGE",
    "ACTIVE_TOGGLE",
    "PROBE_MISSING",
    "UNCLASSIFIED",
)


def parse_capture(path):
    """returns {frame: (hex, field-dict, rawline)} and ordered header keys."""
    frames = {}
    head = []
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n")
            if not line or line.startswith("#"):
                continue
            if not line.startswith("D"):
                head.append(line)
                continue
            tokens = line.split()
            m = FRAME_RE.match(tokens[0])
            if not m:
                raise ValueError(f"{path}: malformed digest line: {line!r}")
            frame = int(m.group(1))
            if len(tokens) < 2:
                raise ValueError(f"{path}: missing hex digest on frame {frame}")
            fields = {}
            for tok in tokens[2:]:
                if "=" in tok:
                    k, _, v = tok.partition("=")
                    fields[k] = v
            frames[frame] = (tokens[1].lower(), fields, line)
    return head, frames


def classify(gold, cand, eps):
    ghex, gfields, _ = gold
    chex, cfields, _ = cand
    if ghex == chex:
        return "OK", None
    if gfields.get("scene") != cfields.get("scene"):
        return "WORLD_STATE_DIVERGE", "scene"
    if gfields.get("gobj") != cfields.get("gobj"):
        return "GOBJ_MISMATCH", "gobj"
    if gfields.get("active") != cfields.get("active"):
        return "ACTIVE_TOGGLE", "active"
    try:
        gx, gy, gz = (float(v) for v in gfields["boy"].split(","))
        cx, cy, cz = (float(v) for v in cfields["boy"].split(","))
    except (KeyError, ValueError):
        return "UNCLASSIFIED", "boy"
    if abs(gx - cx) > eps or abs(gy - cy) > eps or abs(gz - cz) > eps:
        return "POSITION_DRIFT", "boy"
    if gfields.get("pad") != cfields.get("pad"):
        return "INPUT_DIVERGE", "pad"
    return "UNCLASSIFIED", None


def diff(gold, cand, eps, allow_classes=(), allow_substr=()):
    problems = []
    counts = {c: 0 for c in ALL_CLASSES}
    all_frames = sorted(set(gold) | set(cand))
    for frame in all_frames:
        g = gold.get(frame)
        c = cand.get(frame)
        if g is None:
            cls, field = "PROBE_MISSING", None
            detail = cand[frame][2] if c else ""
        elif c is None:
            cls, field = "PROBE_MISSING", None
            detail = g[2]
        else:
            cls, field = classify(g, c, eps)
            detail = c[2]
            if cls == "OK":
                counts["OK"] += 1
                continue
        if cls in allow_classes:
            counts[cls] += 1
            continue
        if any(sub in detail for sub in allow_substr):
            counts[cls] += 1
            continue
        problems.append((frame, cls, field, detail))
        counts[cls] += 1
    return problems, counts


def format_report(problems, counts, gold_path, cand_path):
    out = [f"gold={gold_path} candidate={cand_path}"]
    for cls in ALL_CLASSES:
        if counts.get(cls):
            out.append(f"  {cls}: {counts[cls]}")
    for frame, cls, field, detail in problems:
        out.append(f"  F{frame}: {cls}{'(' + field + ')' if field else ''} | {detail}")
    return "\n".join(out)


def run_selftest():
    gold_lines = [
        "D0 1111222233334444 scene=15 gobj=25 active=0 boy=0.0000,0.0000,0.0000 pad=0,0",
        "D1 1111222233334445 scene=15 gobj=25 active=1 boy=10.5000,0.0000,0.0000 pad=25,0",
        "D2 1111222233334446 scene=15 gobj=25 active=1 boy=19.9500,0.0000,0.0000 pad=25,0",
    ]
    _, gold = parse_raw(gold_lines)
    ok = True
    checks = []

    _, same = parse_raw(gold_lines)
    problems, counts = diff(gold, same, 0.01)
    checks.append(("identical->clean", len(problems) == 0))

    drift = gold_lines[:]
    drift[1] = drift[1].replace("boy=10.5000", "boy=12.5000")
    drift[1] = drift[1].replace("33334445", "33334456")
    _, cand = parse_raw(drift)
    problems, counts = diff(gold, cand, 0.01)
    checks.append(("drift->POSITION_DRIFT", [c for _, c, _, _ in problems] == ["POSITION_DRIFT"]))
    _, counts2 = diff(gold, cand, 5.0)
    checks.append(("drift eps-tolerant", problems and counts2["POSITION_DRIFT"] == 0 and counts["POSITION_DRIFT"] > 0))

    gobj_miss = gold_lines[:]
    gobj_miss[0] = gobj_miss[0].replace("gobj=25", "gobj=24")
    gobj_miss[0] = gobj_miss[0].replace("33334444", "33334447")
    _, cand = parse_raw(gobj_miss)
    problems, _ = diff(gold, cand, 0.01)
    checks.append(("gobj->GOBJ_MISMATCH", [c for _, c, _, _ in problems] == ["GOBJ_MISMATCH"]))

    world = gold_lines[:]
    world[0] = world[0].replace("scene=15", "scene=43")
    world[0] = world[0].replace("33334444", "33334448")
    _, cand = parse_raw(world)
    problems, _ = diff(gold, cand, 0.01)
    checks.append(("scene->WORLD_STATE_DIVERGE", [c for _, c, _, _ in problems] == ["WORLD_STATE_DIVERGE"]))

    missing = gold_lines[:2]
    _, cand = parse_raw(missing)
    problems, _ = diff(gold, cand, 0.01)
    checks.append(("missing->PROBE_MISSING", [c for _, c, _, _ in problems] == ["PROBE_MISSING"]))

    allow_cls, _ = diff(gold, cand, 0.01, allow_classes=("PROBE_MISSING",))
    checks.append(("allow-class whitelists", allow_cls == []))

    _, cand = parse_raw(drift)
    allow_sub, _ = diff(gold, cand, 0.01, allow_substr=("boy=12.5000",))
    checks.append(("allow-substr whitelists", allow_sub == []))

    for name, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {name}")
        ok = ok and passed
    return 0 if ok else 1


def parse_raw(lines):
    import tempfile
    import os
    fd, path = tempfile.mkstemp(suffix=".txt")
    try:
        os.write(fd, ("\n".join(lines)).encode())
        os.close(fd)
        return parse_capture(path)
    finally:
        try:
            os.unlink(path)
        except OSError:
            pass


def main(argv):
    if len(argv) >= 1 and argv[0] == "--selftest":
        return run_selftest()
    if len(argv) < 2:
        print(__doc__)
        return 2
    gold_path, cand_path = argv[0], argv[1]
    eps = 0.01
    allow_classes = []
    allow_substr = []
    i = 2
    while i < len(argv):
        if argv[i] == "--eps" and i + 1 < len(argv):
            eps = float(argv[i + 1])
            i += 2
        elif argv[i] == "--allow-class" and i + 1 < len(argv):
            allow_classes.append(argv[i + 1])
            i += 2
        elif argv[i] == "--allow" and i + 1 < len(argv):
            allow_substr.append(argv[i + 1])
            i += 2
        else:
            print(f"replay_diff: unknown flag {argv[i]}", file=sys.stderr)
            return 2
    try:
        _, gold = parse_capture(gold_path)
        _, cand = parse_capture(cand_path)
    except (OSError, ValueError) as exc:
        print(f"replay_diff: {exc}", file=sys.stderr)
        return 2
    problems, counts = diff(gold, cand, eps, allow_classes, allow_substr)
    print(format_report(problems, counts, gold_path, cand_path))
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))