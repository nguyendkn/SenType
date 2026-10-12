#!/bin/sh
# Regenerate every icon asset from tools/icons/sentype.svg + sentype-mono.svg.
#   sh tools/icons/build-icons.sh        (needs docker; rendering runs in ubuntu:26.04 with librsvg)
# Generated files are committed, so normal builds do not need this toolchain.
set -eu
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
if [ "${IN_CONTAINER:-}" != 1 ]; then
  exec docker run --rm -e IN_CONTAINER=1 -v "$ROOT":/src ubuntu:26.04 sh -c \
    'apt-get update -qq && apt-get install -y -qq --no-install-recommends librsvg2-bin python3 >/dev/null && sh /src/tools/icons/build-icons.sh'
fi
cd /src/tools/icons
W=/src/frontends/windows/res; M=/src/frontends/macos/Resources; L=/src/frontends/linux-fcitx5/icons
P=/tmp/png; mkdir -p "$P" "$W" "$M" "$L/scalable/apps" preview
for s in 16 20 24 32 40 48 64 128 256 512 1024; do
  rsvg-convert -w $s -h $s sentype.svg -o "$P/c$s.png"
done
for s in 16 18 32 36; do rsvg-convert -w $s -h $s sentype-mono.svg -o "$P/m$s.png"; done
rsvg-convert -f pdf sentype-mono.svg -o "$M/sentype-menu.pdf"
cp "$P/m36.png" "$M/sentype-menu.png"   # fallback raster (template, 18pt @2x)
python3 pack-icons.py "$P" "$W/sentype.ico" "$M/sentype.icns"
for s in 16 22 24 32 48 64 128 256; do
  mkdir -p "$L/${s}x${s}/apps"; rsvg-convert -w $s -h $s sentype.svg -o "$L/${s}x${s}/apps/sentype.png"
done
cp sentype.svg "$L/scalable/apps/sentype.svg"
cp sentype-mono.svg "$L/scalable/apps/sentype-symbolic.svg"
for s in 16 32 256; do cp "$P/c$s.png" "preview/color-$s.png"; done
rsvg-convert -w 16 -h 16 sentype-mono.svg -o preview/mono-16.png
rsvg-convert -w 32 -h 32 sentype-mono.svg -o preview/mono-32.png
rsvg-convert -w 256 -h 256 sentype-mono.svg -o preview/mono-256.png
echo done
