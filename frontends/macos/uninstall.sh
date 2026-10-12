#!/bin/sh
# Remove SenType (user-level copy and, with sudo, the system-wide pkg install).
#   ./uninstall.sh            user copy only
#   sudo ./uninstall.sh       also /Library/Input Methods and the pkg receipt
set -e
pkill -x SenType 2>/dev/null || true
CONSOLE_USER="${SUDO_USER:-$(id -un)}"
USER_HOME="$(dscl . -read "/Users/$CONSOLE_USER" NFSHomeDirectory 2>/dev/null | awk '{print $2}')"
rm -rf "${USER_HOME:-$HOME}/Library/Input Methods/SenType.app"
if [ "$(id -u)" -eq 0 ]; then
  rm -rf "/Library/Input Methods/SenType.app"
  pkgutil --forget com.sentype.inputmethod.pkg >/dev/null 2>&1 || true
  echo "Removed system-wide install."
else
  [ -d "/Library/Input Methods/SenType.app" ] && echo "System-wide copy present: re-run with sudo to remove it."
fi
echo "Removed. Log out and back in to drop SenType from Input Sources."
