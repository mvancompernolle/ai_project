/**
SimpleSOAP Library: Implements a simplified version of SOAP
(Simple Object Access Protocol).  

Copyright (C) 2000  Scott Seely, scott@scottseely.com
modifications by Gerard J. Cerchio gjpc@circlesoft.com

This library is public domain software
*/
// SOAPParser.cpp: implementation of the SOAPParser class.
//
//////////////////////////////////////////////////////////////////////

/*
	History:

	24-Aug-04	G. Cerchio	work around isspace assert with debug lib
	21-Aug-04	G. Cerchio	output the input to the debug window
	28-Mar-06	G. Cerchio	added comment parsing on xml stream
	20-Oct-11	A. Salamon 	Clear any old fault so parseMessage can be called repeatedly
	20-Oct-11	A. Salamon 	check for the length of the remaining input before checking 
					for a comment. Would only cause a problem if there were no 
					"<" in the input string.
*/
#ifdef _MSC_VER 
// Microsoft only extension to the compiler
// Turns off noise about debug symbols being "too long".
#pragma warning( disable : 4786 )
#endif // _MSC_VER 

#ifndef _STACK_
    #include <stack>
#endif // _STACK_

#if !defined(SOAPELEMENT_H)
    #include "SOAPElement.h"
#endif // !defined(SOAPELEMENT_H)
#include <iostream>
#ifndef _SSTREAM_
    #include <sstream>
#endif // _SSTREAM_

#include "SOAPParser.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
// the MSVC debug isspace asserts on UTF8 on windows
#undef isspace
static int isspace( int c )
{
 // space, tab, carriage-return, newline, vertical tab, and form-feed
	if ( c == ' ' )
		return true;
	if ( c == '\n' )
		return true;
	if ( c == '\r' )
		return true;
	if ( c == '\t' )
		return true;
	if ( c == '\f' )
		return true;
	if ( c == 0xb )
		return true;
	
	return false;
}
#endif 

// Constants
const std::string g_KszEnvelope = "Envelope";
const std::string g_KszHeader = "Header";
const std::string g_KszBody = "Body";
const std::string g_KszFault = "Fault";
const std::string g_KszQuoteTypes = "\"'";
SOAPParser::SOAPParser()
{

}

SOAPParser::~SOAPParser()
{

}

bool SOAPParser::parseMessage( const std::string& szMessage, 
    SOAPElement& soapMessage )
{
	long nCurrentPos = 0;
	m_namespaceMap.clear();
	m_pFault.reset(); // Clear any old fault so parseMessage can be called repeatedly
	return parseMessage( szMessage, soapMessage, nCurrentPos );
}


