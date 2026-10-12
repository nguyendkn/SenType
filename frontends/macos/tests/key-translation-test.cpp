// Native macOS key translation + full Session flow with real NSEvent-shaped inputs.
#include <cstdio>
#include <string>

#include "sentype/session.hpp"
#include "key-translation.h"

using sentype::Key;
using sentype_mac::translate_key;

namespace {
int failures = 0;
void check(bool ok, const char* what) {
  if (!ok) { ++failures; std::printf("FAIL: %s\n", what); }
}
}  // namespace

int main() {
  const char16_t a[] = u"a", cr[] = u"\r", up[] = u"", vi[] = u"â";
  check(translate_key(a, 1, 0, 0).kind == Key::Kind::Char, "plain char");
  check(translate_key(a, 1, 0, 0).ch == 'a' && !translate_key(a, 1, 0, 0).shortcut, "char value");
  check(translate_key(a, 1, 0, 1ul << 20).shortcut, "cmd is shortcut");
  check(translate_key(a, 1, 0, 1ul << 18).shortcut, "ctrl is shortcut");
  check(translate_key(a, 1, 0, 1ul << 19).shortcut, "option is shortcut");
  check(!translate_key(a, 1, 0, 1ul << 17).shortcut, "shift is not a shortcut");
  check(translate_key(a, 0, 51, 0).kind == Key::Kind::Backspace, "delete key");
  check(translate_key(a, 0, 53, 0).kind == Key::Kind::Escape, "escape key");
  check(translate_key(cr, 1, 36, 0).kind == Key::Kind::Other, "return is Other");
  check(translate_key(up, 1, 126, 0).kind == Key::Kind::Other, "arrow is Other");
  check(translate_key(vi, 1, 0, 0).kind == Key::Kind::Other, "non-ASCII is Other");

  // Drive a Session the way the controller does: Enter commits then passes through.
  sentype::Session s;
  for (char16_t c : std::u16string(u"vieejt")) s.on_key(translate_key(&c, 1, 0, 0));
  sentype::KeyResult r = s.on_key(translate_key(cr, 1, 36, 0));
  check(r.commit == "việt" && !r.consumed, "enter commits and passes through");
  std::printf("macos key-translation tests, %d failure(s)\n", failures);
  return failures == 0 ? 0 : 1;
}
