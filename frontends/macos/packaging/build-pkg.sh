#!/bin/bash
# Build SenType-<version>.pkg (system-wide, /Library/Input Methods) and SenType-<version>-macos.zip
# (user-level fallback with install.sh) from a built SenType.app.
#   build-pkg.sh <SenType.app> <version> <out-dir>
# Optional signing/notarization, each step only when its env vars are set (otherwise the pkg is unsigned
# and Gatekeeper warns on first open):
#   SIGN_APP_IDENTITY   "Developer ID Application: ..."  re-sign the app with hardened runtime
#   SIGN_PKG_IDENTITY   "Developer ID Installer: ..."    sign the pkg (productbuild --sign)
#   NOTARY_APPLE_ID, NOTARY_TEAM_ID, NOTARY_PASSWORD (app-specific)  notarize + staple the pkg
set -euo pipefail
APP="${1:?usage: build-pkg.sh <SenType.app> <version> <out-dir>}"
VERSION="${2:?version}"
OUT="${3:?out-dir}"
HERE="$(cd "$(dirname "$0")" && pwd)"
MAC="$(dirname "$HERE")"
PKG_ID=com.sentype.inputmethod.pkg
mkdir -p "$OUT"; OUT="$(cd "$OUT" && pwd)"
W="$(mktemp -d)"; trap 'rm -rf "$W"' EXIT

[ -d "$APP/Contents/MacOS" ] || { echo "not an app bundle: $APP" >&2; exit 1; }
BUNDLE_VER="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' "$APP/Contents/Info.plist")"
[ "$BUNDLE_VER" = "$VERSION" ] || { echo "app version $BUNDLE_VER != $VERSION (rebuild with -DSENTYPE_VERSION=$VERSION)" >&2; exit 1; }

mkdir -p "$W/root"
ditto --norsrc --noextattr "$APP" "$W/root/SenType.app"
if [ -n "${SIGN_APP_IDENTITY:-}" ]; then
  echo "== signing app with Developer ID (hardened runtime)"
  codesign --force --options runtime --timestamp --sign "$SIGN_APP_IDENTITY" "$W/root/SenType.app"
else
  echo "== app keeps its ad-hoc signature (no SIGN_APP_IDENTITY)"
fi
codesign --verify --deep --strict "$W/root/SenType.app"

# Not relocatable: Installer must never "upgrade" a stray copy elsewhere (e.g. ~/Library/Input Methods).
cat > "$W/components.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><array><dict>
  <key>RootRelativeBundlePath</key><string>SenType.app</string>
  <key>BundleIsRelocatable</key><false/>
  <key>BundleIsVersionChecked</key><false/>
  <key>BundleOverwriteAction</key><string>upgrade</string>
  <key>BundleHasStrictIdentifier</key><false/>
</dict></array></plist>
PLIST

pkgbuild --root "$W/root" --component-plist "$W/components.plist" \
  --identifier "$PKG_ID" --version "$VERSION" \
  --install-location "/Library/Input Methods" --scripts "$HERE/scripts" \
  "$W/SenType-component.pkg"

sed "s/@VERSION@/$VERSION/g; s/@PKG_ID@/$PKG_ID/g" "$HERE/distribution.xml.in" > "$W/distribution.xml"
PKG="$OUT/SenType-$VERSION.pkg"
SIGN_ARGS=()
[ -n "${SIGN_PKG_IDENTITY:-}" ] && SIGN_ARGS=(--sign "$SIGN_PKG_IDENTITY" --timestamp)
productbuild --distribution "$W/distribution.xml" --resources "$HERE/resources" \
  --package-path "$W" ${SIGN_ARGS[@]+"${SIGN_ARGS[@]}"} "$PKG"

if [ -n "${SIGN_PKG_IDENTITY:-}" ] && [ -n "${NOTARY_APPLE_ID:-}" ] && [ -n "${NOTARY_TEAM_ID:-}" ] \
   && [ -n "${NOTARY_PASSWORD:-}" ]; then
  echo "== notarizing (waits for Apple)"
  xcrun notarytool submit "$PKG" --apple-id "$NOTARY_APPLE_ID" --team-id "$NOTARY_TEAM_ID" \
    --password "$NOTARY_PASSWORD" --wait
  xcrun stapler staple "$PKG"
  xcrun stapler validate "$PKG"
else
  echo "== NOT notarized (needs SIGN_PKG_IDENTITY + NOTARY_*): Gatekeeper will warn on download"
fi

# Fallback archive: app + user-level install/uninstall scripts (no admin rights needed).
Z="$W/SenType-$VERSION-macos"
mkdir -p "$Z"
ditto --norsrc --noextattr "$W/root/SenType.app" "$Z/SenType.app"
cp -X "$MAC/install.sh" "$MAC/uninstall.sh" "$Z/"
ditto -c -k --norsrc --noextattr --keepParent "$Z" "$OUT/SenType-$VERSION-macos.zip"
ls -l "$OUT"/SenType-"$VERSION"*
