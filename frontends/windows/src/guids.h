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

constexpr LANGID kLangVietnamese = 0x042A;  // vi-VN
constexpr wchar_t kServiceName[] = L"SenType";
