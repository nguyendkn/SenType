#!/bin/sh
# Windows cross-build (mingw-w64 x64 + x86) and wine checks, inside the sentype-win image:
#   docker run --rm --platform linux/amd64 -v "$PWD":/src sentype-win sh /src/tools/verify/windows.sh
# Checks: DLL builds for both arches, core + key-translation tests run under wine, regsvr32
# registers and unregisters the x64 DLL and the COM CLSID shows up in the wine registry.
set -eu
cd /src
export WINEDEBUG=-all WINEPREFIX=/tmp/wp XDG_RUNTIME_DIR=/tmp/xdg
mkdir -p "$XDG_RUNTIME_DIR" && chmod 700 "$XDG_RUNTIME_DIR"
for arch in i686 x86_64; do
  cmake -S . -B /tmp/bw-$arch -G Ninja -DSENTYPE_FRONTEND=windows -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_CXX_COMPILER=$arch-w64-mingw32-g++-posix > /dev/null
  cmake --build /tmp/bw-$arch 2>&1 | tail -1
  ls -l /tmp/bw-$arch/frontends/windows/sentype-tsf.dll
done
cd /tmp/bw-x86_64
wine64 wineboot -i > /dev/null 2>&1 || true   # first start creates the prefix (flaky under Rosetta)
wine64 wineboot -i > /dev/null 2>&1 || true
for t in core/engine-test.exe core/session-test.exe frontends/windows/key-translation-test.exe; do
  echo "== wine $t"; rc=0; wine64 $t > /tmp/out.txt 2>&1 || rc=$?; echo "exit code $rc"; grep -v -e XDG_RUNTIME -e "^wine:" /tmp/out.txt | tail -2
done
echo "== wine fuzz-random-test (20000 sequences)"
rc=0; wine64 core/fuzz-random-test.exe 20000 1 > /tmp/out.txt 2>&1 || rc=$?; echo "exit code $rc"; tail -1 /tmp/out.txt
for arch in i686 x86_64; do
  echo "== $arch DLL .rsrc (icon resource id 101 is embedded by windres)"
  $arch-w64-mingw32-objdump -h /tmp/bw-$arch/frontends/windows/sentype-tsf.dll | grep -E "\.rsrc" || { echo "no .rsrc"; exit 1; }
done
wine64 regsvr32 /s frontends/windows/sentype-tsf.dll && echo "regsvr32 register: OK"
echo "== profile icon registered (IconFile / IconIndex = -101)"
wine64 reg query 'HKLM\Software\Microsoft\CTF\TIP' /s 2>&1 | grep -i -E "IconFile|IconIndex"
wine64 reg query 'HKCR\CLSID\{3922582A-5FEA-483C-BF9B-BD6D5BE68439}\InprocServer32' 2>&1 | grep -E "REG_SZ"
wine64 regsvr32 /s /u frontends/windows/sentype-tsf.dll && echo "regsvr32 unregister: OK"
