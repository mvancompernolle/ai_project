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
// WinTime.cpp: implementation of the CWinTime class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <Math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define PI		((double)3.141592654)	
#define PI2		((double)2 * PI )	

static char s_days[][ 8 ] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Inv" };
static char s_months[][ 8 ] = {	"Inv",	"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
								"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char s_fdays[][ 16 ] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Invalid" };
static char s_fmonths[][ 16 ] = {	"Invalid",	"January", "February", "March", "April", "May", "June", 
									"July", "August", "September", "October", "November", "December" };
static DWORD g_mdays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

typedef struct tagTZINFO { long lBias; char code[ 8 ]; char name[ 32 ]; } TZINFO, *LPTZINFO;


// 1970 = 1900 + 2208988800
// 1980 = 1970 + 315536400
// 1980 = 1900 + 2524525200
// 1900 = 1601 + 9435484800
// 1970 = 1601 + 11644473600
// 1980 = 1601 + 11960010000

#define FTOFF_1900		9435484800
#define FTOFF_1970		11644473600
#define FTOFF_1980		11960010000

CWinTime::CWinTime( LPSYSTEMTIME pst, LPTIME_ZONE_INFORMATION pTzi )
{_STTEX();
	// No bias
	m_lBias = MAXLONG;

	// Initialize structure
	if ( pTzi == NULL ) ZeroMemory( &m_tz, sizeof( m_tz ) );
	else memcpy( &m_tz, pTzi, sizeof( m_tz ) );
	if ( pst == NULL ) ZeroMemory( &m_st, sizeof( m_st ) );
	else memcpy( &m_st, pst, sizeof( m_st ) );
}

CWinTime::CWinTime( LPSYSTEMTIME pst, LONG lBias )
{_STTEX();
	// No bias
	m_lBias = lBias;

	// Initialize structures
	ZeroMemory( &m_tz, sizeof( m_tz ) );	
	if ( pst == NULL ) ZeroMemory( &m_st, sizeof( m_st ) );
	else memcpy( &m_st, pst, sizeof( m_st ) );
}

CWinTime::CWinTime()
{_STTEX();
	m_lBias = MAXLONG;
	GetTimeZoneInformation( &m_tz );
	GetLocalTime();
}

CWinTime::~CWinTime()
{_STTEX();

}

LPCTSTR CWinTime::GetAbrMonthName(long m)
{_STTEX();
	if ( m < 0 || m > 12 ) return s_months[ 0 ];
	return s_months[ m ];
}

LPCTSTR CWinTime::GetMonthName(long m)
{_STTEX();
	if ( m < 0 || m > 12 ) return s_fmonths[ 0 ];
	return s_fmonths[ m ];
}

LPCTSTR CWinTime::GetAbrDayName(long d)
{_STTEX();
	if ( d < 0 || d > 6 ) return s_days[ 7 ];
	return s_days[ d ];
}

LPCTSTR CWinTime::GetDayName(long d)
{_STTEX();
	if ( d < 0 || d > 6 ) return s_fdays[ 7 ];
	return s_fdays[ d ];
}

