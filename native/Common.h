#pragma once

#include <windows.h>
#include <unknwn.h>
#include <inspectable.h>
#include <ocidl.h>
#include <xamlOM.h>
#undef GetCurrentTime
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <atomic>
#include <cstring>
#include <mutex>
#include <string_view>
#include <thread>

namespace wf = winrt::Windows::Foundation;
namespace wux = winrt::Windows::UI::Xaml;

enum class Mode : WPARAM { Clear = 1, Restore = 2 };

inline constexpr GUID kTapClsid = {0x6f1c2a7e, 0x93d4, 0x4b58, {0xa1, 0x0c, 0x5e, 0x27, 0xd8, 0x44, 0xb3, 0x96}};
inline constexpr wchar_t kWatcherClass[] = L"TaskbarClearWatcher";
inline constexpr wchar_t kStateKey[] = L"Software\\TaskbarClear";
inline constexpr DWORD kSessionTimeoutMs = 30000;
inline constexpr DWORD kTaskbarWaitMs = 60000;
inline constexpr UINT kReapplyDebounceMs = 3000;
inline constexpr UINT_PTR kReapplyTimer = 1;

inline HMODULE g_module;
inline std::atomic<Mode> g_requestedMode{Mode::Clear};
