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
    sentype::Session& session = state(ic).session;
    apply(ic, session.flush());  // keep the word typed so far, then switch settings
    session.set_config(core_config());
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

// Fcitx needs the client preedit cleared before the final text is committed.
void SenTypeEngine::apply(fcitx::InputContext* ic, const sentype::KeyResult& r) {
  if (!r.consumed && r.commit.empty()) return;  // idle key passing through: nothing to update
  show_preedit(ic, r.preedit);
  if (!r.commit.empty()) ic->commitString(r.commit);
}

void SenTypeEngine::keyEvent(const fcitx::InputMethodEntry&, fcitx::KeyEvent& event) {
  if (event.isRelease()) return;
  const fcitx::Key key = event.key();
  if (key.isModifier()) return;  // a lone Shift/Ctrl press must not end the word

  const bool shortcut = key.states().testAny(
      fcitx::KeyStates{fcitx::KeyState::Ctrl, fcitx::KeyState::Alt, fcitx::KeyState::Super});
  const sentype::Key core_key = sentype::make_key(
      key.sym() == FcitxKey_BackSpace, key.sym() == FcitxKey_Escape,
      fcitx::Key::keySymToUnicode(key.sym()), shortcut);
  fcitx::InputContext* ic = event.inputContext();
  const sentype::KeyResult result = state(ic).session.on_key(core_key);
  apply(ic, result);
  if (result.consumed) event.filterAndAccept();
}

void SenTypeEngine::reset(const fcitx::InputMethodEntry&, fcitx::InputContextEvent& event) {
  apply(event.inputContext(), state(event.inputContext()).session.flush());
}

void SenTypeEngine::deactivate(const fcitx::InputMethodEntry&, fcitx::InputContextEvent& event) {
  apply(event.inputContext(), state(event.inputContext()).session.flush());
}

}  // namespace sentype_fcitx

FCITX_ADDON_FACTORY(sentype_fcitx::SenTypeEngineFactory)