// %h = hour 12 hour fixed 2 digits
// %H = hour 12 hour
// %g = hour 24 hour fixed 2 digits
// %G = hour 24 hour
// %m = minute fixed 2 digits
// %M = minute
// %s = second fixed 2 digits
// %S = second
// %l = milli seconds fixed 3 digits
// %L = milli seconds
// %a = am/pm
// %A = AM/PM
// %c = Month [01-12] fixed 2 digits
// %C = Month [1-12]
// %d = Day [01-31] fixed 2 digits
// %D = Day [1-31]
// %i = Day of the week [0-6]
// %I = Day of the week [1-7]
// %y = 2 digit year
// %Y = 4 digit year
// %a = am/pm
// %A = AM/PM
// %w = Abbreviated day of week [Sun,Mon,Tue,Wed,Thu,Fri,Sat]
// %W = Day of week [Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday]
// %b = Abbreviated Month [Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec]
// %B = Month [January,February,March,April,May,June,July,August,September,October,November,December]
BOOL CWinTime::GetString(LPSTR str, LPCTSTR tmpl)
{_STTEX();
	DWORD i = 0, x = 0;

	// Process the template
	while ( tmpl[ x ] != 0 )
	{
		// If not the escape character
		if ( tmpl[ x ] != '%' ) str[ i++ ] = tmpl[ x ];

		// Replace escape sequence
		else switch( tmpl[ ++x ] )
		{
			case 'h' : 
				wsprintf( &str[ i ], "%0.2lu", (DWORD)Get12Hour() );
				i += strlen( &str[ i ] );
				break;

			case 'H' : 
				wsprintf( &str[ i ], "%lu", (DWORD)Get12Hour() );
				i += strlen( &str[ i ] );
				break;

			case 'g' : 
				wsprintf( &str[ i ], "%0.2lu", (DWORD)m_st.wHour );
				i += strlen( &str[ i ] );
				break;

			case 'G' : 
				wsprintf( &str[ i ], "%lu", (DWORD)m_st.wHour );
				i += strlen( &str[ i ] );
				break;

			case 'm' : 
				wsprintf( &str[ i ], "%0.2lu", (DWORD)m_st.wMinute );
				i += strlen( &str[ i ] );
				break;

			case 'M' : 
				wsprintf( &str[ i ], "%lu", (DWORD)m_st.wMinute );
				i += strlen( &str[ i ] );
				break;

			case 's' : 
				wsprintf( &str[ i ], "%0.2lu", (DWORD)m_st.wSecond );
				i += strlen( &str[ i ] );
				break;

			case 'S' : 
				wsprintf( &str[ i ], "%lu", (DWORD)m_st.wSecond );
				i += strlen( &str[ i ] );
				break;

			case 'l' : 
				wsprintf( &str[ i ], "%0.3lu", (DWORD)m_st.wMilliseconds );
				i += strlen( &str[ i ] );
				break;

			case 'L' : 
				wsprintf( &str[ i ], "%lu", (DWORD)m_st.wMilliseconds );
				i += strlen( &str[ i ] );
				break;

			case 'c' : 
				wsprintf( &str[ i ], "%0.2lu", (DWORD)m_st.wMonth );
				i += strlen( &str[ i ] );
				break;

			case 'C' : 
				wsprintf( &str[ i ], "%lu", (DWORD)m_st.wMonth );
				i += strlen( &str[ i ] );
				break;

			case 'd' : 
				wsprintf( &str[ i ], "%0.2lu", (DWORD)m_st.wDay );
				i += strlen( &str[ i ] );
				break;

			case 'D' : 
				wsprintf( &str[ i ], "%lu", (DWORD)m_st.wDay );
				i += strlen( &str[ i ] );
				break;

			case 'i' : 
				wsprintf( &str[ i ], "%lu", (DWORD)m_st.wDayOfWeek );
				i += strlen( &str[ i ] );
				break;

			case 'I' : 
				wsprintf( &str[ i ], "%lu", (DWORD)( m_st.wDayOfWeek + 1 ) );
				i += strlen( &str[ i ] );
				break;

			case 'y' : 
				wsprintf( &str[ i ], "%0.2lu", (DWORD)( m_st.wYear % 100 ) );
				i += strlen( &str[ i ] );
				break;

			case 'Y' : 
				wsprintf( &str[ i ], "%lu", (DWORD)m_st.wYear );
				i += strlen( &str[ i ] );
				break;

			case 'a' : 
				strcpy( &str[ i ], IsPM() ? "pm" : "am" );
				i += strlen( &str[ i ] );
				break;

			case 'A' : 
				strcpy( &str[ i ], IsPM() ? "PM" : "AM" );
				i += strlen( &str[ i ] );
				break;

			case 'w' : 
				strcpy( &str[ i ], GetAbrDayName( m_st.wDayOfWeek ) );
				i += strlen( &str[ i ] );
				break;

			case 'W' : 
				strcpy( &str[ i ], GetDayName( m_st.wDayOfWeek ) );
				i += strlen( &str[ i ] );
				break;

			case 'b' : 
				strcpy( &str[ i ], GetAbrMonthName( m_st.wMonth ) );
				i += strlen( &str[ i ] );
				break;

			case 'B' : 
				strcpy( &str[ i ], GetMonthName( m_st.wMonth ) );
				i += strlen( &str[ i ] );
				break;

			case 'Z' : case 'z' :
			{
				switch( tmpl[ ++x ] )
				{
					case 's' : case 'S' :
						if ( m_tz.Bias < 0 ) str[ i++ ] = '-';
						else str[ i++ ] = '+';
						str[ i ] = 0;
						break;

					case 'h' :
					{	long lBias = GetTzBias(); if ( lBias < 0 ) lBias = -lBias;
						wsprintf( &str[ i ], "%0.2lu", (DWORD)( lBias / 60 ) );
						i += strlen( &str[ i ] );
					} break;

					case 'H' : 
					{	long lBias = GetTzBias(); if ( lBias < 0 ) lBias = -lBias;
						wsprintf( &str[ i ], "%lu", (DWORD)( lBias / 60 ) );
						i += strlen( &str[ i ] );
					} break;

					case 'm' :
					{	long lBias = GetTzBias(); if ( lBias < 0 ) lBias = -lBias;
						wsprintf( &str[ i ], "%0.2lu", (DWORD)( lBias % 60 ) );
						i += strlen( &str[ i ] );
					} break;

					case 'M' : 
					{	long lBias = GetTzBias(); if ( lBias < 0 ) lBias = -lBias;
						wsprintf( &str[ i ], "%lu", (DWORD)( lBias % 60 ) );
						i += strlen( &str[ i ] );
					} break;

					// Bad escape sequence
					default :
						return FALSE; 
						break;

				} // end switch

			} break;

			default : 
				str[ i++ ] = tmpl[ x ]; str[ i ] = 0;
				break;
		
		}; // end switch

		// Next character
		x++;

	} // end while

	// NULL terminate
	str[ i ] = 0;

	return TRUE;
}

BOOL CWinTime::DrawClock(HDC hDC, LPRECT pRect, LPSYSTEMTIME pst, long shadow, COLORREF rgb, COLORREF bck)
{_STTEX();
	if ( bck != MAXDWORD ) CGrDC::FillSolidRect( hDC, pRect, bck );

	DrawTicks( hDC, pRect, rgb, shadow, TRUE, FALSE );
	DrawHands( hDC, pRect, pst, rgb, shadow );

	return TRUE;
}

#define _SCALE( val ) double( ( (double)( val ) / (double)100 ) )

