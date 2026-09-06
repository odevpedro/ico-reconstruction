#!/usr/bin/env bash
# Pre-push verification for the native port: clean configure + build + CTest
# with assertions ENABLED (Debug build type). Release builds strip assert()
# via -DNDEBUG, which silently turns the assert-based tests into no-ops; this
# script exists so a trailing "21/21 ctest passed" means the asserts actually
# ran. Run it before any push on native-port.
#
# Usage:  scripts/pre-push-check.sh [--gl]
#   --gl   also build and run the OpenGL-backed target set (default off when
#          no display is available: the plain engine tests run headless).

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/native/build-check"
USE_GL=OFF

for arg in "$@"; do
    case "$arg" in
        --gl) USE_GL=ON ;;
        *) echo "unknown option: $arg" >&2; exit 2 ;;
    esac
done

echo "==> pre-push-check: clean build at ${BUILD_DIR}"
rm -rf "${BUILD_DIR}"

cmake -S "${ROOT}/native" -B "${BUILD_DIR}" \
    -DICO_ENABLE_OPENGL="${USE_GL}" \
    -DCMAKE_BUILD_TYPE=Debug

cmake --build "${BUILD_DIR}" -j"$(nproc)"

echo "==> ctest (Debug / assertions active)"
ctest --test-dir "${BUILD_DIR}" --output-on-failure

echo "==> pre-push-check: PASS"