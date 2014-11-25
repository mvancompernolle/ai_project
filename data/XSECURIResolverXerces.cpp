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
 * XSECURIResolverXerces := Virtual Interface class that takes a URI and
 *                    creates a binary input stream from it.
 *
 */


#include <xsec/framework/XSECDefs.hpp>
#include <xsec/framework/XSECURIResolverXerces.hpp>
#include <xsec/framework/XSECException.hpp>

#include <xercesc/framework/URLInputSource.hpp>
#include <xercesc/util/BinInputStream.hpp>

#include <string.h>

// --------------------------------------------------------------------------------
//           Constructors and Destructors
// --------------------------------------------------------------------------------

XSECURIResolverXerces::XSECURIResolverXerces(const char * baseURI) {

	if (baseURI != 0) {

		mp_baseURI = strdup(baseURI);

	}
	else
		mp_baseURI = 0;

};

XSECURIResolverXerces::~XSECURIResolverXerces() {

	if (mp_baseURI != 0)
		delete[] mp_baseURI;
}

// --------------------------------------------------------------------------------
//           Interface Methods
// --------------------------------------------------------------------------------

BinInputStream * XSECURIResolverXerces::resolveURI(const char * uri) {

	URLInputSource			* URLS;		// Use Xerces URL Input source
	BinInputStream			* is;		// To handle the actual input


	if (mp_baseURI == 0)
		URLS = new URLInputSource(XMLURL(uri));
	else
		URLS = new URLInputSource(XMLURL(XMLURL(mp_baseURI), uri));

	is = URLS->makeStream();

	delete URLS;

	if (is == NULL) {

		throw XSECException(XSECException::ErrorOpeningURI,
			"An error occurred in XSECURIREsolverXerces when opening an URLInputStream");

	}

	return is;

}


XSECURIResolver * XSECURIResolverXerces::clone(void) {

	XSECURIResolverXerces * ret;

	ret = new XSECURIResolverXerces();

	if (this->mp_baseURI != 0)
		ret->mp_baseURI = strdup(this->mp_baseURI);
	else
		ret->mp_baseURI = 0;

	return ret;

}

// --------------------------------------------------------------------------------
//           Specific Methods
// --------------------------------------------------------------------------------


void XSECURIResolverXerces::setBaseURI(const char * uri) {

	if (mp_baseURI != 0)
		delete mp_baseURI;

	mp_baseURI = strdup(uri);

};
