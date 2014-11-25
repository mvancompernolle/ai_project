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
 * DSIGKeyName := Simply a string that names a key for an application to read
 *
 */

#include <xsec/dsig/DSIGKeyInfoName.hpp>
#include <xsec/framework/XSECError.hpp>
#include <xsec/utils/XSECPlatformUtils.hpp>
#include <xsec/utils/XSECDOMUtils.hpp>
#include <xsec/dsig/DSIGSignature.hpp>

// --------------------------------------------------------------------------------
//           Constructors and Destructors
// --------------------------------------------------------------------------------


DSIGKeyInfoName::DSIGKeyInfoName(DSIGSignature *sig, DOMNode *nameNode) : 
DSIGKeyInfo(sig),
m_name(""),
mp_keyNameTextNode(0) {

	mp_keyInfoDOMNode = nameNode;

}


DSIGKeyInfoName::DSIGKeyInfoName(DSIGSignature *sig) : 
DSIGKeyInfo(sig),
m_name(""),
mp_keyNameTextNode(0) {

	mp_keyInfoDOMNode = 0;

}


DSIGKeyInfoName::~DSIGKeyInfoName() {

};

// --------------------------------------------------------------------------------
//           Load and Get functions
// --------------------------------------------------------------------------------


void DSIGKeyInfoName::load(void) {

	// Assuming we have a valid DOM_Node to start with, load the signing key so that it can
	// be used later on

	if (mp_keyInfoDOMNode == NULL) {

		// Attempt to load an empty signature element
		throw XSECException(XSECException::LoadEmptyInfoName);

	}

	XSECSafeBufferFormatter *formatter = mp_parentSignature->getSBFormatter();


	if (!strEquals(getDSIGLocalName(mp_keyInfoDOMNode), "KeyName")) {

		throw XSECException(XSECException::LoadNonInfoName);

	}

	// Now find the text node containing the name

	DOMNode *tmpElt = mp_keyInfoDOMNode->getFirstChild();

	while (tmpElt != 0 && tmpElt->getNodeType() != DOMNode::TEXT_NODE)
		tmpElt = tmpElt->getNextSibling();

	if (tmpElt != 0) {

		mp_keyNameTextNode = tmpElt;
		m_name << (*formatter << tmpElt->getNodeValue());

	}

	else {

		throw XSECException(XSECException::ExpectedDSIGChildNotFound,
			"Expected TEXT node as child to <KeyName> element");

	}

}

// --------------------------------------------------------------------------------
//           Create and Set functions
// --------------------------------------------------------------------------------

DOMElement * DSIGKeyInfoName::createBlankKeyName(const char * name) {

	// Create the DOM Structure

	safeBuffer str;
	DOMDocument *doc = mp_parentSignature->getParentDocument();
	safeBuffer prefix = mp_parentSignature->getDSIGNSPrefix();

	makeQName(str, prefix, "KeyName");

	DOMElement *ret = doc->createElementNS(DSIGConstants::s_unicodeStrURIDSIG, str.sbStrToXMLCh());
	mp_keyInfoDOMNode = ret;
	mp_keyNameTextNode = doc->createTextNode(MAKE_UNICODE_STRING(name));
	ret->appendChild(mp_keyNameTextNode);

	m_name.sbStrcpyIn(name);

	return ret;

}

void DSIGKeyInfoName::setKeyName(const char * name) {

	if (mp_keyNameTextNode == 0) {

		// Attempt to load an empty element
		throw XSECException(XSECException::LoadEmptySignature,
			"KeyInfoName::set() called prior to load() or createBlank()");

	}

	mp_keyNameTextNode->setNodeValue(MAKE_UNICODE_STRING(name));
	m_name.sbStrcpyIn(name);

}
