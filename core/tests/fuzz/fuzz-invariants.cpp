#include "fuzz-invariants.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>

#include "sentype/session.hpp"
#include "support/vi-units.hpp"

namespace sentype_fuzz {
namespace {

using namespace sentype;

const uint8_t* g_input = nullptr;  // whole input incl. the config byte, for the report
size_t g_input_size = 0;

[[noreturn]] void fail(const char* what, const std::string& detail, const uint8_t*, size_t) {
  const uint8_t* d = g_input;
  const size_t n = g_input_size;
  std::fprintf(stderr, "INVARIANT VIOLATED: %s\n  %s\n  input (%zu bytes):", what, detail.c_str(), n);
  for (size_t i = 0; i < n; ++i) std::fprintf(stderr, " %02x", d[i]);
  std::fprintf(stderr, "\n");
  std::abort();
}

// Valid UTF-8, only ASCII or precomposed Vietnamese letters (=> NFC), no combining marks.
bool clean_text(const std::string& s) {
  for (size_t i = 0; i < s.size();) {
    const unsigned char c = static_cast<unsigned char>(s[i]);
    const size_t len = c < 0x80 ? 1 : (c >> 5) == 6 ? 2 : (c >> 4) == 14 ? 3 : 0;
    if (!len || i + len > s.size()) return false;
    for (size_t k = 1; k < len; ++k) {
      if ((static_cast<unsigned char>(s[i + k]) & 0xC0) != 0x80) return false;
    }
    size_t j = i;
    const char32_t cp = sentype_test::next_cp(s, j);
    i += len;
    if (cp >= 0x300 && cp <= 0x36F) return false;  // stray combining mark
    if (cp >= 0x80) {
      sentype_test::Units u;
      if (!sentype_test::decompose(sentype_test::to_utf8(cp), &u)) return false;  // not a known letter
    }
  }
  return true;
}

size_t cp_count(const std::string& s) {
  size_t n = 0;
  for (unsigned char c : s) n += (c & 0xC0) != 0x80;
  return n;
}

Config pick_config(uint8_t b) {
  Config c;
  c.method = (b & 1) ? Method::Vni : Method::Telex;
  c.tone_style = (b & 2) ? ToneStyle::Classic : ToneStyle::Modern;
  c.spellcheck = !(b & 4);
  return c;
}

std::string typed_of(Engine& e) { return e.raw_text(); }

// Engine fed with the keys that remain after edits must show the same preedit.
std::string replay(const Config& cfg, const std::string& keys) {
  Engine e(cfg);
  for (unsigned char ch : keys) e.feed(ch);
  return e.preedit();
}

void check_engine_sequence(const uint8_t* d, size_t n, const Config& cfg) {
  Engine a(cfg), b(cfg);
  std::string typed;  // model of the current word's keys
  for (size_t i = 0; i < n; ++i) {
    const uint8_t byte = d[i];
    if (byte >= 0x20 && byte < 0x7F) {
      const bool wa = a.feed(byte), wb = b.feed(byte);  // determinism: twin engine
      if (wa != wb) fail("replay determinism", "feed result differs", d, n);
      if (wa) typed += static_cast<char>(byte);
      else {
        a.commit_text();  // boundary key: word ends
        b.commit_text();
        typed.clear();
      }
    } else if (byte == 0x08 || byte == 0x7F) {  // Backspace
      const size_t before = cp_count(a.preedit());
      const bool ba = a.backspace();
      if (ba != b.backspace()) fail("backspace determinism", "result differs", d, n);
      if (ba) {
        typed = typed_of(a);  // after an edit the remaining keys are the model
        const size_t now = cp_count(a.preedit());
        if (now + 1 != before) fail("backspace removes exactly one letter", a.preedit(), d, n);
      }
    } else if (byte == 0x1B) {  // Escape: raw keys come back
      if (typed_of(a) != typed) fail("raw_text == typed keys", typed_of(a) + " vs " + typed, d, n);
      a.reset();
      b.reset();
      typed.clear();
    } else {  // commit
      const std::string shown = a.preedit();
      const std::string ca = a.commit_text(), cb = b.commit_text();
      // A commit is the typed keys (restore) or the composed word: one code point per letter.
      if (ca != typed && cp_count(ca) != cp_count(shown)) fail("commit is raw or composed", ca + " / " + shown, d, n);
      if (!cfg.spellcheck && ca != typed && cp_count(ca) != cp_count(shown)) fail("no spellcheck: composed", ca, d, n);
      if (ca != cb) fail("replay determinism", "commit differs: " + ca + " / " + cb, d, n);
      if (!clean_text(ca)) fail("commit is clean NFC UTF-8", ca, d, n);
      if (!a.empty() || a.commit_text() != "" || !a.preedit().empty()) fail("commit idempotent", ca, d, n);
      typed.clear();
    }
    if (a.preedit() != b.preedit()) fail("replay determinism", "preedit differs", d, n);
    if (!clean_text(a.preedit())) fail("preedit clean", a.preedit(), d, n);
    if (a.empty() != typed.empty()) fail("empty() matches model", typed, d, n);
  }
  const std::string last = a.commit_text();
  if (!clean_text(last)) fail("final commit clean", last, d, n);
  if (cfg.spellcheck == false && !typed.empty() && last.empty()) fail("commit non-empty", typed, d, n);
  a.reset();
  a.reset();
  if (!a.empty() || !a.preedit().empty()) fail("reset idempotent", "", d, n);
}

// Letters that never trigger a transform in either method: Backspace x k == typing the prefix.
void check_prefix_property(const uint8_t* d, size_t n, const Config& cfg) {
  static const char kPlain[] = "bcghklmnpqtv";
  std::string keys;
  for (size_t i = 0; i < n && keys.size() < 12; ++i) keys += kPlain[d[i] % (sizeof(kPlain) - 1)];
  Engine e(cfg);
  for (unsigned char ch : keys) e.feed(ch);
  for (size_t k = keys.size(); k-- > 0;) {
    e.backspace();
    if (e.preedit() != replay(cfg, keys.substr(0, k))) fail("backspace-k == prefix", keys, d, n);
  }
}

// Session-level: Escape returns exactly the keys typed, commits are clean, consumed keys show preedit.
void check_session_sequence(const uint8_t* d, size_t n, const Config& cfg) {
  Session s(cfg);
  std::string word;  // keys of the word as the app would see them
  for (size_t i = 0; i < n; ++i) {
    const uint8_t byte = d[i];
    Key key;
    if (byte >= 0x20 && byte < 0x7F) {
      key = {Key::Kind::Char, byte, false};
    } else if (byte == 0x08 || byte == 0x7F) {
      key.kind = Key::Kind::Backspace;
    } else if (byte == 0x1B) {
      key.kind = Key::Kind::Escape;
    } else if (byte >= 0x0E && byte <= 0x1A) {  // Ctrl/Alt/Cmd + letter
      key = {Key::Kind::Char, static_cast<char32_t>('a' + (byte - 0x0E)), true};
    } else if (byte >= 0x80) {
      key = {Key::Kind::Char, static_cast<char32_t>(byte) * 0x101, false};  // non-ASCII char
    }
    const bool was_composing = s.composing();
    const KeyResult r = s.on_key(key);
    if (!clean_text(r.commit) || !clean_text(r.preedit)) fail("session output clean", r.commit + "|" + r.preedit, d, n);
    if (!r.consumed && !r.preedit.empty()) fail("passthrough keys carry no preedit", r.preedit, d, n);
    if (r.consumed && key.kind == Key::Kind::Char) {
      word += static_cast<char>(key.ch);
      if (r.preedit != s.engine().preedit()) fail("preedit == engine preedit", r.preedit, d, n);
    } else if (r.consumed && key.kind == Key::Kind::Escape) {
      if (r.commit != word) fail("Escape restores raw keys", r.commit + " vs " + word, d, n);
      word.clear();
    } else if (r.consumed && key.kind == Key::Kind::Backspace) {
      word = s.engine().raw_text();
    } else {
      word.clear();  // boundary / shortcut / other: flushed
    }
    if (!was_composing && r.consumed && key.kind != Key::Kind::Char) fail("consumed while idle", "", d, n);
    if (s.composing() != !word.empty()) fail("composing() matches model", word, d, n);
  }
  const KeyResult f = s.flush();
  if (!clean_text(f.commit) || s.composing() || !s.flush().commit.empty()) fail("flush idempotent", f.commit, d, n);
}

}  // namespace

void check_input(const uint8_t* data, size_t size) {
  if (size == 0) return;
  g_input = data;
  g_input_size = size;
  const Config cfg = pick_config(data[0]);
  data += 1;
  size -= 1;
  check_engine_sequence(data, size, cfg);
  check_prefix_property(data, size, cfg);
  check_session_sequence(data, size, cfg);
}

}  // namespace sentype_fuzz
