// Platform-neutral key-event policy shared by the Fcitx5, macOS and Windows frontends. The policy:
//   - printable ASCII feeds the word; everything else ends it (commit, key passes through)
//   - Ctrl/Alt/Cmd/Win + key: commit, shortcut passes through
//   - Enter/Tab/arrows/space/punctuation: commit, key passes through (Enter still submits)
//   - Backspace: edits the word while composing, otherwise passes through
//   - Escape: gives back exactly the keys typed, otherwise passes through
//   - focus loss / deactivate / client reset: flush() commits what is pending
// A frontend translates its native event to Key, calls on_key(), then:
//   1. inserts result.commit (if any) into the client,
//   2. shows result.preedit as the composition (empty = end composition),
//   3. swallows the native key iff result.consumed, otherwise passes it through.
#pragma once

#include <string>

#include "sentype/engine.hpp"

namespace sentype {

struct Key {
  enum class Kind { Char, Backspace, Escape, Other };
  Kind kind = Kind::Other;  // Other: Enter, Tab, arrows, function keys...
  char32_t ch = 0;          // Kind::Char: printable character after layout/shift
  bool shortcut = false;    // Ctrl/Alt/Cmd/Win held
};

// Builds a Key from already-decoded native facts. `ch` is the character the key produces after
// layout/shift (0 if none); only printable ASCII becomes Kind::Char.
Key make_key(bool backspace, bool escape, char32_t ch, bool shortcut);

struct KeyResult {
  bool consumed = false;  // swallow the native key
  std::string commit;     // UTF-8 text to insert before updating the composition
  std::string preedit;    // UTF-8 composition to display; empty = none
};

class Session {
 public:
  explicit Session(Config cfg = {}) : engine_(cfg) {}

  KeyResult on_key(const Key& key);
  // Word boundary without a key (focus loss, deactivate): commit what is pending.
  KeyResult flush();
  // Drop the word without committing (e.g. client reset).
  void reset() { engine_.reset(); }
  // New settings; the word in progress is dropped (flush() first to keep it).
  void set_config(const Config& cfg) { engine_.set_config(cfg); }

  // For hosts that ask before delivering the key (Windows TSF OnTestKeyDown): true when on_key()
  // would do something other than ignore it (a word is pending, or a plain character starts one).
  bool would_handle(const Key& key) const {
    return composing() || (key.kind == Key::Kind::Char && !key.shortcut);
  }

  bool composing() const { return !engine_.empty(); }
  Engine& engine() { return engine_; }

 private:
  Engine engine_;
};

}  // namespace sentype
