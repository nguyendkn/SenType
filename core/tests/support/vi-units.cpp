#include "vi-units.hpp"

#include <map>

namespace sentype_test {
namespace {

struct Family {
  char base;
  M mark;
  const char32_t* lower;  // forms for tone 0..5
  const char32_t* upper;
};

constexpr Family kFamilies[] = {
    {'a', M::None, U"aáàảãạ", U"AÁÀẢÃẠ"},   {'a', M::Breve, U"ăắằẳẵặ", U"ĂẮẰẲẴẶ"},
    {'a', M::Hat, U"âấầẩẫậ", U"ÂẤẦẨẪẬ"},    {'e', M::None, U"eéèẻẽẹ", U"EÉÈẺẼẸ"},
    {'e', M::Hat, U"êếềểễệ", U"ÊẾỀỂỄỆ"},    {'i', M::None, U"iíìỉĩị", U"IÍÌỈĨỊ"},
    {'o', M::None, U"oóòỏõọ", U"OÓÒỎÕỌ"},    {'o', M::Hat, U"ôốồổỗộ", U"ÔỐỒỔỖỘ"},
    {'o', M::Horn, U"ơớờởỡợ", U"ƠỚỜỞỠỢ"},   {'u', M::None, U"uúùủũụ", U"UÚÙỦŨỤ"},
    {'u', M::Horn, U"ưứừửữự", U"ƯỨỪỬỮỰ"},   {'y', M::None, U"yýỳỷỹỵ", U"YÝỲỶỸỴ"}};


const std::map<char32_t, Unit>& table() {
  static const std::map<char32_t, Unit> t = [] {
    std::map<char32_t, Unit> m;
    for (const Family& f : kFamilies) {
      for (int tone = 0; tone < 6; ++tone) {
        m[f.lower[tone]] = {f.base, f.mark, tone, false};
        m[f.upper[tone]] = {f.base, f.mark, tone, true};
      }
    }
    m[U'đ'] = {'d', M::Stroke, 0, false};
    m[U'Đ'] = {'d', M::Stroke, 0, true};
    for (char c = 'a'; c <= 'z'; ++c) {  // plain consonants; vowels were set above
      if (!m.count(c)) m[c] = {c, M::None, 0, false};
      if (!m.count(c - 32)) m[c - 32] = {c, M::None, 0, true};
    }
    return m;
  }();
  return t;
}

}  // namespace

char32_t next_cp(const std::string& s, size_t& i) {
  const unsigned char c = static_cast<unsigned char>(s[i]);
  int n = c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
  char32_t cp = n == 1 ? c : (c & (0xFF >> (n + 1)));
  for (int k = 1; k < n && i + k < s.size(); ++k) cp = (cp << 6) | (s[i + k] & 0x3F);
  i += n;
  return cp;
}

std::string to_utf8(char32_t cp) {
  std::string o;
  if (cp < 0x80) {
    o += static_cast<char>(cp);
  } else if (cp < 0x800) {
    o += static_cast<char>(0xC0 | (cp >> 6));
    o += static_cast<char>(0x80 | (cp & 0x3F));
  } else {
    o += static_cast<char>(0xE0 | (cp >> 12));
    o += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
    o += static_cast<char>(0x80 | (cp & 0x3F));
  }
  return o;
}

bool decompose(const std::string& s, Units* out) {
  out->clear();
  for (size_t i = 0; i < s.size();) {
    auto it = table().find(next_cp(s, i));
    if (it == table().end()) return false;
    out->push_back(it->second);
  }
  return true;
}

std::string recompose(const Units& units) {
  std::string out;
  for (const Unit& u : units) {
    char32_t cp = 0;
    if (u.base == 'd' && u.mark == M::Stroke) {
      cp = u.upper ? U'Đ' : U'đ';
    } else {
      for (const Family& f : kFamilies) {
        if (f.base == u.base && f.mark == u.mark) cp = (u.upper ? f.upper : f.lower)[u.tone];
      }
      if (!cp) cp = u.upper ? u.base - 32 : u.base;
    }
    out += to_utf8(cp);
  }
  return out;
}

}  // namespace sentype_test
