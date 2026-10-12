#!/bin/sh
# Differential run: SenType vs UniKey (ukengine) vs Bamboo (bamboo-core) on identical key sequences.
# Run inside the dev image (needs g++, go, python3, git, curl):
#   docker run --rm -v "$PWD":/src sentype-dev sh /src/tools/diff/run-diff.sh [random-count] [seed]
# References are cloned into build/ref (LGPL-2.0+ ukengine, MIT bamboo-core); nothing is vendored
# or linked into SenType. Results: build/diff/*.txt and a category summary on stdout.
set -eu
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
D="$ROOT/tools/diff"
B="$ROOT/build/diff"
REF="$ROOT/build/ref"
RANDOM_COUNT="${1:-20000}"
SEED="${2:-1}"
mkdir -p "$B" "$REF"

clone() {  # url dir
  [ -d "$REF/$2/.git" ] || git clone --quiet --depth 1 "$1" "$REF/$2"
  echo "$2 $(git -C "$REF/$2" rev-parse HEAD)" >> "$B/refs.txt"
}
: > "$B/refs.txt"
clone https://github.com/fcitx/fcitx5-unikey.git fcitx5-unikey
clone https://github.com/BambooEngine/bamboo-core.git bamboo-core

sh "$ROOT/tools/fetch-test-data.sh" "$ROOT/build/test-data" > /dev/null
TD="$ROOT/build/test-data"
SUP="$ROOT/core/tests/support"

g++ -std=c++20 -O2 -I"$ROOT/core/tests" -o "$B/gen-inputs" "$D/gen-inputs.cpp" "$SUP/typing-orders.cpp" "$SUP/vi-units.cpp"
"$B/gen-inputs" "$TD/vi-syllables.txt" "$D/data/sentences-vi.txt" "$D/data/english-tech-words.txt" \
  "$RANDOM_COUNT" "$SEED" > "$B/inputs.tsv"

g++ -std=c++20 -O2 -I"$ROOT/core/include" -o "$B/sentype-run" "$D/sentype-run.cpp" \
  "$ROOT"/core/src/*.cpp -I"$ROOT/core/src"
UK="$REF/fcitx5-unikey/unikey"
UKSRC=$(ls "$UK"/*.cpp | grep -v -e unikeyinputcontext -e usrkeymap)
g++ -std=c++20 -O2 -w -I"$UK" -o "$B/unikey-run" "$D/unikey-run.cpp" $UKSRC
(cd "$D/bamboo-run" && go build -o "$B/bamboo-run" .)

"$B/sentype-run" < "$B/inputs.tsv" > "$B/sentype.txt"
"$B/unikey-run" < "$B/inputs.tsv" > "$B/unikey.txt"
"$B/bamboo-run" < "$B/inputs.tsv" > "$B/bamboo.txt"
wc -l "$B/inputs.tsv" "$B/sentype.txt" "$B/unikey.txt" "$B/bamboo.txt"
python3 "$D/compare.py" "$B" "$TD/vi-syllables.txt" "$ROOT/core/tests/data/non-native-syllables.txt"
