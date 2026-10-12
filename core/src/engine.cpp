#include "sentype/engine.hpp"

#include "syllable-validator.hpp"
#include "word-state.hpp"

namespace sentype {

struct Engine::Impl {
  Config cfg;
  WordState state;
  std::vector<RawKey> raw;     // keys as interpreted (an undone mark key is dropped)
  std::u32string typed;        // every key exactly as pressed (for restore / Escape)
  bool edited = false;  // backspace used: the user shaped the word, never restore it

  explicit Impl(const Config& c) : cfg(c), state(c) {}

  void rebuild() {
    state.clear();
    for (size_t i = 0; i < raw.size(); ++i) state.push(raw[i], static_cast<int>(i));
  }
  void clear() {
    raw.clear();
    typed.clear();
    state.clear();
    edited = false;
  }
  bool valid() const {
    return !cfg.spellcheck || edited || is_valid_syllable(state.letters(), state.tone());
  }
  std::string raw_text() const {
    std::string s;
    for (char32_t ch : typed) append_utf8(s, ch);
    return s;
  }
  bool is_word_key(char32_t ch) const {
    const bool alpha = (ch >= U'a' && ch <= U'z') || (ch >= U'A' && ch <= U'Z');
    const bool digit = ch >= U'0' && ch <= U'9';
    return alpha || (cfg.method == Method::Vni && digit && !raw.empty());
  }
};

Engine::Engine(Config cfg) : impl_(std::make_unique<Impl>(cfg)) {}
Engine::~Engine() = default;
Engine::Engine(Engine&&) noexcept = default;
Engine& Engine::operator=(Engine&&) noexcept = default;

void Engine::set_config(const Config& cfg) {
  impl_ = std::make_unique<Impl>(cfg);
}
const Config& Engine::config() const { return impl_->cfg; }

bool Engine::feed(char32_t ch) {
  Impl& m = *impl_;
  if (!m.is_word_key(ch)) return false;
  const RawKey key{ch, false};
  const Step st = m.state.push(key, static_cast<int>(m.raw.size()));
  if (st.act != Act::Reject) m.typed.push_back(ch);
  switch (st.act) {
    case Act::Reject:
      return false;
    case Act::Undo:  // repeated mark key: cancel the mark, keep the key as plain text
      m.raw.erase(m.raw.begin() + st.undo_idx);
      m.raw.push_back({ch, true});
      m.rebuild();
      break;
    default:
      m.raw.push_back(key);
  }
  return true;
}

bool Engine::backspace() {
  Impl& m = *impl_;
  if (m.raw.empty()) return false;
  const int last = static_cast<int>(m.state.letters().size()) - 1;
  int vowels_left = 0;
  for (int i = 0; i < last; ++i) vowels_left += is_vowel_base(m.state.letters()[i].base);
  // Drop the last letter with the marks typed for it; tone keys go with the last vowel.
  std::vector<RawKey> kept;
  const std::vector<int>& owner = m.state.owner();
  for (size_t i = 0; i < m.raw.size(); ++i) {
    const int o = owner[i];
    if (o == last || (o == -1 && vowels_left == 0)) continue;
    kept.push_back(m.raw[i]);
  }
  m.raw = std::move(kept);
  m.typed.clear();  // after an edit the remaining keys are what the user sees: restore those
  for (const RawKey& k : m.raw) m.typed.push_back(k.ch);
  m.edited = true;
  m.rebuild();
  if (m.raw.empty()) m.edited = false;
  return true;
}

std::string Engine::preedit() const {
  return render(impl_->state.letters(), impl_->state.tone(), impl_->cfg.tone_style, false);
}

std::string Engine::raw_text() const { return impl_->raw_text(); }

bool Engine::is_valid() const { return impl_->valid(); }

std::string Engine::commit_text() {
  std::string out = impl_->valid() ? render(impl_->state.letters(), impl_->state.tone(),
                                            impl_->cfg.tone_style, !impl_->edited)
                                   : impl_->raw_text();
  impl_->clear();
  return out;
}

void Engine::reset() { impl_->clear(); }
bool Engine::empty() const { return impl_->raw.empty(); }

}  // namespace sentype