bool SOAPParser::parseMessage( const std::string& szMessage, 
    SOAPElement& soapElement, long& nCurrentPos )
{
    bool retval = true;
    std::string szEndTag;

    // Start looking for the start-tag.
    std::string szCurrentstring;
    const long KnLength = szMessage.length();

    // Keep marching in the string until we hit the first '<'
    for ( ; ( szMessage[nCurrentPos] != '<' ) 
         && ( nCurrentPos < KnLength ); ++nCurrentPos )
    {
        // body intentionally left empty.
    }

	// skip past a comment - GJPC
	if ( ( nCurrentPos < KnLength-3 ) &&
	        ( szMessage[ nCurrentPos + 1 ] == '!' ) &&
		( szMessage[ nCurrentPos + 2 ] == '-' ) &&
		( szMessage[ nCurrentPos + 3 ] == '-' ) )
	{
		for ( ; ( szMessage[nCurrentPos]   != '-' )  && 
				( szMessage[nCurrentPos+1] != '-' ) && 
				( szMessage[nCurrentPos+2] != '>' ) &&
				( nCurrentPos < KnLength+2 ); ++nCurrentPos )
		{
			// body intentionally left empty.
		}
		if ( nCurrentPos < KnLength )
			for ( ; ( szMessage[nCurrentPos] != '<' ) 
				&& ( nCurrentPos < KnLength ); ++nCurrentPos )
			{
				// body intentionally left empty.
			}
	}
    // If we got to the end without finding the
    // end tag (or any begin tag), return false.
    if ( nCurrentPos == KnLength )
    {
        setFailed();
        std::ostringstream szStream;
        getFault()->setSpecificFault( "ImproperlyFormattedMessage" );
        getFault()->faultString() = "The message either has no begin "
            "tag or is missing the end tag.  In either case, "
            "the message was not formatted correctly.";
        return false;
    }

    std::string szBeginTag;
    std::string szNamespace;
    std::string szAccessorName;
    std::string szFullAccessorName;
    bool bIsEmptyTag = false;
    bool bIsEndTag = false;
    for ( ; retval && (nCurrentPos < KnLength); ++nCurrentPos )
    {
        // Initialize all variables.
        bIsEmptyTag = false;
        bIsEndTag = false;
        szBeginTag = "";
        szNamespace = "";
        szAccessorName = "";
        szFullAccessorName = "";
        long nInitialBeginPos = nCurrentPos;

        // Get the information contained by the next pair
        // of "<>".
        retval = extractBeginTag( szBeginTag, szNamespace, 
            szAccessorName, szMessage, nCurrentPos, bIsEmptyTag, 
            bIsEndTag );

        // If we are expecting an end tag, but this isn't
        // one, it must be another child item.  Start
        // parsing at this location and attach a new "tree".
        if ( ( szEndTag.length() > 0 ) && (!bIsEndTag ) )
        {
            // extract the contents of this item independently
            nCurrentPos = nInitialBeginPos;
            SOAPElement* pElement = new SOAPElement;
            soapElement.addElement( pElement );
			
            retval = parseMessage( szMessage, *pElement, nCurrentPos );

			//If the current pos is on the tag, back up one place (for loop will bring it correctly)
			if ( (retval) && (szMessage[nCurrentPos] == '<') && (nCurrentPos) )
            {
				nCurrentPos--;
            }
            // We've now parsed all the data within the tree.
            // Go to the top of the loop again.  If retval is
            // false, the error data should be filled in
            // and we will drop out of the loop.
            continue;
        }

        if ( !retval )
        {
            // Unwind the stack-- we have an error so the rest of the
            // doc doesn't matter.
            setFailed();
            std::ostringstream szStream;
            szStream << "Failed near position " << nCurrentPos << 
                " within the message.";
            getFault()->setSpecificFault( "ImproperlyFormattedMessage" );
            getFault()->faultString() = szStream.str();
            break;
        }

        // If we are at the end of this tag, prepare to close
        // out the current SOAPElement.
        if ( bIsEndTag )
        {
            if ( szEndTag.empty() )
            {
                // We got an end tag when we weren't expecting one.
                setFailed();
                std::ostringstream szStream;
                szStream << "Failed near position " << nCurrentPos << 
                    " within the message." << "Expected end tag: " <<
                    szEndTag << std::ends;
                getFault()->setSpecificFault( "EmptyEndTag" );
                getFault()->faultString() = szStream.str();
                retval = false;
                break;
            }
            if ( szBeginTag != szEndTag )
            {
                // This isn't the end tag we were expecting.
                // Let the user know that the doc isn't formatted
                // correctly.
                setFailed();
                std::ostringstream szStream;
                szStream << "Failed near position " << nCurrentPos <<
                    " within the message." << "Expected end tag: " <<
                    szEndTag;
                getFault()->setSpecificFault( "WrongEndTag" );
                getFault()->faultString() = szStream.str();
                retval = false;
                break;
            }
            
            retval = true;

            // This was the expected end tag.  We are done
            // with parsing for this element.
            break;
        }

        // Pull the namespaces out of the tag.  They
        // look like attributes to the rest of the code.
        retval = extractNamespaces( szBeginTag );

        if ( !retval )
        {
            // Gack, something went wrong.
            setFailed();
            std::ostringstream szStream;
            szStream << "Failed near position " << nCurrentPos <<
                " within the message.";
            getFault()->setSpecificFault( "NamespaceExtractionFailed",
                SOAPFault::Server );
            getFault()->faultString() = szStream.str();
            retval = false;
            break;
        }

        // If the given element was specified using a namespace,
        // tack it on to the start of the namespace name.
        if ( szNamespace.length() > 0 )
        {
            szFullAccessorName = szNamespace + std::string(":");
            soapElement.namespaceName() = szNamespace;
        }
        szFullAccessorName += szAccessorName;
        soapElement.accessorName() = szAccessorName;

        // If this is the envelope, check the version.
        if ( soapElement.accessorName() == "Envelope" )
        {
            bool bFound = false;
            // Check to see if the SOAP namespace was set.
            // If not, we have a VersionMismatch problem.
            for ( XMLNStoURN::iterator it = m_namespaceMap.begin();
                it != m_namespaceMap.end(); ++it )
            {
                if ( "http://schemas.xmlsoap.org/soap/envelope/" == 
                    it->second)
                {
                    bFound = true;
                    break;
                }
            }
            if ( !bFound )
            {
                setFailed();
                getFault()->setFaultCode( SOAPFault::VersionMismatch );
                getFault()->faultString() = "Version mismatch found.";
                retval = false;
                break;
            }
        }

        // set the text for the expected end tag.
        szEndTag = std::string( "/" );
        if ( szNamespace.length() > 0 )
        {
            szEndTag += szNamespace + std::string(":");
        }
        szEndTag += soapElement.accessorName();

        // Pull the attribute information out of the begin tag.
        retval = extractAttributes( soapElement, szBeginTag );
        if ( !retval )
        {
            setFailed();
            std::ostringstream szStream;
            szStream << "Attribute extraction: Failed near position " << 
                nCurrentPos << " within the message." << std::ends;
            getFault()->setSpecificFault( "ImproperlyFormattedMessage" );
            getFault()->faultString() = szStream.str();
            break;
        }

		// if this is an empty tag we just move along GJPC
		if ( bIsEmptyTag )
			break;

        // Take out the value contained between the begin and end tags.
		int tmpPos = nCurrentPos;
		//nCurrentPos = tmpPos;
        retval = extractValue( soapElement, szMessage, nCurrentPos );
        if ( !retval )
        {
            setFailed();
            std::ostringstream szStream;
            szStream << "Value extraction: Failed near position " << 
                nCurrentPos << " within the message." << std::ends;
            getFault()->setSpecificFault( "ImproperlyFormattedMessage" );
            getFault()->faultString() = szStream.str();
            break;
        }
        else 
        {
			//Change on 07/03/2001 by SM
			//If the current pos is on the tag, back up one place (for loop will bring it correctly)
			if ( (szMessage[nCurrentPos] == '<') && (nCurrentPos) )
            {
				nCurrentPos--;
            }
		}
    }
    return retval;
}

