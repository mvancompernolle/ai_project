/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software
*/
// SOAPObject.cpp: implementation of the SOAPObject class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning( disable: 4786 )
#include "SOAPObject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SOAPObject::SOAPObject()
{

}

SOAPObject::~SOAPObject()
{

}

SOAPObject::MethodList& SOAPObject::getMethodList()
{
    return m_methodList;
}


void SOAPObject::insertMethod( SOAPMethod* pMethod )
{
    m_methodList.push_back( pMethod );
}