BOOL CWinTime::DrawTicks(HDC hDC, LPRECT pRect, COLORREF rgb, long shadow, BOOL bHour, BOOL bMinute)
{_STTEX();
	if ( hDC == NULL || pRect == NULL ) return FALSE;

	long x,y,lx,ly;
	GetClockMetrics( pRect, &x, &y, &lx, &ly, FALSE );

	if ( bMinute )
	{
		for ( DWORD i = 0; i < 60; i++ )
		{
			if ( ( i % 5 ) )
			{
				// Get the angle in radians
				double angle = i * ( (double)PI2 / (double)60 );

				// Get the point
				long cx = (long)( cos( angle ) * (double)lx ) + x;
				long cy = (long)( sin( angle ) * (double)ly ) + y;
				long size = (long)( ( ( ( lx < ly ) ? lx : ly ) / 4 ) * _SCALE( 30 ) );
				if ( size < 1 ) size = 1;

				COLORREF rgbshadow = CGrDC::ScaleColor( rgb, -200 );

				RECT rect;
				
				SetRect( &rect, shadow + cx - size, shadow + cy - size, shadow + cx + size, shadow + cy + size );
				CGrDC::FillSolidEllipse( hDC, &rect, rgbshadow, rgbshadow );

				SetRect( &rect, cx - size, cy - size, cx + size, cy + size );
				CGrDC::FillSolidEllipse( hDC, &rect, rgb, rgb );
			} // end if
		} // end for
	} // end if

	if ( bHour )
	{ // Draw hour ticks

		for ( DWORD i = 0; i < 12; i++ )
		{
			// Get the angle in radians
			double angle = i * ( (double)PI2 / (double)12 );

			// Get the point
			long cx = (long)( cos( angle ) * (double)lx ) + x;
			long cy = (long)( sin( angle ) * (double)ly ) + y;
			long size = (long)( ( ( ( lx < ly ) ? lx : ly ) / 4 ) * _SCALE( 30 ) );
			if ( size < 1 ) size = 1;

			COLORREF rgbshadow = CGrDC::ScaleColor( rgb, -200 );

			RECT rect;

			SetRect( &rect, shadow + cx - size, shadow + cy - size, shadow + cx + size, shadow + cy + size );
			CGrDC::FillSolidEllipse( hDC, &rect, rgbshadow, rgbshadow );

			SetRect( &rect, cx - size, cy - size, cx + size, cy + size );
			CGrDC::FillSolidEllipse( hDC, &rect, rgb, rgb );

		} // end for

	} // end Draw hour hand

	return TRUE;
}

void CWinTime::GetClockMetrics(LPRECT pRect, long *cx, long *cy, long *rx, long *ry, BOOL bSquare)
{_STTEX();
	// Calculate the center point
	*cx = (long)( pRect->left + ( ( pRect->right - pRect->left ) / 2 ) );
	*cy = (long)( pRect->top + ( ( pRect->bottom - pRect->top ) / 2 ) );

	// Calculate half the box size
	*rx = (long)( ( pRect->right - pRect->left ) / 2 );
	*ry = (long)( ( pRect->bottom - pRect->top ) / 2 );

	if ( bSquare ) { if ( *rx < *ry ) *ry = *rx; else *rx = *ry; }
}

BOOL CWinTime::DrawHands(HDC hDC, LPRECT pRect, LPSYSTEMTIME pst, COLORREF rgb, long shadow)
{_STTEX();
	if ( hDC == NULL || pRect == NULL ) return FALSE;

	long x,y,lx,ly;
	GetClockMetrics( pRect, &x, &y, &lx, &ly, FALSE );
	
	// Are they forcing a time?
	SYSTEMTIME time;
	if ( pst != NULL ) memcpy( &time, pst, sizeof( SYSTEMTIME ) );
	else ::GetLocalTime( &time );

	// Get the angle in radians
	// Get hour 1 - 12
	long hour = ( ( time.wHour ) % 12 );
	hour *= 60;
	hour += time.wMinute;
	hour = ( hour + 540 ) % 720;
	double hangle = hour * ( (double)PI2 / (double)720 );

	long	min = time.wMinute;
	min *= 60;
	min += time.wSecond;
	min = ( min + 2700 ) % 3600;
	double mangle =	 min * ( (double)PI2 / (double)3600 );

	// Get the angle in radians
	double sangle = ( ( time.wSecond + 45 ) % 60 ) * ( (double)PI2 / (double)60 );

	// Do we want a shadow
	if ( true )
	{
		long ysoff = shadow;
		long xsoff = shadow;

		COLORREF rgbshadow = CGrDC::ScaleColor( rgb, -200 );
		// Draw the hands
		DrawHand( hDC, rgbshadow, rgbshadow, rgbshadow, rgbshadow, 4, 3, lx, ly, x + xsoff, y + ysoff, hangle, _SCALE( 40 ), 4 );
		DrawHand( hDC, rgbshadow, rgbshadow, rgbshadow, rgbshadow, 3, 2, lx, ly, x + xsoff, y + ysoff, mangle, _SCALE( 80 ), 4 );
		DrawHand( hDC, rgbshadow, rgbshadow, rgbshadow, rgbshadow, 2, 1, lx, ly, x + xsoff, y + ysoff, sangle, _SCALE( 80 ), 4 );

	} // end if

	// Draw the hands
	DrawHand( hDC, rgb, rgb, rgb, rgb, 4, 3, lx, ly, x, y, hangle, _SCALE( 40 ), 4 );
	DrawHand( hDC, rgb, rgb, rgb, rgb, 3, 2, lx, ly, x, y, mangle, _SCALE( 80 ), 4 );
	DrawHand( hDC, rgb, rgb, rgb, rgb, 2, 1, lx, ly, x, y, sangle, _SCALE( 80 ), 4 );

	return TRUE;
}