bool SOAPParser::extractBeginTag(
    std::string& szBeginTag, 
    std::string& szNamespace, 
    std::string& szAccessorName, 
    std::string szMessage, 
    long &nCurrentPos, 
    bool &bIsEmptyTag, 
    bool &bIsEndTag)
{
    bool retval = true;
    bool bExtractedAccessorName = false;
    bool bIsSpace = false;
    std::string szFullAccessor;
    const long KnLength = szMessage.length();    
    long nBeginEndTagPos = nCurrentPos;
    bIsEmptyTag = false;
    bIsEndTag = false;
    szBeginTag = "";
    szNamespace = "";
    szAccessorName = "";

    for( ; ( nCurrentPos < KnLength ) && ( '>' != szMessage[nCurrentPos] ); ++nCurrentPos )
    {
        // Check if this is an empty tag
        if ( ( '/' == szMessage[nCurrentPos] ) && 
             ( ( nCurrentPos + 1 ) < KnLength ) &&
             ( '>' == szMessage[nCurrentPos + 1] ) )
        {
            bIsEmptyTag = true;
            nCurrentPos += 2;
            break;
        }

        // Check if this is an end tag
        if ( ( '/' == szMessage[nCurrentPos] ) && 
             ( ( nCurrentPos - 1 ) >= 0 ) &&
             ( '<' == szMessage[nCurrentPos - 1] ) )
        {
            bIsEndTag = true;
            long nEndOfTag = szMessage.find( ">", nCurrentPos );
            szBeginTag = szMessage.substr( nCurrentPos, nEndOfTag - nCurrentPos );
            nCurrentPos = nEndOfTag;
            break;
        }

        // Check if we've extracted the accessor name yet.
        if ( bExtractedAccessorName )
        {
            szBeginTag += szMessage[nCurrentPos];
        }
        else
        {
            if ( isspace( szMessage[nCurrentPos] ) )
            {
                if ( szFullAccessor.length() > 0 )
                {
                    bExtractedAccessorName = true;
                }
            }
            else if ( ( szFullAccessor.length() > 0 ) || 
                     ( isalnum( szMessage[nCurrentPos] ) ) )
            {
                szFullAccessor += szMessage[nCurrentPos];
            }
        }
    }

    // Split up the namespace if any and accessor name.
    splitNSAndAccessor( szFullAccessor, szNamespace, szAccessorName );
 
    if ( '>' == szMessage[nCurrentPos] )
    {
        // walk past the end of the element
        ++nCurrentPos;
    }
    return retval;
}

