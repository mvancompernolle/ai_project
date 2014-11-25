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
 * DSIG_Reference := Class for handling a DSIG reference element
 *
 */

// Xerces

#include <xercesc/util/XMLNetAccessor.hpp>

// XSEC includes

#include <xsec/dsig/DSIGReference.hpp>
#include <xsec/transformers/TXFMURL.hpp>
#include <xsec/transformers/TXFMDocObject.hpp>
#include <xsec/transformers/TXFMOutputFile.hpp>
#include <xsec/transformers/TXFMSHA1.hpp>
#include <xsec/transformers/TXFMBase64.hpp>
#include <xsec/transformers/TXFMSB.hpp>
#include <xsec/transformers/TXFMXPath.hpp>
#include <xsec/transformers/TXFMC14n.hpp>
#include <xsec/transformers/TXFMXSL.hpp>
#include <xsec/transformers/TXFMEnvelope.hpp>
#include <xsec/dsig/DSIGConstants.hpp>
#include <xsec/dsig/DSIGSignature.hpp>
#include <xsec/dsig/DSIGTransformList.hpp>
#include <xsec/dsig/DSIGTransformBase64.hpp>
#include <xsec/dsig/DSIGTransformEnvelope.hpp>
#include <xsec/dsig/DSIGTransformXPath.hpp>
#include <xsec/dsig/DSIGTransformXSL.hpp>
#include <xsec/dsig/DSIGTransformC14n.hpp>

#include <xsec/framework/XSECError.hpp>
#include <xsec/utils/XSECPlatformUtils.hpp>
#include <xsec/utils/XSECDOMUtils.hpp>
#include <xsec/utils/XSECBinTXFMInputStream.hpp>

#include <iostream.h>


// --------------------------------------------------------------------------------
//           Constructors and Destructors
// --------------------------------------------------------------------------------


DSIGReference::DSIGReference(DSIGSignature * sig, DOMNode *dom) : m_URI("") {

	mp_referenceNode = dom;
	mp_parentSignature = sig;

	// Should throw an exception if the node is not a REFERENCE element

	XSECnew(mp_formatter, XSECSafeBufferFormatter("UTF-8",XMLFormatter::NoEscapes, 
												XMLFormatter::UnRep_CharRef));

	mp_preHash = NULL;
	mp_manifestList = NULL;
	me_hashMethod = HASH_NONE;
	mp_transformsNode = NULL;
	mp_transformList = NULL;
	m_URISet = m_isManifest = false;

}

DSIGReference::DSIGReference(DSIGSignature *sig) : m_URI("") {

	mp_parentSignature = sig;
	mp_referenceNode = NULL;
	mp_transformsNode = NULL;
	mp_transformList = NULL;

	XSECnew(mp_formatter, XSECSafeBufferFormatter("UTF-8",XMLFormatter::NoEscapes, 
											XMLFormatter::UnRep_CharRef));

	mp_preHash = NULL;		// By default no "special" transform
	mp_manifestList = NULL;
	me_hashMethod = HASH_NONE;
	m_URISet = m_isManifest = false;

};

DSIGReference::~DSIGReference() {

	// Destroy any associated transforms

	if (mp_transformList != NULL) {

		delete mp_transformList;
		mp_transformList = NULL;

	}

};

// --------------------------------------------------------------------------------
//           Creation of Transforms
// --------------------------------------------------------------------------------

void DSIGReference::createTransformList(void) {

	// Creates the transforms list
	safeBuffer str;
	safeBuffer prefix;
	DOMDocument *doc = mp_parentSignature->getParentDocument();

	prefix = mp_parentSignature->getDSIGNSPrefix();
	
	if (mp_transformsNode == NULL) {

		// Need to create a transforms node
		makeQName(str, prefix, "Transforms");
		mp_transformsNode = doc->createElementNS(DSIGConstants::s_unicodeStrURIDSIG, str.sbStrToXMLCh());
		mp_referenceNode->insertBefore(mp_transformsNode, mp_referenceNode->getFirstChild());
		mp_referenceNode->insertBefore(doc->createTextNode(DSIGConstants::s_unicodeStrNL), mp_transformsNode);
		mp_transformsNode->appendChild(doc->createTextNode(DSIGConstants::s_unicodeStrNL));

		// Create the list
		XSECnew(mp_transformList, DSIGTransformList());
	}

}

void DSIGReference::addTransform(DSIGTransform * txfm, DOMElement * txfmElt) {

	DOMDocument *doc = mp_parentSignature->getParentDocument();

	if (mp_transformList == NULL)
		createTransformList();

	mp_transformsNode->appendChild(txfmElt);
	mp_transformsNode->appendChild(doc->createTextNode(DSIGConstants::s_unicodeStrNL));

	mp_transformList->addTransform(txfm);
}


