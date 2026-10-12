// Table-driven tests of the SenType core with real Vietnamese data (no mocks).
// Key strings: '<' = Backspace.
#include <cstdio>
#include <string>
#include <vector>

#include "sentype/engine.hpp"

using sentype::Config;
using sentype::Engine;
using sentype::Method;
using sentype::ToneStyle;

namespace {

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
    {T, M, "ass", "as", "as"},
    {T, M, "aaa", "aa", "aa"},
    {T, M, "ddd", "dd", "dd"},
    {T, M, "oww", "ow", "ow"},
    {T, M, "ww", "w", "w"},
    {T, M, "vieesst", "viêst", "vieest"},
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
    {V, M, "a11", "a1", "a1"},
    {V, M, "a66", "a6", "a6"},
    {V, M, "d99", "d9", "d9"},
    {V, M, "a12", "à", "à"},
    {V, M, "Viet65", "Việt", "Việt"},
    {V, M, "Vie6t5<", "Việ", "Việ"},
    {V, M, "text2", "tèxt", "text2"},
};

// Feeds keys like the addon: boundary keys flush the word.
std::string type(Engine& e, const char* keys, std::string* committed) {
  committed->clear();
  for (const char* p = keys; *p; ++p) {
    if (*p == '<') {
      e.backspace();
    } else if (!e.feed(static_cast<unsigned char>(*p))) {
      *committed += e.commit_text();
      *committed += *p;
    }
  }
  return e.preedit();
}

bool has_combining_mark(const std::string& s) {
  for (size_t i = 0; i + 1 < s.size(); ++i) {
    // U+0300..U+036F = CC 80 .. CD AF in UTF-8
    const unsigned char a = s[i], b = s[i + 1];
    if ((a == 0xCC) || (a == 0xCD && b <= 0xAF)) return true;
  }
  return false;
}

int failures = 0;
void check(bool ok, const std::string& what) {
  if (!ok) {
    ++failures;
    std::printf("FAIL %s\n", what.c_str());
  }
}

}  // namespace

int main() {
  int n = 0;
  for (const Case& c : kCases) {
    Engine e(Config{c.method, c.style, true});
    std::string committed;
    const std::string pre = type(e, c.keys, &committed);
    const std::string done = e.commit_text();
    const std::string tag = std::string(c.method == T ? "telex " : "vni ") + c.keys;
    if (std::string(c.preedit) != "*") {  // "*": preedit not asserted
      check(pre == c.preedit, tag + ": preedit '" + pre + "' != '" + c.preedit + "'");
    }
    check(done == c.commit, tag + ": commit '" + done + "' != '" + c.commit + "'");
    check(!has_combining_mark(pre) && !has_combining_mark(done), tag + ": not NFC");
    check(e.empty(), tag + ": engine not empty after commit");
    ++n;
  }

  {  // Boundary keys: space/punctuation flush the word and pass through.
    Engine e;
    std::string committed;
    type(e, "Vieejt nam,", &committed);
    check(committed == "Việt nam,", "boundary: got '" + committed + "'");
    check(e.empty(), "boundary: empty");
  }
  {  // Spellcheck off keeps the composed text even when it is not Vietnamese.
    Config cfg;
    cfg.spellcheck = false;
    Engine e(cfg);
    std::string committed;
    type(e, "text", &committed);
    check(e.commit_text() == "tẽt", "spellcheck off");
  }
  {  // raw_text and reset.
    Engine e;
    std::string committed;
    type(e, "Vieejt", &committed);
    check(e.raw_text() == "Vieejt", "raw_text");
    e.reset();
    check(e.empty() && e.preedit().empty(), "reset");
    check(!e.backspace(), "backspace on empty buffer is not consumed");
  }
  {  // VNI digits at word start are boundaries, not word keys.
    Engine e(Config{Method::Vni, ToneStyle::Modern, true});
    check(!e.feed('1') && e.empty(), "vni leading digit");
    check(e.feed('k') && !e.feed('1') && e.preedit() == "k", "vni digit without vowel");
  }

  std::printf("%d table cases + 4 scenario groups, %d failure(s)\n", n, failures);
  return failures == 0 ? 0 : 1;
}
