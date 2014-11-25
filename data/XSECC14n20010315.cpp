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
 * XSECC14n20010315 := Canonicaliser object to process XML document in line with
 *					     RFC 3076
 *
 */

//XSEC includes
#include <xsec/canon/XSECC14n20010315.hpp>
#include <xsec/framework/XSECException.hpp>
#include <xsec/utils/XSECDOMUtils.hpp>

// Xerces includes
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>

#ifndef XSEC_NO_XALAN

// Xalan includes
#include <XalanDOM/XalanDocument.hpp>
#include <XercesParserLiaison/XercesDocumentWrapper.hpp>
#include <XercesParserLiaison/XercesDOMSupport.hpp>
#include <XercesParserLiaison/XercesParserLiaison.hpp>
#include <XPath/XPathEvaluator.hpp>
#include <XPath/NodeRefList.hpp>

// Namespace definitions
XALAN_USING_XALAN(XPathEvaluator)
XALAN_USING_XALAN(XercesDOMSupport)
XALAN_USING_XALAN(XercesParserLiaison)
XALAN_USING_XALAN(XalanDocument)
XALAN_USING_XALAN(XalanNode)
XALAN_USING_XALAN(XalanElement)
XALAN_USING_XALAN(XalanDOMString)
XALAN_USING_XALAN(XalanDOMChar)
XALAN_USING_XALAN(NodeRefList)
XALAN_USING_XALAN(XercesDocumentWrapper)
XALAN_USING_XALAN(XercesWrapperNavigator)


#endif

// General includes
#include <stdlib.h>
#include <string.h>
#include <iostream.h>


// --------------------------------------------------------------------------------
//           Some useful utilities
// --------------------------------------------------------------------------------

/*

  Removed - During conversion to DOMNode * (Xerces 2.1) 

XMLFormatter& operator<< (XMLFormatter& strm, const XMLCh *  s)
{
    unsigned int lent = s.length();

	if (lent <= 0)
		lent = 0;

    XMLCh*  buf = new XMLCh[lent + 1];
    if (lent > 0)
		XMLString::copyNString(buf, s.rawBuffer(), lent);
    buf[lent] = 0;
    strm << buf;
    delete [] buf;
    return strm;
}

*/

// Find a node in an XSECNodeList

bool NodeInList(const XSECNodeListElt * lst, const DOMNode * toFind) {

	const XSECNodeListElt * tmp = lst;

	while (tmp != NULL) {

		if (tmp->element == toFind)
			return true;

		tmp = tmp->next;

	}

	return false;

}

XSECNodeListElt * insertNodeIntoList(XSECNodeListElt * lst, XSECNodeListElt *toIns) {

	XSECNodeListElt *tmp, *last;

	if (lst == NULL) {
		
		// Goes at start
		toIns->next = NULL;
		toIns->last = NULL;
		
		return toIns;
		
	} /* if mp_attributes == NULL */
	
	// Need to run through start of list
	
	tmp = lst;
	last = NULL;
	int res = -1;   // Used to remove a gcc warning
	
	while ((tmp != NULL) &&
		((res = toIns->sortString.sbStrcmp(tmp->sortString)) >= 0)) {
		
		last = tmp;
		tmp = tmp->next;
		
	} /* while */

	if (res ==0) {

		// Already exists!
		delete toIns;
		return lst;

	}
	
	if (last == NULL) {
		
		// It sits before first element
		
		toIns->next = lst;
		toIns->last = NULL;
		lst->last = tmp;
				
		return toIns;
		
	} /* if last == NULL */
	
	// We have found where it goes
	
	toIns->next = tmp;
	toIns->last = last;
	
	if (tmp != NULL)
		tmp->last = toIns;
	
	last->next = toIns;
	
	return lst;
	
}
							
// --------------------------------------------------------------------------------
//           Exclusive Canonicalisation Methods
// --------------------------------------------------------------------------------
		

