// Fcitx5 input method adapter: translates Fcitx key events to sentype::Session (the shared key
// policy) and shows the result in client preedit. No policy lives here; no backspace hacks.
#pragma once

#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>

#include "sentype-config.h"
#include "sentype/session.hpp"

namespace sentype_fcitx {

// One session per input context (each text field has its own word).
class SenTypeState : public fcitx::InputContextProperty {
 public:
  explicit SenTypeState(const sentype::Config& cfg) : session(cfg) {}
  sentype::Session session;
};

class SenTypeEngine : public fcitx::InputMethodEngineV2 {
 public:
  explicit SenTypeEngine(fcitx::Instance* instance);

  void keyEvent(const fcitx::InputMethodEntry& entry, fcitx::KeyEvent& event) override;
  void reset(const fcitx::InputMethodEntry& entry, fcitx::InputContextEvent& event) override;
  void deactivate(const fcitx::InputMethodEntry& entry,
                  fcitx::InputContextEvent& event) override;

  void reloadConfig() override;
  const fcitx::Configuration* getConfig() const override { return &config_; }
  void setConfig(const fcitx::RawConfig& raw) override;

 private:
  sentype::Config core_config() const;
  void apply_config();
  SenTypeState& state(fcitx::InputContext* ic) { return *ic->propertyFor(&factory_); }
  void show_preedit(fcitx::InputContext* ic, const std::string& text);
  void apply(fcitx::InputContext* ic, const sentype::KeyResult& result);

  fcitx::Instance* instance_;
  SenTypeConfig config_;
  fcitx::FactoryFor<SenTypeState> factory_;
};

class SenTypeEngineFactory : public fcitx::AddonFactory {
 public:
  fcitx::AddonInstance* create(fcitx::AddonManager* manager) override {
    return new SenTypeEngine(manager->instance());
  }
};

}  // namespace sentype_fcitx
