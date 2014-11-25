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
 * XSECProvider.hpp := The main interface for users wishing to gain access
 *                     to signature objects
 *
 */
#include <xsec/framework/XSECProvider.hpp>
#include <xsec/framework/XSECError.hpp>
#include <xsec/utils/XSECDOMUtils.hpp>
#include <xsec/framework/XSECURIResolverXerces.hpp>

// --------------------------------------------------------------------------------
//           Constructors/Destructors
// --------------------------------------------------------------------------------


XSECProvider::XSECProvider() {

	mp_URIResolver = new XSECURIResolverXerces();

}

XSECProvider::~XSECProvider() {

	SignatureListVectorType::iterator i;

	
	for (i = m_activeSignatures.begin(); i != m_activeSignatures.end(); ++i)
		delete *i;

	m_activeSignatures.clear();
}

// --------------------------------------------------------------------------------
//           Signature Creation/Deletion
// --------------------------------------------------------------------------------


DSIGSignature * XSECProvider::newSignatureFromDOM(DOMDocument *doc, DOMNode *sigNode) {

	DSIGSignature * ret;

	XSECnew(ret, DSIGSignature(doc, sigNode));

	setup(ret);

	return ret;

}

DSIGSignature * XSECProvider::newSignatureFromDOM(DOMDocument *doc) {

	DSIGSignature * ret;

	DOMNode *sigNode = findDSIGNode(doc, "Signature");

	if (sigNode == NULL) {
		
		throw XSECException(XSECException::SignatureCreationError,
			"Could not find a signature node in passed in DOM document");

	}

	XSECnew(ret, DSIGSignature(doc, sigNode));

	setup(ret);

	return ret;

}

DSIGSignature * XSECProvider::newSignature(void) {

	DSIGSignature * ret;

	XSECnew(ret, DSIGSignature());

	setup(ret);

	return ret;

}

void XSECProvider::releaseSignature(DSIGSignature * toRelease) {

	// Find in the active list

	SignatureListVectorType::iterator i;

	i = m_activeSignatures.begin();
	while (i != m_activeSignatures.end() && *i != toRelease)
		++i;

	if (i == m_activeSignatures.end()) {

		throw XSECException(XSECException::ProviderError,
			"Attempt to release a signature that was not created by this provider");

	}
	
	// For now - remove from list.  Would be better to recycle
	m_activeSignatures.erase(i);
	delete toRelease;

}

// --------------------------------------------------------------------------------
//           Environmental methods
// --------------------------------------------------------------------------------


void XSECProvider::setDefaultURIResolver(XSECURIResolver * resolver) {

	if (mp_URIResolver != 0)
		delete mp_URIResolver;

	mp_URIResolver = resolver->clone();

}

// --------------------------------------------------------------------------------
//           Internal functions
// --------------------------------------------------------------------------------

void XSECProvider::setup(DSIGSignature *sig) {

	// Called by all Signature creation methods to set up the sig

	// Add to the active list
	m_activeSignatures.push_back(sig);
	sig->setURIResolver(mp_URIResolver);

}
