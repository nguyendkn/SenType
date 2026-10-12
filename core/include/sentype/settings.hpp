// Platform-neutral settings vocabulary shared by every frontend's preference store
// (NSUserDefaults, registry, Fcitx config): one canonical spelling, case-insensitive parsing.
#pragma once

#include <string_view>

#include "sentype/engine.hpp"

namespace sentype {

// Unknown or empty text yields the default (Telex / Modern).
Method parse_method(std::string_view text);          // "telex" | "vni"
ToneStyle parse_tone_style(std::string_view text);   // "modern" | "classic"
const char* to_string(Method m);                      // "telex" | "vni"
const char* to_string(ToneStyle s);                   // "modern" | "classic"

}  // namespace sentype
