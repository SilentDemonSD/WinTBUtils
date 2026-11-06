#pragma once

#include "Common.h"

class ResultLog {
public:
    static void Write(Mode mode, HRESULT adviseHr, HRESULT unadviseHr, int matched, int expected) {
        Set(L"AdviseHr", static_cast<DWORD>(adviseHr));
        Set(L"UnadviseHr", static_cast<DWORD>(unadviseHr));
        Set(L"Matched", static_cast<DWORD>(matched));
        Set(L"Expected", static_cast<DWORD>(expected));
        Set(L"Mode", static_cast<DWORD>(mode));
    }

    static void WriteGuardHits(DWORD hits) { Set(L"GuardHits", hits); }

private:
    static void Set(const wchar_t* name, DWORD value) {
        RegSetKeyValueW(HKEY_CURRENT_USER, kStateKey, name, REG_DWORD, &value, sizeof(value));
    }
};
