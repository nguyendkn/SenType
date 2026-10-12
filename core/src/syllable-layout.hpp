// Syllable structure: initial consonants, vowel nucleus, coda; tone placement.
#pragma once

#include <string>
#include <vector>

#include "sentype/engine.hpp"
#include "letter.hpp"

namespace sentype {

using Letters = std::vector<Letter>;

struct Layout {
  size_t nuc_begin = 0;  // [nuc_begin, nuc_end) is the vowel nucleus
  size_t nuc_end = 0;    // letters after nuc_end form the coda
};

// "qu" and "gi" (when another vowel follows) count as consonants.
Layout analyze(const Letters& letters);
// Open "ươ" (no coda) is written "uơ" (thuở, huơ).
Letters normalized(const Letters& letters);
// Index of the vowel that carries the tone, or -1.
int tone_index(const Letters& letters, const Layout& lay, ToneStyle style);
// Composed UTF-8 text with the tone placed. `final_form` also applies normalized().
std::string render(const Letters& letters, int tone, ToneStyle style, bool final_form);

}  // namespace sentype
