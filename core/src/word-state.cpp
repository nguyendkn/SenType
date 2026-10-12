#include "word-state.hpp"

namespace sentype {
namespace {

char32_t lower(char32_t c) { return (c >= U'A' && c <= U'Z') ? c - U'A' + U'a' : c; }
bool is_upper(char32_t c) { return c >= U'A' && c <= U'Z'; }

}  // namespace

void WordState::clear() {
  letters_.clear();
  owner_.clear();
  tone_ = 0;
  tone_key_ = -1;
}

void WordState::own(int idx, int letter) {
  if (static_cast<int>(owner_.size()) <= idx) owner_.resize(idx + 1, -1);
  owner_[idx] = letter;
}

Step WordState::push(const RawKey& k, int idx) {
  if (k.literal) return add_letter(k, idx);
  const char32_t c = lower(k.ch);
  return cfg_.method == Method::Telex ? dispatch_telex(k, c, idx) : dispatch_vni(k, c, idx);
}

Step WordState::dispatch_telex(const RawKey& k, char32_t c, int idx) {
  switch (c) {
    case U's': return set_tone(k, idx, 1);
    case U'f': return set_tone(k, idx, 2);
    case U'r': return set_tone(k, idx, 3);
    case U'x': return set_tone(k, idx, 4);
    case U'j': return set_tone(k, idx, 5);
    case U'z': return set_tone(k, idx, 0);
    case U'a': return add_mark(k, idx, U"a", Mod::Hat, false);
    case U'e': return add_mark(k, idx, U"e", Mod::Hat, false);
    case U'o': return add_mark(k, idx, U"o", Mod::Hat, false);
    case U'w': return add_mark(k, idx, U"aou", Mod::Horn, true);
    case U'd': return add_stroke(k, idx, true);
    default: return add_letter(k, idx);
  }
}

Step WordState::dispatch_vni(const RawKey& k, char32_t c, int idx) {
  switch (c) {
    case U'1': case U'2': case U'3': case U'4': case U'5':
      return set_tone(k, idx, static_cast<int>(c - U'0'));
    case U'0': return set_tone(k, idx, 0);
    case U'6': return add_mark(k, idx, U"aeo", Mod::Hat, false);
    case U'7': return add_mark(k, idx, U"ou", Mod::Horn, false);
    case U'8': return add_mark(k, idx, U"a", Mod::Breve, false);
    case U'9': return add_stroke(k, idx, false);
    default: return add_letter(k, idx);
  }
}

Step WordState::add_letter(const RawKey& k, int idx) {
  Letter l;
  l.base = lower(k.ch);
  l.upper = is_upper(k.ch);
  letters_.push_back(l);
  own(idx, static_cast<int>(letters_.size()) - 1);
  return {Act::Letter, -1};
}

// A key that found no target: plain letter in Telex, boundary in VNI (digits).
Step WordState::fallback(const RawKey& k, int idx) {
  return cfg_.method == Method::Telex ? add_letter(k, idx) : Step{Act::Reject, -1};
}

Step WordState::undo_or_fallback(const RawKey& k, int idx, int key_idx) {
  return key_idx == idx - 1 ? Step{Act::Undo, key_idx} : fallback(k, idx);
}

Step WordState::set_tone(const RawKey& k, int idx, int tone) {
  const Layout lay = analyze(letters_);
  if (lay.nuc_begin == lay.nuc_end) return fallback(k, idx);  // no vowel yet
  if (tone == 0) {
    if (tone_ == 0) return fallback(k, idx);
    tone_ = 0;
    tone_key_ = -1;
  } else if (tone == tone_) {
    return undo_or_fallback(k, idx, tone_key_);
  } else {
    tone_ = tone;
    tone_key_ = idx;
  }
  own(idx, -1);
  return {Act::Tone, -1};
}

Step WordState::add_stroke(const RawKey& k, int idx, bool immediate_only) {
  if (letters_.empty() || letters_[0].base != U'd' || (immediate_only && letters_.size() != 1)) {
    return fallback(k, idx);
  }
  Letter& d = letters_[0];
  if (d.mod == Mod::Stroke) return undo_or_fallback(k, idx, d.mod_key);
  d.mod = Mod::Stroke;
  d.mod_key = idx;
  own(idx, 0);
  return {Act::Mark, -1};
}

}  // namespace sentype
