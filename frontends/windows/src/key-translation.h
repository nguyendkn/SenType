// Maps a Windows virtual-key event to the platform-neutral sentype::Key.
#pragma once

#include "sentype/session.hpp"

namespace sentype_win {

constexpr unsigned kVkBack = 0x08, kVkEscape = 0x1B;

// vk: virtual-key code; ch: character the layout yields for this key (0 if none/dead key);
// shortcut: Ctrl, Alt or Win held. Pure function: unit-testable on any OS.
sentype::Key translate_key(unsigned vk, char32_t ch, bool shortcut);

// Shift/Ctrl/Alt/CapsLock/Win alone must not end the word.
bool is_modifier_vk(unsigned vk);

}  // namespace sentype_win
