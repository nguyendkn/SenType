// Maps a native macOS key event to the platform-neutral sentype::Key.
#pragma once

#include "sentype/session.hpp"

namespace sentype_mac {

// characters: NSEvent.characters (layout + shift applied); flags: NSEvent.modifierFlags raw value.
// Pure function (no NSEvent) so it can be unit-tested without a window server.
sentype::Key translate_key(const char16_t* chars, unsigned long length, unsigned short key_code,
                           unsigned long flags);

}  // namespace sentype_mac