DSIGTransformEnvelope * DSIGReference::appendEnvelopedSignatureTransform() {

	DOMElement *txfmElt;
	DSIGTransformEnvelope * txfm;

	XSECnew(txfm, DSIGTransformEnvelope(mp_parentSignature));
	txfmElt = txfm->createBlankTransform(mp_parentSignature->getParentDocument());

	addTransform(txfm, txfmElt);

	return txfm;

}

DSIGTransformBase64 * DSIGReference::appendBase64Transform() {

	DOMElement *txfmElt;
	DSIGTransformBase64 * txfm;

	XSECnew(txfm, DSIGTransformBase64(mp_parentSignature));
	txfmElt = txfm->createBlankTransform(mp_parentSignature->getParentDocument());

	addTransform(txfm, txfmElt);

	return txfm;

}

DSIGTransformXSL * DSIGReference::appendXSLTransform(DOMNode * stylesheet) {

	DOMElement *txfmElt;
	DSIGTransformXSL * txfm;

	XSECnew(txfm, DSIGTransformXSL(mp_parentSignature));
	txfmElt = txfm->createBlankTransform(mp_parentSignature->getParentDocument());
	txfm->setStylesheet(stylesheet);

	addTransform(txfm, txfmElt);

	return txfm;

}

DSIGTransformC14n * DSIGReference::appendCanonicalizationTransform(canonicalizationMethod cm) {

	DOMElement *txfmElt;
	DSIGTransformC14n * txfm;

	XSECnew(txfm, DSIGTransformC14n(mp_parentSignature));
	txfmElt = txfm->createBlankTransform(mp_parentSignature->getParentDocument());
	txfm->setCanonicalizationMethod(cm);

	addTransform(txfm, txfmElt);

	return txfm;

}	

DSIGTransformXPath * DSIGReference::appendXPathTransform(char * expr) {

	DOMElement *txfmElt;
	DSIGTransformXPath * txfm;

	XSECnew(txfm, DSIGTransformXPath(mp_parentSignature));
	txfmElt = txfm->createBlankTransform(mp_parentSignature->getParentDocument());
	txfm->setExpression(expr);

	addTransform(txfm, txfmElt);

	return txfm;
}

// --------------------------------------------------------------------------------
//           Creation of blanks
// --------------------------------------------------------------------------------


DOMElement *DSIGReference::createBlankReference(char * URI, hashMethod hm, char * type) {

	// Reset this Reference just in case
	
	m_URI = URI;
	me_hashMethod = hm;
	m_URISet = true;
	m_isManifest = false;
	mp_preHash = NULL;
	mp_manifestList = NULL;
	mp_transformsNode = NULL;
	mp_transformList = NULL;

	safeBuffer str;
	DOMDocument *doc = mp_parentSignature->getParentDocument();
	safeBuffer prefix = mp_parentSignature->getDSIGNSPrefix();

	makeQName(str, prefix, "Reference");

	DOMElement *ret = doc->createElementNS(DSIGConstants::s_unicodeStrURIDSIG, str.sbStrToXMLCh());
	mp_referenceNode = ret;

	// Set type
	if (type != NULL)
		ret->setAttribute(MAKE_UNICODE_STRING("type"),
			MAKE_UNICODE_STRING(type));

	// Set URI
	ret->setAttribute(MAKE_UNICODE_STRING("URI"), MAKE_UNICODE_STRING(URI));

	// Create hash and hashValue nodes
	makeQName(str, prefix, "DigestMethod");
	DOMElement *digestMethod = doc->createElementNS(DSIGConstants::s_unicodeStrURIDSIG, str.sbStrToXMLCh());
	ret->appendChild(doc->createTextNode(DSIGConstants::s_unicodeStrNL));
	ret->appendChild(digestMethod);
	ret->appendChild(doc->createTextNode(DSIGConstants::s_unicodeStrNL));

	if (!hashMethod2URI(str, hm)) {
	
		throw XSECException(XSECException::SignatureCreationError,
			"Attempt to use undefined Digest Method in SignedInfo Creation");

	}

	digestMethod->setAttribute(DSIGConstants::s_unicodeStrAlgorithm, str.sbStrToXMLCh());

	// DigestValue

	makeQName(str, prefix, "DigestValue");
	mp_hashValueNode = doc->createElementNS(DSIGConstants::s_unicodeStrURIDSIG, str.sbStrToXMLCh());
	ret->appendChild(mp_hashValueNode);
	ret->appendChild(doc->createTextNode(DSIGConstants::s_unicodeStrNL));
	mp_hashValueNode->appendChild(doc->createTextNode(MAKE_UNICODE_STRING("Not yet calculated")));
	
	return ret;

}