bool visiblyUtilises(DOMNode *node, safeBuffer &ns) {

	// Test whether the node uses the name space passed in
	
	if (strEquals(node->getPrefix(), (char *) ns.rawBuffer()))
		return true;

	// Check the attributes
	DOMNamedNodeMap *atts = node->getAttributes();
	if (atts == NULL)
		return false;

	int size = atts->getLength();

	for (int i = 0; i < size; ++i) {

		if (strEquals(atts->item(i)->getPrefix(), (char *) ns.rawBuffer()) &&
			!strEquals(atts->item(i)->getLocalName(), "xmlns"))
			return true;

	}

	return false;

}

bool XSECC14n20010315::inNonExclNSList(safeBuffer &ns) {

	int size = m_exclNSList.size();

	for (int i = 0; i < size; ++i) {

		if (strcmp((char *) ns.rawBuffer(), m_exclNSList[i]))
			return true;

	}

	return false;

}

void XSECC14n20010315::setExclusive(void) {

	m_exclusive = true;
	m_exclusiveDefault = true;

}

void XSECC14n20010315::setExclusive(char * xmlnsList) {

	char * nsBuf;

	setExclusive();

	// Set up the define non-exclusive prefixes

	nsBuf = new char [strlen(xmlnsList) + 1];
	
	if (nsBuf == NULL) {

		throw XSECException (XSECException::MemoryAllocationFail,
			"Error allocating a string buffer in XSECC14n20010315::setExclusive");

	}

	int i, j;

	i = 0;

	while (xmlnsList[i] != '\0') {

		while (xmlnsList[i] == ' ' ||
			   xmlnsList[i] == '\0' ||
			   xmlnsList[i] == '\t' || 
			   xmlnsList[i] == '\r' ||
			   xmlnsList[i] == '\n')

			   ++i;	// Skip white space

		j = 0;
		while (!(xmlnsList[i] == ' ' ||
			   xmlnsList[i] == '\0' ||
			   xmlnsList[i] == '\t' || 
			   xmlnsList[i] == '\r' ||
			   xmlnsList[i] == '\n'))

			   nsBuf[j++] = xmlnsList[i++];	// Copy name

		// Terminate the string
		nsBuf[j] = '\0';
		if (strcmp(nsBuf, "#default") == 0) {

			// Default is not to be exclusive
			m_exclusiveDefault = false;

		}

		else {

			// Add this to the list
			m_exclNSList.push_back(strdup(nsBuf));

		}

	}

}



		

// --------------------------------------------------------------------------------
//           XSECC14n20010315 methods
// --------------------------------------------------------------------------------

// Constructors

void XSECC14n20010315::init() {

	// This does the work of setting us up and checks to make sure everyhing is OK

	// Set up the Xerces formatter

	c14ntarget = new c14nFormatTarget();
	c14ntarget->setBuffer(&formatBuffer);

	formatter = new XMLFormatter("UTF-8", c14ntarget, XMLFormatter::NoEscapes, 
												XMLFormatter::UnRep_CharRef);

	// Set up for first attribute list

	mp_attributes = mp_currentAttribute = mp_firstNonNsAttribute = NULL;
	
	// By default process comments
	m_processComments = true;

	// Set up for tree walking

	m_returnedFromChild = false;
	mp_firstElementNode = mp_startNode;
	m_firstElementProcessed = false;

	// XPath setup
	m_XPathSelection = false;
	m_XPathMap.clear();

	// Exclusive Canonicalisation setup

	m_exclNSList.clear();
	m_exclusive = false;
	m_exclusiveDefault = false;

}


XSECC14n20010315::XSECC14n20010315() {};
XSECC14n20010315::XSECC14n20010315(DOMDocument *newDoc) : XSECCanon(newDoc) {

	// Just call the init function;

	init();	

};
XSECC14n20010315::XSECC14n20010315(DOMDocument *newDoc, 
								   DOMNode *newStartNode) : XSECCanon(newDoc, newStartNode) {

	// Just call the init function

	init();

}

XSECC14n20010315::~XSECC14n20010315() {};

// --------------------------------------------------------------------------------
//           XSECC14n20010315 Comments procesing
// --------------------------------------------------------------------------------


void XSECC14n20010315::setCommentsProcessing(bool onoff) {

	m_processComments = onoff;

}

bool XSECC14n20010315::getCommentsProcessing(void) {

	return m_processComments;

}


// --------------------------------------------------------------------------------
//           XSECC14n20010315 XPathSelectNodes method
// --------------------------------------------------------------------------------

