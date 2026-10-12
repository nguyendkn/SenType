#include "sentype/session.hpp"

namespace sentype {

KeyResult Session::flush() {
  KeyResult r;
  if (!engine_.empty()) r.commit = engine_.commit_text();
  return r;
}

KeyResult Session::on_key(const Key& key) {
  // Shortcuts, Enter, Tab, arrows, space, punctuation: commit and pass the key on.
  if (key.shortcut) return flush();
  switch (key.kind) {
    case Key::Kind::Backspace: {
      KeyResult r;
      if (engine_.empty()) return r;  // nothing composing: app handles Backspace
      engine_.backspace();
      r.consumed = true;
      r.preedit = engine_.preedit();
      return r;
    }
    case Key::Kind::Escape: {
      KeyResult r;
      if (engine_.empty()) return r;
      r.commit = engine_.raw_text();  // give back exactly what was typed
      engine_.reset();
      r.consumed = true;
      return r;
    }
    case Key::Kind::Char:
      if (key.ch >= 0x20 && key.ch < 0x7F && engine_.feed(key.ch)) {
        KeyResult r;
        r.consumed = true;
        r.preedit = engine_.preedit();
        return r;
      }
      return flush();
    case Key::Kind::Other:
      break;
  }
  return flush();
}

}  // namespace sentype