// --------------------------------------------------------------------------------
//           setPreHashTransform - to set a transform to be used pre-hash
// --------------------------------------------------------------------------------


void DSIGReference::setPreHashTXFM(TXFMBase * t) {

	mp_preHash = t;

}

// --------------------------------------------------------------------------------
//           isManifest
// --------------------------------------------------------------------------------

bool DSIGReference::isManifest() {

	return m_isManifest;

}

// --------------------------------------------------------------------------------
//           getURI
// --------------------------------------------------------------------------------

const safeBuffer &DSIGReference::getURISB() {

	return m_URI;

}

// --------------------------------------------------------------------------------
//           getManifestReferenceList
// --------------------------------------------------------------------------------

DSIGReferenceList * DSIGReference::getManifestReferenceList() {
	
	return mp_manifestList;

}

// --------------------------------------------------------------------------------
//           getURIBaseTransform
// --------------------------------------------------------------------------------

TXFMBase * DSIGReference::getURIBaseTXFM(DOMDocument * doc, 
										 safeBuffer URI,
										 XSECURIResolver * resolver) {

	// Determine if this is a full URL or a pointer to a URL

	if (URI[0] != '\0' && URI[0] != '#') {

		TXFMURL * retTransform;

		// Have a URL!

		XSECnew(retTransform, TXFMURL(doc, resolver));
			
		((TXFMURL *) retTransform)->setInput((char *) URI.rawBuffer());

		return retTransform;

	}

	// Have a fragment URI from the local document
	TXFMDocObject * to;
	XSECnew(to, TXFMDocObject(doc));
	
	// Find out what sort of object pointer this is.
	
	if (URI[0] == '\0') {
		
		// empty pointer - use the document itself
		to->setInput(doc);
		to->stripComments();
		
	}
	
	else if (URI.sbOffsetStrncmp("xpointer", 1, 8) == 0) {
		
		// Have an xpointer
		if (URI.sbOffsetStrcmp("(/)", 9) == 0) {
			
			// Root node
			to->setInput(doc);
			
		}
		
		else if (URI.sbOffsetStrncmp("(id('", 9, 4) == 0) {
			
			safeBuffer tmp;
			int j = 14, i = 0;
			
			// Have an ID
			while (URI[j] != '\'') {
				tmp[i++] = URI[j++];
			}
			tmp[i] = '\0';
			
			to->setInput(doc, tmp);
			
		}
		
		else {
			
			throw XSECException(XSECException::UnsupportedXpointerExpr);
			
		}
		
		// Keep comments in these situations
		to->activateComments();
	}
	
	else {
		
		safeBuffer tmp;
		tmp.sbStrcpyIn((char *) &URI[1]);
		
		to->setInput(doc, tmp);
		
		// Remove comments
		to->stripComments();
		
	}
	
	return to;
}

// --------------------------------------------------------------------------------
//           load
// --------------------------------------------------------------------------------