bool SOAPParser::extractNamespaces( std::string &szCompleteAccessor )
{
    bool retval = true;
    
    // The idea here is to extract and remove the namespace declarations.
    const std::string KszXMLNS = "xmlns:";
    const long KnLenXMLNS = KszXMLNS.length();
    const long KnLength = szCompleteAccessor.length();

    std::string szTempAccessor;
    std::string szNamespaceName;
    std::string szNamespaceURI;
    char cQuoteChar = 0;
    long nEqualsPos = 0;
    long nQuotePos = 0;
    long nEndQuotePos = 0;
    bool bFoundANamespace = false;
    for ( long nPos = 0; nPos < KnLength; ++nPos )
    {
        nPos = szCompleteAccessor.find( KszXMLNS, nPos );
        if ( std::string::npos == nPos )
        {
            // No more occurences of the string exist after nPos
            break;
        }
        bFoundANamespace = true;
        szTempAccessor += szCompleteAccessor.substr( nEndQuotePos + 1, nPos - nEndQuotePos - 1 );
        // Capture the namespace name.
        nEqualsPos = szCompleteAccessor.find( std::string("="), nPos );
        szNamespaceName = szCompleteAccessor.substr( nPos + KnLenXMLNS, nEqualsPos - nPos - KnLenXMLNS );
        // Capture the namespace URI.
        // find out the enclosing quote type
        nPos = nEqualsPos;
        szNamespaceURI = extractQuotedString( szCompleteAccessor, nPos );
        nEndQuotePos = nPos;
        m_namespaceMap[szNamespaceName] = szNamespaceURI;
    }
        
    if ( (nEndQuotePos > 0 ) && (KnLength - nEndQuotePos - 1 > 0) ) 
    {
        szTempAccessor += szCompleteAccessor.substr( nEndQuotePos + 1, KnLength - nEndQuotePos - 1 );
    }
    // Replace the string with the one without namespace declarations.
    // We took those out so that they wouldn't look like 
    // attributes later.
    if ( bFoundANamespace )
    {
        szCompleteAccessor = szTempAccessor;
    }
    return retval;
}

bool SOAPParser::extractAttributes(SOAPElement &theElement, std::string szBeginTag)
{
    bool retval = true;
    const long KnLength = szBeginTag.length();
    long nPos = 0;
    long nEqualsPos = 0;
    long nQuotePos = 0;
    long nEndQuotePos = 0;
    std::string szNamespace;
    // Message may have a bunch of leading whitespace.  Eat it up.

    for ( nPos = 0; ( nPos < KnLength ) && isspace( szBeginTag[nPos] ); ++nPos )
    {
        // just advances through the loop until it finds a non-whitespace
        // character.
    }

    for ( ; nPos < KnLength; ++nPos )
    {
        SOAPAttribute anAttribute;
        nEqualsPos = szBeginTag.find( std::string("="), nPos );
        if ( std::string::npos == nEqualsPos )
        {
            break;
        }
        szNamespace = szBeginTag.substr( nPos, nEqualsPos - nPos );
        splitNSAndAccessor( szNamespace, anAttribute.namespaceName(), anAttribute.accessor() );
        nPos = nEqualsPos;
        anAttribute.value() = extractQuotedString( szBeginTag, nPos );
        theElement.addAttribute( anAttribute );
    }
    return retval;
}

