// Platform-neutral key-event policy shared by the macOS and Windows frontends.
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

  bool composing() const { return !engine_.empty(); }
  Engine& engine() { return engine_; }

 private:
  Engine engine_;
};

}  // namespace sentype
