#include "Common.h"
#include "ShellWatcher.h"
#include "Tap.h"
#include "TaskbarChannel.h"

#pragma comment(linker, "/EXPORT:DllGetClassObject,PRIVATE")
#pragma comment(linker, "/EXPORT:DllCanUnloadNow,PRIVATE")

STDAPI DllGetClassObject(REFCLSID clsid, REFIID iid, LPVOID* result) try {
    *result = nullptr;
    if (clsid != kTapClsid) {
        return CLASS_E_CLASSNOTAVAILABLE;
    }
    return winrt::make<TapFactory>().as(iid, result);
} catch (...) {
    return winrt::to_hresult();
}

STDAPI DllCanUnloadNow() { return S_FALSE; }

extern "C" __declspec(dllexport) void CALLBACK Run(HWND, HINSTANCE, LPSTR, int) {
    ShellWatcher::Run();
}

extern "C" __declspec(dllexport) void CALLBACK Restore(HWND, HINSTANCE, LPSTR, int) {
    ShellWatcher::Stop();
    TaskbarChannel::Send(Mode::Restore);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        g_module = instance;
        DisableThreadLibraryCalls(instance);
    }
    return TRUE;
}
