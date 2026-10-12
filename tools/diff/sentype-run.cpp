// SenType side of the differential run: reads "<label>\t<T|V>\t<keys>" lines, prints the text
// the application would receive (words typed through Session, spaces pass through).
#include <iostream>
#include <string>

#include "sentype/session.hpp"

int main() {
  for (std::string line; std::getline(std::cin, line);) {
    const size_t t1 = line.find('\t'), t2 = line.find('\t', t1 + 1);
    if (t2 == std::string::npos) continue;
    sentype::Config cfg;  // Modern tone placement, spellcheck on
    cfg.method = line[t1 + 1] == 'V' ? sentype::Method::Vni : sentype::Method::Telex;
    sentype::Session session(cfg);
    std::string out;
    for (size_t i = t2 + 1; i < line.size(); ++i) {
      const sentype::KeyResult r = session.on_key({sentype::Key::Kind::Char, static_cast<char32_t>(line[i]), false});
      out += r.commit;
      if (!r.consumed) out += line[i];  // boundary key reaches the application
    }
    out += session.flush().commit;
    std::cout << out << '\n';
  }
  return 0;
}