// Use an XPath expression to select a subset of nodes from the document


int XSECC14n20010315::XPathSelectNodes(const char * XPathExpr) {

#ifdef XSEC_NO_XPATH

	throw XSECException(XSECException::UnsupportedFunction,
		"This library has been compiled without XPath support");

#else

	XPathEvaluator::initialize();

	// We use Xalan to process the Xerces DOM tree and get the XPath nodes
	
	XercesDOMSupport theDOMSupport;
	XercesParserLiaison theParserLiaison(theDOMSupport);

	XalanDocument* theDoc = theParserLiaison.createDocument(mp_doc);

	XalanElement * xe = theDoc->createElement(XalanDOMString("ns"));
	xe->setAttribute(/*XalanDOMString(""), */XalanDOMString("xmlns:ietf"), XalanDOMString("http://www.ietf.org"));

	// Set up the XPath evaluator
	
	XPathEvaluator	theEvaluator;

	// OK, let's find the context node...

	XalanDOMString cd = XalanDOMString("/");	// For the moment assume the root is the context

	const XalanDOMChar * cexpr = cd.c_str();

	XalanNode* const	theContextNode =
		theEvaluator.selectSingleNode(
		theDOMSupport,
		theDoc,
		cexpr,
		theDoc->getDocumentElement());

	if (theContextNode == 0)
	{
		
		// No appropriate nodes.
		return 0;
	
	}
	// OK, let's evaluate the expression...

	XalanDOMString ed = XalanDOMString(XPathExpr);
	const XalanDOMChar * expr = ed.c_str();

	NodeRefList	theResult(
		theEvaluator.selectNodeList(
		theDOMSupport,
		theContextNode,
		expr,
		xe));
		//theDoc->getDocumentElement()));

		
	//XercesDocumentBridge *theBridge = theParserLiaison.mapDocument(theDoc);
	XercesDocumentWrapper *theWrapper = theParserLiaison.mapDocumentToWrapper(theDoc);
	XercesWrapperNavigator theWrapperNavigator(theWrapper);

	int size = theResult.getLength();
	const DOMNode *item;
	
	for (int i = 0; i < size; ++ i) {
		
		item = theWrapperNavigator.mapNode(theResult.item(i));
		m_XPathMap.addNode(item);
		//tmp->element = theBridgeNavigator.mapNode(theResult.item(i));
	}

	m_XPathSelection = true;
	return size;

#endif

}

void XSECC14n20010315::setXPathMap(const XSECXPathNodeList & map) {

	// XPath already done!

	m_XPathMap = map;
	m_XPathSelection = true;

}

// --------------------------------------------------------------------------------
//           XSECC14n20010315 processNextNode method
// --------------------------------------------------------------------------------

safeBuffer c14nCleanText(safeBuffer &input) {

	/* c14n Requires :

		& -> &amp
		< -> &lt
		> -> &gt
		LF -> &#xD

	*/

	int len = input.sbStrlen();
	safeBuffer ret;

	int i, j;
	unsigned char c;

	j = 0;
	for (i = 0; i < len; ++i) {

		c = input[i];

		switch (c) {

		case '&' : 

			ret[j++] = '&';
			ret[j++] = 'a';
			ret[j++] = 'm';
			ret[j++] = 'p';
			ret[j++] = ';';

			break;

		case '<' :

			ret[j++] = '&';
			ret[j++] = 'l';
			ret[j++] = 't';
			ret[j++] = ';';

			break;
			
		case '>' :

			ret[j++] = '&';
			ret[j++] = 'g';
			ret[j++] = 't';
			ret[j++] = ';';

			break;
			
		case 0xD :

			ret[j++] = '&';
			ret[j++] = '#';
			ret[j++] = 'x';
			ret[j++] = 'D';
			ret[j++] = ';';

			break;

		default :

			ret[j++] = c;

		}

	}

	// final character:

	ret[j] = '\0';

	return ret;

}

