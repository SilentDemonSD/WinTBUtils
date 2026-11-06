#pragma once

#include "Common.h"
#include "TaskbarChannel.h"

class ShellWatcher {
public:
    static void Run() {
        if (HWND existing = FindWindowW(kWatcherClass, nullptr)) {
            PostMessageW(existing, WM_DISPLAYCHANGE, 0, 0);
            return;
        }
        WNDCLASSW windowClass{};
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = g_module;
        windowClass.lpszClassName = kWatcherClass;
        RegisterClassW(&windowClass);
        if (!CreateWindowExW(WS_EX_TOOLWINDOW, kWatcherClass, nullptr, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, g_module, nullptr)) {
            return;
        }
        TaskbarChannel::Send(Mode::Clear);
        MSG message;
        while (GetMessageW(&message, nullptr, 0, 0) > 0) {
            DispatchMessageW(&message);
        }
    }

    static void Stop() {
        if (HWND existing = FindWindowW(kWatcherClass, nullptr)) {
            SendMessageTimeoutW(existing, WM_CLOSE, 0, 0, SMTO_ABORTIFHUNG, kTaskbarWaitMs, nullptr);
        }
    }

private:
    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
        static const UINT taskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
        if (message == taskbarCreated || message == WM_DISPLAYCHANGE) {
            SetTimer(window, kReapplyTimer, kReapplyDebounceMs, nullptr);
            return 0;
        }
        switch (message) {
        case WM_TIMER:
            KillTimer(window, kReapplyTimer);
            TaskbarChannel::Send(Mode::Clear);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(window, message, wParam, lParam);
        }
    }
};
