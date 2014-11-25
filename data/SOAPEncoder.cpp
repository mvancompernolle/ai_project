/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software

	$Log: SOAPEncoder.cpp,v $
	Revision 1.1  2005/03/22 22:22:14  gjpc
	This is the intial check in of the Simple SOAP library.
	
	The code compiles and executes under MSVC .NET and GNU 3.3
	
	It has been run under Debian, SUSE, CYGWIN and WinXP
	
	Revision 1.3  2004/04/23 16:59:26  gjpc
	expanded the Simple SOAP package to allow RPC's within RPC's
	

*/
// SOAPEncoder.cpp: implementation of the SOAPEncoder class.
//
//////////////////////////////////////////////////////////////////////

const bool g_KbDebugging = true;
#pragma warning ( disable: 4786 )
#include "SOAPEncoder.h"

#if !defined(BASE64ENCODER_H)
    #include "Base64Encoder.h"
#endif // !defined(BASE64ENCODER_H)

#ifndef _SSTREAM_
    #include <sstream>
#endif // _SSTREAM_

#if !defined(SOAPMETHOD_H)
    #include "SOAPMethod.h"
#endif // SOAPMETHOD_H

#if !defined(SOAPFAULT_H)
    #include "SOAPFault.h"
#endif // !defined(SOAPFAULT_H)

#ifndef _IOSTREAM_
    #include <iostream>
#endif // _IOSTREAM_

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



SOAPEncoder::SOAPEncoder() : 
    m_bIsResponse( false )
{

}

SOAPEncoder::~SOAPEncoder()
{

}

bool SOAPEncoder::isEncodingResponse()
{
    return m_bIsResponse;
}

std::string SOAPEncoder::encodeMethodCall( SOAPMethod& aMethod )
{
    m_bIsResponse = false;
    return encodeMethod( aMethod );
}

std::string SOAPEncoder::encodeMethodResponse( SOAPMethod& aMethod )
{
    m_bIsResponse = true;
    return encodeMethod( aMethod );
}

std::string SOAPEncoder::encodeMethodResonseContents( SOAPMethod& aMethod )
{
    // Set the begin tag for the method element.
	std::ostringstream szStream;
	std::string szMethodName = aMethod.methodName() + "Response";

	if ( !aMethod.isGeneric() )
		szStream << "<" << szMethodName << " " << aMethod.methodNameAttributes() << ">" << std::endl;

    // Encode the method.
    m_encodedValue = "";
    aMethod.encode( *this );
    szStream << m_encodedValue;

	if ( !aMethod.isGeneric() )
		szStream << "</" << szMethodName << ">" << std::endl;

	m_encodedValue = szStream.str();
    return m_encodedValue;

}
std::string SOAPEncoder::encodeMethod( SOAPMethod& aMethod )
{
    std::string szHeaderContents = headerContents();
    std::string szMethodName = aMethod.methodName();
    std::ostringstream szStream;
    bool bHeaderNeeded = szHeaderContents.length() > 0;
    // Initialize the envelope.
    szStream << envelopeOpen();

    if ( isEncodingResponse() )
    {
        szMethodName += "Response";
    }

    if ( bHeaderNeeded )
    {
        szStream << headerOpen() << szHeaderContents << headerClose(); 
    }

    szStream << bodyOpen();

    // Set the begin tag for the method element if we are not in a Genric which has the tags already.
	if ( !aMethod.isGeneric() )
		szStream << "<" << szMethodName << " " << aMethod.methodNameAttributes() << ">" << std::endl;

    m_encodedValue = "";

    // Encode the method.
    aMethod.encode( *this );

    szStream << m_encodedValue;

    // Set the end tag for the method element the generic already has a tag.
	if ( !aMethod.isGeneric() )
		szStream << "</" << szMethodName << ">" << std::endl;

    // Close the body.
    szStream << bodyClose();

    // Close the envelope.
    szStream << envelopeClose();
    m_encodedValue = szStream.str();
    return m_encodedValue;
}

