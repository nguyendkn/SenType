// Generates identical key-sequence cases for the differential run.
// Output lines: <label>\t<T|V>\t<keys>   (keys may contain spaces between words)
// Usage: gen-inputs <vi-syllables.txt> <sentences.txt> <english.txt> <random-count> <seed>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "support/typing-orders.hpp"

using namespace sentype_test;

namespace {

std::vector<std::string> lines(const char* path) {
  std::vector<std::string> v;
  std::ifstream in(path);
  for (std::string l; std::getline(in, l);) {
    if (!l.empty() && l[0] != '#') v.push_back(l);
  }
  return v;
}

std::string keys_for(const std::string& syl, bool vni, Order o) {
  Units u;
  if (!decompose(syl, &u)) return "";
  return vni ? vni_keys(u, o) : telex_keys(u, o);
}

void emit(const char* label, bool vni, const std::string& keys) {
  if (!keys.empty()) std::printf("%s\t%c\t%s\n", label, vni ? 'V' : 'T', keys.c_str());
}

// Lowercase syllable words of a sentence; uppercase initial letters become Capitalised words.
std::string sentence_keys(const std::string& text, bool vni, std::mt19937& rng) {
  std::istringstream in(text);
  std::string word, out;
  while (in >> word) {
    Units u;
    if (!decompose(word, &u)) return "";
    const Order o = kAllOrders[rng() % 2];  // inline marks, tone at end / after nucleus
    std::string k = vni ? vni_keys(u, o) : telex_keys(u, o);
    if (k.empty()) k = vni ? vni_keys(u, Order::ToneAtEnd) : telex_keys(u, Order::ToneAtEnd);
    out += (out.empty() ? "" : " ") + k;
  }
  return out;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 6) return 2;
  const std::vector<std::string> syl = lines(argv[1]), sent = lines(argv[2]), words = lines(argv[3]);
  const long random_count = std::atol(argv[4]);
  std::mt19937 rng(static_cast<unsigned>(std::atol(argv[5])));

  for (const std::string& s : syl) {
    for (int vni = 0; vni < 2; ++vni) {
      for (Order o : kAllOrders) emit("syl", vni, keys_for(s, vni, o));
    }
  }
  for (const std::string& s : sent) {
    for (int vni = 0; vni < 2; ++vni) emit("sent", vni, sentence_keys(s, vni, rng));
  }
  for (const std::string& w : words) {
    emit("eng", false, w);
    emit("eng", true, w);
    std::string cap = w;
    cap[0] -= 32;
    emit("eng", false, cap);
  }
  // Random typings drawn from each method's real key set (letters + its mark/tone keys).
  static const char kTelex[] = "aaaaeeeeiioooouuuyadwsfrxjzbcghklmnpqtv";
  static const char kVni[] = "aaaaeeeeiioooouuuyadbcghklmnpqtv0123456789";
  for (long i = 0; i < random_count; ++i) {
    const bool vni = i & 1;
    const char* set = vni ? kVni : kTelex;
    const size_t n = vni ? sizeof(kVni) - 1 : sizeof(kTelex) - 1;
    std::string k(2 + rng() % 8, 'a');
    for (char& c : k) c = set[rng() % n];
    if (rng() % 5 == 0 && k[0] >= 'a' && k[0] <= 'z') k[0] = static_cast<char>(k[0] - 32);
    emit("rand", vni, k);
  }
  return 0;
}
