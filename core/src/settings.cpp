#include "sentype/settings.hpp"

namespace sentype {
namespace {

bool equals_nocase(std::string_view a, std::string_view lower) {
  if (a.size() != lower.size()) return false;
  for (size_t i = 0; i < a.size(); ++i) {
    const char c = (a[i] >= 'A' && a[i] <= 'Z') ? static_cast<char>(a[i] + 32) : a[i];
    if (c != lower[i]) return false;
  }
  return true;
}

}  // namespace

Method parse_method(std::string_view text) {
  return equals_nocase(text, "vni") ? Method::Vni : Method::Telex;
}

ToneStyle parse_tone_style(std::string_view text) {
  return equals_nocase(text, "classic") ? ToneStyle::Classic : ToneStyle::Modern;
}

const char* to_string(Method m) { return m == Method::Vni ? "vni" : "telex"; }
const char* to_string(ToneStyle s) { return s == ToneStyle::Classic ? "classic" : "modern"; }

}  // namespace sentype
