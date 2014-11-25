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
 * TXFMXSL := Class that performs XPath transforms
 *
 */

#include <xsec/transformers/TXFMXSL.hpp>
#include <xsec/dsig/DSIGConstants.hpp>
#include <xsec/framework/XSECError.hpp>

#ifndef XSEC_NO_XSLT

// Xerces
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
//#include <xercesc/dom/DOMParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include <iostream>
#include <strstream>
#include <fstream>

XALAN_USING_XALAN(XSLTResultTarget)

// Function used to output data to a safeBuffer

typedef struct TransformXSLOutputHolderStruct {

	safeBuffer	buffer;
	int			offset;

} TransformXSLOutputHolder;

CallbackSizeType TransformXSLOutputFn(const char * s, CallbackSizeType sz, void * data) {

	TransformXSLOutputHolder * output = (TransformXSLOutputHolder *) data;

	output->buffer.sbMemcpyIn(output->offset, s, sz);
	output->offset += sz;
	output->buffer[output->offset] = '\0';

	return sz;

}

// -----------------------------------------------------------------------
//  For expanding name spaces when necessary
// -----------------------------------------------------------------------

bool TXFMXSL::nameSpacesExpanded(void) {

	// NOTE : Do not check inputs as this has its own document

	return (mp_nse != NULL);

}

void TXFMXSL::expandNameSpaces(void) {

	if (mp_nse != NULL)
		return;		// Already done
	
	if (docOut != NULL) {

		XSECnew(mp_nse, XSECNameSpaceExpander(docOut));

		mp_nse->expandNameSpaces();

	}

}

// -----------------------------------------------------------------------
//  Transform functions
// -----------------------------------------------------------------------


TXFMXSL::TXFMXSL(DOMDocument *doc) : 
	TXFMBase(doc),
xpl(xds) {

	// Zeroise all the pointers

	xd = NULL;

}

TXFMXSL::~TXFMXSL() {

	
}

// Methods to set the inputs

void TXFMXSL::setInput(TXFMBase *newInput) {

	if (newInput->getOutputType() != TXFMBase::BYTE_STREAM) {

		throw XSECException(XSECException::TransformInputOutputFail, "XSL requires DOM_NODES input type");

	}

	// Should have a method to check if the input is a straight URL - if it is, just read the
	// URL name and create an XSLTInputSource with this as the input ID.

	input = newInput;

	int size = 0;
	int count = 0;
	unsigned char buf[512];

	while ((count = input->readBytes((XMLByte *) buf, 512)) != 0) {

		sbInDoc.sbMemcpyIn(size, buf, count);
		size += count;

	}

	sbInDoc[size] = '\0';

}

void TXFMXSL::evaluateStyleSheet(const safeBuffer &sbStyleSheet) {

	// We now evaluate the expression

	// Get ready for output
	XercesDOMSupport	xdsout;
	XercesParserLiaison xplout(xdsout);

	XalanDocument * XalanDocOut = xplout.createDocument();
	XSLTResultTarget theDOMResultTarget(XalanDocOut);

	// Set up iostreams for input
	std::istrstream	theXMLStream((char *) sbInDoc.rawBuffer(), strlen((char *) sbInDoc.rawBuffer()));
	std::istrstream	theXSLStream((char *) sbStyleSheet.rawBuffer(), strlen((char *) sbStyleSheet.rawBuffer()));

	// Now resolve

	XalanTransformer xt;
	TransformXSLOutputHolder txoh;
	txoh.buffer.sbStrcpyIn("");
	txoh.offset = 0;
	
	int res = xt.transform(&theXMLStream, &theXSLStream, (void *) & txoh, TransformXSLOutputFn);

	// Should check res

	//std::cout.flush();
	
	// Now use xerces to "re parse" this back into a DOM_Nodes document
	XercesDOMParser * parser = new XercesDOMParser;
	//parser->setToCreateXMLDeclTypeNode(false);
	parser->setDoNamespaces(true);
	parser->setCreateEntityReferenceNodes(true);
	parser->setDoSchema(true);

	// Create an input source

	MemBufInputSource* memIS = new MemBufInputSource ((const XMLByte*) txoh.buffer.rawBuffer(), txoh.offset, "XSECMem");

	int errorCount = 0;

	parser->parse(*memIS);
    errorCount = parser->getErrorCount();
    if (errorCount > 0)
		throw XSECException(XSECException::XSLError, "Errors occured when XSL result was parsed back to DOM_Nodes");

    docOut = parser->adoptDocument();

	// Clean up

	delete memIS;
	delete parser;

}

// Methods to get tranform output type and input requirement

TXFMBase::ioType TXFMXSL::getInputType(void) {

	return TXFMBase::DOM_NODES;

}
TXFMBase::ioType TXFMXSL::getOutputType(void) {

	return TXFMBase::DOM_NODES;

}

TXFMBase::nodeType TXFMXSL::getNodeType(void) {

	return TXFMBase::DOM_NODE_DOCUMENT;

}

// Methods to get output data

unsigned int TXFMXSL::readBytes(XMLByte * const toFill, unsigned int maxToFill) {

	return 0;

}

DOMDocument * TXFMXSL::getDocument() {

	return docOut;

}

DOMNode * TXFMXSL::getFragmentNode() {

	return NULL;

}

safeBuffer TXFMXSL::getFragmentId() {

	return safeBuffer("");	// Empty string

}

#endif /* NO_XSLT */
