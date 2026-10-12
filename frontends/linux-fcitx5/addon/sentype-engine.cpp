#include "sentype-engine.h"

#include <fcitx-config/iniparser.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/textformatflags.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>
#include <fcitx/text.h>

namespace sentype_fcitx {
namespace {

constexpr char kConfigFile[] = "conf/sentype.conf";

}  // namespace

SenTypeEngine::SenTypeEngine(fcitx::Instance* instance)
    : instance_(instance), factory_([this](fcitx::InputContext&) {
        return new SenTypeState(core_config());
      }) {
  instance_->inputContextManager().registerProperty("sentypeState", &factory_);
  reloadConfig();
}

sentype::Config SenTypeEngine::core_config() const {
  sentype::Config cfg;
  cfg.method = *config_.method == TypingMethod::VNI ? sentype::Method::Vni : sentype::Method::Telex;
  cfg.tone_style = *config_.tonePlacement == TonePlacement::Classic ? sentype::ToneStyle::Classic
                                                                    : sentype::ToneStyle::Modern;
  cfg.spellcheck = *config_.spellCheck;
  return cfg;
}

void SenTypeEngine::reloadConfig() {
  fcitx::readAsIni(config_, kConfigFile);
  apply_config();
}

void SenTypeEngine::setConfig(const fcitx::RawConfig& raw) {
  config_.load(raw, true);
  fcitx::safeSaveAsIni(config_, kConfigFile);
  apply_config();
}

void SenTypeEngine::apply_config() {
  instance_->inputContextManager().foreach([this](fcitx::InputContext* ic) {
    state(ic).engine.set_config(core_config());  // drops the word in progress
    return true;
  });
}

void SenTypeEngine::show_preedit(fcitx::InputContext* ic, const std::string& text) {
  fcitx::Text preedit;
  if (!text.empty()) {
    // DontCommit: Fcitx core commits client preedit text itself on focus-out; we
    // commit the final (spellchecked) word in deactivate() instead, never twice.
    preedit.append(text, fcitx::TextFormatFlags{fcitx::TextFormatFlag::Underline,
                                                fcitx::TextFormatFlag::DontCommit});
    preedit.setCursor(static_cast<int>(text.size()));
  }
  if (ic->capabilityFlags().test(fcitx::CapabilityFlag::Preedit)) {
    ic->inputPanel().setClientPreedit(preedit);
    ic->updatePreedit();
  } else {  // client cannot show preedit: fall back to the input panel
    ic->inputPanel().setPreedit(preedit);
  }
  ic->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
}

void SenTypeEngine::flush(fcitx::InputContext* ic) {
  sentype::Engine& eng = state(ic).engine;
  if (eng.empty()) return;
  const std::string text = eng.commit_text();
  show_preedit(ic, "");
  ic->commitString(text);
}

void SenTypeEngine::keyEvent(const fcitx::InputMethodEntry&, fcitx::KeyEvent& event) {
  if (event.isRelease()) return;
  fcitx::InputContext* ic = event.inputContext();
  sentype::Engine& eng = state(ic).engine;
  const fcitx::Key key = event.key();
  if (key.isModifier()) return;  // a lone Shift/Ctrl press must not end the word

  // Ctrl/Alt/Super shortcuts: commit the word, then let the shortcut through.
  if (key.states().testAny(fcitx::KeyStates{fcitx::KeyState::Ctrl, fcitx::KeyState::Alt,
                                            fcitx::KeyState::Super})) {
    flush(ic);
    return;
  }
  if (key.sym() == FcitxKey_BackSpace) {
    if (eng.empty()) return;
    eng.backspace();
    show_preedit(ic, eng.preedit());
    event.filterAndAccept();
    return;
  }
  if (key.sym() == FcitxKey_Escape) {  // Esc: give back exactly what was typed
    if (eng.empty()) return;
    const std::string raw = eng.raw_text();
    eng.reset();
    show_preedit(ic, "");
    ic->commitString(raw);
    event.filterAndAccept();
    return;
  }
  const uint32_t ch = fcitx::Key::keySymToUnicode(key.sym());
  if (ch >= 0x20 && ch < 0x7F && eng.feed(ch)) {
    show_preedit(ic, eng.preedit());
    event.filterAndAccept();
    return;
  }
  // Space, punctuation, Enter, Tab, arrows...: commit the word and let the key
  // reach the application unchanged (Enter therefore still submits the URL bar).
  flush(ic);
}

void SenTypeEngine::reset(const fcitx::InputMethodEntry&, fcitx::InputContextEvent& event) {
  flush(event.inputContext());
}

void SenTypeEngine::deactivate(const fcitx::InputMethodEntry&, fcitx::InputContextEvent& event) {
  flush(event.inputContext());
}

}  // namespace sentype_fcitx

FCITX_ADDON_FACTORY(sentype_fcitx::SenTypeEngineFactory)
