#include "key-translation.h"

namespace sentype_win {

bool is_modifier_vk(unsigned vk) {
  switch (vk) {
    case 0x10: case 0x11: case 0x12:              // SHIFT, CONTROL, MENU
    case 0xA0: case 0xA1: case 0xA2: case 0xA3:   // L/R SHIFT, L/R CONTROL
    case 0xA4: case 0xA5:                         // L/R MENU
    case 0x14: case 0x5B: case 0x5C:              // CAPITAL, LWIN, RWIN
      return true;
    default:
      return false;
  }
}

sentype::Key translate_key(unsigned vk, char32_t ch, bool shortcut) {
  sentype::Key key;
  key.shortcut = shortcut;
  if (vk == kVkBack) {
    key.kind = sentype::Key::Kind::Backspace;
  } else if (vk == kVkEscape) {
    key.kind = sentype::Key::Kind::Escape;
  } else if (ch >= 0x20 && ch < 0x7F) {
    key.kind = sentype::Key::Kind::Char;
    key.ch = ch;
  }  // else Other: Enter, Tab, arrows, dead keys, non-ASCII
  return key;
}

}  // namespace sentype_win