void CWinTime::DrawHand(	HDC hDC, COLORREF col, COLORREF ol, 
							COLORREF fc, COLORREF bc, 
							long width, long mwidth, 
							long scx, long scy, 
							long ox, long oy, 
							double angle, double len, 
							DWORD gap )
{_STTEX();
	long cx = (long)( cos( angle ) * (double)scx * len ) + ox;
	long cy = (long)( sin( angle ) * (double)scy * len ) + oy;
	long sx = ox;
	long sy = oy;
	long lw = (long)( ( ( scx < scy ) ? scx : scy ) * _SCALE( width ) );
	if ( lw < mwidth ) lw = mwidth;

	{
		sx = (long)( cos( angle ) * ( (double)scx * _SCALE( gap ) ) + ox );
		sy = (long)( sin( angle ) * ( (double)scy * _SCALE( gap ) ) + oy );
	} // end if

	CGrDC::Line( hDC, col, lw, sx, sy, cx, cy );

}

BOOL CWinTime::DrawCalender(HDC hDC, LPRECT pRect, LPSYSTEMTIME pst, COLORREF rgb, COLORREF bck)
{_STTEX();
	if ( hDC == NULL || pRect == NULL ) return FALSE;

	RECT draw;
	CopyRect( &draw, pRect );

	if ( bck != MAXDWORD ) CGrDC::FillSolidRect( hDC, &draw, bck );

	// Ensure no month overflow
	if ( pst->wMonth > 12 ) pst->wMonth = 12;

	// Make room for header
	long hy = ( ( draw.bottom - draw.top ) / 5 ) / 2;
	draw.top += hy;

	long first = ( pst->wDayOfWeek + 7 ) - ( ( pst->wDay - 1 ) % 7 );
	if ( first >= 7 ) first -= 7;
	long col = pst->wDayOfWeek;
	long row = ( first + ( pst->wDay - 1 ) ) / 7;
	long rows = ( first + g_mdays[ pst->wMonth ] + 6 ) / 7;

	long cw = ( draw.right - draw.left ) / 7;
	long cwo = ( ( draw.right - draw.left ) % 7 ) / 2;
	long ch = ( draw.bottom - draw.top ) / rows;
	long cho = ( ( draw.bottom - draw.top ) % rows ) / 2;

	long dw = cw / 3;
	long dh = ch / 3;


	HPEN hPen = CreatePen( PS_SOLID, 1, rgb );
	HPEN hOldPen = (HPEN)SelectObject( hDC, hPen );
	HBRUSH hBrush = CreateSolidBrush( rgb );
	HBRUSH hOldBrush = (HBRUSH)SelectObject( hDC, hBrush );

	// Draw the header
	Rectangle( hDC, draw.left + cwo, draw.top - hy, 
				draw.left + cwo + ( cw * 7 ) + 1, draw.top + cho );

	long y;
	for ( y = 0; y <= rows; y++ )
	{
		long cy = draw.top + cho + ( ch * y );

		MoveToEx( hDC, draw.left + cwo, cy, NULL );
		LineTo( hDC, draw.right - cwo, cy );
	} // end for

	for ( long x = 0; x <= 7; x++ )
	{
		long cx = draw.left + cwo + ( cw * x );
		
		MoveToEx( hDC, cx, draw.top + cho, NULL );
		LineTo( hDC, cx, draw.bottom - cho );

	} // end for

	// Draw day ticks in corner
	DWORD day = 0;
	for ( y = 0; y < rows; y++ )
	{
		long cy = draw.top + cho + ( ch * y );
		for ( long x = 0; x < 7; x++ )
		{
			if ( ( y || x >= first ) && day < g_mdays[ pst->wMonth ] )
			{	day++;
				long cx = draw.left + cwo + ( cw * x );
				Rectangle( hDC, cx, cy, cx + dw, cy + dh );
			} // end if
		} // end for
	} // end for

	// Draw current day
	long cx = draw.left + cwo + ( cw * col ); 
	long cy = draw.top + cho + ( ch * row );
	Rectangle( hDC, cx, cy, cx + cw, cy + ch );

	// Release drawing objects
	SelectObject( hDC, hOldPen );
	DeleteObject( hPen );
	SelectObject( hDC, hOldBrush );
	DeleteObject( hBrush );

	return TRUE;
}

DWORD CWinTime::DivideSeconds(DWORD totalseconds, LPDWORD days, LPDWORD hours, LPDWORD mins, LPDWORD secs)
{_STTEX();
	DWORD s = 0, m = 0, h = 0, d = 0;

	// Get number of seconds
	s = totalseconds % 60; totalseconds -= s;

	// Get minutes
	m = ( totalseconds / 60 ) % 60; totalseconds -= m;

	// Get hours
	h = ( totalseconds / ( 60 * 60 ) ) % 24; totalseconds -= h;

	// Calc days
	d = totalseconds / ( 60 * 60 * 24 );

	// Write variables
	if ( days != NULL ) *days = d; else h += d * 60 * 60 * 24;
	if ( hours != NULL ) *hours = h; else m += h * 60 * 60;
	if ( mins != NULL ) *mins = m; else s = m * 60;
	if ( secs != NULL ) *secs = s;

	return totalseconds;
}

