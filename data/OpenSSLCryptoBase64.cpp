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
 * OpenSSLCryptoBase64 := Base virtual class to define a base64 encoder/decoder
 *
 */

#include <xsec/enc/OpenSSL/OpenSSLCryptoBase64.hpp>
#include <xsec/enc/XSECCryptoException.hpp>

#include <openssl/err.h>

// --------------------------------------------------------------------------------
//           Decoding
// --------------------------------------------------------------------------------

void OpenSSLCryptoBase64::decodeInit(void) {

	EVP_DecodeInit(&m_dctx);

}

unsigned int OpenSSLCryptoBase64::decode(unsigned char * inData, 
						 	    unsigned int inLength,
								unsigned char * outData,
								unsigned int outLength) {

	int rc;
	int outLen;

	if (outLength < inLength) {

		throw XSECCryptoException(XSECCryptoException::MemoryError,
			"OpenSSL:Base64 - Output buffer not big enough for Base64 decode");

	}

	rc = EVP_DecodeUpdate(&m_dctx, 
						  outData, 
						  &outLen, 
						  inData, 
						  inLength);

	if (rc < 0) {

		throw XSECCryptoException(XSECCryptoException::Base64Error,
			"OpenSSL:Base64 - Error during Base64 Decode");
	}

	if (outLen > (int) outLength) {

		throw XSECCryptoException(XSECCryptoException::MemoryError,
			"OpenSSL:Base64 - Output buffer not big enough for Base64 decode and overflowed");

	}
		
	return outLen;

}

unsigned int OpenSSLCryptoBase64::decodeFinish(unsigned char * outData,
							 	      unsigned int outLength) {

	int outLen;
	outLen = outLength;

	EVP_DecodeFinal(&m_dctx, outData, &outLen); 

	return outLen;

}

// --------------------------------------------------------------------------------
//           Encoding
// --------------------------------------------------------------------------------

void OpenSSLCryptoBase64::encodeInit(void) {

	EVP_EncodeInit(&m_ectx);

}


unsigned int OpenSSLCryptoBase64::encode(unsigned char * inData, 
						 	    unsigned int inLength,
								unsigned char * outData,
								unsigned int outLength) {

	int outLen;

	if (outLength + 24 < inLength) {

		throw XSECCryptoException(XSECCryptoException::MemoryError,
			"OpenSSL:Base64 - Output buffer not big enough for Base64 encode");

	}

	EVP_EncodeUpdate(&m_ectx, 
					  outData, 
					  &outLen, 
					  inData, 
					  inLength);

	if (outLen > (int) outLength) {

		throw XSECCryptoException(XSECCryptoException::MemoryError,
			"OpenSSL:Base64 - Output buffer not big enough for Base64 encode and overflowed");

	}
		
	return outLen;

}

unsigned int OpenSSLCryptoBase64::encodeFinish(unsigned char * outData,
							 	      unsigned int outLength) {

	int outLen;
	outLen = outLength;

	EVP_EncodeFinal(&m_ectx, outData, &outLen); 

	return outLen;

}

// --------------------------------------------------------------------------------
//           Utility functions
// --------------------------------------------------------------------------------

BIGNUM * OpenSSLCryptoBase64::b642BN(char * b64in, unsigned int len) {

	if (len > 1024)
		return NULL;

	int bufLen;
	unsigned char buf[1024];

	EVP_ENCODE_CTX m_dctx;
	EVP_DecodeInit(&m_dctx);
	int rc = EVP_DecodeUpdate(&m_dctx, 
						  buf, 
						  &bufLen, 
						  (unsigned char *) b64in, 
						  len);

	if (rc < 0) {

		throw XSECCryptoException(XSECCryptoException::Base64Error,
			"OpenSSL:Base64 - Error during Base64 Decode of BIGNUMS");
	}

	int finalLen;
	EVP_DecodeFinal(&m_dctx, &buf[bufLen], &finalLen); 

	bufLen += finalLen;

	// Now translate to a bignum
	return BN_dup(BN_bin2bn(buf, bufLen, NULL));

}

