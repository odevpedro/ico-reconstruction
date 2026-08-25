#!/usr/bin/env bash
set -euo pipefail

echo "==> Installing Ubuntu dependencies for the ICO PCSX2 instrumented build"
echo "    sudo may ask for your password."

sudo apt update

sudo apt install -y \
  build-essential cmake ninja-build git curl ca-certificates pkg-config python3 \
  clang lld ccache patchelf extra-cmake-modules

sudo apt install -y \
  libfreetype-dev libfontconfig-dev libfontconfig1-dev libharfbuzz-dev \
  libbrotli-dev libpcre2-dev \
  libdbus-1-dev libudev-dev libcurl4-openssl-dev libpcap0.8-dev libssl-dev

sudo apt install -y \
  libgl-dev libegl-dev libglx-dev libopengl-dev libgl1-mesa-dev \
  mesa-common-dev libglvnd-dev libvulkan-dev

sudo apt install -y \
  libx11-dev libx11-xcb-dev libxext-dev libxfixes-dev libxi-dev \
  libxrandr-dev libxrender-dev libice-dev libsm-dev \
  libxkbcommon-dev libxkbcommon-x11-dev

sudo apt install -y \
  libxcb1-dev libxcb-cursor-dev libxcb-icccm4-dev libxcb-util-dev \
  libxcb-image0-dev libxcb-keysyms1-dev libxcb-render-util0-dev \
  libxcb-randr0-dev libxcb-shape0-dev libxcb-shm0-dev libxcb-sync-dev \
  libxcb-xfixes0-dev libxcb-xkb-dev libxcb-render0-dev libxcb-glx0-dev \
  libxcb-xinput-dev libxcb-xinerama0-dev libxcb-present-dev \
  libxcb-dri2-0-dev libxcb-dri3-dev

sudo apt install -y \
  libwayland-dev wayland-protocols libasound2-dev libpulse-dev \
  libpipewire-0.3-dev libusb-1.0-0-dev

echo
echo "==> Verifying pkg-config visibility"
pkg-config --modversion \
  freetype2 fontconfig harfbuzz xkbcommon xkbcommon-x11 dbus-1 \
  gl opengl glx egl

echo
echo "==> Done. Dependencies are installed and visible to pkg-config."