void DSIGReference::load(void) {

	// Load reference info from XML

	DOMNamedNodeMap *atts = mp_referenceNode->getAttributes();
	DOMNode *tmpElt;

	const XMLCh * name;
	safeBuffer sbName;

	if (atts != 0) {

		unsigned int size = atts->getLength();

		for (unsigned int i = 0; i < size; ++i) {

			name = atts->item(i)->getNodeName();
			sbName << (*mp_formatter << atts->item(i)->getNodeName());

			if (strEquals(name, "URI")) {
				m_URI << (*mp_formatter << atts->item(i)->getNodeValue());
				m_URISet = true;
			}

			else if (strEquals(name, "Type")) {

				// Check if a manifest, otherwise ignore for now
				if (strEquals(atts->item(i)->getNodeValue(), DSIGConstants::s_unicodeStrURIMANIFEST))
					m_isManifest = true;

			}

			else if (strEquals(name, "Id")) {

				// For now ignore

			}

			else if (sbName.sbStrncmp("xmlns", 5) == 0) {

				// Ignore name spaces

			}

			else {
				//safeBuffer tmp, error;

				//error << (*mp_formatter << name);
				//tmp.sbStrcpyIn("Unknown attribute in <Reference> Element : ");
				//tmp.sbStrcatIn(error);

				throw XSECException(XSECException::UnknownDSIGAttribute, 
					"Unknown attribute in <Reference> Element");

			}

		}

	}

	// Check to ensure the URI was set

	if (!m_URISet) {
		throw XSECException(XSECException::ExpectedReferenceURI);
	}

	// Now check for Transforms
	tmpElt = mp_referenceNode->getFirstChild();

	while (tmpElt != 0 && (tmpElt->getNodeType() != DOMNode::ELEMENT_NODE))
		// Skip text and comments
		tmpElt = tmpElt->getNextSibling();

	if (tmpElt == 0) {

			throw XSECException(XSECException::ExpectedDSIGChildNotFound, 
					"Expected <Transforms> or <DigestMethod> within <Reference>");

	}

	if (strEquals(getDSIGLocalName(tmpElt), "Transforms")) {

		// Store node for later use
		mp_transformsNode = tmpElt;

		// Load the transforms
		mp_transformList = loadTransforms(tmpElt, mp_formatter, mp_parentSignature);

		// Find next node
		tmpElt = tmpElt->getNextSibling();
		while (tmpElt != 0 && (tmpElt->getNodeType() != DOMNode::ELEMENT_NODE))
			tmpElt = tmpElt->getNextSibling();


	} /* if tmpElt node type = transforms */
	else 
		mp_transformList = NULL;
			

	if (tmpElt == NULL || !strEquals(getDSIGLocalName(tmpElt), "DigestMethod")) {

		throw XSECException(XSECException::ExpectedDSIGChildNotFound, 
							"Expected <DigestMethod> element");

	}

	// Determine what the digest method actually is

	atts = tmpElt->getAttributes();
	unsigned int i;

	for (i = 0; i < atts->getLength() && 
		!strEquals(atts->item(i)->getNodeName(), DSIGConstants::s_unicodeStrAlgorithm); ++i);
		

	if (i == atts->getLength()) {

		throw XSECException(XSECException::ExpectedDSIGChildNotFound, 
							"Expected 'Algorithm' attribute in <DigestMethod>");

	}

	// Determine hashing algorithm

	if (strEquals(atts->item(i)->getNodeValue(), DSIGConstants::s_unicodeStrURISHA1)) {

		me_hashMethod = HASH_SHA1;

	}

	else {

		safeBuffer tmp, error;

		error << (*mp_formatter << atts->item(0)->getNodeValue());

		tmp.sbStrcpyIn("Unknown DigestMethod Algorithm : '");
		tmp.sbStrcatIn(error);
		tmp.sbStrcatIn("'.");

		throw XSECException(XSECException::UnknownTransform, tmp);

	}


	// Find the hash value node

	tmpElt = tmpElt->getNextSibling();

	while (tmpElt != 0 && !(strEquals(getDSIGLocalName(tmpElt), "DigestValue")))
		tmpElt = tmpElt->getNextSibling();

	if (tmpElt == 0) {

		throw XSECException(XSECException::ExpectedDSIGChildNotFound, 
			"Expected <DigestValue> within <Reference>");

	}

	mp_hashValueNode = tmpElt;

	// If we are a manifest, then we need to load the manifest references

	if (m_isManifest) {

		// Find the manifest node - we cheat and use a transform
		TXFMBase				* docObject;
		DOMNode					* manifestNode, * referenceNode;

		docObject = getURIBaseTXFM(mp_referenceNode->getOwnerDocument(), m_URI, 
			mp_parentSignature->getURIResolver());

		manifestNode = docObject->getFragmentNode();
		delete docObject;

		// Now search downwards to find a <Manifest>
		if (manifestNode == 0 || manifestNode->getNodeType() != DOMNode::ELEMENT_NODE ||
			(!strEquals(getDSIGLocalName(manifestNode), "Object") && !strEquals(getDSIGLocalName(manifestNode), "Manifest"))) {

			throw XSECException(XSECException::ExpectedDSIGChildNotFound, 
				"Expected <Manifest> or <Object> URI for Manifest Type <Reference>");

		}

		if (strEquals(getDSIGLocalName(manifestNode), "Object")) {

			// Find Manifest child
			manifestNode = manifestNode->getFirstChild();
			while (manifestNode != 0 && manifestNode->getNodeType() != DOMNode::ELEMENT_NODE)
				manifestNode = manifestNode->getNextSibling();

			if (manifestNode == 0 || !strEquals(getDSIGLocalName(manifestNode), "Manifest"))
				throw XSECException(XSECException::ExpectedDSIGChildNotFound, 
				"Expected <Manifest> as child of <Object> for Manifest Type <Reference>");

		}

		// Now have the manifest node, find the first reference and load!
		referenceNode = manifestNode->getFirstChild();

		while (referenceNode != 0 && !strEquals(getDSIGLocalName(referenceNode), "Reference"))
			referenceNode = referenceNode->getNextSibling();

		if (referenceNode == 0)
			throw XSECException(XSECException::ExpectedDSIGChildNotFound, 
			"Expected <Reference> as child of <Manifest>");

		// Have reference node, so lets create a list!
		mp_manifestList = DSIGReference::loadReferenceListFromXML(mp_parentSignature, referenceNode);

	} /* m_isManifest */

}

