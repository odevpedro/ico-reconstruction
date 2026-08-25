#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

PREFLIGHT_ONLY=0
if [[ "${1:-}" == "--check" ]]; then
  PREFLIGHT_ONLY=1
  shift
fi

# Persistent defaults: everything lives below the repository instead of /tmp.
PCSX2_ROOT="${PCSX2_ROOT:-$PROJECT_ROOT/.local/pcsx2-ico-logpoints-fork}"
PCSX2_DEPS="${PCSX2_DEPS:-$PROJECT_ROOT/.local/pcsx2-fork-deps}"
PCSX2_BIN="${PCSX2_BIN:-$PCSX2_ROOT/build-runtime/bin/pcsx2-qt}"
# This fork boots the raw BIN reliably; its CUE detection failed in the validated session.
ICO_ISO_PATH="${ICO_ISO_PATH:-$PROJECT_ROOT/.local/iso/Ico (USA).bin}"

SESSION_STAMP="$(date +%Y%m%d-%H%M%S)"
ICO_PCSX2_PROBE_SESSION="${ICO_PCSX2_PROBE_SESSION:-ico-runtime-$SESSION_STAMP}"
ICO_PCSX2_PROBE_LOG="${ICO_PCSX2_PROBE_LOG:-$PROJECT_ROOT/.local/pcsx2-logs/$ICO_PCSX2_PROBE_SESSION.jsonl}"
export ICO_PCSX2_PROBE_LOG ICO_PCSX2_PROBE_SESSION

PCSX2_LOCAL_ROOT="${PCSX2_LOCAL_ROOT:-$PROJECT_ROOT/.local/pcsx2-runtime}"
export XDG_CONFIG_HOME="${XDG_CONFIG_HOME:-$PCSX2_LOCAL_ROOT/config}"
export XDG_DATA_HOME="${XDG_DATA_HOME:-$PCSX2_LOCAL_ROOT/data}"
export XDG_CACHE_HOME="${XDG_CACHE_HOME:-$PCSX2_LOCAL_ROOT/cache}"

export LD_LIBRARY_PATH="$PCSX2_DEPS/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="${QT_PLUGIN_PATH:-$PCSX2_DEPS/plugins}"

if [[ ! -x "$PCSX2_BIN" ]]; then
  echo "ERROR: PCSX2 binary not found or not executable: $PCSX2_BIN" >&2
  echo "Build it first, or set PCSX2_BIN=/path/to/pcsx2-qt." >&2
  exit 1
fi

if [[ ! -f "$ICO_ISO_PATH" ]]; then
  echo "ERROR: ICO image not found: $ICO_ISO_PATH" >&2
  echo "Set ICO_ISO_PATH=/path/to/Ico.cue or /path/to/Ico.iso." >&2
  exit 1
fi

if ! grep -a -q 'halfword_second_caller_entry' "$PCSX2_BIN"; then
  echo "ERROR: PCSX2 binary does not contain the expected ICO runtime probes: $PCSX2_BIN" >&2
  echo "Rebuild the instrumented fork before starting a gameplay session." >&2
  exit 1
fi

if [[ "$PREFLIGHT_ONLY" == "1" ]]; then
  echo "READY: persistent ICO gameplay runtime is available."
  echo "PCSX2: $PCSX2_BIN"
  echo "Game:  $ICO_ISO_PATH"
  echo "Deps:  $PCSX2_DEPS"
  exit 0
fi

mkdir -p "$XDG_CONFIG_HOME" "$XDG_DATA_HOME" "$XDG_CACHE_HOME" "$(dirname "$ICO_PCSX2_PROBE_LOG")"

if [[ -s "$ICO_PCSX2_PROBE_LOG" ]]; then
  echo "ERROR: refusing to append a new session to non-empty log: $ICO_PCSX2_PROBE_LOG" >&2
  echo "Use a new ICO_PCSX2_PROBE_SESSION or ICO_PCSX2_PROBE_LOG." >&2
  exit 1
fi

touch "$ICO_PCSX2_PROBE_LOG"

CURRENT_SESSION_FILE="$PROJECT_ROOT/.local/pcsx2-logs/current-session.env"
{
  printf 'ICO_PCSX2_PROBE_SESSION=%q\n' "$ICO_PCSX2_PROBE_SESSION"
  printf 'ICO_PCSX2_PROBE_LOG=%q\n' "$ICO_PCSX2_PROBE_LOG"
  printf 'PCSX2_BIN=%q\n' "$PCSX2_BIN"
  printf 'ICO_ISO_PATH=%q\n' "$ICO_ISO_PATH"
} > "$CURRENT_SESSION_FILE"
ln -sfn "$(basename "$ICO_PCSX2_PROBE_LOG")" "$PROJECT_ROOT/.local/pcsx2-logs/current-session.jsonl"

echo "PCSX2: $PCSX2_BIN"
echo "Game:  $ICO_ISO_PATH"
echo "Session: $ICO_PCSX2_PROBE_SESSION"
echo "Probe log: $ICO_PCSX2_PROBE_LOG"
echo "Current-session manifest: $CURRENT_SESSION_FILE"
echo "Local runtime config: $PCSX2_LOCAL_ROOT"
echo
echo "If PCSX2 asks for BIOS, place your legally dumped PS2 BIOS in the BIOS folder selected by the setup wizard."
echo

exec "$PCSX2_BIN" -batch -fastboot "$ICO_ISO_PATH" "$@"
