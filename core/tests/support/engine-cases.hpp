// Table-driven engine cases (real Vietnamese data, no mocks). Key strings: '<' = Backspace.
#pragma once

#include <vector>

#include "sentype/engine.hpp"

namespace sentype_test {

using sentype::Method;
using sentype::ToneStyle;

struct Case {
  Method method;
  ToneStyle style;
  const char* keys;
  const char* preedit;  // composed text while typing
  const char* commit;   // text committed at the end of the word
};

constexpr auto T = Method::Telex;
constexpr auto V = Method::Vni;
constexpr auto M = ToneStyle::Modern;
constexpr auto C = ToneStyle::Classic;

const std::vector<Case> kCases = {
    // Telex: words from the task.
    {T, M, "Vieejt", "Việt", "Việt"},
    {T, M, "nguwowif", "người", "người"},
    {T, M, "dduowcj", "được", "được"},
    {T, M, "hoaf", "hoà", "hoà"},
    {T, C, "hoaf", "hòa", "hòa"},
    {T, M, "thuys", "thuý", "thuý"},
    {T, C, "thuys", "thúy", "thúy"},
    // Modifiers.
    {T, M, "aa", "â", "â"},
    {T, M, "aw", "ă", "ă"},
    {T, M, "ow", "ơ", "ơ"},
    {T, M, "uw", "ư", "ư"},
    {T, M, "oo", "ô", "ô"},
    {T, M, "ee", "ê", "ê"},
    {T, M, "dd", "đ", "dd"},  // lone "đ" is not a word: raw restored
    {T, M, "ddi", "đi", "đi"},
    {T, M, "DDuwowngf", "Đường", "Đường"},
    {T, M, "tuoij", "tuọi", "tuoij"},  // "uoi" without ô is not Vietnamese
    {T, M, "thuowngf", "thường", "thường"},
    {T, M, "thuowr", "thưở", "thuở"},  // open "ươ" is committed as "uơ"
    {T, M, "quans", "quán", "quán"},
    {T, M, "quys", "quý", "quý"},
    {T, M, "gis", "gí", "gí"},
    {T, M, "giaos", "giáo", "giáo"},
    {T, M, "tieengs", "tiếng", "tiếng"},
    {T, M, "toans", "toán", "toán"},
    {T, M, "hoanf", "hoàn", "hoàn"},
    {T, M, "khoer", "khoẻ", "khoẻ"},
    {T, C, "khoer", "khỏe", "khỏe"},
    {T, M, "muaf", "mùa", "mùa"},
    {T, M, "nhuwx", "nhữ", "nhữ"},
    {T, M, "nghieeng", "nghiêng", "nghiêng"},
    // Free marking: tone typed before the end of the word.
    {T, M, "vieejt", "việt", "việt"},
    {T, M, "toasn", "toán", "toán"},
    {T, M, "vietj", "viẹt", "vietj"},
    // Tone change, tone removal.
    {T, M, "asf", "à", "à"},
    {T, M, "asz", "a", "a"},
    // Undo by repeating the key.
    // Repeating a mark key cancels it in the preedit; an invalid word restores every key pressed
    // ("class" stays "class", not "clas").
    {T, M, "ass", "as", "ass"},
    {T, M, "class", "clas", "class"},
    {T, M, "access", "acces", "access"},
    {T, M, "booongs", "boóng", "boóng"},  // oo + third o cancels the circumflex
    {T, M, "khoeo", "khoeo", "khoeo"},     // o after "oe" is a plain letter, not "khôe"
    {T, M, "oow", "ơ", "ơ"},               // a different mark key switches the mark (ô -> ơ)
    {T, M, "awan", "ân", "ân"},             // ă -> â
    {V, M, "o67", "ơ", "ơ"},
    {T, M, "ruow", "rươ", "ruow"},          // open "ươ" only after h/kh/th: restored
    {T, M, "huuw", "hưu", "hưu"},          // horn goes on the first u
    {T, M, "aaa", "aa", "aaa"},
    {T, M, "ddd", "dd", "ddd"},
    {T, M, "oww", "ow", "oww"},
    {T, M, "ww", "w", "ww"},
    {T, M, "vieesst", "viêst", "vieesst"},
    // Backspace: remove the last letter, keep tone.
    {T, M, "Vieejt<", "Việ", "Việ"},
    {T, M, "nguwowif<", "ngườ", "ngườ"},
    {T, M, "dduowc<<", "đư", "đư"},
    {T, M, "as<", "", ""},
    {T, M, "hoaf<", "hò", "hò"},
    {T, M, "aa<", "", ""},
    {T, M, "Viet<<<<", "", ""},
    // Invalid words restore raw keys on commit (spellcheck).
    {T, M, "text", "tẽt", "text"},
    {T, M, "window", "ưindow", "window"},
    {T, M, "Windows", "Ứindow", "Windows"},
    {T, M, "fun", "fun", "fun"},
    {T, M, "hello", "hello", "hello"},
    {T, M, "world", "*", "world"},
    {T, M, "google", "gôgle", "google"},
    // VNI.
    {V, M, "Vie6t5", "Việt", "Việt"},
    {V, M, "ngu7o7i2", "người", "người"},
    {V, M, "d9uo7c5", "được", "được"},
    {V, M, "d9i", "đi", "đi"},
    {V, M, "tuo6i3", "tuổi", "tuổi"},
    {V, M, "tuoi1", "tuói", "tuoi1"},  // not Vietnamese ("tuôi"/"tuổi" need ô): restored
    {V, M, "hoa2", "hoà", "hoà"},
    {V, C, "hoa2", "hòa", "hòa"},
    {V, M, "thuy1", "thuý", "thuý"},
    {V, M, "a6", "â", "â"},
    {V, M, "a8", "ă", "ă"},
    {V, M, "o7", "ơ", "ơ"},
    {V, M, "u7", "ư", "ư"},
    {V, M, "a1", "á", "á"},
    {V, M, "a11", "a1", "a11"},
    {V, M, "a66", "a6", "a66"},
    {V, M, "d99", "d9", "d99"},
    {V, M, "a12", "à", "à"},
    {V, M, "Viet65", "Việt", "Việt"},
    {V, M, "Vie6t5<", "Việ", "Việ"},
    {V, M, "text2", "tèxt", "text2"},
};

}  // namespace sentype_test
