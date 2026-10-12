#include "text-service.h"

#include "key-translation.h"
#include "settings.h"

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
  session_.engine().set_config(load_config());

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
  return S_OK;
}

STDMETHODIMP TextService::Deactivate() {
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
  // While composing, every key must reach OnKeyDown so the word can be committed
  // before the key passes through. Otherwise only plain characters may start a word.
  *eaten = session_.composing() || (key.kind == sentype::Key::Kind::Char && !key.shortcut);
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

STDMETHODIMP TextService::OnSetFocus(ITfDocumentMgr*, ITfDocumentMgr*) {
  if (comp_ && comp_ctx_) {
    ITfContext* ic = comp_ctx_;
    ic->AddRef();
    run_edit(ic, session_.flush(), TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
    ic->Release();
  }
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
