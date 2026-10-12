// Small COM helpers: module refcount, release-and-null, and a lambda-based edit session.
#pragma once

#include "tsf-compat.h"
#include <windows.h>

#include <functional>
#include <string>

extern LONG g_module_refs;  // DllCanUnloadNow() == (g_module_refs == 0)

template <class T>
void release(T*& p) {
  if (p) { p->Release(); p = nullptr; }
}

inline std::wstring widen(const std::string& utf8) {
  if (utf8.empty()) return {};
  const int n = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), nullptr, 0);
  std::wstring out(n, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, utf8.data(), (int)utf8.size(), out.data(), n);
  return out;
}

// ITfEditSession that runs a callback; keeps `owner` alive until it has run.
class EditSession : public ITfEditSession {
 public:
  using Fn = std::function<HRESULT(TfEditCookie)>;
  EditSession(IUnknown* owner, Fn fn) : owner_(owner), fn_(std::move(fn)) { owner_->AddRef(); }

  STDMETHODIMP QueryInterface(REFIID riid, void** out) override {
    if (riid == IID_IUnknown || riid == IID_ITfEditSession) {
      *out = static_cast<ITfEditSession*>(this);
      AddRef();
      return S_OK;
    }
    *out = nullptr;
    return E_NOINTERFACE;
  }
  STDMETHODIMP_(ULONG) AddRef() override { return InterlockedIncrement(&refs_); }
  STDMETHODIMP_(ULONG) Release() override {
    const ULONG n = InterlockedDecrement(&refs_);
    if (n == 0) delete this;
    return n;
  }
  STDMETHODIMP DoEditSession(TfEditCookie ec) override { return fn_(ec); }

 private:
  ~EditSession() { owner_->Release(); }
  LONG refs_ = 1;
  IUnknown* owner_;
  Fn fn_;
};
