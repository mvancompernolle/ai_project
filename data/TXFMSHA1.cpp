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
 * TXFMSHA1 := Class that performs a SHA1 transform
 *
 */

// XSEC

#include <xsec/transformers/TXFMSHA1.hpp>
#include <xsec/utils/XSECPlatformUtils.hpp>
#include <xsec/framework/XSECException.hpp>

// Standarad includes 

TXFMSHA1::TXFMSHA1(DOMDocument *doc,
									 XSECCryptoKey * key) : TXFMBase (doc) {

	toOutput = 0;					// Nothing yet to output

	if (key == NULL)
		// Get a SHA1 worker
		mp_h = XSECPlatformUtils::g_cryptoProvider->hashSHA1();
	else {
		// Get an HMAC Sha1
		
		mp_h = XSECPlatformUtils::g_cryptoProvider->hashHMACSHA1();
		mp_h->setKey(key);

	}

	
	if (!mp_h) {

		throw XSECException(XSECException::CryptoProviderError, 
				"Error requesting SHA1 object from Crypto Provider");

	}
									
};

TXFMSHA1::~TXFMSHA1() {

	// Clean up
	if (mp_h)
		delete mp_h;

};

	// Methods to set the inputs

//void TXFMSHA1::setInput(TXFMBase *input);

	// Methods to get tranform output type and input requirement

TXFMBase::ioType TXFMSHA1::getInputType(void) {

	return TXFMBase::BYTE_STREAM;

}
TXFMBase::ioType TXFMSHA1::getOutputType(void) {

	return TXFMBase::BYTE_STREAM;

}


TXFMBase::nodeType TXFMSHA1::getNodeType(void) {

	return TXFMBase::DOM_NODE_NONE;

}

	// Methods to set input data

void TXFMSHA1::setInput(TXFMBase * inputT) {

	input = inputT;

	keepComments = input->getCommentsStatus();

	// Now run through the data
	unsigned char buffer[1024];
	unsigned int size;

	while ((size = input->readBytes((XMLByte *) buffer, 1024)) != 0)
		mp_h->hash(buffer, size);
	
	// Finalise

	md_len = mp_h->finish(md_value, CRYPTO_MAX_HASH_SIZE);

	toOutput = md_len;

}


unsigned int TXFMSHA1::readBytes(XMLByte * const toFill, unsigned int maxToFill) {
	
	unsigned int ret;

	if (toOutput == 0)
		return 0;

	// Check if we can just output everything left
	if (toOutput <= maxToFill) {

		memcpy((char *) toFill, &md_value[md_len - toOutput], toOutput);
		ret = toOutput;
		toOutput = 0;

		return ret;

	}

	// Output just some

	memcpy((char *) toFill, &md_value[md_len - toOutput], maxToFill);
	ret = maxToFill;
	toOutput -= maxToFill;

	return ret;

}

DOMDocument * TXFMSHA1::getDocument() {

	return NULL;

}

DOMNode * TXFMSHA1::getFragmentNode() {

	return NULL;		// Return a null node

};

safeBuffer TXFMSHA1::getFragmentId() {

	return safeBuffer("");	// Empty string

}