// --------------------------------------------------------------------------------
//           createReferenceListFromXML
// --------------------------------------------------------------------------------

DSIGReferenceList *DSIGReference::loadReferenceListFromXML(DSIGSignature * sig, DOMNode *firstReference) {

	// Have the first reference element in the document, 
	// so want to find and load them all

	DOMNode *tmpRef = firstReference;
	DSIGReferenceList * refList;
	DSIGReference * r;

	XSECnew(refList, DSIGReferenceList());

	while (tmpRef != 0) {

		// Must be an element node

		if (tmpRef->getNodeType() != DOMNode::ELEMENT_NODE ||
			!strEquals(getDSIGLocalName(tmpRef), "Reference")) {

			throw XSECException(XSECException::ExpectedDSIGChildNotFound, 
				"Expected <Reference> as child of <SignedInfo>");

		}

		XSECnew(r, DSIGReference(sig, tmpRef));

		refList->addReference(r);

		// Load the reference before moving on
		r->load();

		// Find next element Node
		tmpRef = tmpRef->getNextSibling();

		while (tmpRef != 0 && tmpRef->getNodeType() != DOMNode::ELEMENT_NODE)
			tmpRef = tmpRef->getNextSibling();

	}

	return refList;

}
// --------------------------------------------------------------------------------
//           Get the Canonicalised BYTE_STREAM of the reference data (and TXFMS)
// --------------------------------------------------------------------------------

XSECBinTXFMInputStream * DSIGReference::makeBinInputStream(void) const {

	// First set up for input

	TXFMBase * currentTxfm;

	if (!m_URISet) {

		throw XSECException(XSECException::NotLoaded, 
			"calculateHash() called in DSIGReference before load()");
	
	}

	// Find base transform
	currentTxfm = getURIBaseTXFM(mp_referenceNode->getOwnerDocument(), m_URI,
		mp_parentSignature->getURIResolver());

	// Now check for Transforms

	if (mp_transformList != NULL) {

		// Process the transforms using the static function.

		currentTxfm = createTXFMChainFromList(currentTxfm, mp_transformList);
	

	}
			
	DOMDocument *d = mp_referenceNode->getOwnerDocument();

	// All transforms done.  If necessary, change the type from nodes to bytes
	
	if (currentTxfm->getOutputType() == TXFMBase::DOM_NODES) {

		TXFMC14n * c14n;
		XSECnew(c14n, TXFMC14n(d));
		c14n->setInput(currentTxfm);

		currentTxfm= c14n;

	}

	// Now create the InputStream

	XSECBinTXFMInputStream * ret;
	XSECnew(ret, XSECBinTXFMInputStream(currentTxfm));
	return ret;

}

// --------------------------------------------------------------------------------
//           Hash a reference list
// --------------------------------------------------------------------------------


void DSIGReference::hashReferenceList(DSIGReferenceList *lst, bool interlocking) {

// Run through a list of hashes and checkHash for each one

	DSIGReference * r;
	int i = lst->getSize();
	safeBuffer errStr;

	// Run a VERY naieve process at the moment that assumes the list will "settle"
	// after N iterations through the list.  This will settle any inter-locking references
	// Where a hash in a later calculated reference could impact an already calculated hash
	// in a previous references
	//
	// If interlocking is set to false, assume there are no interacting <Reference> nodes

	do {

		for (int j = 0; j < i; ++j) {
			
			r = lst->item(j);

			r->setHash();
		
			// If this is a manifest we need to set all the references in the manifest as well
			
			if (r->isManifest())
				hashReferenceList(r->getManifestReferenceList());

		}

	} while (interlocking && !DSIGReference::verifyReferenceList(lst, errStr) && i-- >= 0);
}

// --------------------------------------------------------------------------------
//           Verify reference list
// --------------------------------------------------------------------------------

