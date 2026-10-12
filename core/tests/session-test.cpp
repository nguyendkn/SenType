// Tests the shared key policy (what macOS/Windows frontends do) with a simulated client.
#include <cstdio>
#include <string>

#include "sentype/session.hpp"

using sentype::Key;
using sentype::KeyResult;
using sentype::Session;

namespace {

int failures = 0;
void check(bool ok, const std::string& what) {
  if (!ok) {
    ++failures;
    std::printf("FAIL: %s\n", what.c_str());
  }
}

// Minimal client: committed text, current composition, and keys the app received.
struct Client {
  Session session;
  std::string text, marked, passed;

  void apply(const KeyResult& r, char native) {
    text += r.commit;
    marked = r.preedit;
    if (!r.consumed && native) passed += native;
  }
  void key(const Key& k, char native = 0) { apply(session.on_key(k), native); }
  void type(const std::string& s) {
    for (char c : s) key({Key::Kind::Char, static_cast<char32_t>(c), false}, c);
  }
};

}  // namespace

int main() {
  {  // Typing shows preedit only; nothing is committed until the boundary.
    Client c;
    c.type("vieejt");
    check(c.marked == "việt" && c.text.empty() && c.passed.empty(), "preedit only");
    c.type(" ");
    check(c.text == "việt" && c.marked.empty() && c.passed == " ", "space commits, passes through");
  }
  {  // Enter commits the word and passes through (URL bar submits).
    Client c;
    c.type("vieejt");
    c.key({Key::Kind::Other, 0, false}, '\n');
    check(c.text == "việt" && c.passed == "\n", "enter");
  }
  {  // Ctrl+key commits and passes through.
    Client c;
    c.type("hoaf");
    c.key({Key::Kind::Char, 'a', true}, 'A');
    check(c.text == "hoà" && c.passed == "A", "shortcut");
  }
  {  // Backspace edits the word; on empty it passes to the app.
    Client c;
    c.type("hoaf");
    c.key({Key::Kind::Backspace, 0, false}, 'B');
    check(c.marked == "hò" && c.passed.empty(), "backspace in word");
    c.key({Key::Kind::Backspace, 0, false}, 'B');
    c.key({Key::Kind::Backspace, 0, false}, 'B');
    check(c.marked.empty() && c.passed.empty() && c.text.empty(), "backspace empties word");
    c.key({Key::Kind::Backspace, 0, false}, 'B');
    check(c.passed == "B", "backspace on empty passes through");
  }
  {  // Esc returns the raw keys.
    Client c;
    c.type("tooi");
    c.key({Key::Kind::Escape, 0, false}, 'E');
    check(c.text == "tooi" && c.marked.empty() && c.passed.empty(), "escape");
    c.key({Key::Kind::Escape, 0, false}, 'E');
    check(c.passed == "E", "escape on empty passes through");
  }
  {  // Focus loss / deactivate commits once; spellcheck restores non-Vietnamese words.
    Client c;
    c.type("text");
    c.apply(c.session.flush(), 0);
    check(c.text == "text" && c.marked.empty(), "flush restores raw word");
    c.apply(c.session.flush(), 0);
    check(c.text == "text", "second flush commits nothing");
  }
  {  // Punctuation after a word.
    Client c;
    c.type("Vieejt,");
    check(c.text == "Việt" && c.passed == ",", "punctuation");
  }
  std::printf("session tests, %d failure(s)\n", failures);
  return failures == 0 ? 0 : 1;
}
