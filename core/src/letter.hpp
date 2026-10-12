// A single typed letter of the word plus its Vietnamese diacritic mark.
#pragma once

#include <string>

namespace sentype {

enum class Mod : unsigned char { None, Hat, Breve, Horn, Stroke };

struct Letter {
  char32_t base = 0;  // lowercase ASCII letter, or a literal char (e.g. a digit)
  Mod mod = Mod::None;
  bool upper = false;
  int mod_key = -1;  // raw-key index that produced `mod`
};

bool is_vowel_base(char32_t base);
// Lowercase Vietnamese letter for validation, e.g. U'ê', U'đ'.
char32_t symbol(const Letter& l);
// NFC code point of the letter carrying tone 0..5 (none, acute, grave, hook, tilde, dot).
char32_t compose(const Letter& l, int tone);
void append_utf8(std::string& out, char32_t cp);

}  // namespace sentype
