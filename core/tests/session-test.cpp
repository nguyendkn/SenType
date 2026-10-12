// Tests the shared key policy (what macOS/Windows frontends do) with a simulated client.
#include <cstdio>
#include <string>

#include "sentype/session.hpp"
#include "sentype/settings.hpp"

using sentype::Config;
using sentype::Key;
using sentype::make_key;
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
  {  // make_key: the one place native facts become a Key.
    check(make_key(false, false, 'a', false).kind == Key::Kind::Char, "make_key char");
    check(make_key(false, false, 'a', true).shortcut, "make_key shortcut");
    check(make_key(true, false, 'a', false).kind == Key::Kind::Backspace, "make_key backspace");
    check(make_key(false, true, 0, false).kind == Key::Kind::Escape, "make_key escape");
    check(make_key(false, false, '\r', false).kind == Key::Kind::Other, "make_key enter");
    check(make_key(false, false, 0x1EA1, false).kind == Key::Kind::Other, "make_key non-ASCII");
  }
  {  // would_handle: idle sessions ignore everything but plain characters.
    Session s;
    check(!s.would_handle({Key::Kind::Other, 0, false}), "idle: Enter not handled");
    check(!s.would_handle({Key::Kind::Char, 'a', true}), "idle: shortcut not handled");
    check(s.would_handle({Key::Kind::Char, 'a', false}), "idle: plain char handled");
    s.on_key({Key::Kind::Char, 'a', false});
    check(s.would_handle({Key::Kind::Other, 0, false}), "composing: Enter must reach on_key");
  }
  {  // Settings vocabulary + switching config mid-session.
    check(sentype::parse_method("VNI") == sentype::Method::Vni, "parse VNI");
    check(sentype::parse_method("") == sentype::Method::Telex, "default Telex");
    check(sentype::parse_tone_style("Classic") == sentype::ToneStyle::Classic, "parse Classic");
    check(std::string(sentype::to_string(sentype::Method::Vni)) == "vni", "to_string");
    Client c;
    c.type("hoaf");
    c.apply(c.session.flush(), 0);
    c.session.set_config(Config{sentype::Method::Vni, sentype::ToneStyle::Classic, true});
    c.type("hoa2");
    c.apply(c.session.flush(), 0);
    check(c.text == "hoàhòa", "config switch: modern then classic VNI");
  }
  std::printf("session tests, %d failure(s)\n", failures);
  return failures == 0 ? 0 : 1;
}
