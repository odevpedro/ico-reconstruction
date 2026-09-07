#!/bin/bash
# ICO Native Demo Runner - automatically loads the STGST00A room mesh
cd "$(dirname "$0")"
cd native

# Default: infinite loop, real-time rendering
exec ./build/ico_native --frames 0 --p2o assets/170_st00a_p1.p2o --tm2 assets/st0_a.tm2 "$@"
