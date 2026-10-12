// Loads the real libsentype.so into a real Fcitx5 Instance (testfrontend, no display)
// and drives it with key events, checking client preedit and commits.
#include <cstdio>
#include <string>

#include <testfrontend_public.h>
#include <fcitx-utils/eventdispatcher.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/testing.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputmethodgroup.h>
#include <fcitx/inputmethodmanager.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>

using namespace fcitx;

namespace {

int failures = 0;
void check(bool ok, const std::string& what) {
  if (!ok) {
    ++failures;
    std::printf("FAIL %s\n", what.c_str());
  }
}

struct Driver {
  Instance& instance;
  AddonInstance* frontend;
  ICUUID uuid;

  InputContext* ic() { return instance.inputContextManager().findByUUID(uuid); }
  std::string preedit() { return ic()->inputPanel().clientPreedit().toString(); }
  // Returns true when the engine swallowed the key.
  bool press(KeySym sym) {
    return frontend->call<ITestFrontend::sendKeyEvent>(uuid, Key(sym), false);
  }
  void type(const std::string& s) {
    for (char c : s) press(static_cast<KeySym>(static_cast<unsigned char>(c)));
  }
  void expect_commit(const std::string& s) {
    frontend->call<ITestFrontend::pushCommitExpectation>(s);
  }
};

void run(Instance& instance) {
  auto& imm = instance.inputMethodManager();
  InputMethodGroup group("Default");
  group.setDefaultLayout("us");
  group.inputMethodList().emplace_back("sentype");
  imm.setGroup(group);
  imm.setCurrentGroup("Default");

  AddonInstance* frontend = instance.addonManager().addon("testfrontend", true);
  check(frontend != nullptr, "testfrontend loaded");
  if (!frontend) return;
  Driver d{instance, frontend, frontend->call<ITestFrontend::createInputContext>("sentype-test")};
  d.ic()->setCapabilityFlags(CapabilityFlags{CapabilityFlag::Preedit});
  d.ic()->focusIn();
  instance.setCurrentInputMethod(d.ic(), "sentype", true);
  check(instance.inputMethod(d.ic()) == "sentype", "sentype is the active input method");

  d.type("Vieejt");
  check(d.preedit() == "Việt", "preedit while typing: '" + d.preedit() + "'");

  // URL-bar scenario: the word is committed, then Enter reaches the application.
  d.expect_commit("Việt");
  check(!d.press(FcitxKey_Return), "Enter passes through to the application");
  check(d.preedit().empty(), "preedit cleared after Enter");

  d.type("nguwowif");
  check(d.preedit() == "người", "preedit 'nguwowif': '" + d.preedit() + "'");
  d.expect_commit("người");
  check(!d.press(FcitxKey_space), "space passes through");

  d.type("text");  // not Vietnamese: committed as typed
  d.expect_commit("text");
  check(!d.press(FcitxKey_space), "space after 'text' passes through");

  d.type("hoaf");
  check(d.preedit() == "hoà", "modern tone placement: '" + d.preedit() + "'");
  check(d.press(FcitxKey_BackSpace), "BackSpace is consumed while composing");
  check(d.preedit() == "hò", "after backspace: '" + d.preedit() + "'");
  d.expect_commit("hò");
  check(!d.press(FcitxKey_period), "punctuation passes through");
  check(!d.press(FcitxKey_BackSpace), "BackSpace passes when idle");

  d.type("tooi");  // Escape gives back the typed keys
  d.expect_commit("tooi");
  check(d.press(FcitxKey_Escape), "Escape consumed while composing");
  check(!d.press(FcitxKey_Escape), "Escape passes when idle");

  d.type("aa");  // Ctrl+key: commit, then the shortcut passes through
  d.expect_commit("â");
  const bool ctrl = frontend->call<ITestFrontend::sendKeyEvent>(
      d.uuid, Key(FcitxKey_c, KeyStates{KeyState::Ctrl}), false);
  check(!ctrl, "Ctrl+C passes through");

  d.type("Vieejt");  // focus-out commits the word
  d.expect_commit("Việt");
  d.ic()->focusOut();
  check(d.preedit().empty(), "preedit cleared on focus out");

  d.ic()->focusIn();
  d.type("Vieet");  // toneless stop coda is not a syllable: every key comes back
  d.expect_commit("Vieet");
  d.ic()->focusOut();

  d.ic()->focusIn();
  d.type("window");  // focus-out must commit the spellchecked word exactly once
  d.expect_commit("window");
  d.ic()->focusOut();

  frontend->call<ITestFrontend::destroyInputContext>(d.uuid);
}

}  // namespace

int main() {
  setupTestingEnvironment(SENTYPE_TEST_BINARY_DIR, {"addon", SENTYPE_FCITX_ADDON_DIR},
                          {"tests/fcitx-data"});
  char arg0[] = "sentype-integration";
  char arg1[] = "--disable=all";
  char arg2[] = "--enable=testfrontend,sentype";
  char* argv[] = {arg0, arg1, arg2};
  Instance instance(3, argv);
  instance.addonManager().registerDefaultLoader(nullptr);
  EventDispatcher dispatcher;
  dispatcher.attach(&instance.eventLoop());
  dispatcher.schedule([&instance]() {
    run(instance);
    instance.exit();
  });
  instance.exec();
  std::printf("addon integration: %d failure(s)\n", failures);
  return failures == 0 ? 0 : 1;
}
