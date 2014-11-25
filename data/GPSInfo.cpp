// GPSInfo.cpp: implementation of the GPSInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GPSInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GPSInfo::GPSInfo():
m_latitude(0),
m_longitude(0),
m_altitude(0),
m_nSentences(0),
m_signalQuality(0),
m_satelitesInUse(0)
{
}

GPSInfo::~GPSInfo()
{}
