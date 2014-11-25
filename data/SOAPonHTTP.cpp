/**
SOAPNetwork Library: Provides for network capabilities with SOAP
(Simple Object Access Protocol).  Relies on two other libraries:
    TcpServer.lib
    SimpleSOAP.lib

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
// SOAPonHTTP.cpp: implementation of the SOAPonHTTP class.
//
//////////////////////////////////////////////////////////////////////

#pragma warning ( disable: 4786 )
#include "SOAPonHTTP.h"
#ifndef _SSTREAM_
    #include <sstream>
#endif // _SSTREAM_
#include <iostream>
#if !defined(SOAPPARSER_H)
    #include "SOAPParser.h"
#endif // !defined(SOAPPARSER_H)
#if !defined(SOAPELEMENT_H)
    #include "SOAPElement.h"
#endif // !defined(SOAPELEMENT_H)
#if !defined(SOAPENCODER_H)
    #include "SOAPEncoder.h"
#endif // SOAPENCODER_H
#if !defined(SOAPMETHOD_H)
    #include "SOAPMethod.h"
#endif // SOAPMETHOD_H
#include "TcpSocket.h"
#include "IpAddress.h"
#include <iostream>

#ifdef _MSC_VER
#define sleep Sleep
#define snprintf _snprintf
#endif


#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Constants used by the SOAP HTTP implementation.
const std::string g_KszPost = "POST ";
const std::string g_KszManHeader = 
    "\"http://schemas.xmlsoap.org.soap/envelope/\"";
const std::string g_KszContentType = 
    "Content-type: text/xml; charset=utf-8";
const std::string g_KszContentLength = "Content-Length: ";
const std::string g_KszSOAPAction = "SOAPAction: ";
const std::string g_KszHTTPversion = "HTTP/1.0";
const std::string g_KszSOAP = "<SOAP";
const std::string g_KszBody = "Body";

SOAPonHTTP::SOAPonHTTP()
{

}

SOAPonHTTP::~SOAPonHTTP()
{

}

std::string SOAPonHTTP::getSendableMessage( 
    const std::string& szSOAPMessage, 
    const std::string& szClassName,
    const std::string& szMethodName )
{
    std::ostringstream szRetval;
    const long KnLength = szSOAPMessage.length();

	// This simply puts an HTTP header in front of the actual
    // SOAP message.
    szRetval << g_KszPost << "/" << szClassName << " " 
        << g_KszHTTPversion << std::endl;
    szRetval << g_KszContentType << std::endl;
    szRetval << g_KszContentLength << szSOAPMessage.length() 
        << std::endl;
    szRetval << g_KszSOAPAction << "\"" << szClassName << "#" 
        << szMethodName << "\"" << std::endl << std::endl;
    szRetval << szSOAPMessage;
    return szRetval.str();
}

void SOAPonHTTP::setHostAndPort( std::string szHost, 
    long nPort /*= 80*/ )
{
	// Initializes the address and port.
    IpAddress ipAddress( szHost );
    m_socketAddress.setIpAddress( ipAddress );
    m_socketAddress.setPort( nPort );
}

bool SOAPonHTTP::getMethodDetails( const std::string& KszMessage, 
    std::string& szObjectName, std::string& szMethodName, 
    SOAPElement& theCall )
{
    bool retval = true;
    
    long nPos = KszMessage.find( g_KszPost );

    // Find the first "/" after the first GET.
    nPos = KszMessage.find( "/", nPos );

    // We know where the object name starts.  It ends by the first
    // space.
    long nEndPos = KszMessage.find( " ", nPos );

    ++nPos;
    szObjectName = KszMessage.substr( nPos, nEndPos - nPos );

    std::string szSOAPMessage;

	// Find the start of the SOAP message.
    nPos = KszMessage.find( g_KszSOAP );
    if ( nPos >= 0 )
    {
        szSOAPMessage = 
            KszMessage.substr( nPos, KszMessage.length() - nPos );
    }

	// Parse the message now that we have it.
    SOAPParser soapParser;
    retval = soapParser.parseMessage( szSOAPMessage, theCall );
    if ( retval )
    {
		// If we succeeded, get the name of the 
		// method being called.  This of course
		// assumes only one method in the body, and
		// that there are no objects outside of the
		// serialization root.  This method will
		// need an override if this assumption is invalid.
        SOAPElement* pBody = NULL;
        theCall.getElement( g_KszBody, &pBody );
        if ( NULL != pBody )
        {
            SOAPElement& aMethod = pBody->elementAt( 0 );
            szMethodName = aMethod.accessorName();
        }
        if ( szMethodName.length() <= 0 )
        {
            // Create a SOAP fault
            retval = false;
        }
    }    
    else
    {
        szObjectName = 
            SOAPEncoder().encodeFault( *(soapParser.getFault()) );
    }
    return retval;
}


std::string SOAPonHTTP::getSendableResponse( 
    const std::string& szSOAPMessage, bool bIsFault /*= false*/ )
{
    std::string retval;

	// This HTTP header seems to have a definite Microsoft
    // feel to it.  I will need to see how well this interoperates 
    // with a Unix machine.
    
    retval = "HTTP/1.1 ";
    if ( bIsFault )
    {
        retval += "500 Internal Server Error\r\n"; // GJPC seperate out the HTTP tokens
    }
    else
    {
        retval += "200 OK\r\n";	//GJPC seperate the HTTP tokens
    }

    retval += g_KszContentType;
	retval += "\r\nContent-Length: ";
	char buffer[33];
    snprintf( buffer, 33, "%d", (int)szSOAPMessage.length() );
    retval += buffer;
	retval += "\r\n\r\n";
    retval += szSOAPMessage;
    return retval;
}

std::string SOAPonHTTP::send( const std::string& szClassName, 
    SOAPMethod& aMethod )
{
    SOAPEncoder soapEncoder;

	// First off, encode the bugger.
    std::string szCall = soapEncoder.encodeMethodCall( aMethod );
        
	// Now get the message we want to send.
    std::string szSend = getSendableMessage( 
        szCall, szClassName, aMethod.methodName() );
    std::cerr << szSend;

	// gjpc - take the buffer off the stack
	const int KnBuffSize = 10000;
    char *buffer = new char[KnBuffSize + 1];
    memset( buffer, 0, KnBuffSize );
    TcpSocket theSocket;

	// Connect to the server.
    theSocket.connectTo( m_socketAddress );

	// Write out the request.
    theSocket.write( szSend.c_str(), szSend.length() );

	szSend = "";

	// TODO find out what happens with improperly formateed messages 
	// Read back the response.
    int bytes, zcnt = 0, cnt = 0;
	memset( buffer, 0, KnBuffSize );
	do
	{
		if ( (bytes = theSocket.read( buffer, KnBuffSize )) == 0 )
		{
			sleep( 100 );
			 ++zcnt;
		}

		if ( bytes )
			zcnt = 0;

		cnt += bytes;
		buffer[bytes] = 0;
		szSend += buffer;
	}
	while ( ( strstr( buffer, "</SOAP-ENV:Envelope>" ) == NULL ) && ( zcnt < 20 ) );

	// Close the connection to the server.
    theSocket.close();
	delete buffer;

	// Return the result of the call.
    return szSend;
}