bool SOAPParser::extractValue(SOAPElement &theElement, const std::string& szMessage, long &nCurrentPos)
{
    bool retval = true;
    const long KnLength = szMessage.length();    
    std::string szValue;
    bool bProcessingWhiteSpace = false;
    bool bIsSpace = false;
    for ( ; ( szMessage[nCurrentPos] != '<' ) && ( nCurrentPos < KnLength );
        ++nCurrentPos )
    {
        bIsSpace = ( 0 != isspace(szMessage[nCurrentPos]) );
        if ( !( bIsSpace && bProcessingWhiteSpace ))
        {
            bProcessingWhiteSpace = false;
            szValue += szMessage[nCurrentPos];
        }
        else if ( bIsSpace && !bProcessingWhiteSpace )
        {
            // Because we eat whitespace, any whitespace should initially
            // generate a space, then nothing until
            // the next non-whitespace character.
            szValue += ' ';
            bProcessingWhiteSpace = true;
        }
    }
    
    theElement.value() = szValue;
    if ( szMessage[nCurrentPos] == '<' )
    {
        long nLTPosition = nCurrentPos;
        // Figure out if this is a begin tag or an end tag
        bool bIsEndTag = false;
        for ( ; ( szMessage[nCurrentPos] != '/' ) && ( nCurrentPos < KnLength );
            ++nCurrentPos )
        {
            if ( isspace( szMessage[nCurrentPos] ) )
            {
                continue;
            }
            if ( isalnum( szMessage[nCurrentPos] ) || (szMessage[nCurrentPos] == '_') )
            {
                break;
            }
        }
        bIsEndTag = szMessage[nCurrentPos] == '/';
        if ( szMessage[nCurrentPos] != '/' )
        {
            SOAPElement* pElement = new SOAPElement();
            theElement.addElement( pElement );
            nCurrentPos = nLTPosition;
            retval = parseMessage( szMessage, *pElement, nCurrentPos );
        }
        else
        {
            nCurrentPos = nLTPosition;
        }
    }
    
    return retval;
}

void SOAPParser::splitNSAndAccessor(std::string szFullString, std::string &szNamespace, std::string &szOther)
{
    const long KnNameLength = szFullString.length();
    long nColonPos = szFullString.find_first_of( std::string(":") );
    bool bHasNameSpace = nColonPos != std::string::npos;
    if ( bHasNameSpace )
    {
        szNamespace = szFullString.substr( 0, nColonPos );
        szOther = szFullString.substr( nColonPos + 1, KnNameLength );
    }
    else
    {
        szOther = szFullString;
    }
}


std::string SOAPParser::extractQuotedString(const std::string &szString, long &nPos)
{
    std::string szRetval;
    const long KnLength = szString.length();
    char cQuoteChar = 0;
    long nQuotePos = 0;
    long nEndQuotePos = 0;
    nQuotePos = szString.find_first_of( g_KszQuoteTypes, nPos );

    if ( std::string::npos != nQuotePos )
    {
        cQuoteChar = szString[nQuotePos];
        nEndQuotePos = szString.find( cQuoteChar, nQuotePos + 1 );
        szRetval = szString.substr( nQuotePos + 1, nEndQuotePos - nQuotePos - 1 );
        nPos = nEndQuotePos + 1;
    }
    return szRetval;
}

SOAPParser::XMLNStoURN& SOAPParser::getNamespacesInUse()
{
    return m_namespaceMap;
}

void SOAPParser::setFailed()
{
    m_pFault = std::auto_ptr<SOAPFault>( new SOAPFault );
}

SOAPFault* SOAPParser::getFault()
{
    // Don't create it if we didn't need it.
    return m_pFault.get();
}
