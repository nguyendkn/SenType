// Composition handling: TextService::apply writes core results into the document.
#include "text-service.h"

namespace {

HRESULT caret_to_end(TfEditCookie ec, ITfContext* ic, ITfRange* range) {
  range->Collapse(ec, TF_ANCHOR_END);
  TF_SELECTION sel = {};
  sel.range = range;
  sel.style.ase = TF_AE_NONE;
  sel.style.fInterimChar = FALSE;
  return ic->SetSelection(ec, 1, &sel);
}

}  // namespace

// Shows `text` as the composition (starting one if needed).
HRESULT TextService::show_text(TfEditCookie ec, ITfContext* ic, const std::wstring& text) {
  if (!comp_) {
    ITfInsertAtSelection* ins = nullptr;
    ITfContextComposition* cc = nullptr;
    ITfRange* at = nullptr;
    HRESULT hr = ic->QueryInterface(IID_ITfInsertAtSelection, (void**)&ins);
    if (SUCCEEDED(hr)) hr = ins->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, nullptr, 0, &at);
    if (SUCCEEDED(hr)) hr = ic->QueryInterface(IID_ITfContextComposition, (void**)&cc);
    if (SUCCEEDED(hr))
      hr = cc->StartComposition(ec, at, static_cast<ITfCompositionSink*>(this), &comp_);
    release(ins);
    release(cc);
    release(at);
    if (FAILED(hr) || !comp_) return FAILED(hr) ? hr : E_FAIL;
    comp_ctx_ = ic;
    comp_ctx_->AddRef();
  }
  ITfRange* range = nullptr;
  HRESULT hr = comp_->GetRange(&range);
  if (SUCCEEDED(hr)) hr = range->SetText(ec, 0, text.c_str(), (LONG)text.size());
  if (SUCCEEDED(hr)) hr = caret_to_end(ec, ic, range);
  release(range);
  return hr;
}

void TextService::end_composition(TfEditCookie ec) {
  if (comp_) comp_->EndComposition(ec);
  release(comp_);
  release(comp_ctx_);
}

HRESULT TextService::apply(TfEditCookie ec, ITfContext* ic, const sentype::KeyResult& r) {
  HRESULT hr = S_OK;
  const std::wstring commit = widen(r.commit), pre = widen(r.preedit);
  if (!commit.empty()) {  // final text replaces the composition, then it ends
    hr = show_text(ec, ic, commit);
    end_composition(ec);
  }
  if (!pre.empty()) {
    hr = show_text(ec, ic, pre);
  } else if (comp_) {  // word emptied by Backspace: erase the leftover composition text
    show_text(ec, ic, L"");
    end_composition(ec);
  }
  return hr;
}
