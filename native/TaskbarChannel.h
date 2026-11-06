#pragma once

#include "Common.h"
#include "ExplorerSide.h"
#include "ResultLog.h"

class TaskbarChannel {
public:
    static void Send(Mode mode) {
        HWND taskbar = WaitForTaskbar();
        if (!taskbar) {
            ResultLog::Write(mode, HRESULT_FROM_WIN32(ERROR_TIMEOUT), S_OK, 0, 0);
            return;
        }
        HHOOK hook = SetWindowsHookExW(WH_CALLWNDPROC, ExplorerSide::HookProc, g_module, GetWindowThreadProcessId(taskbar, nullptr));
        if (!hook) {
            ResultLog::Write(mode, HRESULT_FROM_WIN32(GetLastError()), S_OK, 0, 0);
            return;
        }
        DWORD_PTR ignored;
        SendMessageTimeoutW(taskbar, ExplorerSide::RequestMessage(), static_cast<WPARAM>(mode), 0, SMTO_ABORTIFHUNG, kTaskbarWaitMs, &ignored);
        UnhookWindowsHookEx(hook);
    }

private:
    static HWND WaitForTaskbar() {
        for (DWORD waited = 0; waited < kTaskbarWaitMs; waited += 500) {
            HWND taskbar = FindWindowW(L"Shell_TrayWnd", nullptr);
            if (taskbar && FindWindowExW(taskbar, nullptr, L"Windows.UI.Composition.DesktopWindowContentBridge", nullptr)) {
                return taskbar;
            }
            Sleep(500);
        }
        return nullptr;
    }
};
