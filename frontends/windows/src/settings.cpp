#include "settings.h"

#include <windows.h>

namespace {
bool reg_string_is(const wchar_t* name, const wchar_t* expected) {
  wchar_t buf[16] = {};
  DWORD size = sizeof(buf) - sizeof(wchar_t);
  if (RegGetValueW(HKEY_CURRENT_USER, L"Software\\SenType", name, RRF_RT_REG_SZ, nullptr, buf,
                   &size) != ERROR_SUCCESS)
    return false;
  return lstrcmpiW(buf, expected) == 0;
}
}  // namespace

sentype::Config load_config() {
  sentype::Config cfg;
  if (reg_string_is(L"Method", L"VNI")) cfg.method = sentype::Method::Vni;
  if (reg_string_is(L"TonePlacement", L"Classic")) cfg.tone_style = sentype::ToneStyle::Classic;
  return cfg;
}
