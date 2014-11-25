/////////////////////////////////////////////////////////////////////////////
//
// CmgateWmpPlugin.cpp : Implementation of CCmgateWmpPlugin
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CmgateWmpPlugin.h"
#include "CPropertyDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CCmgateWmpPlugin::CCmgateWmpPlugin
// Constructor

CCmgateWmpPlugin::CCmgateWmpPlugin()
{
    lstrcpyn(m_szPluginText, _T("CmgateWmpPlugin Plugin"), sizeof(m_szPluginText) / sizeof(m_szPluginText[0]));
    m_dwAdviseCookie = 0;
}

/////////////////////////////////////////////////////////////////////////////
// CCmgateWmpPlugin::~CCmgateWmpPlugin
// Destructor

CCmgateWmpPlugin::~CCmgateWmpPlugin()
{
}

/////////////////////////////////////////////////////////////////////////////
// CCmgateWmpPlugin:::FinalConstruct
// Called when an plugin is first loaded. Use this function to do one-time
// intializations that could fail instead of doing this in the constructor,
// which cannot return an error.

HRESULT CCmgateWmpPlugin::FinalConstruct()
{
    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CCmgateWmpPlugin:::FinalRelease
// Called when a plugin is unloaded. Use this function to free any
// resources allocated in FinalConstruct.

void CCmgateWmpPlugin::FinalRelease()
{
    ReleaseCore();
}

/////////////////////////////////////////////////////////////////////////////
// CCmgateWmpPlugin::SetCore
// Set WMP core interface

HRESULT CCmgateWmpPlugin::SetCore(IWMPCore *pCore)
{
    HRESULT hr = S_OK;

    // release any existing WMP core interfaces
    ReleaseCore();

    // If we get passed a NULL core, this  means
    // that the plugin is being shutdown.

    if (pCore == NULL)
    {
        return S_OK;
    }

    m_spCore = pCore;

    // connect up the event interface
    CComPtr<IConnectionPointContainer>  spConnectionContainer;

    hr = m_spCore->QueryInterface( &spConnectionContainer );

    if (SUCCEEDED(hr))
    {
        hr = spConnectionContainer->FindConnectionPoint( __uuidof(IWMPEvents), &m_spConnectionPoint );
    }

    if (SUCCEEDED(hr))
    {
        hr = m_spConnectionPoint->Advise( GetUnknown(), &m_dwAdviseCookie );

        if ((FAILED(hr)) || (0 == m_dwAdviseCookie))
        {
            m_spConnectionPoint = NULL;
        }
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// CCmgateWmpPlugin::ReleaseCore
// Release WMP core interfaces

void CCmgateWmpPlugin::ReleaseCore()
{
    if (m_spConnectionPoint)
    {
        if (0 != m_dwAdviseCookie)
        {
            m_spConnectionPoint->Unadvise(m_dwAdviseCookie);
            m_dwAdviseCookie = 0;
        }
        m_spConnectionPoint = NULL;
    }

    if (m_spCore)
    {
        m_spCore = NULL;
    }
}




/////////////////////////////////////////////////////////////////////////////
// CCmgateWmpPlugin::DisplayPropertyPage
// Display property page for plugin

HRESULT CCmgateWmpPlugin::DisplayPropertyPage(HWND hwndParent)
{
    CPropertyDialog dialog(this);

    dialog.DoModal(hwndParent);

    return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// CCmgateWmpPlugin::GetProperty
// Get plugin property

HRESULT CCmgateWmpPlugin::GetProperty(const WCHAR *pwszName, VARIANT *pvarProperty)
{
    if (NULL == pvarProperty)
    {
        return E_POINTER;
    }

    return E_NOTIMPL;
}

/////////////////////////////////////////////////////////////////////////////
// CCmgateWmpPlugin::SetProperty
// Set plugin property

HRESULT CCmgateWmpPlugin::SetProperty(const WCHAR *pwszName, const VARIANT *pvarProperty)
{
    return E_NOTIMPL;
}
