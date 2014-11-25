/*******************************************************************
// Copyright (c) 2000, Robert Umbehant
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
// Params.cpp: implementation of the CParams class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif				

#define FILEID		"Param_List_rdu"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CParams::CParams()
{_STT();
	Init( FALSE );
}

CParams::~CParams()
{_STT();
	Destroy();
}

void CParams::Init(BOOL bDestroy)
{_STT();
	if ( bDestroy ) Destroy();
}

void CParams::DeleteObject( void *node )
{_STT();
	LPPARAMINFO ppi = (LPPARAMINFO)node;

	if ( ppi->pdata != NULL )
	{	delete [] ppi->pdata;
		ppi->pdata = NULL;
	} // end if

	if ( ppi->param != NULL )
	{	delete [] ppi->param;
		ppi->param = NULL;
	} // end if

	CLList::DeleteObject( node );
}

void CParams::Destroy()
{_STT();
	CLList::Destroy();
}

HPARAM CParams::Add(DWORD function, LPCTSTR pParams)
{_STT();
	// Allocate memory
	LPPARAMINFO node = (LPPARAMINFO)New();
	if ( node == NULL ) return NULL;

	// Copy the function id
	node->function = function;

	// Copy description	if any
	if ( pParams != NULL ) { strcpy_sz( node->pdesc, pParams ); }
	else return node;

	char type[ 512 ];
	char size[ 512 ];
	char name[ 512 ];
	char min[ 512 ];
	char max[ 512 ];
	char def[ 512 ];
	char step[ 512 ];
	DWORD i = 0;

	node->n = 0;

	// Calculate total size needed
	DWORD sz = 0;
	while( ( sz = GetParam( &node->pdesc[ i ], type, size, NULL, NULL, NULL, NULL, NULL ) ) != 0 )
	{	node->n++;
		node->size += GetParamSize( GetParamType( type ), strtoul( size, NULL, 10 ) );		
		i += sz;
	} // end while

	// Anything to allocate?
	if ( node->size == 0 || node->n == 0 )
	{	node->n = 0;
		node->size = 0;
		return node;	
	} // end if

	// Allocate memory for param data
	node->pdata = new BYTE[ node->size + 1 ];
	if ( node->pdata == NULL )
	{	DeleteObject( node );
		return NULL;
	} // end if
	ZeroMemory( node->pdata, node->size + 1 );

	// Allocate memory for param index
	node->param = new PARAM[ node->n ];
	if ( node->param == NULL )
	{	DeleteObject( node );
		return NULL;
	} // end if
	ZeroMemory( node->param, sizeof( PARAM ) * node->n );

	// Save param info so it's easy to index
	i = 0;
	DWORD p = 0, o = 0;
	while( ( sz = GetParam( &node->pdesc[ i ], type, size, name, min, max, def, step ) ) != 0 )
	{
		node->param[ p ].type = GetParamType( type );
		node->param[ p ].data = &node->pdata[ o ];
		node->param[ p ].size = GetParamSize(	node->param[ p ].type, 
												strtoul( size, NULL, 10 ) );
		o += node->param[ p ].size;
		strcpy_sz( node->param[ p ].name, name );

		if ( *min ) 
		{	node->param[ p ].flags |= PFLAG_MIN;
			node->param[ p ].min = strtod( min, NULL );
		} else node->param[ p ].min = 0;
		strcpy_sz( node->param[ p ].pmin, min );

		if ( *max ) 
		{	node->param[ p ].flags |= PFLAG_MAX;
			node->param[ p ].max = strtod( max, NULL );
		} else node->param[ p ].max = 0;
		strcpy_sz( node->param[ p ].pmax, max );

		if ( *def ) 
		{	node->param[ p ].flags |= PFLAG_DEF;
			node->param[ p ].def = strtod( def, NULL );
		} else node->param[ p ].def = 0;
		strcpy_sz( node->param[ p ].pdef, def );

		if ( *step ) 
		{	node->param[ p ].flags |= PFLAG_STEP;
			node->param[ p ].step = strtod( step, NULL );
		} else node->param[ p ].step = 0;
		strcpy_sz( node->param[ p ].pstep, step );

		// Set default value
		SetValue( node, p, GetDef( node, p ) );

		p++;
		i += sz;

	} // end while

	// We're ready
	return node;
}

DWORD CParams::GetParamSize( BYTE type, DWORD size )
{_STT();
	switch( type )
	{
		case PTYPE_DWORD : return sizeof( DWORD ); break;
		case PTYPE_STR : return size; break;
		case PTYPE_BIN : return size; break;
		case PTYPE_LONG : return sizeof( long ); break;
		case PTYPE_ULONG : return sizeof( unsigned long ); break;
		case PTYPE_INT : return sizeof( int ); break;
		case PTYPE_UINT : return sizeof( unsigned int ); break;
		case PTYPE_CHAR : return sizeof( char ); break;
		case PTYPE_UCHAR : return sizeof( unsigned char ); break;
		case PTYPE_BOOL : return sizeof( bool ); break;
		case PTYPE_FLOAT : return sizeof( float ); break;
		case PTYPE_DOUBLE : return sizeof( double ); break;
		case PTYPE_LDOUBLE : return sizeof( long double ); break;

	} // end switch

	return 0;
}

long double CParams::Convert(BYTE type, long double from )
{_STT();
	switch( type )
	{
		case PTYPE_DWORD : return (DWORD)from; break;
		case PTYPE_STR : return (double)0; break;
		case PTYPE_BIN : return (double)0; break;
		case PTYPE_LONG : return (long)from; break;
		case PTYPE_ULONG : return (unsigned long)from; break;
		case PTYPE_INT : return (int)from; break;
		case PTYPE_UINT : return (unsigned int)from; break;
		case PTYPE_CHAR : return (char)from; break;
		case PTYPE_UCHAR : return (unsigned char)from; break;
		case PTYPE_BOOL : return ( from != 0 ); break;
		case PTYPE_FLOAT : return (float)from; break;
		case PTYPE_DOUBLE : return (double)from; break;
		case PTYPE_LDOUBLE : return from; break;

	} // end switch

	return 0;
}

BYTE CParams::GetParamType(LPCTSTR pType)
{_STT();
	if ( pType == NULL ) return PTYPE_VOID;

	// Figure out the type
	if ( !strcmpi( pType, "dword" ) ) return PTYPE_DWORD;
	else if ( !strcmpi( pType, "str" ) ) return PTYPE_STR;
	else if ( !strcmpi( pType, "bin" ) ) return PTYPE_BIN;
	else if ( !strcmpi( pType, "long" ) ) return PTYPE_LONG;
	else if ( !strcmpi( pType, "ulong" ) ) return PTYPE_ULONG;
	else if ( !strcmpi( pType, "int" ) ) return PTYPE_INT;
	else if ( !strcmpi( pType, "uint" ) ) return PTYPE_UINT;
	else if ( !strcmpi( pType, "char" ) ) return PTYPE_CHAR;
	else if ( !strcmpi( pType, "uchar" ) ) return PTYPE_UCHAR;
	else if ( !strcmpi( pType, "bool" ) ) return PTYPE_BOOL;
	else if ( !strcmpi( pType, "float" ) ) return PTYPE_FLOAT;	
	else if ( !strcmpi( pType, "double" ) ) return PTYPE_DOUBLE;
	else if ( !strcmpi( pType, "ldouble" ) ) return PTYPE_LDOUBLE;

	return PTYPE_VOID;
}


BOOL __cdecl CParams::SetParams( HPARAM hParam, ... )
{_STT();
	if ( hParam == NULL || hParam->param == NULL ) return FALSE;

	RULIB_TRY // This could GPF if caller screws up
	{
		LPVOID *ptr = ( (LPVOID*)&hParam ) + 1;

		for ( DWORD i = 0; i < hParam->n; i++ )
			SetParam( &hParam->param[ i ], *ptr ), ptr++;

	} // end try
	RULIB_CATCH_ALL { return FALSE; }

	return TRUE;
}

BOOL __cdecl CParams::GetParams( HPARAM hParam, ... )
{_STT();
	if ( hParam == NULL || hParam->param == NULL ) return FALSE;

	RULIB_TRY // This could GPF if caller screws up
	{
		LPVOID *ptr = ( (LPVOID*)&hParam ) + 1;

		for ( DWORD i = 0; i < hParam->n; i++ )
			GetParam( &hParam->param[ i ], *ptr ), ptr++;

	} // end try
	RULIB_CATCH_ALL { return FALSE; }

	return TRUE;
}

DWORD CParams::Load( LPBYTE buf, DWORD size, BOOL bMerge )
{_STT();
	if ( !bMerge ) Destroy();

	CCfgFile	cfg;
	cfg.LoadFromMem( buf, size );
	return Load( &cfg );
}

DWORD CParams::Save( LPBYTE buf, DWORD size )
{_STT();
	CCfgFile	cfg;
	if ( !Save( &cfg ) ) return 0;
	return cfg.SaveToMem( buf, size );
}

DWORD CParams::Load( LPCTSTR pFile, BOOL bMerge )
{_STT();
	if ( !bMerge ) Destroy();

	return 0;
}

DWORD CParams::Save( LPCTSTR pFile )
{_STT();
	return 0;
}


BOOL CParams::Save(CCfgFile * pCfg)
{_STT();
	if ( pCfg == NULL ) return FALSE;

	DWORD		i = 0;
	char		gname[ 256 ];
	LPPARAMINFO ppi = NULL;

	// For each element
	while( ( ppi = (LPPARAMINFO)GetNext( ppi ) ) != NULL )
	{
		// Create group name
		wsprintf( gname, "F(%lu)", i++ );
		HGROUP hGroup = pCfg->AddGroup( gname );

		// Set param description
		pCfg->SetValue( hGroup, "FunctionType", ppi->function );
		pCfg->SetValue( hGroup, "FunctionDescription", ppi->pdesc );
		if ( ppi->pdata != NULL && ppi->size > 0 )
			pCfg->SetValue( hGroup, "FunctionParams", ppi->pdata, ppi->size );
			   
	} // end while

	return TRUE;
}

BOOL CParams::Load(CCfgFile * pCfg)
{_STT();
	if ( pCfg == NULL ) return FALSE;

	DWORD function;
	char desc[ 1024 ];

	// Load each fx element
	HGROUP hGroup = NULL;
	while ( ( hGroup = pCfg->GetNext( hGroup ) ) != NULL )
	{
		// Get function information
		if ( 	pCfg->GetValue( hGroup, "FunctionType", &function ) &&
				pCfg->GetValue( hGroup, "FunctionDescription", desc, sizeof( desc ) ) )
		{
			LPPARAMINFO ppi = Add( function, desc );
			if ( ppi != NULL )
				pCfg->GetValue(	hGroup, "FunctionParams", 
								ppi->pdata, ppi->size );

		} // end if

	} // end while

	return TRUE;
}

// Param forms ( all params optional except type )
// {type}[ size ] {name} <[min],[max]>, ...
// {type}[ size ] {name} <[max]>, ...
// {type}[ size ] {name}, ...
// {type}[ size ], ...
// {type}, ...
// Example param list: "float Angle <0,359.99>"
// "uchar Hue <0,255,128>, uchar Saturation<0,255,128>"
// "long Level <0,255,80>, long Contrast<0,255,128>"
// "long New X <1,%width,%width>, long New Y<1,%height,%height>"
DWORD CParams::GetParam(LPCTSTR pList, LPSTR pType, LPSTR pSize, LPSTR pName, LPSTR pMin, LPSTR pMax, LPSTR pDef, LPSTR pStep)
{_STT();
	DWORD i = 0;

	if ( pList == NULL ) return 0;
	if ( pType != NULL ) *pType = 0;
	if ( pSize != NULL ) *pSize = 0;
	if ( pName != NULL ) *pName = 0;
	if ( pMin != NULL ) *pMin = 0;
	if ( pMax != NULL ) *pMax = 0;
	if ( pDef != NULL ) *pDef = 0;
	if ( pStep != NULL ) *pStep = 0;

	// Skip white space
	while(	pList[ i ] != 0 && 
			( pList[ i ] <= ' ' || pList[ i ] > '~' || pList[ i ] == ',' ) ) i++;
	if ( pList[ i ] == 0 ) return 0;
	i += GetToken( pType, &pList[ i ] );

	// Check for size
	while( pList[ i ] != 0 && ( pList[ i ] <= ' ' || pList[ i ] > '~' ) ) i++;
	if ( pList[ i ] == '[' ) 
	{
		i++;
		while( pList[ i ] != 0 && ( pList[ i ] <= ' ' || pList[ i ] > '~' ) ) i++;

		i += GetToken( pSize, &pList[ i ] );

		while ( pList[ i ] != 0 && pList[ i ] != ']' ) i++;
		if ( pList[ i ] != ']' ) return i;
		i++;
	} // end if

	// Skip white space
	while( pList[ i ] != 0 && ( pList[ i ] <= ' ' || pList[ i ] > '~' ) ) i++;
	if ( pList[ i ] == 0 || pList[ i ] == ',' ) return i;
	i += GetToken( pName, &pList[ i ], TRUE );

	char temp[ 512 ] = { 0 };

	while( pList[ i ] != 0 && ( pList[ i ] <= ' ' || pList[ i ] > '~' ) ) i++;
	if ( pList[ i ] != '<' ) return i; i++;


	while( pList[ i ] != 0 && ( pList[ i ] <= ' ' || pList[ i ] > '~' ) ) i++;
	if ( pList[ i ] == 0 ) return i;
	i += GetToken( temp, &pList[ i ] );

	while( pList[ i ] <= ' ' || pList[ i ] > '~' ) i++;
	if ( pList[ i ] != ',' )
	{	if ( pMax != NULL ) strcpy( pMax, temp );
		while ( pList[ i ] != 0 && pList[ i ] != '>' ) i++;
		if ( pList[ i ] == '>' )
		{	while ( pList[ i ] != 0 && pList[ i ] != ',' ) i++; }
		return i;
	} // end if

	// Save minimum
	if ( pMin != NULL ) strcpy( pMin, temp );

	i++; // Skip comma
	while( pList[ i ] <= ' ' || pList[ i ] > '~' ) i++;
	i += GetToken( pMax, &pList[ i ] );

	while( pList[ i ] <= ' ' || pList[ i ] > '~' ) i++;
	if ( pList[ i ] != ',' )
	{	while ( pList[ i ] != 0 && pList[ i ] != '>' ) i++;
		if ( pList[ i ] == '>' )
		{	while ( pList[ i ] != 0 && pList[ i ] != ',' ) i++; }
		return i;
	} // end if
	i++; // Skip comma
	while( pList[ i ] <= ' ' || pList[ i ] > '~' ) i++;
	i += GetToken( pDef, &pList[ i ] );

	while( pList[ i ] <= ' ' || pList[ i ] > '~' ) i++;
	if ( pList[ i ] != ',' )
	{	while ( pList[ i ] != 0 && pList[ i ] != '>' ) i++;
		if ( pList[ i ] == '>' )
		{	while ( pList[ i ] != 0 && pList[ i ] != ',' ) i++; }
		return i;
	} // end if
	i++; // Skip comma
	while( pList[ i ] <= ' ' || pList[ i ] > '~' ) i++;
	i += GetToken( pStep, &pList[ i ] );

	// Find start of next param
	while ( pList[ i ] != 0 && pList[ i ] != '>' ) i++;
	if ( pList[ i ] == '>' )
	{	while ( pList[ i ] != 0 && pList[ i ] != ',' ) i++; }

	return i;

}

DWORD CParams::GetToken( LPSTR pToken, LPCTSTR pStr, BOOL bSpaceOk )
{_STT();
	DWORD i = 0;

	// Read in token no spaces
	if ( !bSpaceOk ) while(	( pStr[ i ] > ' ' && pStr[ i ] <= '~' ) &&
							pStr[ i ] != '[' && pStr[ i ] != ']' &&
							pStr[ i ] != '<' && pStr[ i ] != '>' &&
							pStr[ i ] != ',' ) 
	{	if ( pToken != NULL ) pToken[ i ] = pStr[ i ];
		i++;
	} // end while

	// Read in token with spaces
	else
	{
		while(	( pStr[ i ] >= ' ' && pStr[ i ] <= '~' ) &&
				pStr[ i ] != '[' && pStr[ i ] != ']' &&
				pStr[ i ] != '<' && pStr[ i ] != '>' &&
				pStr[ i ] != ',' )
		{	
			if ( pToken != NULL ) pToken[ i ] = pStr[ i ];
			i++;
		} // end while

		if ( pToken != NULL )
		{
			// Lose trailing spaces		
			DWORD x = i;
			while ( x > 0 && pToken[ x - 1 ] == ' ' )
				x--, pToken[ x ] = 0;
		} // end if

	} // end else
		
	if ( pToken != NULL ) pToken[ i ] = 0;

	return i;
}

double CParams::GetValue(HPARAM hParam, DWORD i, double dDef )
{_STT();
	// Sanity check
	if ( hParam == NULL || i >= hParam->n ) return dDef;

	switch( hParam->param->type )
	{
		case PTYPE_DWORD : return *( (DWORD*)hParam->param[ i ].data ); break;
		case PTYPE_STR : return (double)0; break;
		case PTYPE_BIN : return (double)0; break;
		case PTYPE_LONG : return *( (long*)hParam->param[ i ].data ); break;
		case PTYPE_ULONG : return *( (unsigned long*)hParam->param[ i ].data ); break;
		case PTYPE_INT : return *( (int*)hParam->param[ i ].data ); break;
		case PTYPE_UINT : return *( (unsigned int*)hParam->param[ i ].data ); break;
		case PTYPE_CHAR : return *( (char*)hParam->param[ i ].data ); break;
		case PTYPE_UCHAR : return *( (unsigned char*)hParam->param[ i ].data ); break;
		case PTYPE_BOOL : return *( (bool*)hParam->param[ i ].data ); break;
		case PTYPE_FLOAT : return *( (float*)hParam->param[ i ].data ); break;
		case PTYPE_DOUBLE : return *( (double*)hParam->param[ i ].data ); break;
		case PTYPE_LDOUBLE : return *( (long double*)hParam->param[ i ].data ); break;

	} // end switch

	return dDef;
}

BOOL CParams::SetValue(HPARAM hParam, DWORD i, double v)
{_STT();
	// Sanity check
	if ( hParam == NULL || i >= hParam->n ) return FALSE;

	switch( hParam->param->type )
	{
		case PTYPE_DWORD : *( (DWORD*)hParam->param[ i ].data ) = (DWORD)v; break;
		case PTYPE_LONG : *( (long*)hParam->param[ i ].data ) = (long)v; break;
		case PTYPE_ULONG :*( (unsigned long*)hParam->param[ i ].data ) = (unsigned long)v; break;
		case PTYPE_INT : *( (int*)hParam->param[ i ].data ) = (int)v; break;
		case PTYPE_UINT : *( (unsigned int*)hParam->param[ i ].data ) = (unsigned int)v; break;
		case PTYPE_CHAR : *( (char*)hParam->param[ i ].data ) = (char)v; break;
		case PTYPE_UCHAR : *( (unsigned char*)hParam->param[ i ].data ) = (unsigned char)v; break;
		case PTYPE_BOOL : *( (bool*)hParam->param[ i ].data ) = ( v != 0 ); break;
		case PTYPE_FLOAT : *( (float*)hParam->param[ i ].data ) = (float)v; break;
		case PTYPE_DOUBLE : *( (double*)hParam->param[ i ].data ) = (double)v; break;
		case PTYPE_LDOUBLE : *( (long double*)hParam->param[ i ].data ) = (long double)v; break;
		default : return FALSE; break;

	} // end switch

	return TRUE;
}

BOOL CParams::GetParam(LPPARAM pParam, void * ptr)
{_STT();
	switch( pParam->type )
	{
		case PTYPE_STR :
			strncpy( (LPSTR)ptr, (LPSTR)pParam->data, pParam->size );
			break;

		case PTYPE_BIN :
			memcpy( ptr, pParam->data, pParam->size );
			break;

		case PTYPE_DWORD : *( (LPDWORD)ptr ) = *( (LPDWORD)pParam->data );
			break;

		case PTYPE_LONG : *( (long*)ptr ) = *( (long*)pParam->data );
			break;

		case PTYPE_ULONG : *( (unsigned long*)ptr ) = *( (unsigned long*)pParam->data );
			break;

		case PTYPE_INT : *( (int*)ptr ) = *( (int*)pParam->data );
			break;

		case PTYPE_UINT : *( (unsigned int*)ptr ) = *( (unsigned int*)pParam->data );
			break;

		case PTYPE_CHAR : *( (char*)ptr ) = *( (char*)pParam->data );
			break;

		case PTYPE_UCHAR : *( (unsigned char*)ptr ) = *( (unsigned char*)pParam->data );
			break;

		case PTYPE_BOOL : *( (bool*)ptr ) = *( (bool*)pParam->data );
			break;

		case PTYPE_FLOAT : *( (float*)ptr ) = *( (float*)pParam->data );
			break;

		case PTYPE_DOUBLE : *( (double*)ptr ) = *( (double*)pParam->data );
			break;

		case PTYPE_LDOUBLE : *( (long double*)ptr ) = *( (long double*)pParam->data );
			break;

		default : return FALSE; break;

	} // end switch

	return TRUE;
}

BOOL CParams::SetParam(LPPARAM pParam, void * ptr)
{_STT();
	switch( pParam->type )
	{
		case PTYPE_STR :
			strncpy( (LPSTR)pParam->data, (LPSTR)ptr, pParam->size );
			break;

		case PTYPE_BIN :
			memcpy( pParam->data, ptr, pParam->size );
			break;

		case PTYPE_DWORD : *( (LPDWORD)pParam->data ) = *( (LPDWORD)ptr );
			break;

		case PTYPE_LONG : *( (long*)pParam->data ) = *( (long*)ptr );
			break;

		case PTYPE_ULONG : *( (unsigned long*)pParam->data ) = *( (unsigned long*)ptr );
			break;

		case PTYPE_INT : *( (int*)pParam->data ) = *( (int*)ptr );
			break;

		case PTYPE_UINT : *( (unsigned int*)pParam->data ) = *( (unsigned int*)ptr );
			break;

		case PTYPE_CHAR : *( (char*)pParam->data ) = *( (char*)ptr );
			break;

		case PTYPE_UCHAR : *( (unsigned char*)pParam->data ) = *( (unsigned char*)ptr );
			break;

		case PTYPE_BOOL : *( (bool*)pParam->data ) = *( (bool*)ptr );
			break;

		case PTYPE_FLOAT : *( (float*)pParam->data ) = *( (float*)ptr );
			break;

		case PTYPE_DOUBLE : *( (double*)pParam->data ) = *( (double*)ptr );
			break;

		case PTYPE_LDOUBLE : *( (long double*)pParam->data ) = *( (long double*)ptr );
			break;

		default : return FALSE; break;

	} // end switch

	return TRUE;
}
