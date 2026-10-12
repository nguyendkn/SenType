// Language-bar button: SenType icon + "SenType" tooltip, menu to pick Telex / VNI.
// Shown in the legacy language bar / input indicator; Windows 8+ draws the tray icon from the
// registered profile icon instead (see registration.cpp).
#pragma once

#include <functional>

#include "com-util.h"
#include "sentype/engine.hpp"

class LangBarButton : public ITfLangBarItemButton, public ITfSource {
 public:
  using OnMethod = std::function<void(sentype::Method)>;
  explicit LangBarButton(OnMethod on_method);
  void detach() { on_method_ = nullptr; release(sink_); }  // owner is going away

  // IUnknown
  STDMETHODIMP QueryInterface(REFIID riid, void** out) override;
  STDMETHODIMP_(ULONG) AddRef() override { return InterlockedIncrement(&refs_); }
  STDMETHODIMP_(ULONG) Release() override;

  // ITfLangBarItem
  STDMETHODIMP GetInfo(TF_LANGBARITEMINFO* info) override;
  STDMETHODIMP GetStatus(DWORD* status) override { *status = 0; return S_OK; }
  STDMETHODIMP Show(BOOL) override { return S_OK; }
  STDMETHODIMP GetTooltipString(BSTR* tip) override;

  // ITfLangBarItemButton
  STDMETHODIMP OnClick(TfLBIClick, POINT, const RECT*) override { return S_OK; }
  STDMETHODIMP InitMenu(ITfMenu* menu) override;
  STDMETHODIMP OnMenuSelect(UINT id) override;
  STDMETHODIMP GetIcon(HICON* icon) override;
  STDMETHODIMP GetText(BSTR* text) override;

  // ITfSource (the language bar advises its ITfLangBarItemSink here)
  STDMETHODIMP AdviseSink(REFIID riid, IUnknown* unk, DWORD* cookie) override;
  STDMETHODIMP UnadviseSink(DWORD cookie) override;

 private:
  ~LangBarButton() { release(sink_); }
  LONG refs_ = 1;
  OnMethod on_method_;
  ITfLangBarItemSink* sink_ = nullptr;
};
