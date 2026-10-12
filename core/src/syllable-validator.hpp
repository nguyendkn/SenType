// Basic Vietnamese syllable validation (used to decide raw-key restore).
#pragma once

#include "syllable-layout.hpp"

namespace sentype {

// True when the letters form a plausible Vietnamese syllable carrying `tone`.
// Empty input is valid.
bool is_valid_syllable(const Letters& letters, int tone);

}  // namespace sentype