safeBuffer c14nCleanAttribute(safeBuffer &input) {

	/* c14n Requires :

		& -> &amp
		< -> &lt
		" -> &quot
		#x9 ->&#x9
		#xA ->&#xA
		LF -> &#xD

	*/

	int len = input.sbStrlen();
	safeBuffer ret;

	int i, j;
	unsigned char c;

	j = 0;
	for (i = 0; i < len; ++i) {

		c = input[i];

		switch (c) {

		case '&' :

			ret[j++] = '&';
			ret[j++] = 'a';
			ret[j++] = 'm';
			ret[j++] = 'p';
			ret[j++] = ';';

			break;

		case '"' :

			ret[j++] = '&';
			ret[j++] = 'q';
			ret[j++] = 'u';
			ret[j++] = 'o';
			ret[j++] = 't';
			ret[j++] = ';';

			break;

		case '<' :

			ret[j++] = '&';
			ret[j++] = 'l';
			ret[j++] = 't';
			ret[j++] = ';';

			break;
				
		case 0x9 :

			ret[j++] = '&';
			ret[j++] = '#';
			ret[j++] = 'x';
			ret[j++] = '9';
			ret[j++] = ';';

			break;
	
		case 0xA :

			ret[j++] = '&';
			ret[j++] = '#';
			ret[j++] = 'x';
			ret[j++] = 'A';
			ret[j++] = ';';

			break;
	
		case 0xD :

			ret[j++] = '&';
			ret[j++] = '#';
			ret[j++] = 'x';
			ret[j++] = 'D';
			ret[j++] = ';';

			break;

		default :

			ret[j++] = c;

		}

	}

	// final character:

	ret[j] = '\0';

	return ret;

}

bool XSECC14n20010315::checkRenderNameSpaceNode(DOMNode *e, DOMNode *a) {

	DOMNode *parent;
	DOMNode *att;
	DOMNamedNodeMap *atts;

	// If XPath and node not selected, then never print
	if (m_XPathSelection && ! m_XPathMap.hasNode(a))
		return false;

	// First - are we exclusive?

	safeBuffer localName;
	bool processAsExclusive = false;

	if (m_exclusive) {

		if (strEquals(a->getNodeName(), "xmlns")) {
			processAsExclusive = m_exclusiveDefault;
		}
		else {
			formatBuffer[0] = '\0';
			*formatter << e->getLocalName();
			localName.sbStrcpyIn(formatBuffer);

			processAsExclusive = !inNonExclNSList(localName);
		}

	}

	if (processAsExclusive) {

		// Is the parent in the  node-set?
		if (m_XPathSelection && !m_XPathMap.hasNode(e))
			return false;

		// Is the name space visibly utilised?
		formatBuffer[0] = '\0';
		*formatter << a->getLocalName();
		localName.sbStrcpyIn(formatBuffer);

		if (localName.sbStrcmp("xmlns") == 0)
			localName[0] = '\0';			// Is this correct or should Xerces return "" for default?

		if (!visiblyUtilises(e, localName))
			return false;

		// Make sure previous nodes do not use the name space (and have it printed)
		parent = e->getParentNode();

		while (parent != NULL) {

			if (!m_XPathSelection || m_XPathMap.hasNode(parent)) {
				
				// An output ancestor
				if (visiblyUtilises(parent, localName)) {

					// Have a hit!
					atts = parent->getAttributes();
					att = atts->getNamedItem(a->getNodeName());
					if (att != NULL && (!m_XPathSelection || m_XPathMap.hasNode(att))) {

						// Check URI is the same
						if (strEquals(att->getNodeValue(), a->getNodeValue()))
							return false;

						return true;

					}

					return true;
				}
			}

			parent = parent->getParentNode();

		}

		// Didn't find it rendered!
		return true;

	}
	
	// Either we are now in non-exclusive mode, or the name space in question
	// Is to be treated as non-exclusive

	// Never directly render a default
	if (strEquals(a->getNodeName(), "xmlns") && strEquals(a->getNodeValue(), ""))
		return false;

	// Otherwise, of node is at base of selected document, then print
	if (e == mp_firstElementNode)
		return true;

	// Find the parent and check if the node is already defined or if the node
	// was out of scope
	parent = e->getParentNode();
	if (m_XPathSelection && !m_XPathMap.hasNode(parent))
		return true;

	DOMNamedNodeMap *pmap = parent->getAttributes();

	DOMNode *pns = pmap->getNamedItem(a->getNodeName());

	if (pns != NULL) {

		if (m_XPathSelection && !m_XPathMap.hasNode(pns))
			return true;			// Not printed in previous node

		if (strEquals(pns->getNodeValue(), a->getNodeValue()))
			return false;
		else
			return true;		// Was defined but differently

	}

	return true;			// Not defined in previous node

}



	// Check an attribute to see if we should output


