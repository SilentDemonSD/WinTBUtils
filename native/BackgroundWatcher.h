#pragma once

#include "Common.h"

class BackgroundWatcher : public winrt::implements<BackgroundWatcher, IVisualTreeServiceCallback2, winrt::non_agile> {
public:
    BackgroundWatcher(winrt::com_ptr<IXamlDiagnostics> diagnostics, Mode mode, int expected)
        : m_diagnostics(std::move(diagnostics)), m_mode(mode), m_expected(expected), m_finished(CreateEventW(nullptr, TRUE, FALSE, nullptr)) {}

    ~BackgroundWatcher() { CloseHandle(m_finished); }

    void WaitForTargets(DWORD timeoutMs) const { WaitForSingleObject(m_finished, timeoutMs); }
    int Matched() const { return m_matched; }

    HRESULT STDMETHODCALLTYPE OnVisualTreeChange(ParentChildRelation, VisualElement element, VisualMutationType mutation) noexcept override try {
        if (mutation != Add || !IsTargetName(element.Name)) {
            return S_OK;
        }
        wf::IInspectable object;
        winrt::check_hresult(m_diagnostics->GetIInspectableFromHandle(element.Handle, reinterpret_cast<::IInspectable**>(winrt::put_abi(object))));
        auto target = object.try_as<wux::FrameworkElement>();
        if (!target || !IsInsideTaskbarBackground(target)) {
            return S_OK;
        }
        if (m_mode == Mode::Clear) {
            target.Opacity(0.0);
        } else {
            target.ClearValue(wux::UIElement::OpacityProperty());
        }
        if (++m_matched >= m_expected) {
            SetEvent(m_finished);
        }
        return S_OK;
    } catch (...) {
        return winrt::to_hresult();
    }

    HRESULT STDMETHODCALLTYPE OnElementStateChanged(InstanceHandle, VisualElementState, LPCWSTR) noexcept override { return S_OK; }

private:
    static bool IsTargetName(LPCWSTR name) {
        if (!name) {
            return false;
        }
        std::wstring_view view(name);
        return view == L"BackgroundFill" || view == L"BackgroundStroke";
    }

    static bool IsInsideTaskbarBackground(wux::FrameworkElement const& target) {
        auto panel = wux::Media::VisualTreeHelper::GetParent(target);
        auto background = panel ? wux::Media::VisualTreeHelper::GetParent(panel) : nullptr;
        auto frameworkBackground = background ? background.try_as<wux::FrameworkElement>() : nullptr;
        return frameworkBackground && winrt::get_class_name(frameworkBackground) == L"Taskbar.TaskbarBackground" &&
               frameworkBackground.Name() != L"HoverFlyoutBackgroundControl";
    }

    winrt::com_ptr<IXamlDiagnostics> m_diagnostics;
    Mode m_mode;
    int m_expected;
    HANDLE m_finished;
    std::atomic<int> m_matched{0};
};
