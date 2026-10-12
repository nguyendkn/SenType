// Deterministic seeded random-sequence test of the same invariants the libFuzzer target checks.
// Usage: fuzz-random-test [iterations=200000] [seed=1] [vi-syllables.txt]
// With a syllable list, half of the sequences are real Telex/VNI typings of dictionary
// syllables with random edits, so the fuzzer reaches deep composing states quickly.
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#include "fuzz-invariants.hpp"
#include "support/typing-orders.hpp"

using namespace sentype_test;

namespace {

std::vector<std::string> load(const char* path) {
  std::vector<std::string> v;
  std::ifstream in(path);
  for (std::string l; std::getline(in, l);) {
    if (!l.empty() && l[0] != '#') v.push_back(l);
  }
  return v;
}

// Bytes with the weights of real typing: letters, tone/mark keys, digits, boundaries, edits.
std::string random_keys(std::mt19937& rng) {
  static const char kAlpha[] = "aaaaeeeeiioooouuuyadwsfrxjzbcghklmnpqtvABDEOUW0123456789 .,-\n";
  std::string s;
  const size_t len = 1 + rng() % 40;
  for (size_t i = 0; i < len; ++i) {
    const unsigned r = rng() % 100;
    if (r < 6) s += '\x08';        // Backspace
    else if (r < 8) s += '\x1B';   // Escape
    else if (r < 10) s += static_cast<char>(0x0E + rng() % 13);  // shortcut
    else if (r < 12) s += static_cast<char>(0x80 + rng() % 128); // non-ASCII
    else if (r < 13) s += static_cast<char>(rng() % 0x20);       // control / commit
    else s += kAlpha[rng() % (sizeof(kAlpha) - 1)];
  }
  return s;
}

std::string typed_syllables(std::mt19937& rng, const std::vector<std::string>& syl) {
  std::string s;
  for (int w = 1 + rng() % 3; w > 0; --w) {
    Units u;
    if (!decompose(syl[rng() % syl.size()], &u)) continue;
    const Order o = kAllOrders[rng() % 4];
    std::string k = (rng() & 1) ? vni_keys(u, o) : telex_keys(u, o);
    if (k.empty()) k = telex_keys(u, Order::ToneAtEnd);
    s += k;
    if (rng() % 4 == 0) s += '\x08';
    s += (rng() % 3 == 0) ? "\x1B" : (rng() % 2 ? " " : "");
  }
  return s;
}

}  // namespace

int main(int argc, char** argv) {
  const long iterations = argc > 1 ? std::atol(argv[1]) : 200000;
  const unsigned seed = argc > 2 ? static_cast<unsigned>(std::atol(argv[2])) : 1;
  const std::vector<std::string> syl = argc > 3 ? load(argv[3]) : std::vector<std::string>{};
  std::mt19937 rng(seed);
  long keys = 0;
  for (long i = 0; i < iterations; ++i) {
    std::string input(1, static_cast<char>(rng()));  // first byte selects the config
    input += (!syl.empty() && (i & 1)) ? typed_syllables(rng, syl) : random_keys(rng);
    sentype_fuzz::check_input(reinterpret_cast<const uint8_t*>(input.data()), input.size());
    keys += static_cast<long>(input.size());
  }
  std::printf("fuzz-random: %ld sequences, %ld key events, seed %u: all invariants held\n",
              iterations, keys, seed);
  return 0;
}
