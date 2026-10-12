// Test-side decomposition of Vietnamese text into (base letter, mark, tone) units.
// Written independently of core/src/letter.cpp so the tests do not share its tables.
#pragma once

#include <string>
#include <vector>

namespace sentype_test {

enum class M : char { None = 0, Hat = '^', Breve = '(', Horn = '+', Stroke = '-' };

struct Unit {
  char base = 0;  // ASCII a-z (lowercase)
  M mark = M::None;
  int tone = 0;  // 0 none, 1 acute, 2 grave, 3 hook, 4 tilde, 5 dot
  bool upper = false;
};
using Units = std::vector<Unit>;

// Decodes one UTF-8 code point at s[i], advancing i.
char32_t next_cp(const std::string& s, size_t& i);
// Decomposes a lowercase or Capitalized NFC syllable; false if a char is not Vietnamese.
bool decompose(const std::string& syllable, Units* out);
// Re-encodes units as NFC UTF-8 (tone placed on the unit that carries it).
std::string recompose(const Units& units);
std::string to_utf8(char32_t cp);

}  // namespace sentype_test
