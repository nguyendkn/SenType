#include "text-service.h"

#include "key-translation.h"
#include "settings.h"

#include <new>

LONG g_module_refs = 0;

TextService::TextService() : session_(load_config()) { InterlockedIncrement(&g_module_refs); }
TextService::~TextService() {
  release(comp_);
  release(comp_ctx_);
  InterlockedDecrement(&g_module_refs);
}

STDMETHODIMP TextService::QueryInterface(REFIID riid, void** out) {
  *out = nullptr;
  if (riid == IID_IUnknown || riid == IID_ITfTextInputProcessor || riid == IID_ITfTextInputProcessorEx)
    *out = static_cast<ITfTextInputProcessorEx*>(this);
  else if (riid == IID_ITfKeyEventSink) *out = static_cast<ITfKeyEventSink*>(this);
  else if (riid == IID_ITfThreadMgrEventSink) *out = static_cast<ITfThreadMgrEventSink*>(this);
  else if (riid == IID_ITfCompositionSink) *out = static_cast<ITfCompositionSink*>(this);
  if (!*out) return E_NOINTERFACE;
  AddRef();
  return S_OK;
}
STDMETHODIMP_(ULONG) TextService::AddRef() { return InterlockedIncrement(&refs_); }
STDMETHODIMP_(ULONG) TextService::Release() {
  const ULONG n = InterlockedDecrement(&refs_);
  if (n == 0) delete this;
  return n;
}

STDMETHODIMP TextService::ActivateEx(ITfThreadMgr* tm, TfClientId id, DWORD) {
  thread_mgr_ = tm;
  thread_mgr_->AddRef();
  client_id_ = id;
  session_.set_config(load_config());

  ITfKeystrokeMgr* keys = nullptr;
  if (SUCCEEDED(tm->QueryInterface(IID_ITfKeystrokeMgr, (void**)&keys))) {
    keys->AdviseKeyEventSink(id, static_cast<ITfKeyEventSink*>(this), TRUE);
    keys->Release();
  }
  ITfSource* source = nullptr;
  if (SUCCEEDED(tm->QueryInterface(IID_ITfSource, (void**)&source))) {
    source->AdviseSink(IID_ITfThreadMgrEventSink, static_cast<ITfThreadMgrEventSink*>(this),
                       &thread_sink_cookie_);
    source->Release();
  }
  add_langbar_button();
  return S_OK;
}

void TextService::add_langbar_button() {
  langbar_ = new (std::nothrow) LangBarButton([this](sentype::Method) { reload_config(); });
  if (!langbar_) return;
  ITfLangBarItemMgr* mgr = nullptr;
  if (SUCCEEDED(thread_mgr_->QueryInterface(IID_ITfLangBarItemMgr, (void**)&mgr))) {
    // Best effort: without the button the input method still works (profile icon stays).
    mgr->AddItem(langbar_);
    mgr->Release();
  }
}

void TextService::remove_langbar_button() {
  if (!langbar_) return;
  ITfLangBarItemMgr* mgr = nullptr;
  if (thread_mgr_ && SUCCEEDED(thread_mgr_->QueryInterface(IID_ITfLangBarItemMgr, (void**)&mgr))) {
    mgr->RemoveItem(langbar_);
    mgr->Release();
  }
  langbar_->detach();  // the language bar may still hold a reference for a moment
  release(langbar_);
}

STDMETHODIMP TextService::Deactivate() {
  remove_langbar_button();
  if (thread_mgr_) {
    ITfKeystrokeMgr* keys = nullptr;
    if (SUCCEEDED(thread_mgr_->QueryInterface(IID_ITfKeystrokeMgr, (void**)&keys))) {
      keys->UnadviseKeyEventSink(client_id_);
      keys->Release();
    }
    ITfSource* source = nullptr;
    if (thread_sink_cookie_ != TF_INVALID_COOKIE &&
        SUCCEEDED(thread_mgr_->QueryInterface(IID_ITfSource, (void**)&source))) {
      source->UnadviseSink(thread_sink_cookie_);
      source->Release();
    }
  }
  thread_sink_cookie_ = TF_INVALID_COOKIE;
  session_.reset();
  release(comp_);
  release(comp_ctx_);
  release(thread_mgr_);
  client_id_ = TF_CLIENTID_NULL;
  return S_OK;
}

