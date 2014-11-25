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
 * DSIGTransformXPath := Class that holds XPath transforms
 *
 */


#include <xsec/dsig/DSIGTransformXPath.hpp>
#include <xsec/transformers/TXFMXPath.hpp>
#include <xsec/framework/XSECException.hpp>
#include <xsec/transformers/TXFMC14n.hpp>
#include <xsec/framework/XSECError.hpp>
#include <xsec/dsig/DSIGSignature.hpp>

// --------------------------------------------------------------------------------
//           Constructors and Destructors
// --------------------------------------------------------------------------------

DSIGTransformXPath::DSIGTransformXPath(DSIGSignature *sig, DOMNode * node) :
DSIGTransform(sig, node) {

	mp_exprTextNode = NULL;
	mp_xpathNode = NULL;
	mp_NSMap = NULL;
	m_expr = "";

}
	

DSIGTransformXPath::DSIGTransformXPath(DSIGSignature *sig) :
DSIGTransform(sig) {

	mp_exprTextNode = NULL;
	mp_xpathNode = NULL;
	mp_NSMap = NULL;
	m_expr = "";
}
		  
DSIGTransformXPath::~DSIGTransformXPath() {};

// --------------------------------------------------------------------------------
//           Interface Methods
// --------------------------------------------------------------------------------
	
transformType DSIGTransformXPath::getTransformType() {

	return TRANSFORM_XPATH;

}

TXFMBase * DSIGTransformXPath::createTransformer(TXFMBase * input) {

#ifdef XSEC_NO_XPATH

	throw XSECException(XSECException::UnsupportedFunction,
		"XPath transforms are not supported in this compilation of the XSEC library");

#else

	TXFMXPath *x;
	// XPath transform
	XSECnew(x, TXFMXPath(mp_txfmNode->getOwnerDocument()));
	x->setInput(input);
	x->setNameSpace(mp_NSMap);
	x->evaluateExpr(mp_txfmNode, m_expr);

	return x;
	
#endif /* NO_XPATH */

}

DOMElement * DSIGTransformXPath::createBlankTransform(DOMDocument * parentDoc) {

	safeBuffer str;
	safeBuffer prefix;
	DOMElement *ret;
	DOMDocument *doc = mp_parentSignature->getParentDocument();

	prefix = mp_parentSignature->getDSIGNSPrefix();
	
	// Create the transform node
	makeQName(str, prefix, "Transform");
	ret = doc->createElementNS(DSIGConstants::s_unicodeStrURIDSIG, str.sbStrToXMLCh());
	ret->setAttribute(DSIGConstants::s_unicodeStrAlgorithm, DSIGConstants::s_unicodeStrURIXPATH);
	
	// Create the XPath element
	
	makeQName(str, prefix, "XPath");
	mp_xpathNode = doc->createElementNS(DSIGConstants::s_unicodeStrURIDSIG, str.sbStrToXMLCh());
	mp_exprTextNode = doc->createTextNode(MAKE_UNICODE_STRING(""));
	ret->appendChild(mp_xpathNode);
	mp_xpathNode->appendChild(mp_exprTextNode);

	mp_txfmNode = ret
		;
	return ret;

}

void DSIGTransformXPath::load(void) {

	// Find the XPath expression
	
	mp_xpathNode = mp_txfmNode->getFirstChild();

	while (mp_xpathNode != 0 && 
		mp_xpathNode->getNodeType() != DOMNode::ELEMENT_NODE && !strEquals(mp_xpathNode->getNodeName(), "XPath"))
		mp_xpathNode = mp_xpathNode->getNextSibling();
	
	if (mp_xpathNode == 0) {

		throw XSECException(XSECException::ExpectedDSIGChildNotFound,
			"Expected <XPath> Node in DSIGTransformXPath::load");
	}
	
	else {
		
		// Check for attributes - in particular any namespaces

		mp_NSMap = mp_xpathNode->getAttributes();
		
		// Find the text
		mp_exprTextNode = findFirstChildOfType(mp_xpathNode, DOMNode::TEXT_NODE);

		if (mp_exprTextNode == NULL) {
			throw XSECException(XSECException::ExpectedDSIGChildNotFound,
				"Expected Text Node in beneath <XPath> in DSIGTransformXPath::load");
		}

		m_expr << (*(mp_parentSignature->getSBFormatter()) << mp_exprTextNode->getNodeValue());
				
	}

}


// --------------------------------------------------------------------------------
//           XPath Transform Specific Methods
// --------------------------------------------------------------------------------

void DSIGTransformXPath::setExpression(const char * expr) {

	mp_exprTextNode->setNodeValue(MAKE_UNICODE_STRING(expr));

	m_expr.sbStrcpyIn((char *) expr);

}



const char * DSIGTransformXPath::getExpression(void) {

	return m_expr.rawCharBuffer();

}


void DSIGTransformXPath::setNamespace(const char * prefix, const char * value) {

	safeBuffer str;

	str.sbStrcpyIn("xmlns:");
	str.sbStrcatIn((char *) prefix);

	DOMElement *x;

	x = static_cast <DOMElement *> (mp_xpathNode);

//	if (x == NULL) {
//
//		throw XSECException(XSECException::TransformError,
//			"Found a non ELEMENT node as the XPath node in DSIGTransformXPath");
//	}

	x->setAttributeNS(DSIGConstants::s_unicodeStrURIXMLNS,
		str.sbStrToXMLCh(),
		MAKE_UNICODE_STRING(value));

	mp_NSMap = mp_xpathNode->getAttributes();


}

void DSIGTransformXPath::deleteNamespace(const char * prefix) {

	DOMElement *x;

	x = static_cast <DOMElement *> (mp_xpathNode);

//	if (x == NULL) {
//
//		throw XSECException(XSECException::TransformError,
//			"Found a non ELEMENT node as the XPath node in DSIGTransformXPath");
//	}

	x->removeAttributeNS(DSIGConstants::s_unicodeStrURIXMLNS,
		MAKE_UNICODE_STRING(prefix));

}

