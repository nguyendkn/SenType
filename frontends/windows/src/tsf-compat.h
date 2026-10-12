// Fills gaps in the mingw-w64 TSF headers (MSVC's Windows SDK already has all of these).
#pragma once

#include <msctf.h>

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
