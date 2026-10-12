#!/bin/sh
# Fetches the independent test corpora into <out-dir> (default <repo>/build/test-data).
# Nothing here is committed or shipped: the Vietnamese list is GPL-2 data (see docs/testing.md).
#   vi-syllables.txt    6.6k lowercase Vietnamese syllables  <- LibreOffice dictionaries vi/vi_VN.dic
#   english-words.txt   English words (negatives)            <- LibreOffice dictionaries en/en_US.dic
set -eu
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${1:-$ROOT/build/test-data}"
COMMIT=32b006a2c22a4ac7e8ed3f03346f7b3d85a970a4   # LibreOffice/dictionaries, pinned
BASE="https://raw.githubusercontent.com/LibreOffice/dictionaries/$COMMIT"
VI_SHA256=21d59c8385d2ac8d708bc5dfe83b62753d7769a8b2c9c38d319ce5c57bfba0c7
EN_SHA256=f0b1a234bd178bdd01875b2a392a9647f888b8fe879f79c52aae62c2759b3647

mkdir -p "$OUT"
sha() { if command -v sha256sum >/dev/null; then sha256sum "$1" | cut -d' ' -f1; else shasum -a 256 "$1" | cut -d' ' -f1; fi; }
fetch() {  # url file expected-sha
  [ -f "$OUT/$2" ] || curl -fsSL "$1" -o "$OUT/$2"
  [ "$(sha "$OUT/$2")" = "$3" ] || { echo "checksum mismatch: $2" >&2; rm -f "$OUT/$2"; exit 1; }
}
fetch "$BASE/vi/vi_VN.dic" vi_VN.dic "$VI_SHA256"
fetch "$BASE/en/en_US.dic" en_US.dic "$EN_SHA256"

# Keep only all-lowercase Vietnamese-letter entries (drops acronyms, names, loanword spellings).
perl -Mutf8 -CSD -ne 'next if $.==1; s/[\/\r\n].*//; print "$_\n" if /^[a-zàáảãạăằắẳẵặâầấẩẫậđèéẻẽẹêềếểễệìíỉĩịòóỏõọôồốổỗộơờớởỡợùúủũụưừứửữựỳýỷỹỵ]+$/' \
  "$OUT/vi_VN.dic" | LC_ALL=C sort -u > "$OUT/vi-syllables.txt"
perl -ne 'next if $.==1; s/[\/\r\n].*//; print lc($_)."\n" if /^[A-Za-z]{2,}$/' \
  "$OUT/en_US.dic" | LC_ALL=C sort -u > "$OUT/english-words.txt"
echo "vi-syllables: $(wc -l < "$OUT/vi-syllables.txt")  english-words: $(wc -l < "$OUT/english-words.txt")"
