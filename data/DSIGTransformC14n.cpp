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
 * DSIGTransformC14n := Class that performs C14n canonicalisation
 *
 */

#include <xsec/dsig/DSIGTransformC14n.hpp>
#include <xsec/framework/XSECException.hpp>
#include <xsec/transformers/TXFMC14n.hpp>
#include <xsec/framework/XSECError.hpp>
#include <xsec/dsig/DSIGSignature.hpp>

// --------------------------------------------------------------------------------
//           Constructors and Destructors
// --------------------------------------------------------------------------------

DSIGTransformC14n::DSIGTransformC14n(DSIGSignature *sig, DOMNode * node) :
DSIGTransform(sig, node) {

	m_cMethod = CANON_NONE;
	mp_inclNSNode = NULL;
	mp_inclNSStr = NULL;
}
	

DSIGTransformC14n::DSIGTransformC14n(DSIGSignature *sig) :
DSIGTransform(sig) {

	m_cMethod = CANON_NONE;
	mp_inclNSNode = NULL;
	mp_inclNSStr = NULL;

}
		  
DSIGTransformC14n::~DSIGTransformC14n() {};

// --------------------------------------------------------------------------------
//           Interface Methods
// --------------------------------------------------------------------------------
	
transformType DSIGTransformC14n::getTransformType() {

	return TRANSFORM_C14N;

}

TXFMBase * DSIGTransformC14n::createTransformer(TXFMBase * input) {

	TXFMC14n * c;
	
	XSECnew(c, TXFMC14n(mp_txfmNode->getOwnerDocument()));
	c->setInput(input);

	switch (m_cMethod) {

	case (CANON_C14N_NOC) :
	case (CANON_C14NE_NOC) :
		c->stripComments();
		break;
	case (CANON_C14N_COM) :
	case (CANON_C14NE_COM) :
		c->activateComments();
		break;
	default:
		break;
	}

	// Check for exclusive
	if (m_cMethod == CANON_C14NE_COM || m_cMethod == CANON_C14NE_NOC) {

		if (mp_inclNSStr == NULL) {

			c->setExclusive();

		}
		else {

			safeBuffer incl;
			incl << (*(mp_parentSignature->getSBFormatter()) << mp_inclNSStr);
			c->setExclusive(incl);

		}

	}

	return c;

}

DOMElement * DSIGTransformC14n::createBlankTransform(DOMDocument * parentDoc) {

	safeBuffer str;
	safeBuffer prefix;
	DOMElement *ret;
	DOMDocument *doc = mp_parentSignature->getParentDocument();

	prefix = mp_parentSignature->getDSIGNSPrefix();
	
	// Create the transform node
	makeQName(str, prefix, "Transform");
	ret = doc->createElementNS(DSIGConstants::s_unicodeStrURIDSIG, str.sbStrToXMLCh());
	ret->setAttribute(DSIGConstants::s_unicodeStrAlgorithm, DSIGConstants::s_unicodeStrURIC14N_NOC);

	mp_txfmNode = ret;
	mp_inclNSStr = NULL;
	mp_inclNSNode = NULL;

	return ret;

}

void DSIGTransformC14n::load(void) {

	const XMLCh * uri;
	DOMNamedNodeMap * atts;
	DOMNode *att;

	// Read the URI for the type
	if (mp_txfmNode == NULL) {

		throw XSECException(XSECException::ExpectedDSIGChildNotFound,
			"Expected <Transform> Node in DSIGTrasnformC14n::load");

	}

	atts = mp_txfmNode->getAttributes();

	if (atts == NULL || 
		((att = atts->getNamedItem(DSIGConstants::s_unicodeStrAlgorithm)) == NULL)) {

		throw XSECException(XSECException::ExpectedDSIGChildNotFound,
			"Expected to find Algorithm attribute in <Transform> node");

	}

	uri = att->getNodeValue();

	if (strEquals(uri, DSIGConstants::s_unicodeStrURIC14N_COM)) {
		m_cMethod = CANON_C14N_COM;
	}
	else if (strEquals(uri, DSIGConstants::s_unicodeStrURIC14N_NOC)) {
		m_cMethod = CANON_C14N_NOC;
	}
	else if (strEquals(uri, DSIGConstants::s_unicodeStrURIEXC_C14N_COM)) {
		m_cMethod = CANON_C14NE_NOC;
	}
	else if (strEquals(uri, DSIGConstants::s_unicodeStrURIEXC_C14N_NOC)) {
		m_cMethod = CANON_C14NE_NOC;
	}
	else {
		throw XSECException(XSECException::ExpectedDSIGChildNotFound,
			"Unexpected URI found in canonicalisation <Transform>");
	}

	// Determine whether there is an InclusiveNamespaces list

	if (m_cMethod == CANON_C14NE_NOC || m_cMethod == CANON_C14NE_COM) {

		// Exclusive, so there may be an InclusiveNamespaces node

		DOMNode *mp_inclNSNode = mp_txfmNode->getFirstChild();
		
		while (mp_inclNSNode != NULL && mp_inclNSNode->getNodeType() != DOMNode::ELEMENT_NODE &&
			!strEquals(mp_inclNSNode->getNodeName(), "InclusiveNamespaces"))
				mp_inclNSNode = mp_inclNSNode->getNextSibling();

		if (mp_inclNSNode != 0) {

			// Have a prefix list
			atts = mp_inclNSNode->getAttributes();
			safeBuffer inSB;

			if (atts == 0 || ((att = atts->getNamedItem(MAKE_UNICODE_STRING("PrefixList"))) == NULL)) {
				throw XSECException(XSECException::ExpectedDSIGChildNotFound,
					"Expected PrefixList in InclusiveNamespaces");
			}

			mp_inclNSStr = att->getNodeValue();

		}
	}

}

