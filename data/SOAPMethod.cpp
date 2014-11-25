/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software
*/
// SOAPMethod.cpp: implementation of the SOAPMethod class.
//
//	$Log: SOAPMethod.cpp,v $
//	Revision 1.1  2005/03/22 22:22:14  gjpc
//	This is the intial check in of the Simple SOAP library.
//	
//	The code compiles and executes under MSVC .NET and GNU 3.3
//	
//	It has been run under Debian, SUSE, CYGWIN and WinXP
//	
//	Revision 1.3  2004/04/23 16:59:26  gjpc
//	expanded the Simple SOAP package to allow RPC's within RPC's
//	
//
//////////////////////////////////////////////////////////////////////

#pragma warning ( disable: 4786 )
#include "SOAPMethod.h"
#include <assert.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SOAPMethod::SOAPMethod() :
    m_bSucceeded( true )
{

}

SOAPMethod::~SOAPMethod()
{

}


bool SOAPMethod::mustIUnderstand( SOAPElement& theCall )
{
    SOAPElement* pHeader = NULL;
    theCall.getElement( "Header", &pHeader );
    bool retval = false;
    if ( pHeader != NULL )
    {
        // check if anybody has mustUnderstand set to true.
        long nNumElements = pHeader->numElements();
        long nNumAttributes = 0;
        SOAPAttribute soapAttribute;
        for ( long i = 0; i < nNumElements; ++i )
        {
            SOAPElement& aHeaderElement = pHeader->elementAt( i );
            if ( aHeaderElement.getAttribute( "mustUnderstand", 
                soapAttribute ) )
            {
                retval = soapAttribute.value() == std::string("1");
            }
        }
    }

    if ( retval )
	{
		std::ostringstream szStream;

        setFailed();
        getFault()->setFaultCode( SOAPFault::MustUnderstand );
        szStream << "This object does not understand any mustUnderstand header entries." << std::ends;
        getFault()->faultString() = szStream.str();
	}
    return retval;
}

bool SOAPMethod::extractMethod( SOAPElement &theCall, SOAPElement &theMethod )
{
    if ( mustIUnderstand( theCall ) )
    {
        setFailed();
        getFault()->setFaultCode( SOAPFault::MustUnderstand );
		return false;
    }
    // Grab the body and get the first two elements.
    SOAPElement* pBody = NULL;
    theCall.getElement( "Body", &pBody );
    if ( NULL == pBody ) 
	{
		setFailed();
		getFault()->faultString() = "NoBody";
		getFault()->detail() = "The SOAP message is missing a Body Element";
		return false;
	}

	if ( pBody->numElements() == 1 ) 
	{
		theMethod = pBody->elementAt( 0 );
		return true;
	}

	setFailed();
	getFault()->faultString() = "NoMethod";
	getFault()->detail() = "The SOAP message is missing a Method Element";
	return false;

}

bool SOAPMethod::execute( SOAPElement& argList )
{
    // If this assert fires, you need to implement the method.
    // We include a default implementation because
    // client-sdie SOAP Methods should never call execute.
    assert( false );
    return false;
}

bool SOAPMethod::succeeded()
{
    return m_bSucceeded;
}

void SOAPMethod::setFailed()
{
    m_pFault = std::auto_ptr<SOAPFault>( new SOAPFault );
    m_bSucceeded = false;
}

SOAPFault* SOAPMethod::getFault()
{
    // Don't create it if we didn't need it.
    return m_pFault.get();
}

//  here is a helper function to remove some repetitive coding from the executioners
void SOAPMethod::unkownMethodFault( SOAPElement aElement )
{
	std::ostringstream szStream;

	setFailed();
	getFault()->faultString() = "Invalid method name.";
	getFault()->setSpecificFault( "MethodName" );
	szStream << "The fault occurred because the requsted method is unkown "
		<< aElement.accessorName() << "." << std::ends;
	getFault()->detail() = szStream.str();
}

//  here is a helper function to remove some repetitive coding from the executioners
void SOAPMethod::wrongNumberArgumentsFault( SOAPElement aElement, const char *num )
{
	std::ostringstream szStream;

	setFailed();
	getFault()->setSpecificFault( "InvalidArgumentCount" );
	getFault()->faultString() = "Invalid number of arugments.";
	szStream << "The fault occurred because the " 
		<< aElement.accessorName() << " "
		<< "method expected " << num << " argument(s) but received " 
		<< aElement.numElements()
		<< " arguments.";
	getFault()->detail() = szStream.str();
}

//  here is a helper function to remove some repetitive coding from the executioners
bool SOAPMethod::ImustUnderstand( SOAPElement aElement )
{
	if ( mustIUnderstand( aElement ) )
	{
		std::ostringstream szStream;
		setFailed();
		getFault()->faultString() = "not understanding";
		getFault()->setSpecificFault( "MustUnderstand" );
		szStream << "The fault occurred because the requsted method is unkown"
			<< aElement.accessorName() << "." << std::ends;
		getFault()->detail() = szStream.str();
		return true;
	}
	return false;
}

std::string SOAPMethod::methodNameAttributes( )
{
    return std::string("SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" ");
}