// This is the main worker function of this class

int XSECC14n20010315::processNextNode() {

	// The information currently in the buffer has all been used.  We now process the
	// next node.

	DOMNode *next;				// For working (had *ns)
	DOMNamedNodeMap *tmpAtts;	//  "     "
	safeBuffer currentName(128), currentValue(1024), sbWork;
	bool done, xmlnsFound;


	if (m_allNodesDone) {

		return 0;					// No bytes copied because nothing more to be done

	}

	// Always zeroise buffers to make work simpler
	formatBuffer[0] = '\0';
	m_bufferLength = m_bufferPoint = 0;
	m_buffer.sbStrcpyIn("");

	// Find out if this is a node to process
	bool processNode;
	int nodeT;

	if (mp_nextNode == 0) {

		// Dummy element - we need to insert a default namespace

		nodeT = DOMNode::ATTRIBUTE_NODE;
		processNode = true;

	}
	else {

		processNode = ((!m_XPathSelection) || (m_XPathMap.hasNode(mp_nextNode)));
		nodeT = mp_nextNode->getNodeType();

	}

	switch (nodeT) {

	case DOMNode::DOCUMENT_NODE : // Start of a document

		// Check if finished
		if (m_returnedFromChild) {

			// All done!
			m_allNodesDone = true;
			
			return 0;

		}

		// In c14n we don't actually do anything for a document node except
		// process the childeren

		mp_firstElementNode = ((DOMDocument *) mp_nextNode)->getDocumentElement();
		next = mp_nextNode->getFirstChild();

		if (next == NULL) {

			// Empty document?
			m_allNodesDone = true;
			
		}

		mp_nextNode = next;
		m_bufferLength = m_bufferPoint = 0;		// To ensure nobody copies "nothing"

		return 0;

	case DOMNode::DOCUMENT_TYPE_NODE : // Ignore me

		m_returnedFromChild = true;
		m_buffer.sbStrcpyIn("");
		break;

	case DOMNode::PROCESSING_INSTRUCTION_NODE : // Just print

		if (processNode) {
			if ((mp_nextNode->getParentNode() == mp_doc) && m_firstElementProcessed) {
				
				// this is a top level node and first element done
				m_buffer.sbStrcpyIn("\x00A<?");
				
			}
			else
				m_buffer.sbStrcpyIn("<?");
			
			*formatter << mp_nextNode->getNodeName();
			m_buffer.sbStrcatIn(formatBuffer);
			
			//*formatter << mp_nextNode.getNodeValue();
			formatBuffer[0] = '\0';
			*formatter << ((DOMProcessingInstruction *) mp_nextNode)->getData();
			if (formatBuffer.sbStrlen() > 0) {
				m_buffer.sbStrcatIn(" ");
				m_buffer.sbStrcatIn(formatBuffer);
			}
			
			m_buffer.sbStrcatIn("?>");
			
			if ((mp_nextNode->getParentNode() == mp_doc) && !m_firstElementProcessed) {
				
				// this is a top level node and first element done
				m_buffer.sbStrcatIn("\x00A");
				
			}
		}

		// Node fully processed
		m_returnedFromChild = true;

		break;


	case DOMNode::COMMENT_NODE : // Just print out

		if (processNode && m_processComments) {
			if ((mp_nextNode->getParentNode() == mp_doc) && m_firstElementProcessed) {
				
				// this is a top level node and first element done
				m_buffer.sbStrcpyIn("\x00A<!--");
				
			}
			else
				m_buffer.sbStrcpyIn("<!--");
			
			formatBuffer[0] = '\0';
			*formatter << mp_nextNode->getNodeValue();

			if (formatBuffer.sbStrlen() > 0) {
				m_buffer.sbStrcatIn(formatBuffer);
			}
			
			m_buffer.sbStrcatIn("-->");
			
			if ((mp_nextNode->getParentNode() == mp_doc) && !m_firstElementProcessed) {
				
				// this is a top level node and first element done
				m_buffer.sbStrcatIn("\x00A");
				
			}
		}

		m_returnedFromChild = true;	// Fool the tree processor

		break;

	case DOMNode::CDATA_SECTION_NODE :
	case DOMNode::TEXT_NODE : // Straight copy for now

		if (processNode) {
			*formatter << mp_nextNode->getNodeValue();

			// Do c14n cleaning on the text string

			m_buffer = c14nCleanText(formatBuffer);

		}

		// Fall through
		m_returnedFromChild = true;		// Fool the tree processor
		break;

	case DOMNode::ELEMENT_NODE : // This is an element that we can easily process

		// If we are going "up" then we simply close off the element

		if (m_returnedFromChild) {
			if (processNode) {		
				m_buffer.sbStrcpyIn ("</");
				*formatter << mp_nextNode->getNodeName();
				m_buffer.sbStrcatIn(formatBuffer);
				m_buffer.sbStrcatIn(">");
			}

			break;
		}

		if (processNode) {	

			m_buffer.sbStrcpyIn("<");
			*formatter << mp_nextNode->getNodeName();
			m_buffer.sbStrcatIn(formatBuffer);
		}

		// We now set up for attributes and name spaces
		mp_attributes = NULL;
		tmpAtts = mp_nextNode->getAttributes();
		next = mp_nextNode;

		done = false;
		xmlnsFound = false;

		while (!done) {

			// Need to sort the attributes

			int size;

			if (tmpAtts != NULL)
				size = tmpAtts->getLength();
			else 
				size = 0;

			XSECNodeListElt *toIns;
			int i;

			for (i = 0; i < size; ++i) {

				// Get the name and value of the attribute
				formatBuffer[0] = '\0';
				*formatter << tmpAtts->item(i)->getNodeName();
				currentName.sbStrcpyIn(formatBuffer);

				formatBuffer[0] = '\0';
				*formatter << tmpAtts->item(i)->getNodeValue();
				currentValue.sbStrcpyIn(formatBuffer);

				// Build the string used to sort this node
				
				if ((next == mp_nextNode) && currentName.sbStrncmp("xmlns", 5) == 0) {

					// Is this the default?
					if (currentName.sbStrcmp("xmlns") == 0 &&
						(!m_XPathSelection || m_XPathMap.hasNode(tmpAtts->item(i))) &&
						!currentValue.sbStrcmp("") == 0)
						xmlnsFound = true;

					// A namespace node - See if we need to output
					if (checkRenderNameSpaceNode(mp_nextNode, tmpAtts->item(i))) {

						// Add to the list
				
						*formatter << tmpAtts->item(i)->getNodeName();
						if (formatBuffer[5] == ':')
							currentName.sbStrcpyIn((char *) &formatBuffer[6]);
						else
							currentName.sbStrcpyIn("");
				
						toIns = new XSECNodeListElt;
						toIns->element = tmpAtts->item(i);
				
						// Build and insert name space node
						toIns->sortString.sbStrcpyIn(XMLNS_PREFIX);
						toIns->sortString.sbStrcatIn(currentName);
				
						// Insert node
						mp_attributes = insertNodeIntoList(mp_attributes, toIns);
				
					}

				}

				else {
					
					// A "normal" attribute - only process if selected or no XPath or is an
					// XML node from a previously un-printed Element node

					bool XMLElement = (next != mp_nextNode) && !m_exclusive && !currentName.sbStrncmp("xml:", 4);

					// If we have an XML element, make sure it was not printed between this
					// node and the node currently  being worked on

					if (XMLElement) {

						DOMNode *t = mp_nextNode->getParentNode();
						if (m_XPathSelection && m_XPathMap.hasNode(t))
							XMLElement = false;
					
					}

						 
					
					if ((!m_XPathSelection && next == mp_nextNode) || XMLElement || ((next == mp_nextNode) && m_XPathMap.hasNode(tmpAtts->item(i)))) { 
						
						toIns = new XSECNodeListElt;
						toIns->element = tmpAtts->item(i);
						
						// First the correct prefix						
						toIns->sortString.sbStrcpyIn(ATTRIBUTE_PREFIX);
						
						// Now break down the name to see if it has a namespace
						int len = currentName.sbStrlen();
						int k;
						
						for (k = 0; k < len; ++k) {
							
							if (currentName[k] == ':')
								break;
							
						}

						safeBuffer	tsb, tsc;

						tsb.sbStrcpyIn("xmlns:");
						
						if (k < len) {
							
							// Have a namespace delcleration
							
							
							tsc.sbStrncpyIn(currentName, k);
							tsb.sbStrcatIn(tsc);
							
							
						}
						// find the URL matching identifier tsb
						XMLCh * tsbXMLCh = XMLString::transcode(tsb.rawCharBuffer());
						DOMNode *nsd = mp_nextNode->getAttributes()->getNamedItem(tsbXMLCh);
						delete[] tsbXMLCh;

						if (nsd != NULL) {
							// Found a namespace!
							*formatter << nsd->getNodeValue();
							toIns->sortString.sbStrcatIn(formatBuffer);
							toIns->sortString.sbStrcatIn(":");
						}

						// Should throw!

						// Append rest of name.  Don't need value as name should be unique
						toIns->sortString.sbStrcatIn(currentName);
						
						// Insert node
						mp_attributes = insertNodeIntoList(mp_attributes, toIns);
						
					} /* else (sbStrCmp xmlns) */
				}
			} /* for */
#if 1
			// Now go upwards and find parent for xml name spaces
			if (processNode && (m_XPathSelection || mp_nextNode == mp_firstElementNode)) {
				next = next->getParentNode();

				if (next == 0) // || NodeInList(mp_XPathMap, next))
					done = true;
				else
					tmpAtts = next->getAttributes();
			}
			else
#endif
				done = true;

		} /* while tmpAtts != NULL */

		// Check to see if we add xmlns=""
		if (processNode && !xmlnsFound && mp_nextNode != mp_firstElementNode) {

			// Is this exclusive?

			safeBuffer sbLocalName("");

			if (m_exclusiveDefault) {

				if (visiblyUtilises(mp_nextNode, sbLocalName)) {

					// May have to output!
					next = mp_nextNode->getParentNode();

					while (next != NULL) {

						if (!m_XPathSelection || m_XPathMap.hasNode(next)) {

							DOMNode *tmpAtt;
				
							// An output ancestor
							if (visiblyUtilises(next, sbLocalName)) {

							// Have a hit!
								tmpAtts = next->getAttributes();
								if (tmpAtts != NULL)
									tmpAtt = tmpAtts->getNamedItem(DSIGConstants::s_unicodeStrXmlns);
								if (tmpAtts != NULL && tmpAtt != NULL && (!m_XPathSelection || m_XPathMap.hasNode(tmpAtt))) {

									// Check URI is the same
									if (!strEquals(tmpAtt->getNodeValue(), "")) {
										xmlnsFound = true;
									}
								}
							}
						}

						next = next->getParentNode();
					}

				}
			} /* m_exclusiveDefault */

			else {

				//DOM_Node next;

				next = mp_nextNode->getParentNode();
				while (next != NULL && (m_XPathSelection && !m_XPathMap.hasNode(next)))
					next = next->getParentNode();

				int size;
				if (next != NULL)
					tmpAtts = next->getAttributes();
				
				if (next != NULL && tmpAtts != NULL)
					size = tmpAtts->getLength();
				else
					size = 0;

				for (int i = 0; i < size; ++i) {

					formatBuffer[0] = '\0';
					*formatter << tmpAtts->item(i)->getNodeName();
					currentName.sbStrcpyIn(formatBuffer);

					formatBuffer[0] = '\0';
					*formatter << tmpAtts->item(i)->getNodeValue();
					currentValue.sbStrcpyIn(formatBuffer);

					if ((currentName.sbStrcmp("xmlns") == 0) &&
						(currentValue.sbStrcmp("") != 0) &&
						(!m_XPathSelection || m_XPathMap.hasNode(tmpAtts->item(i))))
						xmlnsFound = true;

				}
			}

			// Did we find a non empty namespace?
			if (xmlnsFound) {
				
				currentName.sbStrcpyIn("xmlns");
				XSECNodeListElt * toIns;
		
				toIns = new XSECNodeListElt;
				toIns->element = NULL;		// To trigger the state engine
		
				// Build and insert name space node
				toIns->sortString.sbStrcpyIn(XMLNS_PREFIX);
				toIns->sortString.sbStrcatIn(currentName);
		
				// Insert node
				mp_attributes = insertNodeIntoList(mp_attributes, toIns);
			}
		}


		if (mp_attributes != NULL) {

			// Now we have set up the attribute list, set next node and return!

			mp_attributeParent = mp_nextNode;
			mp_nextNode = mp_attributes->element;
			mp_currentAttribute = mp_attributes;
			m_bufferLength = m_buffer.sbStrlen();
			m_bufferPoint = 0;

			return m_bufferLength;

		} /* attrributes != NULL */
		

		if (processNode)
			m_buffer.sbStrcatIn(">");
	
		// Fall through to find next node

		break;

	case DOMNode::ATTRIBUTE_NODE : // Output attr_name="value"

		// Always process an attribute node as we have already checked they should
		// be printed
		
		m_buffer.sbStrcpyIn(" ");

		if (mp_nextNode != 0) {
			
			*formatter << mp_nextNode->getNodeName();
			m_buffer.sbStrcatIn(formatBuffer);
				
			m_buffer.sbStrcatIn("=\"");
				
			formatBuffer[0] = '\0';
			*formatter << mp_nextNode->getNodeValue();
			sbWork = c14nCleanAttribute(formatBuffer);
			m_buffer.sbStrcatIn(sbWork);
				
			m_buffer.sbStrcatIn("\"");
		}
		else {
			m_buffer.sbStrcatIn("xmlns");
			m_buffer.sbStrcatIn("=\"");
			m_buffer.sbStrcatIn("\"");
		}
	

		// Now see if next node is an attribute

		mp_currentAttribute = mp_currentAttribute->next;
		if (mp_currentAttribute != NULL) {

			// Easy case
			mp_nextNode = mp_currentAttribute->element;
			m_bufferLength = m_buffer.sbStrlen();
			m_bufferPoint = 0;

			return m_bufferLength;


		} /* if mp_currentAttributes != NULL) */

		// need to clear out the node list
		while (mp_attributes != NULL) {

			mp_currentAttribute = mp_attributes->next;
			delete mp_attributes;
			mp_attributes = mp_currentAttribute;
		}

		mp_attributes = mp_currentAttribute = mp_firstNonNsAttribute = NULL;

		// return us to the element node
		mp_nextNode = mp_attributeParent;

		// End the element definition
		if (!m_XPathSelection || (m_XPathMap.hasNode(mp_nextNode))) 
			m_buffer.sbStrcatIn(">");

		m_returnedFromChild = false;


		break;

	default:

		break;

	}

	// A node has fallen through to the default case for finding the next node.

	m_bufferLength = m_buffer.sbStrlen();;
	m_bufferPoint = 0;
	
	// Firstly, was the last piece of processing because we "came up" from a child node?
	
	if (m_returnedFromChild) {

		if (mp_nextNode == mp_startNode) {

			// we have closed off the document!
			m_allNodesDone = true;
			return m_bufferLength;

		}

		if (mp_nextNode == mp_firstElementNode) {

			// we have closed off the main mp_doc elt
			m_firstElementProcessed = true;
			
		}
		
	}

	else {

		// Going down - so check for children nodes
		next = mp_nextNode->getFirstChild();

		if (next != NULL)

			mp_nextNode = next;
			
		else

			// No children, so need to close this node off!
			m_returnedFromChild = true;

		return m_bufferLength;

	}

	// If we get here all childeren (if there are any) are done

	next = mp_nextNode->getNextSibling();
	if (next != NULL) {

		m_returnedFromChild = false;
		mp_nextNode = next;
		return m_bufferLength;

	}

	// No more nodes at this level either!

	mp_nextNode = mp_nextNode->getParentNode();
	m_returnedFromChild = true;
	return m_bufferLength;

}
