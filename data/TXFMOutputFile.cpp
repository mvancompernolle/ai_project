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
 * TXFMOutputFile := Transform that outputs the byte stream to a file without changing
 *							  the actual data in any way
 *
 */

#include <xsec/transformers/TXFMOutputFile.hpp>
#include <xsec/framework/XSECException.hpp>

// Destructor

TXFMOutputFile::~TXFMOutputFile() {

	f.close();

}

// Methods to set the inputs

void TXFMOutputFile::setInput(TXFMBase *newInput) {

	if (newInput->getOutputType() != TXFMBase::BYTE_STREAM) {

		throw XSECException(XSECException::TransformInputOutputFail, "OutputFile transform requires BYTE_STREAM input");

	}

	input = newInput;

	keepComments = input->getCommentsStatus();

}

bool TXFMOutputFile::setFile(char * const fileName) {

	// Open a file for outputting

	f.open(fileName, ios::binary);

	if (f.is_open())
		return true;

	return false;

}



// Methods to get tranform output type and input requirement

TXFMBase::ioType TXFMOutputFile::getInputType(void) {

	return TXFMBase::BYTE_STREAM;

}

TXFMBase::ioType TXFMOutputFile::getOutputType(void) {

	return TXFMBase::BYTE_STREAM;

}


TXFMBase::nodeType TXFMOutputFile::getNodeType(void) {

	return TXFMBase::DOM_NODE_NONE;

}


// Methods to get output data

unsigned int TXFMOutputFile::readBytes(XMLByte * const toFill, unsigned int maxToFill) {

	unsigned int sz;

	sz = input->readBytes(toFill, maxToFill);

	if (f.is_open())
		f.write(toFill, sz);

	return sz;

}

DOMDocument * TXFMOutputFile::getDocument() {

	return NULL;

}

DOMNode * TXFMOutputFile::getFragmentNode() {

	return NULL;

};

safeBuffer TXFMOutputFile::getFragmentId() {

	return safeBuffer("");	// Empty string

}