// here is a walk around encoder for a pure XML argument - GJPC
std::string SOAPEncoder::encodeArgument( const std::string& szArg )
{
    std::ostringstream szStream;    

    // Have an encoder for pure XML
    szStream << szArg << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName, const std::string& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
    szStream << "<" << szArgName << " xsi:type=\"xsd:string\">" << value;
       

    // Set the end tag for the argument element.
    szStream << "</" << szArgName << ">" << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName, const int& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
    szStream << "<" << szArgName << " xsi:type=\"xsd:int\">" << value;
       

    // Set the end tag for the argument element.
    szStream << "</" << szArgName << ">" << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName, const __int64& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
#ifndef _MSC_VER
	szStream << "<" << szArgName << " xsi:type=\"xsd:long\">" << value;
#else	// TODO:: see if this is really required
    wchar_t buffer[10];
    memset( buffer, 0, 10 );
    _i64tow( value, buffer, 10 );
    szStream << "<" << szArgName << " xsi:type=\"xsd:long\">" << buffer;
#endif       

    // Set the end tag for the argument element.
    szStream << "</" << szArgName << ">" << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName, const short& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
    szStream << "<" << szArgName << " xsi:type=\"xsd:short\">" << value;
       

    // Set the end tag for the argument element.
    szStream << "</" << szArgName << ">" << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName, const char& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
    szStream << "<" << szArgName << " xsi:type=\"xsd:byte\">" << value;
       

    // Set the end tag for the argument element.
    szStream << "</" << szArgName << ">" << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName, const unsigned short& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
    szStream << "<" << szArgName << " xsi:type=\"xsd:unsignedShort\">" << value;
       

    // Set the end tag for the argument element.
    szStream << "</" << szArgName << ">" << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName, const unsigned __int64& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
#ifndef _MSC_VER
	szStream << "<" << szArgName << " xsi:type=\"xsd:unsignedLong\">" << value;
#else	// TODO:: see if this is really required
    wchar_t buffer[10];
    memset( buffer, 0, 10 );
    _ui64tow( value, buffer, 10 );
    szStream << "<" << szArgName << " xsi:type=\"xsd:unsignedLong\">" << buffer;
#endif       

    // Set the end tag for the argument element.
    szStream << "</" << szArgName << ">" << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName, const unsigned int& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
    szStream << "<" << szArgName << " xsi:type=\"xsd:unsignedInt\">" << value;
       

    // Set the end tag for the argument element.
    szStream << "</" << szArgName << ">" << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName,
    const unsigned char& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
    szStream << "<" << szArgName << " xsi:type=\"xsd:unsignedByte\">" 
        << value;

    // Set the end tag for the argument element.
    szStream << "</" << szArgName << ">" << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName,
    const float& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
    szStream << "<" << szArgName << " xsi:type=\"xsd:float\">" << value;
       

    // Set the end tag for the argument element.
    szStream << "</" << szArgName << ">" << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName, const double& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
    szStream << "<" << szArgName << " xsi:type=\"xsd:double\">" << value;
       

    // Set the end tag for the argument element.
    szStream << "</" << szArgName << ">" << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName, const bool& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
    szStream << "<" << szArgName << " xsi:type=\"xsd:boolean\">" << value ? 1 : 0;
       

    // Set the end tag for the argument element.
    szStream << "</" << szArgName << ">" << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeBase64  ( const std::string& szArgName, void* value, unsigned long ulSizeofValue )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
    szStream << "<" << szArgName << " xsi:type=\"xsd:binary\">" << Base64Encoder().encodeValue( value, ulSizeofValue );
    

    // Set the end tag for the argument element.
    szStream << "</" << szArgName << ">" << std::endl;
    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::envelopeOpen()
{
    std::string retval ( "<SOAP-ENV:Envelope\n"
        " xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\"\n"
        " xmlns:xsd=\"http://www.w3.org/1999/XMLSchema\"\n"
        " xmlns:xsi=\"http://www.w3.org/1999/XMLSchema-instance\"\n"        
        " SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\n" );
   
    return retval;
}

std::string SOAPEncoder::envelopeClose()
{
    std::string retval( "</SOAP-ENV:Envelope>\n" );
    return retval;
}

std::string SOAPEncoder::headerOpen()
{
    std::string retval( "<SOAP-ENV:Header\n" );
    return retval;
}

std::string SOAPEncoder::headerContents()
{
    std::string retval;
    
    return retval;
}

std::string SOAPEncoder::headerClose()
{
    std::string retval( "</SOAP-ENV:Header\n" );
    return retval;
}

std::string SOAPEncoder::bodyOpen()
{
    std::string retval( "<SOAP-ENV:Body>\n" );
   
    return retval;
}

std::string SOAPEncoder::bodyClose()
{
    std::string retval( "</SOAP-ENV:Body>\n" );
    return retval;
}

std::string SOAPEncoder::faultOpen()
{
    std::string retval( "<SOAP-ENV:Fault>\n" );
    return retval;
}

std::string SOAPEncoder::faultClose()
{
    std::string retval( "</SOAP-ENV:Fault>\n" );
    return retval;
}

std::string SOAPEncoder::clientFaultClass()
{
    std::string retval( "Client" );
    return retval;
}

std::string SOAPEncoder::serverFaultClass()
{
    std::string retval( "Server" );
    return retval;
}

std::string SOAPEncoder::versionMismatchFaultClass()
{
    std::string retval( "VersionMismatch" );
    return retval;
}

std::string SOAPEncoder::mustUnderstandFaultClass()
{
    std::string retval( "MustUnderstand" );
    return retval;
}

std::string SOAPEncoder::addBeginTag( const std::string& szValue )
{
    std::ostringstream szStream;

    // Set the begin tag for the element.
    szStream << "<" << szValue << ">" << std::endl;

    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::addEndTag( const std::string& szValue )
{
    std::ostringstream szStream;

    // Set the end tag for the element.
    szStream << "</" << szValue << ">" << std::endl;

    m_encodedValue += szStream.str();
    return szStream.str();
}

void SOAPEncoder::encodeFaultContent( SOAPFault& soapFault, std::ostringstream &szStream )
{
    // Set the begin tag for the Fault.
    szStream << faultOpen();

    // Encode the faultcode
    szStream << faultcodeOpen() << soapFault.getFaultCode()
        << faultcodeClose() << std::endl;

    // Encode the faultstring
    szStream << faultstringOpen() << soapFault.faultString()
        << faultstringClose() << std::endl;

    // Only encode the faultactor if it is filled in.
    if ( soapFault.faultActor().length() > 0 )
    {
        szStream << faultactorOpen() << soapFault.faultActor()
            << faultactorClose() << std::endl;
    }

    // Only encode the detail if it is filled in.
    if ( soapFault.detail().length() > 0 )
    {
        szStream << faultdetailOpen() << soapFault.detail()
            << faultdetailClose() << std::endl;
    }

	// close off the fault
    szStream << faultClose();
}
std::string SOAPEncoder::encodeFault( SOAPFault& soapFault )
{
    std::ostringstream szStream;
    // Initialize the envelope.
    szStream << envelopeOpen();

    szStream << bodyOpen();

    // Encode the fault
	encodeFaultContent( soapFault, szStream );

	// Close the body.
    szStream << bodyClose();

    // Close the envelope.
    szStream << envelopeClose();
    m_encodedValue = szStream.str();
    return m_encodedValue;
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName, LongArray& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
    szStream << "<" << szArgName << " SOAP-ENC:arrayType=\"xsi:int[" << value.size() << "]\">" << std::endl;
       
	for (LongArray::iterator i = value.begin(); i != value.end(); ++i) {
		szStream << "<SOAP-ENC:int>" << *i << "</SOAP-ENC:int>" << std::endl;
	}
	
	szStream << "</" << szArgName << ">" << std::endl;

    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::encodeArgument( const std::string& szArgName, StringArray& value )
{
    std::ostringstream szStream;

    // Set the begin tag for the argument element.
    szStream << "<" << szArgName << " SOAP-ENC:arrayType=\"xsd:string[" << value.size() << "]\">" << std::endl;
       
	for (StringArray::iterator i = value.begin(); i != value.end(); ++i) {
		szStream << "<SOAP-ENC:string>" << *i << "</SOAP-ENC:string>" << std::endl;
	}
	
	szStream << "</" << szArgName << ">" << std::endl;

    m_encodedValue += szStream.str();
    return szStream.str();
}

std::string SOAPEncoder::faultcodeOpen()
{
    return "<SOAP-ENV:faultcode>";
}

std::string SOAPEncoder::faultcodeClose()
{
    return "</SOAP-ENV:faultcode>";
}

std::string SOAPEncoder::faultstringOpen()
{
    return "<SOAP-ENV:faultstring>";
}

std::string SOAPEncoder::faultstringClose()
{
    return "</SOAP-ENV:faultstring>";
}

std::string SOAPEncoder::faultactorOpen()
{
    return "<SOAP-ENV:faultactor>";
}

std::string SOAPEncoder::faultactorClose()
{
    return "</SOAP-ENV:faultactor>";
}

std::string SOAPEncoder::faultdetailOpen()
{
    return "<SOAP-ENV:detail>";
}

std::string SOAPEncoder::faultdetailClose()
{
    return "</SOAP-ENV:detail>";
}

