// COM + TSF registration, used by DllRegisterServer / DllUnregisterServer (regsvr32).
#include "tsf-compat.h"
#include <windows.h>

#include <string>

#include "guids.h"

extern HINSTANCE g_instance;

namespace {

std::wstring guid_string(REFGUID g) {
  wchar_t buf[40];
  StringFromGUID2(g, buf, 40);
  return buf;
}

LSTATUS set_string(HKEY key, const wchar_t* name, const std::wstring& value) {
  return RegSetValueExW(key, name, 0, REG_SZ, (const BYTE*)value.c_str(),
                        (DWORD)((value.size() + 1) * sizeof(wchar_t)));
}

HRESULT write_clsid_keys(const wchar_t* path) {
  const std::wstring base = L"Software\\Classes\\CLSID\\" + guid_string(CLSID_SenTypeTextService);
  HKEY key = nullptr;
  LSTATUS st = RegCreateKeyExW(HKEY_LOCAL_MACHINE, (base + L"\\InprocServer32").c_str(), 0, nullptr,
                               0, KEY_WRITE, nullptr, &key, nullptr);
  if (st != ERROR_SUCCESS) return HRESULT_FROM_WIN32(st);
  set_string(key, nullptr, path);
  set_string(key, L"ThreadingModel", L"Apartment");
  RegCloseKey(key);
  if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, base.c_str(), 0, KEY_WRITE, &key) == ERROR_SUCCESS) {
    set_string(key, nullptr, kServiceName);
    RegCloseKey(key);
  }
  return S_OK;
}

const GUID* const kCategories[] = {
    &GUID_TFCAT_TIP_KEYBOARD,
    &GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT,  // Windows 8+ / UWP apps
    &GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,
};

}  // namespace

HRESULT register_service() {
  wchar_t path[MAX_PATH];
  const DWORD len = GetModuleFileNameW(g_instance, path, MAX_PATH);
  if (!len || len >= MAX_PATH) return HRESULT_FROM_WIN32(GetLastError());
  HRESULT hr = write_clsid_keys(path);
  if (FAILED(hr)) return hr;

  ITfInputProcessorProfiles* profiles = nullptr;
  hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,
                        IID_ITfInputProcessorProfiles, (void**)&profiles);
  if (FAILED(hr)) return hr;
  hr = profiles->Register(CLSID_SenTypeTextService);
  if (SUCCEEDED(hr))
    hr = profiles->AddLanguageProfile(CLSID_SenTypeTextService, kLangVietnamese, GUID_SenTypeProfile,
                                      kServiceName, (ULONG)lstrlenW(kServiceName), path, len,
                                      (ULONG)(-kIconResourceId));  // icon = resource id in this DLL
  // Make the profile selectable right away (Wine defaults it to disabled; best effort).
  if (SUCCEEDED(hr))
    profiles->EnableLanguageProfile(CLSID_SenTypeTextService, kLangVietnamese, GUID_SenTypeProfile,
                                    TRUE);
  profiles->Release();
  if (FAILED(hr)) return hr;

  ITfCategoryMgr* cats = nullptr;
  hr = CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr,
                        (void**)&cats);
  if (FAILED(hr)) return hr;
  for (const GUID* c : kCategories) {
    hr = cats->RegisterCategory(CLSID_SenTypeTextService, *c, CLSID_SenTypeTextService);
    if (FAILED(hr)) break;
  }
  cats->Release();
  return hr;
}

HRESULT unregister_service() {
  ITfCategoryMgr* cats = nullptr;
  if (SUCCEEDED(CoCreateInstance(CLSID_TF_CategoryMgr, nullptr, CLSCTX_INPROC_SERVER,
                                 IID_ITfCategoryMgr, (void**)&cats))) {
    for (const GUID* c : kCategories)
      cats->UnregisterCategory(CLSID_SenTypeTextService, *c, CLSID_SenTypeTextService);
    cats->Release();
  }
  ITfInputProcessorProfiles* profiles = nullptr;
  if (SUCCEEDED(CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER,
                                 IID_ITfInputProcessorProfiles, (void**)&profiles))) {
    profiles->Unregister(CLSID_SenTypeTextService);  // also drops the language profiles
    profiles->Release();
  }
  RegDeleteTreeW(HKEY_LOCAL_MACHINE,
                 (L"Software\\Classes\\CLSID\\" + guid_string(CLSID_SenTypeTextService)).c_str());
  return S_OK;
}
