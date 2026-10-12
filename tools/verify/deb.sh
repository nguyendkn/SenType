#!/bin/sh
# Build, test, package and install-test the .deb. Used by CI (release.yml) and locally:
#   docker run --rm [--platform linux/amd64|linux/arm64] -v "$PWD":/src sentype-dev \
#     sh /src/tools/verify/deb.sh 0.1.202610120 /src/dist
# Needs: cmake ninja build-essential dpkg-dev fcitx5 + Fcitx5 dev packages (see tools/docker/linux-dev.Dockerfile).
set -eu
VERSION="${1:?usage: deb.sh <version> <out-dir>}"
OUT="${2:?usage: deb.sh <version> <out-dir>}"
SRC="$(cd "$(dirname "$0")/../.." && pwd)"
B="${BUILD_DIR:-/tmp/sentype-deb-build}"
mkdir -p "$OUT"
sh "$SRC/tools/fetch-test-data.sh" > /dev/null
cmake -S "$SRC" -B "$B" -G Ninja -DSENTYPE_FRONTEND=linux-fcitx5 -DSENTYPE_VERSION="$VERSION" > /dev/null
cmake --build "$B" 2>&1 | tail -1
ctest --test-dir "$B" --output-on-failure 2>&1 | grep -E "Test +#|tests passed"
(cd "$B" && cpack -G DEB --config CPackConfig.cmake > /dev/null)
DEB="$(ls "$B"/*.deb)"
cp "$DEB" "$OUT/"
DEB="$OUT/$(basename "$DEB")"

echo "== package metadata"
dpkg-deb -I "$DEB" | grep -E "Package:|Version:|Architecture:|Depends:"
dpkg-deb -c "$DEB" | awk '{print $1, $6}'
echo "== install (apt resolves fcitx5 dependency)"
apt-get install -y "$DEB" 2>&1 | grep -E "SenType installed|fcitx5 -r|Setting up fcitx5-sentype|E:" || true
dpkg -s fcitx5-sentype | grep -E "^(Status|Version):"
for f in $(dpkg -L fcitx5-sentype | grep -E '\.(so|conf)$'); do test -f "$f" || { echo "missing $f"; exit 1; }; done
echo "== icons: hicolor files + Icon= in the input method conf"
for f in /usr/share/icons/hicolor/scalable/apps/sentype.svg /usr/share/icons/hicolor/scalable/apps/sentype-symbolic.svg \
         /usr/share/icons/hicolor/16x16/apps/sentype.png /usr/share/icons/hicolor/256x256/apps/sentype.png; do
  test -f "$f" || { echo "missing icon $f"; exit 1; }
done
grep -rx "Icon=sentype" /usr/share/fcitx5/inputmethod/sentype.conf
ldd "$(dpkg -L fcitx5-sentype | grep 'sentype.so$')" | grep -c "not found" | grep -qx 0 && echo "ldd: all libs resolved"
echo "== remove"
apt-get purge -y fcitx5-sentype > /dev/null 2>&1
if dpkg -L fcitx5-sentype >/dev/null 2>&1; then echo "still installed"; exit 1; fi
echo "removed cleanly; leftover files: $(find /usr -name '*sentype*' 2>/dev/null | wc -l)"
