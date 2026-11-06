#pragma once

#include "BackgroundWatcher.h"
#include "Common.h"
#include "CompositionDiagShim.h"
#include "ResultLog.h"

class Session {
public:
    static void Start(winrt::com_ptr<IXamlDiagnostics> diagnostics, Mode mode) {
        std::thread(&Session::Run, std::move(diagnostics), mode).detach();
    }

private:
    static void Run(winrt::com_ptr<IXamlDiagnostics> diagnostics, Mode mode) {
        std::lock_guard lock(s_serial);
        int expected = CountTaskbars() * 2;
        int matched = 0;
        HRESULT adviseHr = S_OK;
        HRESULT unadviseHr = S_OK;
        try {
            auto service = diagnostics.as<IVisualTreeService3>();
            auto watcher = winrt::make_self<BackgroundWatcher>(diagnostics, mode, expected);
            {
                CompositionDiagShim shim;
                adviseHr = service->AdviseVisualTreeChange(watcher.get());
            }
            if (SUCCEEDED(adviseHr) || watcher->Matched() > 0) {
                watcher->WaitForTargets(kSessionTimeoutMs);
            }
            unadviseHr = service->UnadviseVisualTreeChange(watcher.get());
            matched = watcher->Matched();
        } catch (...) {
            adviseHr = winrt::to_hresult();
        }
        ResultLog::WriteGuardHits(CompositionDiagShim::GuardHits());
        ResultLog::Write(mode, adviseHr, unadviseHr, matched, expected);
    }

    static int CountTaskbars() {
        int count = 0;
        EnumWindows([](HWND window, LPARAM state) -> BOOL {
            DWORD processId;
            wchar_t name[32];
            GetWindowThreadProcessId(window, &processId);
            if (processId == GetCurrentProcessId() && GetClassNameW(window, name, ARRAYSIZE(name)) &&
                (!wcscmp(name, L"Shell_TrayWnd") || !wcscmp(name, L"Shell_SecondaryTrayWnd"))) {
                ++*reinterpret_cast<int*>(state);
            }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&count));
        return count;
    }

    static inline std::mutex s_serial;
};
