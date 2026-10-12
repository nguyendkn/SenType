// Real-data test: every syllable of an independent Vietnamese dictionary (hunspell vi_VN)
// must be accepted by the validator and be typeable in Telex and VNI in several orders.
// Usage: syllable-corpus-test <vi-syllables.txt> [english-words.txt] [non-native.txt] [must-restore.txt]
// Exit: 0 ok, 1 failures, 77 data missing (ctest SKIP).
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "sentype/engine.hpp"
#include "support/test-util.hpp"
#include "support/typing-orders.hpp"
#include "syllable-validator.hpp"

using namespace sentype;
using namespace sentype_test;

namespace {

std::string type_word(Method m, ToneStyle st, const std::string& keys) {
  Engine e(Config{m, st, true});
  for (size_t i = 0; i < keys.size(); ++i) {
    if (!e.feed(static_cast<unsigned char>(keys[i]))) {  // boundary: commit, key passes through
      return e.commit_text() + keys.substr(i);
    }
  }
  return e.commit_text();
}

Letters to_letters(const Units& u) {
  Letters L;
  for (const Unit& x : u) {
    Letter l;
    l.base = x.base;
    l.upper = x.upper;
    switch (x.mark) {
      case M::Hat: l.mod = Mod::Hat; break;
      case M::Breve: l.mod = Mod::Breve; break;
      case M::Horn: l.mod = Mod::Horn; break;
      case M::Stroke: l.mod = Mod::Stroke; break;
      default: break;
    }
    L.push_back(l);
  }
  return L;
}

int tone_of(const Units& u) {
  for (const Unit& x : u) {
    if (x.tone) return x.tone;
  }
  return 0;
}

std::string upper_keys(std::string k) {
  for (char& c : k) {
    if (c >= 'a' && c <= 'z') c -= 32;
  }
  return k;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) return 77;
  const std::vector<std::string> syllables = read_lines(argv[1]);
  if (syllables.empty()) {
    std::printf("SKIP: no syllables in %s (run tools/fetch-test-data.sh)\n", argv[1]);
    return 77;
  }
  std::set<std::string> truth(syllables.begin(), syllables.end());
  const std::set<std::string> excluded = [&] {
    std::set<std::string> e;
    if (argc > 3) for (const std::string& w : read_lines(argv[3])) e.insert(w);
    return e;
  }();
  std::printf("dictionary: %zu syllables, %zu excluded as non-native -> ground truth %zu\n",
              truth.size(), excluded.size(), truth.size() - excluded.size());

  std::map<std::string, Tally> t;
  int modern_only = 0, classic_only = 0, both = 0;
  for (const std::string& syl : syllables) {
    if (excluded.count(syl)) continue;
    Units u;
    if (!decompose(syl, &u)) {
      t["decomposable"].add(false, syl);
      continue;
    }
    t["decomposable"].add(true, "");
    t["validator accepts"].add(is_valid_syllable(to_letters(u), tone_of(u)), syl);

    for (int vni = 0; vni < 2; ++vni) {
      for (Order o : kAllOrders) {
        const std::string keys = vni ? vni_keys(u, o) : telex_keys(u, o);
        if (keys.empty()) continue;
        const Method m = vni ? Method::Vni : Method::Telex;
        const std::string a = type_word(m, ToneStyle::Modern, keys);
        const std::string b = type_word(m, ToneStyle::Classic, keys);
        if (vni == 0 && o == Order::ToneAtEnd) {  // which tone style does the dictionary use?
          if (a == syl && b == syl) ++both;
          else if (a == syl) ++modern_only;
          else if (b == syl) ++classic_only;
        }
        t[std::string(vni ? "VNI " : "Telex ") + order_name(o)].add(
            a == syl, syl + " <- " + keys + " => " + a);
      }
    }
    // Capitalised and ALL-CAPS typing (Shift held), Telex and VNI.
    Units cap = u, all = u;
    cap[0].upper = true;
    for (Unit& x : all) x.upper = true;
    for (int vni = 0; vni < 2; ++vni) {
      const Method m = vni ? Method::Vni : Method::Telex;
      const std::string kc = vni ? vni_keys(cap, Order::ToneAtEnd) : telex_keys(cap, Order::ToneAtEnd);
      const std::string ka = upper_keys(vni ? vni_keys(all, Order::ToneAtEnd) : telex_keys(all, Order::ToneAtEnd));
      const std::string ec = recompose(cap), ea = recompose(all);
      auto ok = [&](const std::string& keys, const std::string& want) {
        return type_word(m, ToneStyle::Modern, keys) == want;  // the dictionary uses modern placement
      };
      t[std::string(vni ? "VNI " : "Telex ") + "Capitalised"].add(ok(kc, ec), ec + " <- " + kc);
      t[std::string(vni ? "VNI " : "Telex ") + "ALL CAPS"].add(ok(ka, ea), ea + " <- " + ka);
    }
  }

  // Hard test: known English/technical words must come back exactly as typed.
  if (argc > 4) {
    for (const std::string& w : read_lines(argv[4])) {
      for (int cap = 0; cap < 2; ++cap) {
        std::string word = w;
        if (cap) word[0] -= 32;
        t["English words restored"].add(type_word(Method::Telex, ToneStyle::Modern, word) == word,
                                        word + " => " + type_word(Method::Telex, ToneStyle::Modern, word));
      }
    }
  }

  // Negatives: raw keys must come back unless the engine composed a real syllable.
  std::vector<std::string> words = argc > 2 ? read_lines(argv[2]) : std::vector<std::string>{};
  std::mt19937 rng(12345);  // fixed seed: deterministic
  for (int i = 0; i < 20000; ++i) {
    std::string w(1 + rng() % 8, 'a');
    for (char& c : w) c = "abcdefghijklmnopqrstuvwxyz"[rng() % 26];
    words.push_back(w);
  }
  int restored = 0, composed = 0;
  Tally neg_telex, neg_vni;
  for (const std::string& w : words) {
    for (int vni = 0; vni < 2; ++vni) {
      std::string keys = w;
      if (vni) keys += static_cast<char>('0' + w.size() % 10);  // a trailing tone/mark digit
      const std::string out = type_word(vni ? Method::Vni : Method::Telex, ToneStyle::Modern, keys);
      if (out == keys) {
        ++restored;
        continue;
      }
      ++composed;
      (vni ? neg_vni : neg_telex).add(truth.count(out) > 0,
                                      keys + " => " + out + " (not in dictionary)");
    }
  }
  std::printf("negatives: %zu words x2 methods: %d restored raw, %d composed\n", words.size(),
              restored, composed);

  bool failed = false;
  for (const auto& [name, tally] : t) {  // map order: ALL CAPS ... validator accepts
    tally.print(name);
    failed |= tally.failed();
  }
  // Informational only: the dictionary is a lexicon, so a legal-but-unattested syllable such as
  // "bẻn" counts as a miss here. Over-acceptance candidates are listed with SENTYPE_VERBOSE=1.
  neg_telex.print("[info] Telex composed word in dictionary");
  neg_vni.print("[info] VNI composed word in dictionary");
  std::printf("tone style of dictionary (Telex tone-at-end): modern-only %d, classic-only %d, both %d\n",
              modern_only, classic_only, both);
  return failed ? 1 : 0;
}
