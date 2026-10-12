// Table-driven tests of the SenType core with real Vietnamese data (no mocks).
// Key strings: '<' = Backspace.
#include <cstdio>
#include <string>
#include <vector>

#include "sentype/engine.hpp"
#include "support/engine-cases.hpp"

using sentype::Config;
using sentype::Engine;
using sentype::Method;
using sentype::ToneStyle;

using sentype_test::Case;
using sentype_test::kCases;

namespace {

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
    const std::string tag = std::string(c.method == Method::Telex ? "telex " : "vni ") + c.keys;
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