// --------------------------------------------------------------------------------
//           Canonicalization Specific Methods
// --------------------------------------------------------------------------------


void DSIGTransformC14n::setCanonicalizationMethod(canonicalizationMethod method) {

	const XMLCh * m = canonicalizationMethod2UNICODEURI(method);

	if (strEquals(m, DSIGConstants::s_unicodeStrEmpty) || mp_txfmNode == NULL) {

		throw XSECException(XSECException::TransformError,
			"Either method unknown or Node not set in setCanonicalizationMethod");

	}

	if (method == CANON_C14N_NOC || method == CANON_C14N_COM) {

		if (m_cMethod == CANON_C14NE_NOC || m_cMethod == CANON_C14NE_COM) {
	
			if (mp_inclNSNode != 0) {

				mp_txfmNode->removeChild(mp_inclNSNode);
				mp_inclNSNode->release();		// No longer required

				mp_inclNSNode = NULL;
				mp_inclNSStr = NULL;

			}
		}

	}

	// Now do the set.

	((DOMElement *) mp_txfmNode)->setAttribute(MAKE_UNICODE_STRING("Algorithm"), m);
	m_cMethod = method;

}

canonicalizationMethod DSIGTransformC14n::getCanonicalizationMethod(void) {

	return m_cMethod;

}

void DSIGTransformC14n::addInclusiveNamespace(const char * ns) {

	if (m_cMethod != CANON_C14NE_COM && m_cMethod != CANON_C14NE_NOC) {

		throw XSECException(XSECException::TransformError,
			"Cannot set inclusive namespaces on non Exclusive Canonicalisation");

	}

	if (mp_inclNSNode == NULL) {

		safeBuffer str;
		safeBuffer prefix;
		DOMDocument *doc = mp_parentSignature->getParentDocument();

		prefix = mp_parentSignature->getDSIGNSPrefix();
	
		// Create the transform node
		makeQName(str, prefix, "InclusiveNamespaces");
		mp_inclNSNode = doc->createElementNS(DSIGConstants::s_unicodeStrURIDSIG, str.sbStrToXMLCh());
		mp_inclNSNode->setAttribute(MAKE_UNICODE_STRING("PrefixList"), MAKE_UNICODE_STRING(ns));
		mp_inclNSStr = mp_inclNSNode->getAttributes()->getNamedItem(MAKE_UNICODE_STRING("PrefixList"))->getNodeValue();

	}

	else {

		// More tricky
		safeBuffer str;

		str << (*(mp_parentSignature->getSBFormatter()) << mp_inclNSStr);
		str.sbStrcatIn(" ");
		str.sbStrcatIn((char *) ns);
		mp_inclNSNode->setAttribute(MAKE_UNICODE_STRING("PrefixList"), str.sbStrToXMLCh());
		mp_inclNSStr = mp_inclNSNode->getAttributes()->getNamedItem(MAKE_UNICODE_STRING("PrefixList"))->getNodeValue();

	}

}

const XMLCh * DSIGTransformC14n::getPrefixList(void) {

	return mp_inclNSStr;

}

void DSIGTransformC14n::clearInclusiveNamespaces(void) {

	if (mp_inclNSNode != 0) {

		mp_txfmNode->removeChild(mp_inclNSNode);
		mp_inclNSNode->release();		// No longer required

		mp_inclNSNode = NULL;
		mp_inclNSStr = NULL;

	}
}