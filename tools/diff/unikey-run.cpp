// UniKey (ukengine, LGPL-2.0+, from github.com/fcitx/fcitx5-unikey/unikey) side of the
// differential run. Built by tools/diff/run-diff.sh against a fresh clone; never linked into
// SenType and no UniKey code is vendored. Mirrors fcitx5-unikey's default key flow (macro on =>
// no leading-consonant auto-commit; 'w' at word start is processed; word break commits).
// Options: free marking ON, modern tone style ON, spell check ON, auto non-VN restore ON.
#include <cctype>
#include <iostream>
#include <string>

#include "keycons.h"
#include "ukengine.h"

namespace {

bool is_word_break(unsigned char c) {
  return std::string(",;:.\"'!? <>=+-*/\\_@#$%&(){}[]|").find(static_cast<char>(c)) != std::string::npos;
}

struct Ctx {
  UkSharedMem mem;
  UkEngine engine;
  std::string preedit;  // text of the word being composed
  std::string out;      // committed text
  int shift = 0;

  Ctx(bool vni) {
    SetupUnikeyEngine();
    mem.input.init();
    mem.macStore.init();
    mem.vietKey = true;
    mem.usrKeyMapLoaded = false;
    mem.input.setIM(vni ? UkVni : UkTelex);
    mem.charsetId = CONV_CHARSET_XUTF8;
    mem.options = {};
    mem.options.freeMarking = 1;
    mem.options.modernStyle = 1;
    mem.options.spellCheckEnabled = 1;
    mem.options.autoNonVnRestore = 1;
    engine.setCtrlInfo(&mem);
    engine.setCheckKbCaseFunc([this](int* s, int* c) { *s = shift; *c = 0; });
  }

  void erase_chars(int n) {  // erase n UTF-8 characters from the end of preedit
    while (n > 0 && !preedit.empty()) {
      while (!preedit.empty() && (static_cast<unsigned char>(preedit.back()) & 0xC0) == 0x80) {
        preedit.pop_back();  // continuation bytes
      }
      if (!preedit.empty()) preedit.pop_back();  // lead byte
      --n;
    }
  }

  void key(unsigned char c) {
    shift = std::isupper(c) ? 1 : 0;
    int backs = 0, size = 1024;
    unsigned char buf[1024];
    UkOutputType type;
    engine.process(c, backs, buf, size, type);
    if (backs > 0) erase_chars(backs);
    if (size > 0) preedit.append(reinterpret_cast<char*>(buf), size);
    else preedit += static_cast<char>(c);
    if (!preedit.empty() && static_cast<unsigned char>(preedit.back()) == c && is_word_break(c)) {
      out += preedit;
      preedit.clear();
      engine.reset();
    }
  }
};

}  // namespace

int main() {
  for (std::string line; std::getline(std::cin, line);) {
    const size_t t1 = line.find('\t'), t2 = line.find('\t', t1 + 1);
    if (t2 == std::string::npos) continue;
    Ctx ctx(line[t1 + 1] == 'V');
    for (size_t i = t2 + 1; i < line.size(); ++i) ctx.key(static_cast<unsigned char>(line[i]));
    ctx.key(' ');  // end of line = word break: spell check / restore runs, then commit
    std::string out = ctx.out;
    if (!out.empty() && out.back() == ' ') out.pop_back();
    std::cout << out << '\n';
  }
  return 0;
}
