#!/usr/bin/env bash
# Multi-piece scene demo for the ICO native port.
#
# Loads the 9 static st00a pieces (door, p1, p2, torch x3, window flare/glow,
# bridge) from assets/scene/pieces/, one .p2o per piece, with per-piece
# material->TM2 textures resolved by name from assets/scene/texture/.
#
# Usage:
#   ./run_native_demo.sh            # live 60fps window, orbiting the p1 room
#   ./run_native_demo.sh --shot F   # one frame, write PPM to /tmp/F
#   ./run_native_demo.sh --angle D  # camera azimuth in degrees (for stills)
set -euo pipefail
cd "$(dirname "$0")"

FRAMES=0
SHOT=""
ANGLE=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        --shot|--frames|--angle) arg="$1"; shift; val="$1"; shift ;;
        *) echo "unknown: $1" >&2; exit 2 ;;
    esac
    case "$arg" in
        --shot)   SHOT="$val" ;;
        --frames) FRAMES="$val" ;;
        --angle)  ANGLE="--cam-angle $val" ;;
    esac
done

EXTRA=()
[[ -n "$SHOT" ]] && EXTRA+=(--shot "$SHOT")
[[ -n "$ANGLE" ]] && EXTRA+=(--cam-angle "$ANGLE")
exec ./build/ico_native --scene assets/scene/pieces --frames "$FRAMES" "${EXTRA[@]}"