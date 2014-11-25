/////////////////////////////////////////////////////////////////////////////
//
// CmgateWmpPluginEvents.cpp : Implementation of CCmgateWmpPlugin events
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CmgateWmpPlugin.h"

void CCmgateWmpPlugin::OpenStateChange( long NewState )
{
    switch (NewState)
    {
    case wmposUndefined:
        break;
	case wmposPlaylistChanging:
        break;
	case wmposPlaylistLocating:
        break;
	case wmposPlaylistConnecting:
        break;
	case wmposPlaylistLoading:
        break;
	case wmposPlaylistOpening:
        break;
	case wmposPlaylistOpenNoMedia:
        break;
	case wmposPlaylistChanged:
        break;
	case wmposMediaChanging:
        break;
	case wmposMediaLocating:
        break;
	case wmposMediaConnecting:
        break;
	case wmposMediaLoading:
        break;
	case wmposMediaOpening:
        break;
	case wmposMediaOpen:
        break;
	case wmposBeginCodecAcquisition:
        break;
	case wmposEndCodecAcquisition:
        break;
	case wmposBeginLicenseAcquisition:
        break;
	case wmposEndLicenseAcquisition:
        break;
	case wmposBeginIndividualization:
        break;
	case wmposEndIndividualization:
        break;
	case wmposMediaWaiting:
        break;
	case wmposOpeningUnknownURL:
        break;
    default:
        break;
    }
}

void CCmgateWmpPlugin::PlayStateChange( long NewState )
{
    switch (NewState)
    {
    case wmppsUndefined:
        break;
	case wmppsStopped:
        break;
	case wmppsPaused:
        break;
	case wmppsPlaying:
        break;
	case wmppsScanForward:
        break;
	case wmppsScanReverse:
        break;
	case wmppsBuffering:
        break;
	case wmppsWaiting:
        break;
	case wmppsMediaEnded:
        break;
	case wmppsTransitioning:
        break;
	case wmppsReady:
        break;
	case wmppsReconnecting:
        break;
	case wmppsLast:
        break;
    default:
        break;
    }
}

void CCmgateWmpPlugin::AudioLanguageChange( long LangID )
{
}

void CCmgateWmpPlugin::StatusChange()
{
}

void CCmgateWmpPlugin::ScriptCommand( BSTR scType, BSTR Param )
{
}

void CCmgateWmpPlugin::NewStream()
{
}

void CCmgateWmpPlugin::Disconnect( long Result )
{
}

void CCmgateWmpPlugin::Buffering( VARIANT_BOOL Start )
{
}

void CCmgateWmpPlugin::Error()
{
    CComPtr<IWMPError>      spError;
    CComPtr<IWMPErrorItem>  spErrorItem;
    HRESULT                 dwError = S_OK;
    HRESULT                 hr = S_OK;

    if (m_spCore)
    {
        hr = m_spCore->get_error(&spError);

        if (SUCCEEDED(hr))
        {
            hr = spError->get_item(0, &spErrorItem);
        }

        if (SUCCEEDED(hr))
        {
            hr = spErrorItem->get_errorCode( (long *) &dwError );
        }
    }
}

void CCmgateWmpPlugin::Warning( long WarningType, long Param, BSTR Description )
{
}

void CCmgateWmpPlugin::EndOfStream( long Result )
{
}

void CCmgateWmpPlugin::PositionChange( double oldPosition, double newPosition)
{
}

void CCmgateWmpPlugin::MarkerHit( long MarkerNum )
{
}

void CCmgateWmpPlugin::DurationUnitChange( long NewDurationUnit )
{
}

void CCmgateWmpPlugin::CdromMediaChange( long CdromNum )
{
}

void CCmgateWmpPlugin::PlaylistChange( IDispatch * Playlist, WMPPlaylistChangeEventType change )
{
    switch (change)
    {
    case wmplcUnknown:
        break;
	case wmplcClear:
        break;
	case wmplcInfoChange:
        break;
	case wmplcMove:
        break;
	case wmplcDelete:
        break;
	case wmplcInsert:
        break;
	case wmplcAppend:
        break;
	case wmplcPrivate:
        break;
	case wmplcNameChange:
        break;
	case wmplcMorph:
        break;
	case wmplcSort:
        break;
	case wmplcLast:
        break;
    default:
        break;
    }
}

