#!/bin/bash
# CI only: import a base64 .p12 (Developer ID Application + Installer certs) into a throwaway keychain.
# Env in: MACOS_CERTS_P12 (base64), MACOS_CERTS_PASSWORD, RUNNER_TEMP. Writes KEYCHAIN_PATH to $GITHUB_ENV.
set -euo pipefail
KEYCHAIN="$RUNNER_TEMP/sentype-signing.keychain-db"
PW="$(uuidgen)"
printf '%s' "$MACOS_CERTS_P12" | base64 --decode > "$RUNNER_TEMP/certs.p12"
security create-keychain -p "$PW" "$KEYCHAIN"
security set-keychain-settings -lut 21600 "$KEYCHAIN"
security unlock-keychain -p "$PW" "$KEYCHAIN"
security import "$RUNNER_TEMP/certs.p12" -P "$MACOS_CERTS_PASSWORD" -A -t cert -f pkcs12 -k "$KEYCHAIN"
security set-key-partition-list -S apple-tool:,apple: -s -k "$PW" "$KEYCHAIN" > /dev/null
security list-keychains -d user -s "$KEYCHAIN" login.keychain-db
rm -f "$RUNNER_TEMP/certs.p12"
echo "KEYCHAIN_PATH=$KEYCHAIN" >> "$GITHUB_ENV"
security find-identity -v | grep -E "Developer ID" | sed 's/^ *[0-9]*) [0-9A-F]* /imported: /'
