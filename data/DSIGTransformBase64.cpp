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
 * DSIGTransformBase64 := Class that holds a Base64 transform structure
 *
 */

// XSEC

#include <xsec/dsig/DSIGTransformBase64.hpp>
#include <xsec/dsig/DSIGSignature.hpp>
#include <xsec/transformers/TXFMBase64.hpp>
#include <xsec/transformers/TXFMC14n.hpp>
#include <xsec/transformers/TXFMXPath.hpp>
#include <xsec/framework/XSECException.hpp>
#include <xsec/utils/XSECDOMUtils.hpp>
#include <xsec/framework/XSECError.hpp>

// --------------------------------------------------------------------------------
//           Constructors and Destructors
// --------------------------------------------------------------------------------

DSIGTransformBase64::DSIGTransformBase64(DSIGSignature *sig, DOMNode * node) :
DSIGTransform(sig, node) {};


DSIGTransformBase64::DSIGTransformBase64(DSIGSignature *sig) :
DSIGTransform(sig) {};
		  

DSIGTransformBase64::~DSIGTransformBase64() {};

// --------------------------------------------------------------------------------
//           Interface Methods
// --------------------------------------------------------------------------------


transformType DSIGTransformBase64::getTransformType() {

	return TRANSFORM_BASE64;

}


TXFMBase * DSIGTransformBase64::createTransformer(TXFMBase * input) {

	TXFMBase *newInput;

	// If the input is a Nodeset then we need to find the text from the input

	if (input->getOutputType() == TXFMBase::DOM_NODES) {


#ifdef XSEC_NO_XPATH

		throw XSECException(XSECException::UnsupportedFunction,
			"Unable to extract Base64 text from Nodes without XPath support");

#else
		
		// Use an XPath transform to get "Self::text()" from the nodeset
		
		TXFMXPath *x;
		
		XSECnew(x, TXFMXPath(mp_txfmNode->getOwnerDocument()));
		x->setInput(input);
		((TXFMXPath *) x)->evaluateExpr(mp_txfmNode, "self::text()");
		
		TXFMC14n *c;
		
		// Now use c14n to translate to BYTES
		
		XSECnew(c, TXFMC14n(mp_txfmNode->getOwnerDocument()));
		c->setInput(x);

		newInput = c;
#endif

	}

	else {

		newInput = input;

	}

	// Now the actual Base64

	TXFMBase64 *b = new TXFMBase64(mp_txfmNode->getOwnerDocument());
	b->setInput(newInput);

	return b;

}

DOMElement * DSIGTransformBase64::createBlankTransform(DOMDocument * parentDoc) {

	safeBuffer str;
	safeBuffer prefix;
	DOMElement *ret;
	DOMDocument *doc = mp_parentSignature->getParentDocument();

	prefix = mp_parentSignature->getDSIGNSPrefix();
	
	// Create the transform node
	makeQName(str, prefix, "Transform");
	ret = doc->createElementNS(DSIGConstants::s_unicodeStrURIDSIG, str.sbStrToXMLCh());
	ret->setAttribute(DSIGConstants::s_unicodeStrAlgorithm, DSIGConstants::s_unicodeStrURIBASE64);

	mp_txfmNode = ret;

	return ret;

}

void DSIGTransformBase64::load(void) {

	// Do nothing for a Base64 transform

}
