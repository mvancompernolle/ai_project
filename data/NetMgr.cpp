/*******************************************************************
// Copyright (c) 2002, Robert Umbehant
// mailto:rumbehant@wheresjames.com
//
// This library is free software; you can redistribute it and/or 
// modify it under the terms of the GNU Lesser General Public 
// License as published by the Free Software Foundation; either 
// version 2.1 of the License, or (at your option) any later 
// version.
//
// This library is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public 
// License along with this library; if not, write to the Free 
// Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
// MA 02111-1307 USA 
//
*******************************************************************/
// NetMgr.cpp: implementation of the CNetMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNetMgr::CNetMgr()
{_STT();

}

CNetMgr::~CNetMgr()
{_STT();

}

long CNetMgr::NetMsg( LPVOID pData, WPARAM wParam, LPARAM lParam, const GUID *pGuid )
{_STT();
	// Get a pointer to the class
	CNetMgr *pNm = (CNetMgr*)pData;
	if ( pNm == NULL ) return FALSE;

	// Let the instance handle it
	return pNm->OnNetMsg( wParam, lParam, pGuid );
}

long CNetMgr::OnNetMsg(WPARAM wParam, LPARAM lParam, const GUID *pGuid)
{_STT();
	CNetCom *pNc = (CNetCom*)lParam;
	if ( pNc == NULL ) return E_FAIL;

	// Authenticate the connection
	if ( !OnAuthenticate( pNc ) ) return E_FAIL;
	
	// Hook NetCmd messages
	if ( pGuid != NULL && IsEqualGUID( NetCmd::IID, *pGuid ) )
	{
		// Save network error if any
		if ( pNc->GetNetError() )
			SetNetError( pNc->GetNetError() );

		// Cleanup if node is disconnecting
		// +++ This doesn't work because it causes the calling 
		// class to delete itself.  For now, Cleanup() must be 
		// called from the outside.  This is just to remind me 
		// to come up with something.
//		if ( wParam == NetCmd::efDisconnect ) Cleanup();

	} // end if

	// Check for update request
	if ( ( wParam & CNetMsg::fWantUpdate ) != 0 )
	{	AddUpdateNotification(	pNc->Tx()->GetConnectionId(), pGuid, 
								wParam & CNetMsg::fMask );

	} // end if

	// Check for remove notification command
	if ( ( wParam & CNetMsg::fCancelUpdate ) != 0 )
	{	RemoveUpdateNotification(	pNc->Tx()->GetConnectionId(), pGuid, 
									wParam & CNetMsg::fMask );

		// Don't process this message
		return S_OK;

	} // end if

	// Turn off internal flags
	wParam &= CNetMsg::fCmdMask;
	pNc->SMsg().dwFunction &= CNetMsg::fCmdMask;

	// Pass on the message
	return MMessage( wParam, lParam, pGuid );
}

CNetCom* CNetMgr::FindSession( const GUID *guidId )
{_STT();
	if ( !m_lstSession.Size() ) return NULL;
	if ( guidId == NULL ) return *m_lstSession.begin();
	return m_lstSession[ *( (GUID*)guidId ) ];
}

CNetCom* CNetMgr::AddSession( const GUID *guidId, CNetCom* pNc )
{_STT();
	if ( pNc == NULL ) return FALSE;

	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return NULL;

	// Set the connection id
	pNc->Tx()->SetConnectionId( guidId );

	// Set global description if applicable
	if ( m_sGlobalDescription.size() )
		pNc->Tx()->SetConnectionDescription( m_sGlobalDescription );

	// Save session pointer into list	
	m_lstSession.push_back( *( (GUID*)guidId ), pNc );

	// Notify of new node
	MMessage( NetCmd::efNodeAdded, (LPARAM)this, &NetCmd::IID );

	// Return a pointer to the session object
	return pNc;
}

CNetCom* CNetMgr::AddSession( const GUID *guidId )
{_STT();
	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return NULL;

	// Create new session
	CNetCom* pNc = CreateNewSession();
	if ( pNc == NULL ) return NULL;

	// Add to the list
	return AddSession( guidId, pNc );
}

BOOL CNetMgr::InitSession(CNetCom *pNc)
{_STT();
	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return FALSE;

	// Want callbacks
	if ( !pNc->SetCallbackFunction( CNetMgr::NetMsg, this ) )
		return FALSE;

	// Let the session know it's being initialized
	pNc->OnInitSession();

	return TRUE;
}

void CNetMgr::Destroy()
{_STT();
	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return;

	// Lose all the sessions
	m_lstSession.Destroy();
}


BOOL CNetMgr::Cleanup()
{_STT();
	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return FALSE;

	BOOL bChanged = FALSE;
	
	// Get the first object
	THList< GUID, CNetCom >::iterator it = NULL;
	while ( ( it = m_lstSession.next( it ) ) != NULL )
	{
		CNetCom* pNc = (*it);

		// Erase this session if invalid
		if ( !pNc || !pNc->IsValid() )
		{
			bChanged = TRUE;

			it = RemoveNode( it );

		} // end if

	} // end while

	return bChanged;
}

CNetCom* CNetMgr::GetNode( const GUID *pGuid )
{_STT();
	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return NULL;

	return FindSession( pGuid );
}

CNetCom* CNetMgr::CreateNewSession()
{_STT();
	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return NULL;

	// Delete invalid connections
	Cleanup();

	// Create specialized session object
	CNetCom* pNc = CreateSessionObject();
	if ( pNc == NULL ) return FALSE;

	// Initialize
	InitSession( pNc );

	return pNc;
}