bool DSIGReference::verifyReferenceList(DSIGReferenceList * lst, safeBuffer &errStr) {

	// Run through a list of hashes and checkHash for each one

	DSIGReference * r;
	bool res = true;
	safeBuffer sb;

	int size = lst->getSize();

	for (int i = 0; i < size; ++i) {

		r = lst->item(i);

		try {
			if (!r->checkHash()) {

				// Failed
				sb.sbStrcpyIn("Reference URI=\"");
				sb.sbStrcatIn(r->getURISB());
				sb.sbStrcatIn("\" failed to verify\n");

				errStr.sbStrcatIn(sb);

				res = false;

			}
		}
		catch (NetAccessorException e) {

			res = false;

			sb.sbStrcpyIn("Error accessing network URI=\"");
			sb.sbStrcatIn(r->getURISB());
			sb.sbStrcatIn("\".  Reference failed to verify\n");

			errStr.sbStrcatIn(sb);

		}

		// if a manifest, check the manifest list
		if (r->isManifest())
			res = res & verifyReferenceList(r->getManifestReferenceList(), errStr);

	}

	return res;
}

// --------------------------------------------------------------------------------
//           processTransforms
// --------------------------------------------------------------------------------

TXFMBase * DSIGReference::createTXFMChainFromList(TXFMBase * input, 
							DSIGTransformList * lst) {

	if (lst == NULL)
		return input;

	DSIGTransformList::TransformListVectorType::size_type size, i;

	size = lst->getSize();

	if (size == 0)
		return input;

	TXFMBase * txfm;		// The transform we are working on;
	TXFMBase * nextTxfm;	// The transform we are creating
	
	txfm = input;

	// Iterate through the list

	for (i = 0; i < size; ++i) {
	
		nextTxfm = lst->item(i)->createTransformer(txfm);
		// nextTxfm->setInput(txfm);
		txfm = nextTxfm;

	}

	return txfm;

}

// --------------------------------------------------------------------------------
//           loadTransforms
// --------------------------------------------------------------------------------

DSIGTransformList * DSIGReference::loadTransforms( 
							DOMNode *transformsNode,
							XSECSafeBufferFormatter * formatter,
							DSIGSignature * sig) {

	DOMDocument *d = transformsNode->getOwnerDocument();

	// This is defined as a static function, not because it makes use of any static variables
	// in the DSIGReference class, but to neatly link it to the other users

	if (transformsNode == 0 || !strEquals(getDSIGLocalName(transformsNode), "Transforms")) {

			throw XSECException(XSECException::ExpectedDSIGChildNotFound, 
					"Expected <Transforms> in function DSIGReference::processTransforms");

	}

	// Create the list
	DSIGTransformList * lst;
	XSECnew(lst, DSIGTransformList());

	// Find First transform
	
	DOMNode * transforms = transformsNode->getFirstChild();
	while (transforms != NULL && transforms->getNodeType() != DOMNode::ELEMENT_NODE)
		transforms = transforms->getNextSibling();
	
	while (transforms != NULL) {
		
		// Process each transform in turn
		if (!strEquals(getDSIGLocalName(transforms), "Transform")) {
			
			// Not what we expected to see!
			safeBuffer tmp, error;
			
			error << (*formatter << getDSIGLocalName(transforms));
			tmp.sbStrcpyIn("Unknown attribute in <Transforms> - Expected <Transform> found ");
			tmp.sbStrcatIn(error);
			tmp.sbStrcatIn(">.");
			
			throw XSECException(XSECException::ExpectedDSIGChildNotFound, error);
			
		}
		
		DOMNamedNodeMap * transformAtts = transforms->getAttributes();
		
		unsigned int i;

		for (i = 0; i < transformAtts->getLength() && 
				!strEquals(transformAtts->item(i)->getNodeName(), DSIGConstants::s_unicodeStrAlgorithm); ++i);

		if (i == transformAtts->getLength()) {
			
			throw XSECException(XSECException::ExpectedDSIGChildNotFound,
				"Algorithm attribute not found in <Transform> element");
			
		}
		
		safeBuffer algorithm;
		algorithm << (*formatter << transformAtts->item(i)->getNodeValue());
		
		// Determine what the transform is
		
		if (algorithm.sbStrcmp(URI_ID_BASE64) == 0) {
			
			DSIGTransformBase64 * b;
			b = new DSIGTransformBase64(sig, transforms);
			b->load();
			lst->addTransform(b);
		}
		
		else if (algorithm.sbStrcmp(URI_ID_XPATH) == 0) {

			DSIGTransformXPath * x;
			x = new DSIGTransformXPath(sig, transforms);
			x->load();
			lst->addTransform(x);
		}
		
		else if (algorithm.sbStrcmp(URI_ID_ENVELOPE) == 0) {

			DSIGTransformEnvelope * e;
			e = new DSIGTransformEnvelope(sig, transforms);
			e->load();
			lst->addTransform(e);
		}

		else if (algorithm.sbStrcmp(URI_ID_XSLT) == 0) {
			
			DSIGTransformXSL * x;
			x = new DSIGTransformXSL(sig, transforms);
			x->load();
			lst->addTransform(x);

		}

		
		else if (algorithm.sbStrcmp(URI_ID_C14N_COM) == 0 ||
			     algorithm.sbStrcmp(URI_ID_C14N_NOC) == 0 ||
				 algorithm.sbStrcmp(URI_ID_EXC_C14N_COM) == 0 ||
				 algorithm.sbStrcmp(URI_ID_EXC_C14N_NOC) == 0) {
			
			DSIGTransformC14n * c;
			c = new DSIGTransformC14n(sig, transforms);
			c->load();
			lst->addTransform(c);

		}
		
		else {
			
			// Not what we expected to see!
			safeBuffer tmp;
			
			tmp.sbStrcpyIn("Unknown transform : ");
			tmp.sbStrcatIn(algorithm);
			tmp.sbStrcatIn(" found.");
			
			throw XSECException(XSECException::UnknownTransform, tmp);
		}
		
		// Now find next element
		
		transforms = transforms->getNextSibling();
		while (transforms != NULL && transforms->getNodeType() != DOMNode::ELEMENT_NODE)
			transforms = transforms->getNextSibling();
		
	} /* while (transforms != NULL) */
			

	return lst;
}

