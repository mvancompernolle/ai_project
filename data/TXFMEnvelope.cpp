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
 * TXFMEnvelope := Class that calculates an Envelope with an XPath evaluator
 *
 */

#include <xsec/transformers/TXFMEnvelope.hpp>
#include <xsec/framework/XSECException.hpp>
#include <xsec/utils/XSECDOMUtils.hpp>

TXFMEnvelope::TXFMEnvelope(DOMDocument *doc) :
TXFMBase(doc) {


}

TXFMEnvelope::~TXFMEnvelope() {


}

// Methods to set the inputs

void TXFMEnvelope::setInput(TXFMBase *newInput) {

		if (newInput->getOutputType() != TXFMBase::DOM_NODES) {

		throw XSECException(XSECException::TransformInputOutputFail, "XPath requires DOM_NODES input type");

	}

	input = newInput;

	// Expand if necessary
	this->expandNameSpaces();

	keepComments = input->getCommentsStatus();

	// Set up for the new document
	mp_document = input->getDocument();

	// Now work out what we have to set up in the new processing

	TXFMBase::nodeType inputType = input->getNodeType();


	switch (inputType) {

	case DOM_NODE_DOCUMENT :

		mp_startNode = mp_document;
		break;

	case DOM_NODE_DOCUMENT_FRAGMENT :

		mp_startNode = input->getFragmentNode();
		break;

	default :

		throw XSECException(XSECException::EnvelopeError);	// Should never get here

	}

	// Ready to evaluate

}


// Methods to get tranform output type and input requirement

TXFMBase::ioType TXFMEnvelope::getInputType(void) {

	return TXFMBase::DOM_NODES;

}

TXFMBase::ioType TXFMEnvelope::getOutputType(void) {

	return TXFMBase::DOM_NODES;

}

TXFMBase::nodeType TXFMEnvelope::getNodeType(void) {

	return TXFMBase::DOM_NODE_XPATH_NODESET;

}

// Envelope (and XPath) unique

void addEnvelopeNode(DOMNode *startNode, XSECXPathNodeList & XPathMap, DOMNode * sigNode) {

	DOMNode *tmp;
	DOMNamedNodeMap *atts;
	int attsSize, i;
	
	if (startNode == sigNode)
		return;

	XPathMap.addNode(startNode);

	if (startNode->getNodeType() == DOMNode::ELEMENT_NODE) {

		atts = startNode->getAttributes();
		if (atts != NULL)
			attsSize = atts->getLength();
		else
			attsSize = 0;

		for (i = 0; i < attsSize; ++i) {

			tmp = atts->item(i);
			XPathMap.addNode(tmp);

		}

	}

	// Now do any childeren
	
	tmp = startNode->getFirstChild();

	while (tmp != NULL) {

		addEnvelopeNode(tmp, XPathMap, sigNode);
		tmp = tmp->getNextSibling();

	}
}


void TXFMEnvelope::evaluateEnvelope(DOMNode *t) {

	DOMNode *sigNode;

	// Find the signature node
	sigNode = t->getParentNode();

	while (sigNode != NULL && !strEquals(getDSIGLocalName(sigNode), "Signature"))
		sigNode = sigNode->getParentNode();

	if (sigNode == NULL) {

		throw XSECException(XSECException::EnvelopeError,
			"Unable to find signature owner of node passed to Envelope Transform");

	}

	addEnvelopeNode(mp_startNode, m_XPathMap, sigNode);

}

// Methods to get output data

unsigned int TXFMEnvelope::readBytes(XMLByte * const toFill, unsigned int maxToFill) {

	return 0;

}

DOMDocument *TXFMEnvelope::getDocument() {

	return mp_document;

}

DOMNode *TXFMEnvelope::getFragmentNode() {

	return NULL;

}

safeBuffer TXFMEnvelope::getFragmentId() {

	return safeBuffer("");	// Empty string

}

XSECXPathNodeList	& TXFMEnvelope::getXPathNodeList() {

	return m_XPathMap;

}
