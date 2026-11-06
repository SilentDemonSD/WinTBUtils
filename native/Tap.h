#pragma once

#include "Common.h"
#include "Session.h"

class Tap : public winrt::implements<Tap, IObjectWithSite, winrt::non_agile> {
public:
    HRESULT STDMETHODCALLTYPE SetSite(IUnknown* site) noexcept override try {
        m_site.copy_from(site);
        if (m_site) {
            Session::Start(m_site.as<IXamlDiagnostics>(), g_requestedMode);
        }
        return S_OK;
    } catch (...) {
        return winrt::to_hresult();
    }

    HRESULT STDMETHODCALLTYPE GetSite(REFIID iid, void** result) noexcept override { return m_site.as(iid, result); }

private:
    winrt::com_ptr<IUnknown> m_site;
};

class TapFactory : public winrt::implements<TapFactory, IClassFactory, winrt::non_agile> {
public:
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* outer, REFIID iid, void** result) noexcept override try {
        *result = nullptr;
        if (outer) {
            return CLASS_E_NOAGGREGATION;
        }
        return winrt::make<Tap>().as(iid, result);
    } catch (...) {
        return winrt::to_hresult();
    }

    HRESULT STDMETHODCALLTYPE LockServer(BOOL) noexcept override { return S_OK; }
};
