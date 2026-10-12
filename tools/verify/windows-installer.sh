#!/bin/sh
# Build the NSIS installer from mingw-built x64+x86 DLLs and exercise it under wine (silent /S):
#   docker run --rm --platform linux/amd64 -v "$PWD":/src sentype-win sh /src/tools/verify/windows-installer.sh 0.1.202610120
# Always: makensis builds, payload listing, x64 DLL register/unregister with the exact regsvr32 command lines the
# installer uses (CLSID + CTF TIP profile keys in the wine registry).
# Only when wine can execute 32-bit x86 (NSIS stubs are 32-bit PE; Rosetta/QEMU hosts such as Apple Silicon
# cannot): silent install, upgrade over an existing install, uninstall, with registry checks.
# Never proves real Windows behaviour (WOW64 regsvr32 split, locked DLLs, UAC, SmartScreen, TSF loading):
# the release workflow runs tools/verify/windows-installer-smoke.ps1 on a real windows runner for that.
set -u
VERSION="${1:?usage: windows-installer.sh <version>}"
BASE="${VERSION%.*}"                    # major.minor (version is <major>.<minor>.<date><index>)
SRC=/src; OUT="${OUT:-/src/build/win-installer}"
export WINEDEBUG=-all WINEPREFIX=/tmp/wp XDG_RUNTIME_DIR=/tmp/xdg
mkdir -p "$XDG_RUNTIME_DIR" "$OUT" && chmod 700 "$XDG_RUNTIME_DIR"
for arch in i686 x86_64; do
  cmake -S $SRC -B /tmp/bw-$arch -G Ninja -DSENTYPE_FRONTEND=windows -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_CXX_COMPILER=$arch-w64-mingw32-g++-posix -DSENTYPE_VERSION="$VERSION" > /dev/null || exit 1
  cmake --build /tmp/bw-$arch 2>&1 | tail -1
done
mknsis() {  # version outfile
  makensis -V2 -DVERSION="$1" -DVERSION_BASE="$BASE" -DDLL_X64=/tmp/bw-x86_64/frontends/windows/sentype-tsf.dll \
    -DDLL_X86=/tmp/bw-i686/frontends/windows/sentype-tsf.dll -DOUTFILE="$2" $SRC/frontends/windows/installer/sentype.nsi
}
EXE="$OUT/SenType-$VERSION-setup.exe"
mknsis "$VERSION" "$EXE" || { echo "makensis FAILED"; exit 1; }
ls -l "$EXE"
echo "== installer payload"; (command -v 7z >/dev/null || { apt-get update >/dev/null 2>&1; apt-get install -y 7zip >/dev/null 2>&1; }); 7z l "$EXE" | grep -E "sentype-tsf|uninstall|Files"

wine64 wineboot -i > /dev/null 2>&1; wine64 wineboot -i > /dev/null 2>&1
CLSID='{3922582A-5FEA-483C-BF9B-BD6D5BE68439}'
reg() { wine64 reg query "$@" 2>&1 | tr -d '\r'; }
evidence() {
  reg "HKLM\\Software\\Classes\\CLSID\\$CLSID\\InprocServer32" /reg:64 | grep REG_SZ | sed 's/^ */   x64 CLSID: /'
  reg "HKLM\\Software\\Microsoft\\CTF\\TIP\\$CLSID" /s /reg:64 | grep -c "LanguageProfile" | sed 's/^/   CTF TIP LanguageProfile keys: /'
}
run() { echo "== $*"; wine64 "$@" > /tmp/o.txt 2>&1; rc=$?; echo "exit code $rc"; grep -v -e XDG_RUNTIME /tmp/o.txt | tail -3; }

echo "== can this wine run a 32-bit exe?"
printf 'int main(void){return 0;}\n' > /tmp/h.c; i686-w64-mingw32-gcc -static /tmp/h.c -o /tmp/h32.exe
if wine64 /tmp/h32.exe > /dev/null 2>&1; then CAN32=1; echo yes; else CAN32=0; echo "no: installer cannot run here"; fi

if [ "$CAN32" = 1 ]; then
  run "$EXE" /S;  echo "-- after install"; evidence
  reg 'HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\SenType' /reg:64 | grep -E "DisplayVersion|InstallLocation"
  find "$WINEPREFIX/drive_c/Program Files/SenType" -type f | sed "s|$WINEPREFIX/drive_c|   C:|"
  V2="${VERSION%?}1"; mknsis "$V2" "$OUT/upg.exe" > /dev/null
  run "$OUT/upg.exe" /S; echo "-- after upgrade -> $V2"; evidence
  reg 'HKLM\Software\Microsoft\Windows\CurrentVersion\Uninstall\SenType' /reg:64 | grep DisplayVersion
  INST='C:\Program Files\SenType'
  run "$INST\\uninstall.exe" /S "_?=$INST"; echo "-- after uninstall"; evidence
  rm -f "$OUT/upg.exe"
else
  echo "== fallback: the installer's regsvr32 calls for the x64 DLL, run directly"
  DLL=/tmp/bw-x86_64/frontends/windows/sentype-tsf.dll
  wine64 regsvr32 /s "$DLL"; echo "regsvr32 /s: exit $?"; evidence
  wine64 regsvr32 /s /u "$DLL"; echo "regsvr32 /s /u: exit $?"; evidence
fi