// --------------------------------------------------------------------------------
//           Set hash
// --------------------------------------------------------------------------------

void DSIGReference::setHash(void) {

	// First determine the hash value
	XMLByte calculatedHashVal[CRYPTO_MAX_HASH_SIZE];	// The hash that we determined
	unsigned int calculatedHashLen;
	XMLByte base64Hash [CRYPTO_MAX_HASH_SIZE * 2];
	unsigned int base64HashLen;

	calculatedHashLen = calculateHash(calculatedHashVal, CRYPTO_MAX_HASH_SIZE);

	// Calculate the base64 value

	XSECCryptoBase64 *	b64 = XSECPlatformUtils::g_cryptoProvider->base64();
	
	if (!b64) {

		throw XSECException(XSECException::CryptoProviderError, 
				"Error requesting Base64 object from Crypto Provider");

	}

	b64->encodeInit();
	base64HashLen = b64->encode(calculatedHashVal, 
								calculatedHashLen, 
								base64Hash, 
								CRYPTO_MAX_HASH_SIZE * 2);
	base64HashLen += b64->encodeFinish(&base64Hash[base64HashLen],
										(CRYPTO_MAX_HASH_SIZE * 2) - base64HashLen);

	// Ensure the string is terminated
	if (base64Hash[base64HashLen-1] == '\n')
		base64Hash[base64HashLen-1] = '\0';
	else
		base64Hash[base64HashLen] = '\0';

	// Now find the correct text node to re-set

	DOMNode *tmpElt = mp_hashValueNode;

	if (mp_hashValueNode == 0) {

		throw XSECException(XSECException::NotLoaded,
			"setHash() called in DSIGReference before load()");

	}

	tmpElt = mp_hashValueNode->getFirstChild();

	while (tmpElt != NULL && tmpElt->getNodeType() != DOMNode::TEXT_NODE)
		tmpElt = tmpElt->getNextSibling();

	if (tmpElt == NULL) {
		// Need to create the underlying TEXT_NODE
		DOMDocument *doc = mp_referenceNode->getOwnerDocument();
		tmpElt = doc->createTextNode(MAKE_UNICODE_STRING((char *) base64Hash));
		mp_hashValueNode->appendChild(tmpElt);
	}
	else {
		tmpElt->setNodeValue(MAKE_UNICODE_STRING((char *) base64Hash));
	}

}




// --------------------------------------------------------------------------------
//           Create hash
// --------------------------------------------------------------------------------