DWORD CWinTime::IntegrateSeconds(DWORD days, DWORD hours, DWORD mins, DWORD secs)
{_STTEX();
	// Tally total seconds
	return (	( days * ( 60 * 60 * 24 ) ) + 
				( hours * ( 60 * 60 ) ) +
				( mins * 60 ) +
				secs );
}

BOOL CWinTime::ParseString(LPCTSTR str, LPCTSTR tmpl)
{_STTEX();
	DWORD i = 0, x = 0, g = 0;

	DWORD dwYear = MAXDWORD;
	DWORD dwMonth = MAXDWORD;
	DWORD dwDay = MAXDWORD;
	DWORD dwHour = MAXDWORD;
	DWORD dwMinute = MAXDWORD;
	DWORD dwSecond = MAXDWORD;
	DWORD dwMilliseconds = MAXDWORD;
	DWORD dwDayOfWeek = MAXDWORD;

	BOOL bTzNeg = FALSE;
	LONG lHBias = MAXLONG;
	LONG lMBias = MAXLONG;

	// Process the template
	while ( tmpl[ x ] != 0 )
	{
		// If not the escape character
		if ( tmpl[ x ] != '%' ) 
		{	
			if ( tmpl[ x ] == '*' || tmpl[ x ] == '?' ) i++;
			
			else if ( str[ i++ ] != tmpl[ x ] ) return FALSE; 
		}

		// Replace escape sequence
		else switch( tmpl[ ++x ] )
		{
			case 'h' :
				if ( ( g = strtonum( &str[ i ], &dwHour, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwHour < 1 || dwHour > 12 ) return FALSE;
				dwHour %= 13;
				break;

			case 'H' : 
				if ( ( g = strtonum( &str[ i ], &dwHour, 0, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwHour < 1 || dwHour > 12 ) return FALSE;
				dwHour %= 13;
				break;

			case 'g' : 
				if ( ( g = strtonum( &str[ i ], &dwHour, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwHour > 24 ) return FALSE;
				dwHour %= 24;
				break;

			case 'G' : 
				if ( ( g = strtonum( &str[ i ], &dwHour, 0, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwHour > 24 ) return FALSE;
				dwHour %= 24;
				break;

			case 'm' : 
				if ( ( g = strtonum( &str[ i ], &dwMinute, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwMinute > 60 ) return FALSE;
				dwMinute %= 60;
				break;

			case 'M' : 
				if ( ( g = strtonum( &str[ i ], &dwMinute, 0, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwMinute > 60 ) return FALSE;
				dwMinute %= 60;
				break;

			case 's' : 
				if ( ( g = strtonum( &str[ i ], &dwSecond, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwSecond > 60 ) return FALSE;
				dwSecond %= 60;
				break;

			case 'S' : 
				if ( ( g = strtonum( &str[ i ], &dwSecond, 0, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwSecond > 60 ) return FALSE;
				dwSecond %= 60;
				break;

			case 'l' : 
				if ( ( g = strtonum( &str[ i ], &dwMilliseconds, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwMilliseconds > 1000 ) return FALSE;
				dwMilliseconds %= 1000;
				break;

			case 'L' : 
				if ( ( g = strtonum( &str[ i ], &dwMilliseconds, 0, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwMilliseconds > 1000 ) return FALSE;
				dwMilliseconds %= 1000;
				break;

			case 'c' : 
				if ( ( g = strtonum( &str[ i ], &dwMonth, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwMonth < 1 || dwMonth > 12 ) return FALSE;
				dwMonth %= 13;
				break;

			case 'C' : 
				if ( ( g = strtonum( &str[ i ], &dwMonth, 0, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwMonth < 1 || dwMonth > 12 ) return FALSE;
				dwMonth %= 13;
				break;

			case 'd' : 
				if ( ( g = strtonum( &str[ i ], &dwDay, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwDay < 1 || dwDay > 31 ) return FALSE;
				dwDay %= 32;
				break;

			case 'D' : 
				if ( ( g = strtonum( &str[ i ], &dwDay, 0, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwDay < 1 || dwDay > 31 ) return FALSE;
				dwDay %= 32;
				break;

			case 'i' : 
				if ( ( g = strtonum( &str[ i ], &dwDayOfWeek, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwDayOfWeek < 1 || dwDayOfWeek > 7 ) return FALSE;
				dwDayOfWeek %= 8;
				break;

			case 'I' : 
				if ( ( g = strtonum( &str[ i ], &dwDayOfWeek, 0, 2 ) ) == 0 )
					return FALSE; else i += g;
				if ( dwDayOfWeek < 1 || dwDayOfWeek > 7 ) return FALSE;
				dwDayOfWeek %= 8;
				break;

			case 'y' : 
				if ( ( g = strtonum( &str[ i ], &dwYear, 2 ) ) == 0 )
					return FALSE; else i += g;
				break;

			case 'Y' : 
				if ( ( g = strtonum( &str[ i ], &dwYear, 4 ) ) == 0 )
					return FALSE; else i += g;
				break;

			case 'a' : 
			case 'A' : 
				if ( !strnicmp( &str[ i ], "pm", 2 ) )
					dwHour = ( dwHour + 12 ) % 24;
				else if ( strnicmp( &str[ i ], "am", 2 ) )
					return FALSE;
				break;

			case 'w' : 
			{
				g = 0;
				for ( DWORD k = 1; k <= 7; k++ )
					if ( !strnicmp( &str[ k ], GetAbrDayName( k ), strlen( GetAbrDayName( k ) ) ) )
						dwDayOfWeek = k, g = strlen( GetAbrDayName( i ) ), k = 8;
				if ( g == 0 ) return FALSE; i += g;
			} break;

			case 'W' : 
			{
				g = 0;
				for ( DWORD k = 1; k <= 7; k++ )
					if ( !strnicmp( &str[ k ], GetDayName( k ), strlen( GetDayName( k ) ) ) )
						dwDayOfWeek = k, g = strlen( GetDayName( i ) ), k = 8;
				if ( g == 0 ) return FALSE; i += g;
			} break;

			case 'b' : 
			{
				g = 0;
				for ( DWORD k = 1; k <= 12; k++ )
					if ( !strnicmp( &str[ k ], GetAbrMonthName( k ), strlen( GetAbrMonthName( k ) ) ) )
						dwMonth = k, g = strlen( GetAbrMonthName( i ) ), k = 13;
				if ( g == 0 ) return FALSE; i += g;
			} break;

			case 'B' : 
			{
				g = 0;
				for ( DWORD k = 1; k <= 12; k++ )
					if ( !strnicmp( &str[ k ], GetMonthName( k ), strlen( GetMonthName( k ) ) ) )
						dwMonth = k, g = strlen( GetMonthName( i ) ), k = 13;
				if ( g == 0 ) return FALSE; i += g;
			} break;

			case 'Z' : case 'z' :
			{
				switch( tmpl[ ++x ] )
				{
					case 's' : case 'S' :
						if ( str[ i ] == '+' ) i++, bTzNeg = FALSE;
						else if ( str[ i ] == '-' ) i++, bTzNeg = TRUE;						
						break;

					case 'h' :
						if ( ( g = strtosnum( &str[ i ], &lHBias, 2 ) ) == 0 )
							return FALSE; else i += g;
						break;

					case 'H' : 
						if ( ( g = strtosnum( &str[ i ], &lHBias, 0, 2 ) ) == 0 )
							return FALSE; else i += g;
						break;

					case 'm' :
						if ( ( g = strtosnum( &str[ i ], &lMBias, 2 ) ) == 0 )
							return FALSE; else i += g;
						break;

					case 'M' : 
						if ( ( g = strtosnum( &str[ i ], &lMBias, 0, 2 ) ) == 0 )
							return FALSE; else i += g;
						break;

					default :
						if ( str[ i++ ] != tmpl[ x ] ) return FALSE; 
						break;

				} // end switch

			} break;

			default :
				if ( str[ i++ ] != tmpl[ x ] ) return FALSE; 
				break;

	
		}; // end switch

		// Next character
		x++;

	} // end while

	// Calculate bias
	long lBias = MAXLONG;
	if ( lHBias != MAXLONG ) lBias = lHBias * 60;
	if ( lMBias != MAXLONG ) lBias = ( lBias == MAXLONG ) ? lMBias : lBias + lMBias;
	if ( ( bTzNeg && lBias > 0 ) || ( !bTzNeg && lBias < 0 ) ) lBias = -lBias;

	// Set valid parts of the time
	SetTime( dwYear, dwMonth, dwDay, dwHour, dwMinute, dwSecond, dwMilliseconds, dwDayOfWeek, lBias );

	return TRUE;
}


DWORD CWinTime::strtonum(LPCTSTR str, LPDWORD pNum, DWORD len, DWORD max)
{_STTEX();
	// Sanity checks
	if ( str == NULL || pNum == NULL ) return FALSE;
	if ( len > 128 ) len = 0;
	if ( max > 128 ) max = 0;

	if ( len == 0 )
	{
		char *end = NULL;
		if ( max != 0 )
		{	char num[ 256 ];
			strncpy( num, str, max );
			num[ max ] = 0;
			*pNum = strtoul( num, &end, 10 );
			return end - str;
		} // end if
		
		*pNum = strtoul( str, &end, 10 );
		return end - str;

	} // end if

	// Copy substring
	char num[ 256 ];

	// Verify that the number contains numbers
	DWORD i;
	for ( i = 0; i < len; i++ )
	{	if ( str[ i ] >= '0' && str[ i ] <= '9' ) 
			num[ i ] = str[ i ];
		else { *pNum = 0; return 0; }
	} // end for
	num[ i ] = 0;

	// Try to make sense of the number
	*pNum = strtoul( num, NULL, 10 );

	return len;
}

DWORD CWinTime::strtosnum(LPCTSTR str, LPLONG pNum, DWORD len, DWORD max)
{_STTEX();
	// Sanity checks
	if ( str == NULL || pNum == NULL ) return FALSE;
	if ( len > 128 ) len = 0;
	if ( max > 128 ) max = 0;

	if ( len == 0 )
	{
		char *end = NULL;
		if ( max != 0 )
		{	char num[ 256 ];
			strncpy( num, str, max );
			num[ max ] = 0;
			*pNum = strtol( num, &end, 10 );
			return end - str;
		} // end if
		
		*pNum = strtoul( str, &end, 10 );
		return end - str;

	} // end if

	// Check for sign
	BOOL bNeg = FALSE;
	if ( *str == '+' ) str++;
	else if ( *str == '-' ) str++, bNeg = TRUE;

	// Copy substring
	char num[ 256 ];

	// Verify that the number contains numbers
	DWORD i;
	for ( i = 0; i < len; i++ )
	{	if ( str[ i ] >= '0' && str[ i ] <= '9' ) 
			num[ i ] = str[ i ];
		else { *pNum = 0; return 0; }
	} // end for
	num[ i ] = 0;

	// Try to make sense of the number
	*pNum = strtol( num, NULL, 10 );
	if ( bNeg ) *pNum = -(*pNum);

	return len;
}

BOOL CWinTime::GetFileTime( LPFILETIME pFt )
{_STTEX();
	return SystemTimeToFileTime( &m_st, pFt );
}

BOOL CWinTime::SetFileTime(LPFILETIME pFt)
{_STTEX();
	return FileTimeToSystemTime( pFt, &m_st );
}

DWORD CWinTime::GetUnixTime()
{_STTEX();
	FILETIME ft;
	if ( !GetFileTime( &ft ) ) return 0;

	LONGLONG ll;
	ll = ( ( ( (LONGLONG)ft.dwHighDateTime ) << 32 ) + (LONGLONG)ft.dwLowDateTime );

	ll /= (LONGLONG)10000000;
	ll -= (LONGLONG)FTOFF_1970;

	return (DWORD)ll;
}

BOOL CWinTime::SetUnixTime(DWORD dwTime, long lBias )
{_STTEX();
	LONGLONG ll = dwTime;

	ll += (LONGLONG)FTOFF_1970;
	ll *= (LONGLONG)10000000;

	// Set via filetime
	FILETIME ft;
	ft.dwHighDateTime = (DWORD)( ll >> 32 ); 
	ft.dwLowDateTime = (DWORD)( ll & 0xffffffff );

	// Set time zone information
	if ( lBias != -1 ) { m_lBias = lBias; m_tz.Bias = lBias; }

	return SetFileTime( &ft );
}

DWORD CWinTime::GetDosTime()
{_STTEX();
	FILETIME ft;
	if ( !GetFileTime( &ft ) ) return 0;

	LONGLONG ll;
	ll = ( ( ( (LONGLONG)ft.dwHighDateTime ) << 32 ) + (LONGLONG)ft.dwLowDateTime );

	ll /= (LONGLONG)10000000;
	ll -= (LONGLONG)FTOFF_1980;

	return (DWORD)ll;
}

BOOL CWinTime::SetDosTime( DWORD dwTime, long lBias )
{_STTEX();
	LONGLONG ll = dwTime;

	ll += (LONGLONG)FTOFF_1980;
	ll *= (LONGLONG)10000000;

	// Set via filetime
	FILETIME ft;
	ft.dwHighDateTime = (DWORD)( ll >> 32 ); 
	ft.dwLowDateTime = (DWORD)( ll & 0xffffffff );

	// Set time zone information
	if ( lBias != -1 ) { m_lBias = lBias; m_tz.Bias = lBias; }

	return SetFileTime( &ft );
}


DWORD CWinTime::GetNetTime()
{_STTEX();
	FILETIME ft;
	if ( !GetFileTime( &ft ) ) return 0;

	LONGLONG ll;
	ll = ( ( ( (LONGLONG)ft.dwHighDateTime ) << 32 ) + (LONGLONG)ft.dwLowDateTime );

	ll /= (LONGLONG)10000000;
	ll -= (LONGLONG)FTOFF_1900;

	return (DWORD)ll;
}

BOOL CWinTime::SetNetTime(DWORD dwTime, long lBias )
{_STTEX();
	LONGLONG ll = dwTime;

	ll += (LONGLONG)FTOFF_1900;
	ll *= (LONGLONG)10000000;

	// Set via filetime
	FILETIME ft;
	ft.dwHighDateTime = (DWORD)( ll >> 32 ); 
	ft.dwLowDateTime = (DWORD)( ll & 0xffffffff );

	// Set time zone information
	if ( lBias != -1 ) { m_lBias = lBias; m_tz.Bias = lBias; }

	return SetFileTime( &ft );
}

BOOL CWinTime::ToGmt()
{_STTEX();
	// Any time zone information?
	if ( m_lBias == 0 && m_tz.Bias == 0 ) return FALSE;

	// Is there a hard offset for this time?
	if ( m_lBias != 0 ) AddBias( m_st, m_lBias );

	else
	{	if ( !IsDaylightTime( m_st, m_tz ) ) 
			AddBias( m_st, -m_tz.StandardBias );
		else AddBias( m_st, -m_tz.DaylightBias );
	} // end else

	// Clear the time zone information
	m_lBias = 0;
	ZeroMemory( &m_tz, sizeof( m_tz ) );
	
	return TRUE;
}

BOOL CWinTime::ToLocal()
{_STTEX();
	// First to GMT
	ToGmt();

	// Get current timezone information
	::GetTimeZoneInformation( &m_tz );

	// Convert time
	AddBias( m_st, m_tz.Bias );


//	AddBias( m_st, m_tz.DaylightBias );

	if ( !IsDaylightTime( m_st, m_tz ) ) 
		AddBias( m_st, m_tz.StandardBias );
	else AddBias( m_st, m_tz.DaylightBias );


	return TRUE;
}

// jw
void CWinTime::AddBias( SYSTEMTIME& time, long bias )
{_STTEX();
	if (bias < 0)
	{
		time.wHour   += (WORD)( -bias / 60 );
		time.wMinute += (WORD)( -bias % 60 );

		if (time.wMinute > 59)
		{
			time.wMinute -= 60;
			time.wHour++;
		}

		if (time.wHour > 23)
		{
			time.wHour -= 24;
			time.wDay++;
			if (++time.wDayOfWeek == 7)
				time.wDayOfWeek = 0;

			WORD daysInMonth = GetDaysInMonth(time.wMonth, time.wYear);
			if (time.wDay > daysInMonth)
			{
				time.wDay = 1;
				if (++time.wMonth > 12)
				{
					time.wMonth = 1;
					time.wYear++;
				}
			}
		}
	}
	else if (bias > 0)
	{
		int hour		= (int)time.wHour - (bias / 60);
		int minute		= (int)time.wMinute - (bias % 60);

		if (minute < 0)
		{
			minute += 60;
			hour--;
		}

		if (hour < 0)
		{
			hour += 24;
			if (--time.wDayOfWeek > 6)
				time.wDayOfWeek = 6;

			if (--time.wDay == 0)
			{
				if (time.wMonth == 1)
				{
					time.wMonth = 12;
					time.wYear--;
					time.wDay = 31;
				}
				else
					time.wDay = GetDaysInMonth(--time.wMonth, time.wYear);
			}

		}

		time.wHour = (WORD) hour;
		time.wMinute = (WORD) minute;
	}
}

// jw
// Get day to nth "day of week" of month (i.e first Sunday in October)
WORD CWinTime::GetNthDayOfWeek(
	WORD year,			// the current year
	WORD month,			// the current month
	WORD day,			// the current day
	WORD dayOfWeek,		// the current dayOfWeek
	WORD nthWeek,		// The nth week after that day (i.e. 1 = first, 2 = second, 5 = last)
	WORD nthDayOfWeek)	// the nth day (i.e. 1 = Sunday, 6 = Saturday)
{_STTEX();
	return GetNextDayOfWeek(year, month, day, dayOfWeek, 1, nthWeek, nthDayOfWeek);
}

// jw
// Get day for nth "day of week" on or after a day (i.e first Sunday after October 15)
WORD CWinTime::GetNextDayOfWeek(
	WORD year,			// the current year
	WORD month,			// the current month
	WORD day,			// the current day
	WORD dayOfWeek,		// the current dayOfWeek
	WORD dayOnOrAfter,	// The day (i.e. 15)
	WORD nthWeek,		// The nth week after that day (i.e. 1 = first, 2 = second, 5 = last)
	WORD nthDayOfWeek)	// the nth day (i.e. 1 = Sunday, 6 = Saturday)
{_STTEX();
	WORD daysInMonth = GetDaysInMonth(month, year);
	if (day < 1 || day > daysInMonth)
		return nthWeek != 5 ? dayOnOrAfter : daysInMonth;

	if (dayOfWeek > 6)
		dayOfWeek = 6;

	if (nthDayOfWeek < 1)
		nthDayOfWeek = 1;
	else if (nthDayOfWeek > 7)
		nthDayOfWeek = 7;

	if (dayOnOrAfter != day)
		dayOfWeek = (((dayOnOrAfter + 35) - day) + dayOfWeek) % 7;

	// dayOfWeek is now the day of the week of dayOnOrAfter

	int adjust = nthDayOfWeek - 1 - dayOfWeek;
	if (adjust < 0)
		day = (WORD)(dayOnOrAfter + 7 + adjust);
	else      
		day = (WORD)(dayOnOrAfter + adjust);

	if (nthWeek)
	{
		day += (nthWeek - 1) * 7;
		if (day > daysInMonth)
			day -= 7;
	}
	return (WORD) day;
}

// jw
bool CWinTime::IsDaylightTime(const SYSTEMTIME &time, const TIME_ZONE_INFORMATION &tz )
{_STTEX();
	if ( !m_tz.Bias ) return false;

	// Month difference
	if ( tz.DaylightDate.wMonth > tz.StandardDate.wMonth )
	{
		// Southern Hemisphere
		if (	time.wMonth < tz.StandardDate.wMonth || 
				time.wMonth > tz.DaylightDate.wMonth )
			return true;
	}

	// Northen Hemisphere
	else if (	time.wMonth > tz.DaylightDate.wMonth &&
				time.wMonth < tz.StandardDate.wMonth )
		return true;

	if ( time.wMonth == tz.DaylightDate.wMonth)
	{
		WORD day = tz.DaylightDate.wDay;
		if (!tz.DaylightDate.wYear)
			day = GetNthDayOfWeek(time.wYear, time.wMonth, time.wDay, time.wDayOfWeek, tz.DaylightDate.wDay, tz.DaylightDate.wDayOfWeek);

		if (time.wDay > day || 
			(time.wDay == day && 
			 (time.wHour > tz.DaylightDate.wHour || 
			 (time.wHour == tz.DaylightDate.wHour && time.wMinute >= tz.DaylightDate.wMinute))))
			 return true;
	}

	else if (time.wMonth == tz.StandardDate.wMonth)
	{
		WORD day = tz.StandardDate.wDay;
		if (!tz.StandardDate.wYear)
			day = GetNthDayOfWeek(time.wYear, time.wMonth, time.wDay, time.wDayOfWeek, tz.StandardDate.wDay, tz.StandardDate.wDayOfWeek);

		if (time.wDay < day ||
			(time.wDay == day &&
			 (time.wHour < tz.StandardDate.wHour ||
			 (time.wHour == tz.StandardDate.wHour && time.wMinute < tz.StandardDate.wMinute))))
			 return true;
	}
	return false;

}

long CWinTime::CalcTzBias()
{_STTEX();
	TIME_ZONE_INFORMATION tz;

	// Get current timezone information
	::GetTimeZoneInformation( &tz );

	return tz.Bias;
}
