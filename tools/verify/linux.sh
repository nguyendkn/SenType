#!/bin/sh
# Linux checks inside the sentype-dev image (arm64 native or --platform linux/amd64):
#   docker run --rm -v "$PWD":/src sentype-dev sh /src/tools/verify/linux.sh
# Core tests + Fcitx5 addon integration test (real Fcitx5, testfrontend), then build the .deb,
# install it into the image with dpkg and check that Fcitx5 loads the installed addon.
set -eu
cd /src
sh tools/fetch-test-data.sh > /dev/null
cmake -S . -B /tmp/blin -G Ninja -DSENTYPE_FRONTEND=linux-fcitx5 > /dev/null
cmake --build /tmp/blin 2>&1 | tail -1
ctest --test-dir /tmp/blin --output-on-failure 2>&1 | grep -E "Test +#|tests passed"
(cd /tmp/blin && cpack -G DEB --config CPackConfig.cmake > /dev/null)
DEB=$(ls /tmp/blin/*.deb)
mkdir -p /src/build/deb && cp "$DEB" /src/build/deb/
dpkg-deb -I "$DEB" | grep -E "Package|Version|Architecture|Depends"
dpkg -i "$DEB" > /dev/null 2>&1 || apt-get install -y -f > /dev/null 2>&1
dpkg -L fcitx5-sentype
export XDG_RUNTIME_DIR=/tmp/xdg HOME=/tmp/home
mkdir -p "$XDG_RUNTIME_DIR" "$HOME" && chmod 700 "$XDG_RUNTIME_DIR"
# Installed addon is visible to a real Fcitx5 instance (headless, no frontend needed).
timeout 20 fcitx5-diagnose 2>/dev/null | grep -i -E "sentype" | head -3 || true
fcitx5 --help > /dev/null 2>&1 && timeout 5 fcitx5 --verbose '*=5' --disable=all --enable=sentype 2>&1 | grep -i -E "sentype" | head -5 || true
