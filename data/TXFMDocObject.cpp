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
 * TXFMDocObject := Class that takes an input Document object ID to start a txfm pipe
 *
 */

#include <xsec/transformers/TXFMDocObject.hpp>
#include <xsec/framework/XSECException.hpp>
#include <xsec/utils/XSECDOMUtils.hpp>

// Construct/Destruct

TXFMDocObject::TXFMDocObject(DOMDocument *doc) : TXFMBase(doc) {

	input = NULL;
	fragmentId.sbStrcpyIn("");				// No object ID by default
	type = TXFMBase::DOM_NODE_NONE;	// No nodes currently held

}

TXFMDocObject::~TXFMDocObject() {}

// Methods to set the inputs

void TXFMDocObject::setInput(TXFMBase *newInput) {

	// Probably should error, but we store it so we can later clear out pipes.

	input = newInput;

}

DOMNode * findDSIGId(DOMNode *current, safeBuffer &newFragmentId) {

	DOMNode *tmp, *ret;
	DOMNamedNodeMap *atts;

	if (current->getNodeType() == DOMNode::ELEMENT_NODE) {

		atts = current->getAttributes();
		if (atts != NULL) {
			tmp = atts->getNamedItem(MAKE_UNICODE_STRING("Id"));
			if (tmp != 0 && strEquals(tmp->getNodeValue(), newFragmentId.sbStrToXMLCh())) {

				// Found it!

				return current;

			}

		}

	}

	// Check children

	tmp = current->getFirstChild();
	while (tmp != NULL) {

		if ((ret = findDSIGId(tmp, newFragmentId)) != 0)
			return ret;

		tmp = tmp->getNextSibling();

	}

	return NULL;

}

void TXFMDocObject::setInput(DOMDocument *doc, safeBuffer &newFragmentId) {

	// We have a document fragment marked by an objectID string.

	// Now try to find the node that the objectId belongs to

	fragmentObject = doc->getElementById(newFragmentId.sbStrToXMLCh());

	if (fragmentObject == 0) {

		// It might be that no DSIG DTD was attached and that the ID is in a
		// DSIG element

		fragmentObject = findDSIGId(doc, newFragmentId);

		if (fragmentObject == 0)

			throw XSECException(XSECException::IDNotFoundInDOMDoc);

	}

	document = doc;
	fragmentId = newFragmentId;
	type = TXFMBase::DOM_NODE_DOCUMENT_FRAGMENT;

}

void TXFMDocObject::setInput(DOMDocument * doc, DOMNode * newFragmentObject) {

	// Have a document fragment directly notified by a DOM_Node

	document = doc;
	fragmentObject = newFragmentObject;
	type = TXFMBase::DOM_NODE_DOCUMENT_NODE;

}

void TXFMDocObject::setInput(DOMDocument *doc) {

	document = doc;
	type = TXFMBase::DOM_NODE_DOCUMENT;

}

	// Methods to get tranform output type and input requirement

TXFMBase::ioType TXFMDocObject::getInputType(void) {

	return TXFMBase::NONE;

}

TXFMBase::ioType TXFMDocObject::getOutputType(void) {

	return TXFMBase::DOM_NODES;

}

TXFMBase::nodeType TXFMDocObject::getNodeType(void) {

	return type;

}

	// Methods to get output data

unsigned int TXFMDocObject::readBytes(XMLByte * const toFill, unsigned int maxToFill) {

	return 0;		// Cannot read a bytestream from a DOM_NODES transform

}

DOMDocument * TXFMDocObject::getDocument() {

	return document;

}

DOMNode * TXFMDocObject::getFragmentNode() {

	return fragmentObject;

}

safeBuffer TXFMDocObject::getFragmentId() {

	return fragmentId;	

}
