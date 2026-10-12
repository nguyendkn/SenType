// Vowel diacritics: circumflex (â ê ô), breve (ă), horn (ơ ư), incl. Telex "w".
#include "syllable-validator.hpp"
#include "word-state.hpp"

namespace sentype {
namespace {

Mod mod_for(char32_t base, Mod kind) {
  return (kind == Mod::Horn && base == U'a') ? Mod::Breve : kind;
}

bool plain_or_horn(const Letter& l) { return l.mod == Mod::None || l.mod == Mod::Horn; }

}  // namespace

// `allowed` lists the vowel bases the key may modify. `standalone`: with no
// vowel at all the key itself writes "ư" (Telex "w").
Step WordState::add_mark(const RawKey& k, int idx, std::u32string_view allowed, Mod kind,
                         bool standalone) {
  const Layout lay = analyze(letters_);
  std::vector<size_t> plain;
  int marked = -1;
  for (size_t i = lay.nuc_begin; i < lay.nuc_end; ++i) {
    const Letter& l = letters_[i];
    if (allowed.find(l.base) == std::u32string_view::npos) continue;
    if (l.mod == Mod::None) {
      plain.push_back(i);
    } else if (l.mod == mod_for(l.base, kind)) {
      marked = static_cast<int>(i);
    }
  }

  if (plain.empty()) {
    if (marked >= 0) return undo_or_fallback(k, idx, letters_[marked].mod_key);
    // A different mark key on an already marked vowel switches the mark: "oow" -> ơ, "awa" -> â.
    for (size_t i = lay.nuc_end; i-- > lay.nuc_begin;) {
      Letter& l = letters_[i];
      if (allowed.find(l.base) == std::u32string_view::npos) continue;
      if (l.mod != Mod::Hat && l.mod != Mod::Breve && l.mod != Mod::Horn) continue;
      const Letters before = letters_;
      l.mod = mod_for(l.base, kind);
      l.mod_key = idx;
      if (!nucleus_viable(letters_)) {
        letters_ = before;
        continue;
      }
      own(idx, static_cast<int>(i));
      return {Act::Mark, -1};
    }
    if (standalone && lay.nuc_begin == lay.nuc_end) {
      Letter l;
      l.base = U'u';
      l.mod = Mod::Horn;
      l.upper = k.ch >= U'A' && k.ch <= U'Z';
      l.mod_key = idx;
      letters_.push_back(l);
      own(idx, static_cast<int>(letters_.size()) - 1);
      return {Act::Mark, -1};
    }
    return fallback(k, idx);
  }

  size_t first = plain.back();
  size_t second = first;  // equal to `first` when a single letter changes
  if (kind == Mod::Horn) {
    for (size_t i = lay.nuc_begin; i + 1 < lay.nuc_end; ++i) {
      const Letter &a = letters_[i], &b = letters_[i + 1];
      if (a.base == U'u' && b.base == U'o' && plain_or_horn(a) && plain_or_horn(b) &&
          (a.mod == Mod::None || b.mod == Mod::None)) {  // "uo" -> "ươ"
        first = a.mod == Mod::None ? i : i + 1;
        second = i + 1;
        break;
      }
      if (a.base == U'u' && a.mod == Mod::None && b.base == U'a') {  // "ua" -> "ưa"
        first = second = i;
        break;
      }
      if (a.base == U'u' && a.mod == Mod::None && b.base == U'u' && b.mod == Mod::None) {
        first = second = i;  // "uu" -> "ưu" (hưu, cứu), not "uư"
        break;
      }
    }
  }
  const Letters before = letters_;
  for (size_t i = first; i <= second; ++i) {
    if (letters_[i].mod != Mod::None) continue;
    letters_[i].mod = mod_for(letters_[i].base, kind);
    letters_[i].mod_key = idx;
  }
  if (!nucleus_viable(letters_)) {  // e.g. "oe"+o -> "ôe": not Vietnamese, keep the key as a letter
    letters_ = before;
    return fallback(k, idx);
  }
  own(idx, static_cast<int>(first));
  return {Act::Mark, -1};
}

}  // namespace sentype
