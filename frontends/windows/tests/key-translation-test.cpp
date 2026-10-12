// Runs natively on any OS (and under wine for the .exe build).
#include <cstdio>
#include <string>

#include "key-translation.h"
#include "sentype/session.hpp"

using namespace sentype_win;
using sentype::Key;

static int failures = 0;
static void check(bool ok, const char* what) {
  if (!ok) { ++failures; std::printf("FAIL: %s\n", what); }
}

int main() {
  check(translate_key('A', 'a', false).kind == Key::Kind::Char, "char");
  check(translate_key('A', 'a', true).shortcut, "ctrl/alt/win is shortcut");
  check(translate_key(kVkBack, 0, false).kind == Key::Kind::Backspace, "backspace");
  check(translate_key(kVkEscape, 0x1B, false).kind == Key::Kind::Escape, "escape");
  check(translate_key(0x0D, '\r', false).kind == Key::Kind::Other, "enter is Other");
  check(translate_key(0x25, 0, false).kind == Key::Kind::Other, "arrow is Other");
  check(translate_key('A', 0x00E2, false).kind == Key::Kind::Other, "non-ASCII is Other");
  check(is_modifier_vk(0x10) && is_modifier_vk(0xA2) && !is_modifier_vk('A'), "modifiers");

  sentype::Session s;
  for (char c : std::string("vieejt")) s.on_key(translate_key(c - 32, c, false));
  const sentype::KeyResult r = s.on_key(translate_key(0x0D, '\r', false));
  check(r.commit == "việt" && !r.consumed, "enter commits and passes through");
  std::printf("windows key-translation tests, %d failure(s)\n", failures);
  return failures == 0 ? 0 : 1;
}
