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
 * TXFMSB := Class that takes an input safeBuffer to start a transform pipe
 *
 */

#include <xsec/transformers/TXFMSB.hpp>

// General includes 

#include <memory.h>

TXFMSB::TXFMSB(DOMDocument *doc) : TXFMBase(doc) {

	toOutput = 0;

}


TXFMSB::~TXFMSB() {

}

	// Methods to set the inputs

void TXFMSB::setInput(TXFMBase *input) {

	// Not possible as this needs to be the start of a tranform pipe

	return;

}

void TXFMSB::setInput(const safeBuffer sbIn) {

	// Assume this is a string

	sb = sbIn;
	toOutput = sb.sbStrlen();
	sbs = toOutput;

}

void TXFMSB::setInput(const safeBuffer sbIn, unsigned int sbSize) {

	// Assume this is a string

	sb = sbIn;
	if (sbSize > sb.sbRawBufferSize())
		toOutput = sb.sbRawBufferSize();
	else
		toOutput = sbSize;
	sbs = toOutput;

}


	// Methods to get tranform output type and input requirement

TXFMBase::ioType TXFMSB::getInputType(void) {

	return TXFMBase::BYTE_STREAM;

}

TXFMBase::ioType TXFMSB::getOutputType(void) {

	return TXFMBase::BYTE_STREAM;

}


TXFMBase::nodeType TXFMSB::getNodeType(void) {

	return TXFMBase::DOM_NODE_NONE;

}



	// Methods to get output data

unsigned int TXFMSB::readBytes(XMLByte * const toFill, unsigned int maxToFill) {
	
	// Return from the buffer
	
	unsigned int ret;

	if (toOutput == 0)
		return 0;

	// Check if we can just output everything left
	if (toOutput <= maxToFill) {

		memcpy((char *) toFill, &(sb.rawBuffer()[sbs - toOutput]), toOutput);
		ret = toOutput;
		toOutput = 0;

		return ret;

	}

	// Output just some

	memcpy((char *) toFill, &(sb.rawBuffer()[sbs - toOutput]), maxToFill);
	ret = maxToFill;
	toOutput -= maxToFill;

	return ret;
}

DOMDocument *TXFMSB::getDocument() {

	return NULL;

};

DOMNode * TXFMSB::getFragmentNode() {

	return NULL;

};

safeBuffer TXFMSB::getFragmentId() {

	return safeBuffer("");	// Empty string

}