unsigned int DSIGReference::calculateHash(XMLByte *toFill, unsigned int maxToFill) {

	// Determine the hash value of the element

	// First set up for input

	TXFMBase * currentTxfm, * nextInput;
	unsigned int size;

	if (!m_URISet) {

		throw XSECException(XSECException::NotLoaded, 
			"calculateHash() called in DSIGReference before load()");
	
	}

	// Find base transform
	currentTxfm = getURIBaseTXFM(mp_referenceNode->getOwnerDocument(), m_URI,
		mp_parentSignature->getURIResolver());

	// Now check for Transforms

	if (mp_transformList != NULL) {

		// Process the transforms using the static function.

		currentTxfm = createTXFMChainFromList(currentTxfm, mp_transformList);
	

	}
			
	DOMDocument *d = mp_referenceNode->getOwnerDocument();

	// All transforms done.  If necessary, change the type from nodes to bytes
	
	if (currentTxfm->getOutputType() == TXFMBase::DOM_NODES) {

		TXFMC14n * c14n;
		XSECnew(c14n, TXFMC14n(d));
		c14n->setInput(currentTxfm);

		currentTxfm= c14n;

	}
	
	// Check to see if there is a final "application" transform prior to the hash

	if (mp_preHash != NULL) {

		mp_preHash->setInput(currentTxfm);
		currentTxfm= mp_preHash;

	}

#if 0
	TXFMOutputFile * of = new TXFMOutputFile(d);

	of->setFile("Output");
	of->setInput(currentTxfm);
	currentTxfm =of;
#endif

	nextInput = currentTxfm;
	
	// Determine what the digest method actually is

	switch (me_hashMethod) {

	case HASH_SHA1 :

		XSECnew(currentTxfm, TXFMSHA1(d));
		break;

	default :

		throw XSECException(XSECException::UnknownTransform, 
			"Unknown Hash type in <Reference> node");

	}

	// Now we have the hashing transform, run it.
	
	currentTxfm->setInput(nextInput);

	// Now get the value

	size = currentTxfm->readBytes(toFill, maxToFill);

	// Clean out document if necessary
	currentTxfm->deleteExpandedNameSpaces();
	deleteTXFMChain(currentTxfm);

	return size;

}

// --------------------------------------------------------------------------------
//           Read hash
// --------------------------------------------------------------------------------

unsigned int DSIGReference::readHash(XMLByte *toFill, unsigned int maxToFill) {

	// Determine the hash value stored in the reference

	// First set up for input

	unsigned int size;
	DOMNode *tmpElt;
	//const XMLCh * stringHash;

	TXFMBase * nextInput, *currentTransform;

	DOMDocument *d = mp_referenceNode->getOwnerDocument();

	safeBuffer b64HashVal;

	// Find the hash value

	tmpElt = mp_referenceNode->getFirstChild();

	while (tmpElt != 0 && !strEquals(getDSIGLocalName(tmpElt), "DigestValue"))
		tmpElt = tmpElt->getNextSibling();

	if (tmpElt == NULL)
		// ERROR
		return 0;

	// Now read the DOMString of the hash

	tmpElt = tmpElt->getFirstChild();
	while (tmpElt != NULL && tmpElt->getNodeType() != DOMNode::TEXT_NODE)
		tmpElt = tmpElt->getNextSibling();

	if (tmpElt == NULL)
		// Something wrong with the underlying XML if no text was found
		throw XSECException(XSECException::NoHashFoundInDigestValue);

	b64HashVal << (*mp_formatter << tmpElt->getNodeValue());

	// Now have the value of the string - create a transform around it
	
	nextInput = (TXFMBase *) new TXFMSB(d);

	if (nextInput == NULL)
		throw XSECException(XSECException::MemoryAllocationFail);

	((TXFMSB *) nextInput)->setInput(b64HashVal);

	// Now create the base64 transform

	currentTransform = new TXFMBase64(d);

	if (currentTransform == NULL)
		throw XSECException(XSECException::MemoryAllocationFail);
	
	currentTransform->setInput(nextInput);

	// Now get the value

	size = currentTransform->readBytes(toFill, maxToFill);

	// Delete the transforms

	currentTransform->deleteExpandedNameSpaces();
	deleteTXFMChain(currentTransform);

	return size;

}



// --------------------------------------------------------------------------------
//           Check a reference
// --------------------------------------------------------------------------------

bool DSIGReference::checkHash() {

	// Determine the hash value of the element and check if matches that stored in the 
	// DigestValue part of the element

	// First set up for input

	XMLByte calculatedHashVal[CRYPTO_MAX_HASH_SIZE];		// The hash that we determined
	XMLByte readHashVal[CRYPTO_MAX_HASH_SIZE];			// The hash in the element

	unsigned int calculatedHashSize, i;

	if ((calculatedHashSize = calculateHash(calculatedHashVal, CRYPTO_MAX_HASH_SIZE)) == 0)
		return false;

	if (readHash(readHashVal, CRYPTO_MAX_HASH_SIZE) != calculatedHashSize)
		return false;

	for (i = 0; i < calculatedHashSize; ++i) {
		if (calculatedHashVal[i] != readHashVal[i])
			return false;

	}

	// Got through with flying colours!
	return true;

}





