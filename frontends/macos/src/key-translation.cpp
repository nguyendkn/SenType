#include "key-translation.h"

namespace sentype_mac {
namespace {

// NSEventModifierFlag raw values (AppKit/NSEvent.h), duplicated so this file needs no AppKit.
constexpr unsigned long kControl = 1ul << 18;
constexpr unsigned long kOption = 1ul << 19;
constexpr unsigned long kCommand = 1ul << 20;

constexpr unsigned short kKeyDelete = 51;  // Backspace
constexpr unsigned short kKeyEscape = 53;

}  // namespace

sentype::Key translate_key(const char16_t* chars, unsigned long length, unsigned short key_code,
                           unsigned long flags) {
  sentype::Key key;
  key.shortcut = (flags & (kControl | kOption | kCommand)) != 0;
  if (key_code == kKeyDelete) {
    key.kind = sentype::Key::Kind::Backspace;
  } else if (key_code == kKeyEscape) {
    key.kind = sentype::Key::Kind::Escape;
  } else if (length == 1 && chars[0] >= 0x20 && chars[0] < 0x7F) {
    key.kind = sentype::Key::Kind::Char;
    key.ch = chars[0];
  }  // else Other: Enter, Tab, arrows (private-use chars), non-ASCII
  return key;
}

}  // namespace sentype_mac
