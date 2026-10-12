// Small helpers for the real-data tests: line files and a pass/fail tally with sample misses.
#pragma once

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

namespace sentype_test {

inline std::vector<std::string> read_lines(const char* path) {
  std::vector<std::string> v;
  std::ifstream in(path);
  for (std::string l; std::getline(in, l);) {
    if (!l.empty() && l.back() == '\r') l.pop_back();
    if (!l.empty() && l[0] != '#') v.push_back(l);
  }
  return v;
}

inline size_t max_miss() { return std::getenv("SENTYPE_VERBOSE") ? 1000000 : 25; }

struct Tally {
  int total = 0, ok = 0;
  std::vector<std::string> bad;
  void add(bool good, const std::string& note) {
    ++total;
    if (good) ++ok;
    else if (bad.size() < max_miss()) bad.push_back(note);
  }
  void print(const std::string& name) const {
    std::printf("  %-34s %7d / %-7d %7.3f%%\n", name.c_str(), ok, total,
                total ? 100.0 * ok / total : 100.0);
    for (const std::string& b : bad) std::printf("      miss: %s\n", b.c_str());
  }
  bool failed() const { return ok != total; }
};


}  // namespace sentype_test
