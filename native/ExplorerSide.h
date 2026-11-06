#pragma once

#include "Common.h"
#include "ResultLog.h"

class ExplorerSide {
public:
    static UINT RequestMessage() {
        static const UINT message = RegisterWindowMessageW(L"TaskbarClear_Request");
        return message;
    }

    static LRESULT CALLBACK HookProc(int code, WPARAM wParam, LPARAM lParam) {
        auto call = reinterpret_cast<const CWPSTRUCT*>(lParam);
        if (code == HC_ACTION && call->message == RequestMessage()) {
            Attach(static_cast<Mode>(call->wParam));
        }
        return CallNextHookEx(nullptr, code, wParam, lParam);
    }

private:
    using InitializeDiagnostics = decltype(&InitializeXamlDiagnosticsEx);

    static void Attach(Mode mode) {
        HMODULE pinned;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(&HookProc), &pinned);
        g_requestedMode = mode;
        HRESULT hr = Connect();
        if (FAILED(hr)) {
            ResultLog::Write(mode, hr, S_OK, 0, 0);
        }
    }

    static HRESULT Connect() {
        wchar_t path[MAX_PATH];
        if (!GetModuleFileNameW(g_module, path, ARRAYSIZE(path))) {
            return HRESULT_FROM_WIN32(GetLastError());
        }
        HMODULE xaml = LoadLibraryExW(L"Windows.UI.Xaml.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        auto initialize = xaml ? reinterpret_cast<InitializeDiagnostics>(GetProcAddress(xaml, "InitializeXamlDiagnosticsEx")) : nullptr;
        if (!initialize) {
            return HRESULT_FROM_WIN32(GetLastError());
        }
        HRESULT hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
        for (int index = 1; index <= 10000 && hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND); ++index) {
            wchar_t connection[64];
            wsprintfW(connection, L"VisualDiagConnection%d", index);
            hr = initialize(connection, GetCurrentProcessId(), L"", path, kTapClsid, nullptr);
        }
        return hr;
    }
};
