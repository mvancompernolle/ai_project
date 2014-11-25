/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2002 Berin Lautenbach.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by
 *                   Berin Lautenbach"
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "XSEC", "xml-security-c" and Berin Lautenbach must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact berin@users.sourceforge.net.
 *
 * 5. Products derived from this software may not be called "xml-security-c",
 *    nor may "xml-security-c" appear in their name, without prior written
 *    permission of Berin Lautenbach.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL BERIN LAUTENBACH OR OTHER
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 */

/*
 * XSEC
 *
 * XSECSafeBufferFormatter := Class for formatting DOMStrings into SafeBuffers
 *
 */

#include <xsec/utils/XSECSafeBufferFormatter.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xsec/framework/XSECError.hpp>

//XMLFormatter& operator<< (XMLFormatter& strm, const DOMString& s);
/*
{
    unsigned int lent = s.length();

	if (lent <= 0)
		lent = 0;

    XMLCh*  buf = new XMLCh[lent + 1];
    if (lent > 0)
		XMLString::copyNString(buf, s.rawBuffer(), lent);
    buf[lent] = 0;
    strm << buf;
    delete [] buf;
    return strm;
}
*/

// -----------------------------------------------------------------------
//  If the caller tells us the local coding type, this allows us to work 
//  easily with safe buffers and DOMStrings
// -----------------------------------------------------------------------


/* class XSECSafeBufferFormatter {

	XMLFormatter		* formatter;		// To actually perform the formatting
	safeBuffer			formatBuffer;		// Storage of translated strings
	sbFormatTarget		* sbf;				// Format target used by XMLFormatter

public:

  */

// Constructor

XSECSafeBufferFormatter::XSECSafeBufferFormatter(
						const XMLCh * const			outEncoding,
						const XMLFormatter::EscapeFlags	escapeFlags,
						const XMLFormatter::UnRepFlags unrepFlags) {

	
	XSECnew(sbf, sbFormatTarget());
	sbf->setBuffer(&formatBuffer);

	XSECnew(formatter, XMLFormatter(outEncoding, 
									sbf, 
									escapeFlags, 
									unrepFlags));

}





XSECSafeBufferFormatter::XSECSafeBufferFormatter(
						const char * const			outEncoding,
						const XMLFormatter::EscapeFlags	escapeFlags,
						const XMLFormatter::UnRepFlags unrepFlags) {

	XSECnew(sbf, sbFormatTarget());
	sbf->setBuffer(&formatBuffer);

	XSECnew(formatter, XMLFormatter(outEncoding, 
									sbf, 
									escapeFlags, 
									unrepFlags));

}

// Destructor

XSECSafeBufferFormatter::~XSECSafeBufferFormatter() {

	if (formatter != NULL)
		delete formatter;

	if (sbf != NULL)
		delete sbf;

}

// Reimplementation of XMLFormatter functions

void  XSECSafeBufferFormatter::formatBuf (
				 const XMLCh *const toFormat,
				 const unsigned int count,
				 const XMLFormatter::EscapeFlags escapeFlags,
				 const XMLFormatter::UnRepFlags unrepFlags) {


	formatter->formatBuf(toFormat, count, escapeFlags, unrepFlags);

}

XSECSafeBufferFormatter&  XSECSafeBufferFormatter::operator<< (const XMLCh *const toFormat) {

	formatBuffer[0] = '\0';

	*formatter << toFormat;

	return *this;

}

XSECSafeBufferFormatter&  
     XSECSafeBufferFormatter::operator<< (const XMLCh toFormat) {

	*formatter << toFormat;
	return *this;

}

const XMLCh*  XSECSafeBufferFormatter::getEncodingName ()const {

	return formatter->getEncodingName();

}

void  XSECSafeBufferFormatter::setEscapeFlags (const XMLFormatter::EscapeFlags newFlags) {

	formatter->setEscapeFlags(newFlags);

}
void  XSECSafeBufferFormatter::setUnRepFlags (const XMLFormatter::UnRepFlags newFlags) {

	formatter->setUnRepFlags(newFlags);

}

XSECSafeBufferFormatter&  XSECSafeBufferFormatter::operator<< (const XMLFormatter::EscapeFlags newFlags) {

	*formatter << newFlags;
	return *this;

}

XSECSafeBufferFormatter&  XSECSafeBufferFormatter::operator<< (const XMLFormatter::UnRepFlags newFlags) {

	*formatter << newFlags;
	return *this;

}

// Friends for working with safestrings

safeBuffer& operator<< (safeBuffer &to, const XSECSafeBufferFormatter & from) {

	// Simply copy out the format buffer, but zeroise the original first

	to = from.formatBuffer;
	return to;

}

/*

XSECSafeBufferFormatter& XSECSafeBufferFormatter::operator<< (const DOMString &s) {

	// Clear out buffer
	formatBuffer[0] = '\0';

    unsigned int lent = s.length();

	if (lent <= 0)
		lent = 0;

    XMLCh*  buf = new XMLCh[lent + 1];
    
	if (lent > 0)
		XMLString::copyNString(buf, s.rawBuffer(), lent);
    
	buf[lent] = 0;
    *(this) << buf;

    delete [] buf;

    return *this;
}

*/