// SOAPAttribute.cpp: implementation of the SOAPAttribute class.
//
//////////////////////////////////////////////////////////////////////

#include "SOAPAttribute.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SOAPAttribute::SOAPAttribute()
{

}

SOAPAttribute::~SOAPAttribute()
{

}

std::string& SOAPAttribute::namespaceName()
{
    return m_szNamespaceName;
}

std::string& SOAPAttribute::accessor()
{
    return m_szAccessor;
}

std::string& SOAPAttribute::value()
{
    return m_szValue;
}

