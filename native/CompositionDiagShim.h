#pragma once

#include "Common.h"

class CompositionDiagShim {
public:
    CompositionDiagShim() {
        s_owner = GetCurrentThreadId();
        ForEachRegistrySlot([](const char* name, void** slot) {
            if (!std::strcmp(name, "RegOpenKeyExW")) {
                s_openOriginal = reinterpret_cast<decltype(&RegOpenKeyExW)>(*slot);
                Swap(slot, reinterpret_cast<void*>(&OpenKey));
            } else if (!std::strcmp(name, "RegQueryValueExW")) {
                s_queryOriginal = reinterpret_cast<decltype(&RegQueryValueExW)>(*slot);
                Swap(slot, reinterpret_cast<void*>(&QueryValue));
            }
        });
    }

    static DWORD GuardHits() { return s_guardHits; }

    ~CompositionDiagShim() {
        ForEachRegistrySlot([](const char* name, void** slot) {
            if (!std::strcmp(name, "RegOpenKeyExW")) {
                Swap(slot, reinterpret_cast<void*>(s_openOriginal));
            } else if (!std::strcmp(name, "RegQueryValueExW")) {
                Swap(slot, reinterpret_cast<void*>(s_queryOriginal));
            }
        });
        s_owner = 0;
    }

private:
    template <class Visitor>
    static void ForEachRegistrySlot(Visitor visit) {
        auto base = reinterpret_cast<BYTE*>(GetModuleHandleW(L"Windows.UI.Xaml.dll"));
        if (!base) {
            return;
        }
        auto dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        auto nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
        auto& directory = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
        auto descriptor = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + directory.VirtualAddress);
        for (; descriptor->Name; ++descriptor) {
            if (!std::strstr(reinterpret_cast<const char*>(base + descriptor->Name), "registry")) {
                continue;
            }
            auto names = reinterpret_cast<IMAGE_THUNK_DATA*>(base + descriptor->OriginalFirstThunk);
            auto slots = reinterpret_cast<IMAGE_THUNK_DATA*>(base + descriptor->FirstThunk);
            for (; names->u1.AddressOfData; ++names, ++slots) {
                if (IMAGE_SNAP_BY_ORDINAL(names->u1.Ordinal)) {
                    continue;
                }
                auto import = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base + names->u1.AddressOfData);
                visit(import->Name, reinterpret_cast<void**>(&slots->u1.Function));
            }
        }
    }

    static void Swap(void** slot, void* target) {
        DWORD previous;
        if (VirtualProtect(slot, sizeof(void*), PAGE_READWRITE, &previous)) {
            InterlockedExchangePointer(slot, target);
            VirtualProtect(slot, sizeof(void*), previous, &previous);
        }
    }

    static bool OnOwnerThread() { return s_owner == GetCurrentThreadId(); }

    static LSTATUS WINAPI OpenKey(HKEY key, LPCWSTR subKey, DWORD options, REGSAM access, PHKEY result) {
        LSTATUS status = s_openOriginal(key, subKey, options, access, result);
        if (status == ERROR_SUCCESS || !OnOwnerThread() || key != HKEY_LOCAL_MACHINE || !subKey ||
            _wcsicmp(subKey, L"Software\\Microsoft\\XAML\\Debug")) {
            return status;
        }
        return s_openOriginal(HKEY_LOCAL_MACHINE, L"Software\\Microsoft", options, access, result);
    }

    static LSTATUS WINAPI QueryValue(HKEY key, LPCWSTR name, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD size) {
        if (!OnOwnerThread() || !name || _wcsicmp(name, L"DisableCompositionDiag")) {
            return s_queryOriginal(key, name, reserved, type, data, size);
        }
        ++s_guardHits;
        if (type) {
            *type = REG_DWORD;
        }
        if (data && size && *size >= sizeof(DWORD)) {
            *reinterpret_cast<DWORD*>(data) = 1;
        }
        if (size) {
            *size = sizeof(DWORD);
        }
        return ERROR_SUCCESS;
    }

    static inline std::atomic<DWORD> s_owner{0};
    static inline std::atomic<DWORD> s_guardHits{0};
    static inline decltype(&RegOpenKeyExW) s_openOriginal{};
    static inline decltype(&RegQueryValueExW) s_queryOriginal{};
};
