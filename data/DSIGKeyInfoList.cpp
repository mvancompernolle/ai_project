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
 * DSIGKeyInfoList := Class for Loading and storing a list of KeyInfo elements
 *					 
 *
 */

// XSEC Includes
#include <xsec/dsig/DSIGKeyInfoList.hpp>
#include <xsec/dsig/DSIGKeyInfoX509.hpp>
#include <xsec/dsig/DSIGKeyInfoName.hpp>
#include <xsec/dsig/DSIGKeyInfoValue.hpp>
#include <xsec/framework/XSECError.hpp>
#include <xsec/utils/XSECDOMUtils.hpp>
#include <xsec/dsig/DSIGSignature.hpp>

DSIGKeyInfoList::DSIGKeyInfoList(DSIGSignature * sig) :
mp_parentSignature(sig) {}

DSIGKeyInfoList::~DSIGKeyInfoList() {

	empty();

}

// Actions

void DSIGKeyInfoList::addKeyInfo(DSIGKeyInfo * ref) {

	m_keyInfoList.push_back(ref);

}

DSIGKeyInfo * DSIGKeyInfoList::removeKeyInfo(size_type index) {

	if (index < m_keyInfoList.size())
		return m_keyInfoList[index];

	return NULL;

}

size_t DSIGKeyInfoList::getSize() {

	return m_keyInfoList.size();

}


DSIGKeyInfo * DSIGKeyInfoList::item(size_type index) {

	if (index < m_keyInfoList.size())
		return m_keyInfoList[index];
	
	return NULL;

}

void DSIGKeyInfoList::empty() {

	size_type i, s;
	s = getSize();

	for (i = 0; i < s; ++i)
		delete m_keyInfoList[i];

	m_keyInfoList.clear();

}

bool DSIGKeyInfoList::isEmpty() {

		return (m_keyInfoList.size() == 0);

}

// --------------------------------------------------------------------------------
//           Add a KeyInfo based on XML DomNode source
// --------------------------------------------------------------------------------


bool DSIGKeyInfoList::addXMLKeyInfo(DOMNode *ki) {

	// return true if successful - does not throw if the node type is unknown

	if (ki == 0)
		return false;

	XSECSafeBufferFormatter *formatter = mp_parentSignature->getSBFormatter();

	if (strEquals(getDSIGLocalName(ki), "X509Data")) {

		DSIGKeyInfoX509 * k;

		// Have a certificate!
		XSECnew(k, DSIGKeyInfoX509(mp_parentSignature, ki));
		k->load();

		// Add to the KeyInfo list
		this->addKeyInfo(k);

		return true;

	}

	if (strEquals(getDSIGLocalName(ki), "KeyName")) {

		DSIGKeyInfoName * k;

		XSECnew(k, DSIGKeyInfoName(mp_parentSignature, ki));
		k->load();

		this->addKeyInfo(k);

		return true;
	}

	if (strEquals(getDSIGLocalName(ki), "KeyValue")) {

		DSIGKeyInfoValue * k;

		XSECnew(k, DSIGKeyInfoValue(mp_parentSignature, ki));
		k->load();

		// Add
		this->addKeyInfo(k);

		return true;

	}

	return false;

}



