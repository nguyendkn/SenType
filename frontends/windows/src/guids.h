// Identity of the SenType text service. Never reuse these GUIDs for another product.
#pragma once

#include <initguid.h>
#include <windows.h>

// {3922582A-5FEA-483C-BF9B-BD6D5BE68439}  COM class of the text service
DEFINE_GUID(CLSID_SenTypeTextService, 0x3922582a, 0x5fea, 0x483c, 0xbf, 0x9b, 0xbd, 0x6d, 0x5b,
            0xe6, 0x84, 0x39);
// {7DF76D53-8F4D-4146-8B10-552CD79BB65F}  Vietnamese language profile
DEFINE_GUID(GUID_SenTypeProfile, 0x7df76d53, 0x8f4d, 0x4146, 0x8b, 0x10, 0x55, 0x2c, 0xd7, 0x9b,
            0xb6, 0x5f);

// {199B0D02-29D8-4F65-85BE-614D95EFD15C}  language-bar button (notification-area / language bar item)
DEFINE_GUID(GUID_SenTypeLangBarItem, 0x199b0d02, 0x29d8, 0x4f65, 0x85, 0xbe, 0x61, 0x4d, 0x95,
            0xef, 0xd1, 0x5c);

// ICON resource id in res/sentype.rc. Profile registration passes -id as the icon index.
constexpr int kIconResourceId = 101;
constexpr LANGID kLangVietnamese = 0x042A;  // vi-VN
constexpr wchar_t kServiceName[] = L"SenType";
