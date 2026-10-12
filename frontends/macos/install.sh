#!/bin/sh
# Usage: ./install.sh path/to/SenType.app   (installs for the current user)
set -e
APP="${1:-SenType.app}"
DEST="$HOME/Library/Input Methods"
mkdir -p "$DEST"
pkill -x SenType 2>/dev/null || true
rm -rf "$DEST/SenType.app"
cp -R "$APP" "$DEST/SenType.app"
xattr -dr com.apple.quarantine "$DEST/SenType.app" 2>/dev/null || true
echo "Installed. Log out and back in, then add SenType under"
echo "System Settings > Keyboard > Input Sources > Edit > + > Vietnamese > SenType."
