#include "settings.h"

#include <windows.h>

#include <string>

#include "sentype/settings.hpp"

namespace {
// Registry value HKCU\Software\SenType\<name> as narrow ASCII ("" when missing).
std::string reg_string(const wchar_t* name) {
  wchar_t buf[16] = {};
  DWORD size = sizeof(buf) - sizeof(wchar_t);
  if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\SenType", name, RRF_RT_REG_SZ, nullptr, buf,
                   &size) != ERROR_SUCCESS)
    return "";
  std::string out;
  for (const wchar_t* p = buf; *p; ++p) out += *p < 0x80 ? static_cast<char>(*p) : '?';
  return out;
}
}  // namespace

sentype::Config load_config() {
  sentype::Config cfg;
  cfg.method = sentype::parse_method(reg_string(L"Method"));
  cfg.tone_style = sentype::parse_tone_style(reg_string(L"TonePlacement"));
  return cfg;
}
