// DLL entry points required by COM (regsvr32 and TSF activation).
#include <windows.h>

#include <new>

#include "guids.h"
#include "text-service.h"

HINSTANCE g_instance = nullptr;
HRESULT register_service();
HRESULT unregister_service();

namespace {

class ClassFactory : public IClassFactory {
 public:
  STDMETHODIMP QueryInterface(REFIID riid, void** out) override {
    if (riid == IID_IUnknown || riid == IID_IClassFactory) {
      *out = static_cast<IClassFactory*>(this);
      return S_OK;  // static object: no refcounting needed
    }
    *out = nullptr;
    return E_NOINTERFACE;
  }
  STDMETHODIMP_(ULONG) AddRef() override { return 2; }
  STDMETHODIMP_(ULONG) Release() override { return 1; }
  STDMETHODIMP CreateInstance(IUnknown* outer, REFIID riid, void** out) override {
    *out = nullptr;
    if (outer) return CLASS_E_NOAGGREGATION;
    auto* svc = new (std::nothrow) TextService();
    if (!svc) return E_OUTOFMEMORY;
    const HRESULT hr = svc->QueryInterface(riid, out);
    svc->Release();
    return hr;
  }
  STDMETHODIMP LockServer(BOOL lock) override {
    if (lock) InterlockedIncrement(&g_module_refs); else InterlockedDecrement(&g_module_refs);
    return S_OK;
  }
};

ClassFactory g_factory;

}  // namespace

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    g_instance = inst;
    DisableThreadLibraryCalls(inst);
  }
  return TRUE;
}

extern "C" {
STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void** out) {
  if (clsid != CLSID_SenTypeTextService) { *out = nullptr; return CLASS_E_CLASSNOTAVAILABLE; }
  return g_factory.QueryInterface(riid, out);
}
STDAPI DllCanUnloadNow() { return g_module_refs == 0 ? S_OK : S_FALSE; }
STDAPI DllRegisterServer() {
  const HRESULT init = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  const HRESULT hr = register_service();
  if (SUCCEEDED(init)) CoUninitialize();
  return hr;
}
STDAPI DllUnregisterServer() {
  const HRESULT init = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  const HRESULT hr = unregister_service();
  if (SUCCEEDED(init)) CoUninitialize();
  return hr;
}
}