CNetCom* CNetMgr::FindByRxName(LPCTSTR pName)
{_STT();
	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return NULL;

	// Attempt to find the connection in the list
	THList< GUID, CNetCom >::iterator it = m_lstSession.begin();
	for ( it = m_lstSession.begin(); it != m_lstSession.end(); it = m_lstSession.next( it ) )
		if ( !strcmpi( (*it)->Rx()->GetConnectionName(), pName ) )
			return *it;

	return NULL;
}

CNetCom* CNetMgr::FindByTxName(LPCTSTR pName)
{_STT();
	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return NULL;

	// Attempt to find the connection in the list
	THList< GUID, CNetCom >::iterator it = m_lstSession.begin();
	for ( it = m_lstSession.begin(); it != m_lstSession.end(); it = m_lstSession.next( it ) )
		if ( !strcmpi( (*it)->Tx()->GetConnectionName(), pName ) )
			return *it;

	return NULL;
}

BOOL CNetMgr::AddUpdateNotification(const GUID *pguidNode, const GUID *pguidClass, DWORD dwCmd)
{_STT();
	// Sanity checks
	if ( pguidNode == NULL || pguidClass == NULL )
		return FALSE;

	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return FALSE;
	
	// Get a list of nodes for the specified class
	THList< DWORD, THList< GUID, CTimeout > > *pCmds = m_lstUpdates.get( *pguidClass );
	if ( pCmds == NULL ) return FALSE;

	// Get the list of nodes wanting updates on this command
	THList< GUID, CTimeout > *pNodes = pCmds->get( dwCmd );
	if ( pNodes == NULL ) return FALSE;

	// Add if not in the list already
	if ( (*pNodes)[ *pguidNode ] == NULL )
		pNodes->push_back( *pguidNode );

	return TRUE;
}

BOOL CNetMgr::NotifyNetwork(const GUID *pguidClass, DWORD dwCmd)
{_STT();
	// Sanity checks
	if ( pguidClass == NULL ) return FALSE;

	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return FALSE;

	// Get a list of nodes for the specified class
	THList< DWORD, THList< GUID, CTimeout > > *pCmds = m_lstUpdates.get( *pguidClass );
	if ( pCmds == NULL ) return FALSE;

	// Get the list of nodes wanting updates on this command
	THList< GUID, CTimeout > *pNodes = pCmds->get( dwCmd );
	if ( pNodes == NULL ) return FALSE;

	BOOL bErased = FALSE;
	THList< GUID, CTimeout >::iterator it = NULL;
	while ( ( it = pNodes->next( it ) ) != NULL )

		// Attempt to send the message
		if ( !Msg( it->key(), pguidClass, dwCmd | CNetMsg::fUpdate ) )
		{	bErased = TRUE; it = pNodes->erase( it ); }

	// Cleanup invalid nodes as we go...
	if ( bErased && !pNodes->Size() )
	{	pCmds->erase( dwCmd );
		if ( !pCmds->Size() ) m_lstUpdates.erase( *pguidClass );
	} // end if

	return TRUE;
}

BOOL CNetMgr::CleanupNotifications()
{_STT();
	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return FALSE;
	
	// Validate the entire update list
	THList< GUID, THList< DWORD, THList< GUID, CTimeout > > >::iterator itUpdates = NULL;
	while ( ( itUpdates = m_lstUpdates.next( itUpdates ) ) != NULL )
	{
		// Validate items if any
		if ( (*itUpdates)->Size() )
		{
			THList< DWORD, THList< GUID, CTimeout > >::iterator itCmds = NULL;
			while ( ( itCmds = (*itUpdates)->next( itCmds ) ) != NULL )
			{
				// Validate items if any
				if ( (*itCmds)->Size() )
				{
					THList< GUID, CTimeout >::iterator itNodes = NULL;
					while ( ( itNodes = (*itCmds)->next( itNodes ) ) != NULL )
					{
						// Erase if invalid node
						if ( GetNode( itNodes->key() ) == NULL )
							itNodes = (*itCmds)->erase( itNodes );

					} // end while

				} // end if

				// Erase empty node list
				if ( !(*itCmds)->Size() )
					itCmds = (*itUpdates)->erase( itCmds );

			} // end while

		} // end if

		// Erase empty command list
		if ( !(*itUpdates)->Size() )
			itUpdates = m_lstUpdates.erase( itUpdates );

	} // end while

	return TRUE;
}

BOOL CNetMgr::RemoveUpdateNotification(const GUID *pguidNode, const GUID *pguidClass, DWORD dwCmd)
{_STT();
	// Sanity checks
	if ( pguidNode == NULL || pguidClass == NULL )
		return FALSE;

	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return FALSE;
	
	// Get a list of nodes for the specified class
	THList< DWORD, THList< GUID, CTimeout > > *pCmds = m_lstUpdates.get( *pguidClass );
	if ( pCmds == NULL ) return FALSE;

	// Get the list of nodes wanting updates on this command
	THList< GUID, CTimeout > *pNodes = pCmds->get( dwCmd );
	if ( pNodes == NULL ) return FALSE;

	// Erase this node
	pNodes->erase( *pguidNode );

	// Cleanup invalid nodes as we go...
	if ( !pNodes->Size() )
	{	pCmds->erase( dwCmd );
		if ( !pCmds->Size() ) m_lstUpdates.erase( *pguidClass );
	} // end if

	return TRUE;
}

THList< GUID, CNetCom>::iterator CNetMgr::RemoveNode( THList< GUID, CNetCom>::iterator itNode )
{_STT();
	// Acquire lock
	CTlLocalLock ll( *this );
	if ( !ll.IsLocked() ) return NULL;
	
	// Erase this session
	THList< GUID, CNetCom>::iterator itPrev = m_lstSession.erase( itNode );

	// Notify of node change
	MMessage( NetCmd::efNodeRemoved, (LPARAM)this, &NetCmd::IID );

	return itPrev;
}
