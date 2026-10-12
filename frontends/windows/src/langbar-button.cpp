#include "langbar-button.h"

#include <olectl.h>  // CONNECT_E_*

#include "guids.h"
#include "settings.h"

extern HINSTANCE g_instance;

namespace {
constexpr UINT kIdTelex = 1;
constexpr UINT kIdVni = 2;
constexpr DWORD kSinkCookie = 1;  // one sink at most
}  // namespace

LangBarButton::LangBarButton(OnMethod on_method) : on_method_(std::move(on_method)) {}

STDMETHODIMP LangBarButton::QueryInterface(REFIID riid, void** out) {
  *out = nullptr;
  if (riid == IID_IUnknown || riid == IID_ITfLangBarItem || riid == IID_ITfLangBarItemButton)
    *out = static_cast<ITfLangBarItemButton*>(this);
  else if (riid == IID_ITfSource) *out = static_cast<ITfSource*>(this);
  if (!*out) return E_NOINTERFACE;
  AddRef();
  return S_OK;
}

STDMETHODIMP_(ULONG) LangBarButton::Release() {
  const ULONG n = InterlockedDecrement(&refs_);
  if (n == 0) delete this;
  return n;
}

STDMETHODIMP LangBarButton::GetInfo(TF_LANGBARITEMINFO* info) {
  if (!info) return E_INVALIDARG;
  *info = {};
  info->clsidService = CLSID_SenTypeTextService;
  info->guidItem = GUID_SenTypeLangBarItem;
  info->dwStyle = TF_LBI_STYLE_BTN_MENU | TF_LBI_STYLE_SHOWNINTRAY;
  info->ulSort = 0;
  lstrcpynW(info->szDescription, kServiceName, TF_LBI_DESC_MAXLEN);
  return S_OK;
}

STDMETHODIMP LangBarButton::GetTooltipString(BSTR* tip) {
  *tip = SysAllocString(kServiceName);
  return *tip ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP LangBarButton::GetText(BSTR* text) { return GetTooltipString(text); }

STDMETHODIMP LangBarButton::GetIcon(HICON* icon) {
  // The caller owns (and destroys) the returned handle, so load a fresh one each time.
  *icon = (HICON)LoadImageW(g_instance, MAKEINTRESOURCEW(kIconResourceId), IMAGE_ICON,
                            GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                            LR_DEFAULTCOLOR);
  return *icon ? S_OK : HRESULT_FROM_WIN32(GetLastError());
}

STDMETHODIMP LangBarButton::InitMenu(ITfMenu* menu) {
  const bool vni = load_config().method == sentype::Method::Vni;
  menu->AddMenuItem(kIdTelex, vni ? 0 : TF_LBMENUF_RADIOCHECKED, nullptr, nullptr, L"Telex", 5,
                    nullptr);
  menu->AddMenuItem(kIdVni, vni ? TF_LBMENUF_RADIOCHECKED : 0, nullptr, nullptr, L"VNI", 3,
                    nullptr);
  return S_OK;
}

STDMETHODIMP LangBarButton::OnMenuSelect(UINT id) {
  if (id != kIdTelex && id != kIdVni) return S_OK;
  const sentype::Method m = id == kIdVni ? sentype::Method::Vni : sentype::Method::Telex;
  save_method(m);
  if (on_method_) on_method_(m);
  return S_OK;
}

STDMETHODIMP LangBarButton::AdviseSink(REFIID riid, IUnknown* unk, DWORD* cookie) {
  if (riid != IID_ITfLangBarItemSink) return CONNECT_E_CANNOTCONNECT;
  if (sink_) return CONNECT_E_ADVISELIMIT;
  if (FAILED(unk->QueryInterface(IID_ITfLangBarItemSink, (void**)&sink_))) return E_NOINTERFACE;
  *cookie = kSinkCookie;
  return S_OK;
}

STDMETHODIMP LangBarButton::UnadviseSink(DWORD cookie) {
  if (cookie != kSinkCookie || !sink_) return CONNECT_E_NOCONNECTION;
  release(sink_);
  return S_OK;
}
