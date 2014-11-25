/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software
*/
// SOAPFault.cpp: implementation of the SOAPFault class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER 
// Microsoft only extension to the compiler
// Turns off noise about debug symbols being "too long".
    #pragma warning( disable : 4786 )
#endif // _MSC_VER 

#include "SOAPFault.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SOAPFault::SOAPFault()
{

}

SOAPFault::~SOAPFault()
{

}

void SOAPFault::setFaultCode( FaultCode faultCode )
{
    m_faultCode = faultCode;
}

void SOAPFault::setSpecificFault( const std::string& szSpecificFault, FaultCode faultCode /*= Client*/ )
{
    m_szSpecificFault = szSpecificFault;
    m_faultCode = faultCode;
}

std::string SOAPFault::getFaultCode()
{
    std::string retval;
    switch ( m_faultCode )
    {
    case Client:
        retval = "Client";
        break;
    case Server:
        retval = "Server";
        break;
    case MustUnderstand:
        retval = "MustUnderstand";
        break;
    case VersionMismatch:
        retval = "VersionMismatch";
        break;
    }

    if ( m_szSpecificFault.length() > 0 )
    {
        retval += "." + m_szSpecificFault;
    }
    return retval;
}

std::string& SOAPFault::faultString()
{
    return m_szFaultString;
}

std::string& SOAPFault::faultActor()
{
    return m_szFaultActor;
}

std::string& SOAPFault::detail()
{
    return m_szDetail;
}

