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
  const bool shortcut = (flags & (kControl | kOption | kCommand)) != 0;
  const char32_t ch = length == 1 ? chars[0] : 0;  // arrows etc. are private-use chars: not printable
  return sentype::make_key(key_code == kKeyDelete, key_code == kKeyEscape, ch, shortcut);
}

}  // namespace sentype_mac
