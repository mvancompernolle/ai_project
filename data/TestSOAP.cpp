// TestSOAP.cpp: implementation of the TestSOAP class.
//
// this is a simple server that allows testing of Simple SOAP 
//
// $Id: TestSOAP.cpp,v 1.1 2005/03/22 22:30:30 gjpc Exp $
//
//  $Log: TestSOAP.cpp,v $
//  Revision 1.1  2005/03/22 22:30:30  gjpc
//  This is the intial check in of the Simple SOAP library.
//
//  The code compiles and executes under MSVC .NET and GNU 3.3
//
//  It has been run under Debian, SUSE, CYGWIN and WinXP
//

#include <string.h>
#include <iostream>

#include "SOAPDispatcher.h"
#include "SOAPMethod.h"
#include "Base64Encoder.h"

#if !defined(SOAPENCODER_H)
    #include "SOAPEncoder.h"
#endif // SOAPENCODER_H
#if !defined(SOAPELEMENT_H)
    #include "SOAPElement.h"
#endif // !defined(SOAPELEMENT_H)
#ifndef _SSTREAM_
    #include <sstream>
#endif // _SSTREAM_
#ifndef __TestSOAP__
	#include "TestSOAP.h"
#endif // __TestSOAP__

// TestSOAP::MyData
// here we declare the structures for the RPC's
// Each RPC has an entry describing it the MyData structure
// We must declare the structures before the TestSOAP ctor to
// avoid any forward referecning problems when building the tables
struct TestSOAP::MyData
{
////////////////////////// Addition ////////////////////////////
	class Addition : public SOAPMethod
	{
	public:
		Addition() : returnValue( 0 ) {}
        virtual std::string methodName() { return "Addition"; }
        virtual bool encode( SOAPEncoder& soapEncoder );
        virtual bool execute( SOAPElement& theCall );

    private:
        double returnValue;

	} m_Addition;

////////////////////////// Subtraction ////////////////////////////
	class Subtraction : public SOAPMethod
	{
	public:
		Subtraction() : returnValue( 0 ) {}
        virtual std::string methodName() { return "Subtraction"; }
        virtual bool encode( SOAPEncoder& soapEncoder );
        virtual bool execute( SOAPElement& theCall );

    private:
        double returnValue;

	} m_Subtraction;

////////////////////////// Base64Test  ////////////////////////////
	class Base64Test : public SOAPMethod
	{
	public:
		Base64Test() : returnValue( 0 ) { myMethod = NULL; }
        virtual std::string methodName() { return "Base64Test"; }
        virtual bool encode( SOAPEncoder& soapEncoder );
        virtual bool execute( SOAPElement& theCall );

    private:
        unsigned char  *returnValue;
		unsigned long	returnLength;
		SOAPElement	   *myMethod;

	} m_Base64Test;


};


////////////////////////////// TestSOAP object ////////////////////////////////
// TestSOAP
// This single object is used by both clients and servers to perform SOAP RPc's
// Add any Method above to this table so that the Dispatcher can find it
TestSOAP::TestSOAP() :
    m_pData( new MyData )
{
	insertMethod( &(m_pData->m_Addition			) );
	insertMethod( &(m_pData->m_Subtraction		) );
	insertMethod( &(m_pData->m_Base64Test		) );
}

TestSOAP::~TestSOAP()
{
	// nothing to do
}

////////////////////////// SOAP RPC work routines //////////////////////////////
// now we implement the encoders and executioners this differs from the book 
// because some helper functions were added to SoapMethod to hadle common faults 

////////////////////////// Addition ////////////////////////////
// the Dispatcher calls this method if the execute succeeds
bool TestSOAP::MyData::Addition::encode( SOAPEncoder& soapEncoder )
{
	soapEncoder.encodeArg( returnValue );
	return true;
}

// When an Addition RPC arrives the method is fed into this executioner
bool TestSOAP::MyData::Addition::execute( SOAPElement& theMethod )
{
	if ( ImustUnderstand( theMethod ) )
		return false;

    if ( theMethod.accessorName() == methodName() )
    {
        if ( theMethod.numElements() == 2 )
        {
			// ok we have a leagal call get the args and perform the addition
			double a = atof( theMethod.elementAt( 0 ).value().c_str() );
			double b = atof( theMethod.elementAt( 1 ).value().c_str() );
			returnValue = a + b;
			return true;
        }
        else
			wrongNumberArgumentsFault( theMethod, "2" );
    }
    else
		unkownMethodFault( theMethod );

	return false;
}

////////////////////////// Subtraction ////////////////////////////
// the Dispatcher calls this method if the execute succeeds
bool TestSOAP::MyData::Subtraction::encode( SOAPEncoder& soapEncoder )
{
	soapEncoder.encodeArg( returnValue );
	return true;
}

// When a Subtraction RPC arrives the method is fed into this executioner
bool TestSOAP::MyData::Subtraction::execute( SOAPElement& theMethod )
{
	if ( ImustUnderstand( theMethod ) )
		return false;

    if ( theMethod.accessorName() == methodName() )
    {
        if ( theMethod.numElements() == 2 )
        {
			double a = atof( theMethod.elementAt( 0 ).value().c_str() );
			double b = atof( theMethod.elementAt( 1 ).value().c_str() );
			returnValue = a - b;
			return true;
        }
        else
			wrongNumberArgumentsFault( theMethod, "2" );
    }
    else
		unkownMethodFault( theMethod );

	return false;
}


////////////////////////// Base64Test ////////////////////////////
// the Dispatcher calls this method if the execute succeeds
bool TestSOAP::MyData::Base64Test::encode( SOAPEncoder& soapEncoder )
{
	soapEncoder.encodeBase64( myMethod->elementAt( 0 ).accessorName().c_str(), returnValue, returnLength );
	return true;
}

// When a Subtraction RPC arrives the method is fed into this executioner
bool TestSOAP::MyData::Base64Test::execute( SOAPElement& theMethod )
{
	if ( ImustUnderstand( theMethod ) )
		return false;

    if ( theMethod.accessorName() == methodName() )
    {
        if ( theMethod.numElements() == 1 )
        {
			myMethod = &theMethod;
			returnValue = Base64Encoder().decodeValue( theMethod.elementAt( 0 ).value().c_str(), returnLength );
			return true;
        }
        else
			wrongNumberArgumentsFault( theMethod, "2" );
    }
    else
		unkownMethodFault( theMethod );

	return false;
}


