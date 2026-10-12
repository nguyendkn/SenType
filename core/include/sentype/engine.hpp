// SenType core: pure C++20 Vietnamese composing engine (no Fcitx dependency).
// Usage: feed() each key of a word; show preedit(); on a word boundary call
// commit_text() and let the boundary key pass through to the application.
#pragma once

#include <memory>
#include <string>

namespace sentype {

enum class Method { Telex, Vni };
enum class ToneStyle { Modern, Classic };  // Modern: "hoà/thuý", Classic: "hòa/thúy"

struct Config {
  Method method = Method::Telex;
  ToneStyle tone_style = ToneStyle::Modern;
  bool spellcheck = true;  // restore raw keys on commit when the word is not Vietnamese
};

class Engine {
 public:
  explicit Engine(Config cfg = {});
  ~Engine();
  Engine(Engine&&) noexcept;
  Engine& operator=(Engine&&) noexcept;

  // Changing the config resets the current word.
  void set_config(const Config& cfg);
  const Config& config() const;

  // true: key became part of the word (preedit changed).
  // false: key is a word boundary; caller should commit_text() and pass the key on.
  bool feed(char32_t ch);

  // true if a key was consumed (buffer was non-empty).
  bool backspace();

  // Current composed word, UTF-8 NFC.
  std::string preedit() const;
  // Keys exactly as the user typed them (after tone/mark undo), UTF-8.
  std::string raw_text() const;
  // Final text for the word (raw keys if spellcheck rejects it); resets the word.
  std::string commit_text();
  // true when the current word would be committed as-is (valid or spellcheck off).
  bool is_valid() const;

  void reset();
  bool empty() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace sentype
