// Incremental composing state of one word: letters + one tone, built from raw keys.
#pragma once

#include <string_view>
#include <vector>

#include "sentype/engine.hpp"
#include "syllable-layout.hpp"

namespace sentype {

struct RawKey {
  char32_t ch = 0;
  bool literal = false;  // typed as plain text, never interpreted as a mark
};

enum class Act {
  Letter,  // appended as a letter
  Tone,    // set / cleared the tone
  Mark,    // added a diacritic to an existing letter
  Undo,    // repeated key: caller must drop raw[undo_idx] and add this key as literal
  Reject   // key does nothing here (VNI digit without target): word boundary
};

struct Step {
  Act act = Act::Letter;
  int undo_idx = -1;
};

class WordState {
 public:
  explicit WordState(const Config& cfg) : cfg_(cfg) {}

  void clear();
  // Interpret key `k`, which sits at raw index `idx`. Undo/Reject leave the state untouched.
  Step push(const RawKey& k, int idx);

  const Letters& letters() const { return letters_; }
  int tone() const { return tone_; }
  // owner()[raw idx] = index of the letter the key belongs to, -1 for tone keys.
  const std::vector<int>& owner() const { return owner_; }

 private:
  Step dispatch_telex(const RawKey& k, char32_t c, int idx);
  Step dispatch_vni(const RawKey& k, char32_t c, int idx);
  Step add_letter(const RawKey& k, int idx);
  Step fallback(const RawKey& k, int idx);
  // Repeated mark key cancels the mark only when it directly follows the key that made it.
  Step undo_or_fallback(const RawKey& k, int idx, int key_idx);
  Step set_tone(const RawKey& k, int idx, int tone);
  Step add_mark(const RawKey& k, int idx, std::u32string_view allowed, Mod kind, bool standalone);
  Step add_stroke(const RawKey& k, int idx, bool immediate_only);
  void own(int idx, int letter);

  Config cfg_;
  Letters letters_;
  int tone_ = 0;
  int tone_key_ = -1;
  std::vector<int> owner_;
};

}  // namespace sentype
