#include "syllable-validator.hpp"

#include <algorithm>
#include <array>

namespace sentype {
namespace {

using SV = std::u32string_view;

constexpr std::array<SV, 28> kInitials = {
    U"",   U"b",  U"c",  U"ch", U"d",  U"đ",  U"g",  U"gh", U"gi", U"h",
    U"k",  U"kh", U"l",  U"m",  U"n",  U"ng", U"ngh", U"nh", U"p", U"ph",
    U"qu", U"r",  U"s",  U"t",  U"th", U"tr", U"v",  U"x"};
constexpr std::array<SV, 9> kCodas = {U"", U"c", U"ch", U"m", U"n", U"ng", U"nh", U"p", U"t"};

// Nuclei usable open or closed.
constexpr std::array<SV, 18> kEither = {U"a",  U"ă",  U"â",  U"e",  U"ê",  U"i",  U"o",  U"ô",  U"ơ",
                                        U"u",  U"ư",  U"y",  U"oa", U"oă", U"oe", U"uâ", U"uê", U"uy"};
// Nuclei that need a coda.
constexpr std::array<SV, 5> kNeedCoda = {U"iê", U"yê", U"uô", U"ươ", U"uyê"};
// Nuclei that cannot take a coda.
constexpr std::array<SV, 26> kOpenOnly = {
    U"ai",  U"ao",  U"au",  U"ay",  U"âu",  U"ây",  U"eo",  U"êu", U"ia",
    U"iu",  U"oai", U"oay", U"oeo", U"oi",  U"ôi",  U"ơi",  U"ua", U"ui",
    U"ưa",  U"ưi",  U"ưu",  U"uai", U"uây", U"uôi", U"uya", U"uyu"};
constexpr std::array<SV, 5> kOpenOnly2 = {U"ươi", U"ươu", U"iêu", U"yêu", U"uơ"};

template <size_t N>
bool in(const std::array<SV, N>& set, SV s) {
  return std::find(set.begin(), set.end(), s) != set.end();
}

std::u32string symbols(const Letters& L, size_t b, size_t e) {
  std::u32string s;
  for (size_t i = b; i < e; ++i) s += symbol(L[i]);
  return s;
}

bool is_front(char32_t c) { return c == U'i' || c == U'e' || c == U'ê' || c == U'y'; }

bool initial_fits(SV init, char32_t first) {
  if (init == U"c") return !is_front(first);
  if (init == U"k") return is_front(first);
  if (init == U"g") return first != U'e' && first != U'ê';
  if (init == U"gh" || init == U"ngh") return first == U'i' || first == U'e' || first == U'ê';
  if (init == U"ng") return !(first == U'i' || first == U'e' || first == U'ê');
  return true;
}

bool nucleus_fits(SV nuc, bool has_coda) {
  if (in(kEither, nuc)) return true;
  if (in(kNeedCoda, nuc)) return has_coda;
  return !has_coda && (in(kOpenOnly, nuc) || in(kOpenOnly2, nuc));
}

}  // namespace

bool is_valid_syllable(const Letters& letters, int tone) {
  if (letters.empty()) return true;
  const Letters L = normalized(letters);
  const Layout lay = analyze(L);
  if (lay.nuc_begin == lay.nuc_end) return false;

  const std::u32string init = symbols(L, 0, lay.nuc_begin);
  const std::u32string nuc = symbols(L, lay.nuc_begin, lay.nuc_end);
  const std::u32string coda = symbols(L, lay.nuc_end, L.size());
  if (!in(kInitials, init) || !in(kCodas, coda)) return false;
  if (!initial_fits(init, nuc.front()) || !nucleus_fits(nuc, !coda.empty())) return false;
  if (coda == U"ch" || coda == U"nh") {
    const char32_t last = nuc.back();
    if (!(last == U'a' || last == U'e' || last == U'ê' || last == U'i' || last == U'y')) {
      return false;
    }
  }
  const bool stop = coda == U"c" || coda == U"ch" || coda == U"p" || coda == U"t";
  return !stop || tone == 0 || tone == 1 || tone == 5;  // stops take only acute / dot
}

}  // namespace sentype
