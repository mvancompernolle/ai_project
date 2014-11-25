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
 * XSECDOMUtils:= Utilities to manipulate DOM within XML-SECURITY
 *					 
 *
 */

// XSEC

#include <xsec/utils/XSECDOMUtils.hpp>

// --------------------------------------------------------------------------------
//           Utilities to manipulate DSIG namespaces
// --------------------------------------------------------------------------------

const XMLCh * getDSIGLocalName(const DOMNode *node) {

	if (!strEquals(node->getNamespaceURI(), URI_ID_DSIG))
		return NULL; //DOMString("");
	else
		return node->getLocalName();

}

// --------------------------------------------------------------------------------
//           Find a nominated DSIG node in a document
// --------------------------------------------------------------------------------

DOMNode *findDSIGNode(DOMNode *n, char * nodeName) {

	const XMLCh * name = getDSIGLocalName(n);

	if (strEquals(name, nodeName)) {

		return n;

	}

	DOMNode *child = n->getFirstChild();

	while (child != NULL) {

		DOMNode *ret = findDSIGNode(child, nodeName);
		if (ret != NULL)
			return ret;
		child = child->getNextSibling();

	}

	return child;

}

// --------------------------------------------------------------------------------
//           Find particular type of node child
// --------------------------------------------------------------------------------

DOMNode *findFirstChildOfType(DOMNode *n, DOMNode::NodeType t) {

	DOMNode *c;

	if (n == NULL) 
		return n;

	c = n->getFirstChild();

	while (c != NULL && c->getNodeType() != t)
		c = c->getNextSibling();

	return c;

}

// --------------------------------------------------------------------------------
//           Make a QName
// --------------------------------------------------------------------------------

safeBuffer &makeQName(safeBuffer & qname, safeBuffer &prefix, char * localName) {

	if (prefix[0] == '\0') {
		qname = localName;
	}
	else {
		qname = prefix;
		qname.sbStrcatIn(":");
		qname.sbStrcatIn(localName);
	}

	return qname;

}

// --------------------------------------------------------------------------------
//           "Quick" Transcode (low performance)
// --------------------------------------------------------------------------------



XMLT::XMLT(const char * str) {

	mp_unicodeStr = XMLString::transcode(str);

}

XMLT::~XMLT (void) {

	delete[] mp_unicodeStr;

}

XMLCh * XMLT::getUnicodeStr(void) {
	
	return mp_unicodeStr;

}

