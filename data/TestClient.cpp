/**
// TestClient.cpp: implementation of the TestClient class.
//
// Copyright Gerard J. Cerchio 2005
//
// this is a simple client that allows testing of the Simple SOAP 
//
// History:
//	
//	20-aug-2003		GJPC		Intial type in
//
//	$Log: TestClient.cpp,v $
//	Revision 1.1  2005/03/22 22:27:19  gjpc
//	This is the intial check in of the Simple SOAP library.
//	
//	The code compiles and executes under MSVC .NET and GNU 3.3
//	
//	It has been run under Debian, SUSE, CYGWIN and WinXP
//	
//	Revision 1.3  2004/04/23 16:58:00  gjpc
//	expanded the Simple SOAP package to allow RPC's within RPC's
//	extended the test client to allow for programable validation
//	
//
//////////////////////////////////////////////////////////////////////
*/

#ifdef _MSC_VER 
// Microsoft only extension to the compiler
// Turns off noise about debug symbols being "too long".
    #pragma warning( disable : 4786 )
#endif // _MSC_VER 

#include "TestClient.h"
#include "SOAPEncoder.h"
#include <iostream>
#include <fstream>

#include "SOAPMethod.h"
#include "SOAPonHTTP.h"    
#include "SOAPParser.h"
#include "SOAPElement.h"


extern char *ServerObject;
const bool g_KbDebugging = true;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
struct TestSOAPClient::SOAPMethodTable
{
	/////////////////////////////////////////////////////////////////
	// Genric 
    class CGenric : public SOAPMethod  
    {
    public:
        CGenric():   m_nPort(0){}

        virtual ~CGenric(){}
        virtual std::string methodName(){ return std::string( theName ); }

        virtual bool encode( SOAPEncoder& soapEncoder ){
			soapEncoder.encodeArgument( *theRPC );
			return true;  
		}

		bool isGeneric() { return true; };

		std::string methodNameAttributes()
		{
			return std::string("");
		}

		std::string Generic( const std::string &rpc )
        {
			theRPC = &rpc;
			theName.clear();
			// first isolate the method name
			for ( unsigned int i = 1; i < rpc.size(); i++ )
				if ( isspace(rpc[i]) || ( rpc[i] == '>' ) )
					break;
				else
					theName += rpc[i];

            SOAPonHTTP soapOnHTTP;
            soapOnHTTP.setHostAndPort( m_szEndpoint, m_nPort );
            return soapOnHTTP.send( ServerObject, *this );
        }
        
        std::string m_szEndpoint, theName;
		const std::string *theRPC;
        long m_nPort;
		bool LogError, LogDiscovery, LogVolume;

    } m_GenericSOAP;

    std::string m_szEndPointIP;
    long m_nPort;

};

TestSOAPClient::TestSOAPClient() :
    m_pMethodTable( new SOAPMethodTable )
{
}

TestSOAPClient::~TestSOAPClient()
{
    
}

std::string  TestSOAPClient::Generic( std::string rpc )
{
    m_pMethodTable->m_GenericSOAP.m_szEndpoint = m_pMethodTable->m_szEndPointIP;
    m_pMethodTable->m_GenericSOAP.m_nPort = m_pMethodTable->m_nPort;

	return  m_pMethodTable->m_GenericSOAP.Generic(rpc);
}

void TestSOAPClient::setEndPoint( const std::string& szEndPoint, long nPort )
{
    m_pMethodTable->m_szEndPointIP = szEndPoint;
    m_pMethodTable->m_nPort = nPort;
}
