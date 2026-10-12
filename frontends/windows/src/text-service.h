// SenType TSF text service: preedit-only composition driven by sentype::Session.
#pragma once

#include "com-util.h"
#include "langbar-button.h"
#include "sentype/session.hpp"

class TextService : public ITfTextInputProcessorEx,
                    public ITfKeyEventSink,
                    public ITfThreadMgrEventSink,
                    public ITfCompositionSink {
 public:
  TextService();

  // IUnknown
  STDMETHODIMP QueryInterface(REFIID riid, void** out) override;
  STDMETHODIMP_(ULONG) AddRef() override;
  STDMETHODIMP_(ULONG) Release() override;

  // ITfTextInputProcessor(Ex)
  STDMETHODIMP Activate(ITfThreadMgr* tm, TfClientId id) override { return ActivateEx(tm, id, 0); }
  STDMETHODIMP ActivateEx(ITfThreadMgr* tm, TfClientId id, DWORD flags) override;
  STDMETHODIMP Deactivate() override;

  // ITfKeyEventSink
  STDMETHODIMP OnSetFocus(BOOL) override { return S_OK; }
  STDMETHODIMP OnTestKeyDown(ITfContext* ic, WPARAM wp, LPARAM lp, BOOL* eaten) override;
  STDMETHODIMP OnKeyDown(ITfContext* ic, WPARAM wp, LPARAM lp, BOOL* eaten) override;
  STDMETHODIMP OnTestKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* eaten) override { *eaten = FALSE; return S_OK; }
  STDMETHODIMP OnKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* eaten) override { *eaten = FALSE; return S_OK; }
  STDMETHODIMP OnPreservedKey(ITfContext*, REFGUID, BOOL* eaten) override { *eaten = FALSE; return S_OK; }

  // ITfThreadMgrEventSink: focus moved to another document -> commit the word.
  STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr*) override { return S_OK; }
  STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr*) override { return S_OK; }
  STDMETHODIMP OnSetFocus(ITfDocumentMgr* focus, ITfDocumentMgr* prev) override;
  STDMETHODIMP OnPushContext(ITfContext*) override { return S_OK; }
  STDMETHODIMP OnPopContext(ITfContext*) override { return S_OK; }

  // ITfCompositionSink: the app ended our composition (click elsewhere, etc).
  STDMETHODIMP OnCompositionTerminated(TfEditCookie ec, ITfComposition* comp) override;

 private:
  bool make_key(WPARAM vk, LPARAM lp, sentype::Key* key) const;
  HRESULT run_edit(ITfContext* ic, const sentype::KeyResult& r, DWORD flags);
  HRESULT apply(TfEditCookie ec, ITfContext* ic, const sentype::KeyResult& r);
  HRESULT show_text(TfEditCookie ec, ITfContext* ic, const std::wstring& text);
  void end_composition(TfEditCookie ec);
  void reload_config();  // commit the word in progress, then re-read the settings
  void add_langbar_button();
  void remove_langbar_button();

  ~TextService();
  LONG refs_ = 1;
  ITfThreadMgr* thread_mgr_ = nullptr;
  TfClientId client_id_ = TF_CLIENTID_NULL;
  DWORD thread_sink_cookie_ = TF_INVALID_COOKIE;
  sentype::Session session_;
  ITfComposition* comp_ = nullptr;
  ITfContext* comp_ctx_ = nullptr;  // context that owns comp_
  LangBarButton* langbar_ = nullptr;
};
