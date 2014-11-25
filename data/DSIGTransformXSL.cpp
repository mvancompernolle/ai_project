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
 * DSIGTransformXSL := Class that Handles DSIG XSLT Transforms
 *
 */

// XSEC

#include <xsec/dsig/DSIGTransformXSL.hpp>
#include <xsec/dsig/DSIGSignature.hpp>
#include <xsec/transformers/TXFMXSL.hpp>
#include <xsec/transformers/TXFMC14n.hpp>
#include <xsec/framework/XSECException.hpp>
#include <xsec/utils/XSECDOMUtils.hpp>
#include <xsec/framework/XSECError.hpp>

// --------------------------------------------------------------------------------
//           Constructors and Destructors
// --------------------------------------------------------------------------------

DSIGTransformXSL::DSIGTransformXSL(DSIGSignature *sig, DOMNode * node) :
DSIGTransform(sig, node),
mp_stylesheetNode(NULL) {};


DSIGTransformXSL::DSIGTransformXSL(DSIGSignature *sig) :
DSIGTransform(sig),
mp_stylesheetNode(NULL) {};
		  

DSIGTransformXSL::~DSIGTransformXSL() {};

// --------------------------------------------------------------------------------
//           Interface Methods
// --------------------------------------------------------------------------------


transformType DSIGTransformXSL::getTransformType() {

	return TRANSFORM_XSLT;

}


TXFMBase * DSIGTransformXSL::createTransformer(TXFMBase * input) {


#ifdef XSEC_NO_XSLT

	throw XSECException(XSECException::UnsupportedFunction,
		"XSLT Transforms not supported in this compilation of the library");
#else

	if (mp_stylesheetNode == 0)
		throw XSECException(XSECException::XSLError, "Style Sheet not found for XSL Transform");


	TXFMBase * nextInput;

	// XSLT Transform - requires a byte stream input
	
	if (input->getOutputType() == TXFMBase::DOM_NODES) {
		
		// Use c14n to translate to BYTES
		
		XSECnew(nextInput, TXFMC14n(mp_txfmNode->getOwnerDocument()));
		nextInput->setInput(input);		
	}
	else {
		nextInput = input;

	}

	TXFMXSL * x;
	
	// Create the XSLT transform
	XSECnew(x, TXFMXSL(mp_txfmNode->getOwnerDocument()));
	x->setInput(nextInput);
	
	// Again use C14n (convenient) to translate to a SafeBuffer
	
	XSECC14n20010315 c14n(mp_txfmNode->getOwnerDocument(), mp_stylesheetNode);
	safeBuffer sbStyleSheet;
	unsigned int size, count;
	unsigned char buf[512];
	size = 0;
	
	while ((count = c14n.outputBuffer(buf, 512)) != 0) {
		
		sbStyleSheet.sbMemcpyIn(size, buf, count);
		size += count;
		
	}
	
	sbStyleSheet[size] = '\0';		// Terminate as though a string
	
	x->evaluateStyleSheet(sbStyleSheet);

	return x;

#endif /* NO_XSLT */

}


DOMElement * DSIGTransformXSL::createBlankTransform(DOMDocument * parentDoc) {

	safeBuffer str;
	safeBuffer prefix;
	DOMElement *ret;
	DOMDocument *doc = mp_parentSignature->getParentDocument();

	prefix = mp_parentSignature->getDSIGNSPrefix();
	
	// Create the transform node
	makeQName(str, prefix, "Transform");
	ret = doc->createElementNS(DSIGConstants::s_unicodeStrURIDSIG, str.sbStrToXMLCh());
	ret->setAttribute(DSIGConstants::s_unicodeStrAlgorithm, DSIGConstants::s_unicodeStrURIXSLT);

	mp_txfmNode = ret;
	mp_stylesheetNode = NULL;

	return ret;

}

void DSIGTransformXSL::load(void) {

	// find the style sheet
	mp_stylesheetNode = mp_txfmNode->getFirstChild();
	while (mp_stylesheetNode != 0 && 
		mp_stylesheetNode->getNodeType() != DOMNode::ELEMENT_NODE && !strEquals(mp_stylesheetNode->getNodeName(), "xsl:stylesheet"))
		mp_stylesheetNode = mp_stylesheetNode->getNextSibling();

	if (mp_stylesheetNode == 0)
		throw XSECException(XSECException::XSLError, "Style Sheet not found for XSL Transform");


}
// --------------------------------------------------------------------------------
//           XSLT Specific Methods
// --------------------------------------------------------------------------------

DOMNode * DSIGTransformXSL::setStylesheet(DOMNode * stylesheet) {

	DOMNode * ret = mp_stylesheetNode;

	if (mp_stylesheetNode != 0) {
		mp_txfmNode->insertBefore(stylesheet, mp_stylesheetNode);
		mp_txfmNode->removeChild(mp_stylesheetNode);
	}

	mp_stylesheetNode = stylesheet;

	return ret;

}

DOMNode * DSIGTransformXSL::getStylesheet(void) {

	return mp_stylesheetNode;

}
