#include "typing-orders.hpp"

namespace sentype_test {
namespace {

struct Scheme {
  bool telex;
  char mark_key(const Unit& u) const {
    if (telex) {
      switch (u.mark) {
        case M::Hat: return u.base;  // aa ee oo
        case M::Breve: case M::Horn: return 'w';
        default: return 0;
      }
    }
    switch (u.mark) {
      case M::Hat: return '6';
      case M::Horn: return '7';
      case M::Breve: return '8';
      default: return 0;
    }
  }
  char tone_key(int tone) const {
    static const char kTelex[] = "zsfrxj";
    return telex ? kTelex[tone] : static_cast<char>('0' + tone);
  }
  char stroke_key() const { return telex ? 'd' : '9'; }
};

bool is_vowel(char c) { return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'y'; }

bool is_horn(const Units& u, size_t i, char base) {
  return i < u.size() && u[i].base == base && u[i].mark == M::Horn;
}

// Plain "oo"/"ee"/"aa" would turn into a circumflex: a third key cancels it.
bool needs_cancel(const Units& u, size_t i, bool telex) {
  return telex && i > 0 && u[i].mark == M::None && u[i - 1].mark == M::None &&
         u[i].base == u[i - 1].base && (u[i].base == 'a' || u[i].base == 'e' || u[i].base == 'o');
}

std::string generate(const Units& u, Order order, const Scheme& s) {
  size_t tone_at = u.size();
  for (size_t i = 0; i < u.size(); ++i) {
    if (u[i].tone) tone_at = i;
  }
  const bool free_marks = order == Order::FreeMarks || order == Order::FreeToneFirst;
  const bool tone_early = order == Order::ToneAfterNuc || order == Order::FreeToneFirst;
  if (tone_early && tone_at == u.size()) return "";
  size_t nuc_end = tone_at;  // last vowel of the run that holds the toned unit
  while (nuc_end + 1 < u.size() && is_vowel(u[nuc_end + 1].base)) ++nuc_end;

  std::string keys, late;
  bool late_mark = false;
  for (size_t i = 0; i < u.size(); ++i) {
    keys += u[i].upper ? static_cast<char>(u[i].base - 32) : u[i].base;
    if (needs_cancel(u, i, s.telex)) keys += u[i].base;
    char mk = u[i].mark == M::Stroke ? s.stroke_key() : s.mark_key(u[i]);
    if (mk) {
      const bool late_ok = free_marks && (u[i].mark != M::Stroke || !s.telex);  // Telex "dd" is immediate
      if (!late_ok) {
        keys += mk;
      } else if (!(is_horn(u, i, 'o') && is_horn(u, i - 1, 'u'))) {  // one key marks the "ươ" pair
        late += mk;
        late_mark = true;
      }
    }
    if (tone_early && i == nuc_end) keys += s.tone_key(u[tone_at].tone);
  }
  if (!tone_early && tone_at < u.size()) late += s.tone_key(u[tone_at].tone);
  if (free_marks && !late_mark) return "";  // nothing was typed late: duplicate of another order
  return keys + late;
}

}  // namespace

const char* order_name(Order o) {
  switch (o) {
    case Order::ToneAtEnd: return "tone-at-end";
    case Order::ToneAfterNuc: return "tone-after-nucleus";
    case Order::FreeMarks: return "free-marks";
    case Order::FreeToneFirst: return "free-tone-first";
  }
  return "?";
}

std::string telex_keys(const Units& u, Order o) { return generate(u, o, Scheme{true}); }
std::string vni_keys(const Units& u, Order o) { return generate(u, o, Scheme{false}); }

}  // namespace sentype_test