void CCmgateWmpPlugin::CurrentPlaylistChange( WMPPlaylistChangeEventType change )
{
    switch (change)
    {
    case wmplcUnknown:
        break;
	case wmplcClear:
        break;
	case wmplcInfoChange:
        break;
	case wmplcMove:
        break;
	case wmplcDelete:
        break;
	case wmplcInsert:
        break;
	case wmplcAppend:
        break;
	case wmplcPrivate:
        break;
	case wmplcNameChange:
        break;
	case wmplcMorph:
        break;
	case wmplcSort:
        break;
	case wmplcLast:
        break;
    default:
        break;
    }
}

void CCmgateWmpPlugin::CurrentPlaylistItemAvailable( BSTR bstrItemName )
{
}

void CCmgateWmpPlugin::MediaChange( IDispatch * Item )
{
}

void CCmgateWmpPlugin::CurrentMediaItemAvailable( BSTR bstrItemName )
{
}

void CCmgateWmpPlugin::CurrentItemChange( IDispatch *pdispMedia)
{
}

void CCmgateWmpPlugin::MediaCollectionChange()
{
}

void CCmgateWmpPlugin::MediaCollectionAttributeStringAdded( BSTR bstrAttribName,  BSTR bstrAttribVal )
{
}

void CCmgateWmpPlugin::MediaCollectionAttributeStringRemoved( BSTR bstrAttribName,  BSTR bstrAttribVal )
{
}

void CCmgateWmpPlugin::MediaCollectionAttributeStringChanged( BSTR bstrAttribName, BSTR bstrOldAttribVal, BSTR bstrNewAttribVal)
{
}

void CCmgateWmpPlugin::PlaylistCollectionChange()
{
}

void CCmgateWmpPlugin::PlaylistCollectionPlaylistAdded( BSTR bstrPlaylistName)
{
}

void CCmgateWmpPlugin::PlaylistCollectionPlaylistRemoved( BSTR bstrPlaylistName)
{
}

void CCmgateWmpPlugin::PlaylistCollectionPlaylistSetAsDeleted( BSTR bstrPlaylistName, VARIANT_BOOL varfIsDeleted)
{
}

void CCmgateWmpPlugin::ModeChange( BSTR ModeName, VARIANT_BOOL NewValue)
{
}

void CCmgateWmpPlugin::MediaError( IDispatch * pMediaObject)
{
}

void CCmgateWmpPlugin::OpenPlaylistSwitch( IDispatch *pItem )
{
}

void CCmgateWmpPlugin::DomainChange( BSTR strDomain)
{
}

void CCmgateWmpPlugin::SwitchedToPlayerApplication()
{
}

void CCmgateWmpPlugin::SwitchedToControl()
{
}

void CCmgateWmpPlugin::PlayerDockedStateChange()
{
}

void CCmgateWmpPlugin::PlayerReconnect()
{
}

void CCmgateWmpPlugin::Click( short nButton, short nShiftState, long fX, long fY )
{
}

void CCmgateWmpPlugin::DoubleClick( short nButton, short nShiftState, long fX, long fY )
{
}

void CCmgateWmpPlugin::KeyDown( short nKeyCode, short nShiftState )
{
}

void CCmgateWmpPlugin::KeyPress( short nKeyAscii )
{
}

void CCmgateWmpPlugin::KeyUp( short nKeyCode, short nShiftState )
{
}

void CCmgateWmpPlugin::MouseDown( short nButton, short nShiftState, long fX, long fY )
{
}

void CCmgateWmpPlugin::MouseMove( short nButton, short nShiftState, long fX, long fY )
{
}

void CCmgateWmpPlugin::MouseUp( short nButton, short nShiftState, long fX, long fY )
{
}
