// Basic Vietnamese syllable validation (used to decide raw-key restore).
#pragma once

#include "syllable-layout.hpp"

namespace sentype {

// True when the letters form a plausible Vietnamese syllable carrying `tone`.
// Empty input is valid.
bool is_valid_syllable(const Letters& letters, int tone);

// True when the vowel nucleus could still grow into a valid one (typed vowels may yet get marks).
bool nucleus_viable(const Letters& letters);

}  // namespace sentype
