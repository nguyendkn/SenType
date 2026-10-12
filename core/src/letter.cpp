#include "letter.hpp"

#include <array>

namespace sentype {
namespace {

// Rows: a ă â e ê i o ô ơ u ư y; columns: tone 0..5.
constexpr std::array<std::u32string_view, 12> kLower = {
    U"aáàảãạ", U"ăắằẳẵặ", U"âấầẩẫậ", U"eéèẻẽẹ", U"êếềểễệ", U"iíìỉĩị",
    U"oóòỏõọ", U"ôốồổỗộ", U"ơớờởỡợ", U"uúùủũụ", U"ưứừửữự", U"yýỳỷỹỵ"};
constexpr std::array<std::u32string_view, 12> kUpper = {
    U"AÁÀẢÃẠ", U"ĂẮẰẲẴẶ", U"ÂẤẦẨẪẬ", U"EÉÈẺẼẸ", U"ÊẾỀỂỄỆ", U"IÍÌỈĨỊ",
    U"OÓÒỎÕỌ", U"ÔỐỒỔỖỘ", U"ƠỚỜỞỠỢ", U"UÚÙỦŨỤ", U"ƯỨỪỬỮỰ", U"YÝỲỶỸỴ"};

// Row index for a vowel letter, or -1 when the letter is not a vowel form.
int row_of(const Letter& l) {
  switch (l.base) {
    case U'a': return l.mod == Mod::Breve ? 1 : l.mod == Mod::Hat ? 2 : 0;
    case U'e': return l.mod == Mod::Hat ? 4 : 3;
    case U'i': return 5;
    case U'o': return l.mod == Mod::Hat ? 7 : l.mod == Mod::Horn ? 8 : 6;
    case U'u': return l.mod == Mod::Horn ? 10 : 9;
    case U'y': return 11;
    default: return -1;
  }
}

}  // namespace

bool is_vowel_base(char32_t b) {
  return b == U'a' || b == U'e' || b == U'i' || b == U'o' || b == U'u' || b == U'y';
}

char32_t symbol(const Letter& l) {
  if (l.base == U'd' && l.mod == Mod::Stroke) return U'đ';
  const int row = row_of(l);
  return row < 0 ? l.base : kLower[row][0];
}

char32_t compose(const Letter& l, int tone) {
  if (l.base == U'd' && l.mod == Mod::Stroke) return l.upper ? U'Đ' : U'đ';
  const int row = row_of(l);
  if (row < 0) {
    return (l.upper && l.base >= U'a' && l.base <= U'z') ? l.base - U'a' + U'A' : l.base;
  }
  return (l.upper ? kUpper : kLower)[row][tone];
}

void append_utf8(std::string& out, char32_t cp) {
  if (cp < 0x80) {
    out += static_cast<char>(cp);
  } else if (cp < 0x800) {
    out += static_cast<char>(0xC0 | (cp >> 6));
    out += static_cast<char>(0x80 | (cp & 0x3F));
  } else if (cp < 0x10000) {
    out += static_cast<char>(0xE0 | (cp >> 12));
    out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    out += static_cast<char>(0x80 | (cp & 0x3F));
  } else {
    out += static_cast<char>(0xF0 | (cp >> 18));
    out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
    out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    out += static_cast<char>(0x80 | (cp & 0x3F));
  }
}

}  // namespace sentype