// Builds a Key from the native event; false for lone modifier keys (ignored entirely).
bool TextService::make_key(WPARAM vk, LPARAM lp, sentype::Key* key) const {
  if (sentype_win::is_modifier_vk((unsigned)vk)) return false;
  BYTE state[256] = {};
  GetKeyboardState(state);
  const bool shortcut = (state[VK_CONTROL] | state[VK_MENU] | state[VK_LWIN] | state[VK_RWIN]) & 0x80;
  wchar_t buf[4] = {};
  // Flag 0x4: do not alter the keyboard/dead-key state (Windows 10 1607+).
  const int n = ToUnicode((UINT)vk, (lp >> 16) & 0xFF, state, buf, 4, 0x4);
  *key = sentype_win::translate_key((unsigned)vk, n == 1 ? buf[0] : 0, shortcut);
  return true;
}

STDMETHODIMP TextService::OnTestKeyDown(ITfContext*, WPARAM wp, LPARAM lp, BOOL* eaten) {
  sentype::Key key;
  *eaten = FALSE;
  if (!make_key(wp, lp, &key)) return S_OK;
  // While composing every key must reach OnKeyDown so the word is committed before the key
  // passes through (the policy lives in Session::would_handle).
  *eaten = session_.would_handle(key);
  return S_OK;
}

STDMETHODIMP TextService::OnKeyDown(ITfContext* ic, WPARAM wp, LPARAM lp, BOOL* eaten) {
  sentype::Key key;
  *eaten = FALSE;
  if (!make_key(wp, lp, &key)) return S_OK;
  const bool had_comp = comp_ != nullptr;
  const sentype::KeyResult r = session_.on_key(key);
  *eaten = r.consumed;  // FALSE for boundaries: the key continues to the application
  if (had_comp || r.consumed || !r.commit.empty()) run_edit(ic, r, TF_ES_SYNC | TF_ES_READWRITE);
  return S_OK;
}

void TextService::reload_config() {
  if (comp_ && comp_ctx_) {
    ITfContext* ic = comp_ctx_;
    ic->AddRef();
    run_edit(ic, session_.flush(), TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
    ic->Release();
  }
  session_.set_config(load_config());  // also picks up a Telex/VNI change made in another process
}

STDMETHODIMP TextService::OnSetFocus(ITfDocumentMgr*, ITfDocumentMgr*) {
  reload_config();
  return S_OK;
}

STDMETHODIMP TextService::OnCompositionTerminated(TfEditCookie, ITfComposition* comp) {
  if (comp == comp_) {  // text stays in the document as the app left it
    session_.reset();
    release(comp_);
    release(comp_ctx_);
  }
  return S_OK;
}

namespace {
// Keeps the context alive for an asynchronous edit session (copyable for std::function).
struct ContextRef {
  explicit ContextRef(ITfContext* c) : p(c) { p->AddRef(); }
  ContextRef(const ContextRef& o) : p(o.p) { p->AddRef(); }
  ~ContextRef() { p->Release(); }
  ContextRef& operator=(const ContextRef&) = delete;
  ITfContext* p;
};
}  // namespace

HRESULT TextService::run_edit(ITfContext* ic, const sentype::KeyResult& r, DWORD flags) {
  ContextRef ref(ic);
  auto* es = new EditSession(static_cast<ITfTextInputProcessorEx*>(this),
                             [this, ref, r](TfEditCookie ec) { return apply(ec, ref.p, r); });
  HRESULT hr = E_FAIL;
  const HRESULT ret = ic->RequestEditSession(client_id_, es, flags, &hr);
  es->Release();
  return SUCCEEDED(ret) ? hr : ret;
}
