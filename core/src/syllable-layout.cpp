#include "syllable-layout.hpp"

namespace sentype {
namespace {

bool has_mod(const Letter& l) { return l.mod != Mod::None && l.mod != Mod::Stroke; }

}  // namespace

Layout analyze(const Letters& L) {
  Layout lay{L.size(), L.size()};
  for (size_t i = 0; i < L.size(); ++i) {
    if (!is_vowel_base(L[i].base)) continue;
    if (L[i].base == U'u' && L[i].mod == Mod::None && i > 0 && L[i - 1].base == U'q') continue;
    if (L[i].base == U'i' && i > 0 && L[i - 1].base == U'g' && i + 1 < L.size() &&
        is_vowel_base(L[i + 1].base)) {
      continue;
    }
    lay.nuc_begin = i;
    size_t e = i;
    while (e < L.size() && is_vowel_base(L[e].base)) ++e;
    lay.nuc_end = e;
    break;
  }
  return lay;
}

Letters normalized(const Letters& L) {
  Letters out = L;
  const Layout lay = analyze(L);
  if (lay.nuc_end - lay.nuc_begin == 2 && lay.nuc_end == L.size()) {
    Letter& a = out[lay.nuc_begin];
    const Letter& b = out[lay.nuc_begin + 1];
    if (a.base == U'u' && a.mod == Mod::Horn && b.base == U'o' && b.mod == Mod::Horn) {
      a.mod = Mod::None;
    }
  }
  return out;
}

int tone_index(const Letters& L, const Layout& lay, ToneStyle style) {
  const size_t n = lay.nuc_end - lay.nuc_begin;
  if (n == 0) return -1;
  for (size_t i = lay.nuc_end; i-- > lay.nuc_begin;) {
    if (has_mod(L[i])) return static_cast<int>(i);  // â ê ô ă ơ ư: last marked vowel
  }
  const int first = static_cast<int>(lay.nuc_begin);
  if (n == 1) return first;
  if (n >= 3) return first + 1;
  if (lay.nuc_end < L.size()) return first + 1;  // closed syllable: toán, hoàn
  const char32_t a = L[lay.nuc_begin].base, b = L[lay.nuc_begin + 1].base;
  const bool pair = (a == U'o' && (b == U'a' || b == U'e')) || (a == U'u' && b == U'y');
  return (pair && style == ToneStyle::Modern) ? first + 1 : first;
}

std::string render(const Letters& letters, int tone, ToneStyle style, bool final_form) {
  const Letters L = final_form ? normalized(letters) : letters;
  const int ti = tone_index(L, analyze(L), style);
  std::string out;
  for (size_t i = 0; i < L.size(); ++i) {
    append_utf8(out, compose(L[i], static_cast<int>(i) == ti ? tone : 0));
  }
  return out;
}

}  // namespace sentype
