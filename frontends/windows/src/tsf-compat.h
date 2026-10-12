// Fills gaps in the mingw-w64 TSF headers (MSVC's Windows SDK already has all of these).
#pragma once

#include <msctf.h>
#include <ctfutb.h>  // language bar: ITfLangBarItem*, ITfMenu (mingw ships only part of it)

#ifndef TF_CLIENTID_NULL
#define TF_CLIENTID_NULL ((TfClientId)0)
#endif

#ifndef __ITfTextInputProcessorEx_INTERFACE_DEFINED__
#define __ITfTextInputProcessorEx_INTERFACE_DEFINED__
// {6E4E2102-F9CD-433D-B496-303CE03A6507}
static const IID IID_ITfTextInputProcessorEx = {
    0x6e4e2102, 0xf9cd, 0x433d, {0xb4, 0x96, 0x30, 0x3c, 0xe0, 0x3a, 0x65, 0x07}};
struct ITfTextInputProcessorEx : public ITfTextInputProcessor {
  virtual HRESULT STDMETHODCALLTYPE ActivateEx(ITfThreadMgr* ptim, TfClientId tid,
                                               DWORD flags) = 0;
};
#endif

#ifdef __MINGW32__
// {13A016DF-560B-46CD-947A-4C3AF1E0E35D} Windows 8+ immersive (UWP) support
static const GUID GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT = {
    0x13a016df, 0x560b, 0x46cd, {0x94, 0x7a, 0x4c, 0x3a, 0xf1, 0xe0, 0xe3, 0x5d}};
// {25504FB4-7BAB-4BC1-9C69-CF81890F0EF5} tray (language bar) support
static const GUID GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT = {
    0x25504fb4, 0x7bab, 0x4bc1, {0x9c, 0x69, 0xcf, 0x81, 0x89, 0x0f, 0x0e, 0xf5}};
#endif

// Language-bar pieces missing from the mingw-w64 ctfutb.h (the Windows SDK has all of them).
#ifndef TF_LBI_STYLE_BTN_MENU
#define TF_LBI_STYLE_SHOWNINTRAY 0x00000002
#define TF_LBI_STYLE_BTN_MENU 0x00020000
#define TF_LBMENUF_RADIOCHECKED 0x00000008
#endif

#ifndef __ITfMenu_INTERFACE_DEFINED__
#define __ITfMenu_INTERFACE_DEFINED__
// {6F8A98E4-AAA0-4F15-8C5B-07E0DF0A3DD8}
static const IID IID_ITfMenu = {
    0x6f8a98e4, 0xaaa0, 0x4f15, {0x8c, 0x5b, 0x07, 0xe0, 0xdf, 0x0a, 0x3d, 0xd8}};
struct ITfMenu : public IUnknown {
  virtual HRESULT STDMETHODCALLTYPE AddMenuItem(UINT id, DWORD flags, HBITMAP bmp, HBITMAP mask,
                                                const WCHAR* text, ULONG cch,
                                                ITfMenu** sub) = 0;
};
#endif

#ifndef __ITfLangBarItemButton_INTERFACE_DEFINED__
#define __ITfLangBarItemButton_INTERFACE_DEFINED__
// {28C7F1D0-DE25-11D2-AFDD-00105A2799B5}
static const IID IID_ITfLangBarItemButton = {
    0x28c7f1d0, 0xde25, 0x11d2, {0xaf, 0xdd, 0x00, 0x10, 0x5a, 0x27, 0x99, 0xb5}};
enum TfLBIClick { TF_LBI_CLK_RIGHT = 1, TF_LBI_CLK_LEFT = 2 };
struct ITfLangBarItemButton : public ITfLangBarItem {
  virtual HRESULT STDMETHODCALLTYPE OnClick(TfLBIClick click, POINT pt, const RECT* area) = 0;
  virtual HRESULT STDMETHODCALLTYPE InitMenu(ITfMenu* menu) = 0;
  virtual HRESULT STDMETHODCALLTYPE OnMenuSelect(UINT id) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetIcon(HICON* icon) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetText(BSTR* text) = 0;
};
#endif
