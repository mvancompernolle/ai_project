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
 * OpenSSLCryptoHashSHA1 := OpenSSL Implementation of SHA1
 *
 */

#include <xsec/enc/OpenSSL/OpenSSLCryptoHash.hpp>
#include <xsec/enc/XSECCryptoException.hpp>

#include <memory.h>

// Constructors/Destructors

OpenSSLCryptoHash::OpenSSLCryptoHash(HashType alg) {

	switch (alg) {

	case (XSECCryptoHash::HASH_SHA1) :
	
		mp_md = EVP_get_digestbyname("SHA1");
		break;

	default :

		mp_md = NULL;

	}

	if(!mp_md) {

		throw XSECCryptoException(XSECCryptoException::MDError,
			"OpenSSL:Hash - Error loading Message Digest"); 
	}

	EVP_DigestInit(&m_mdctx, mp_md);
	m_hashType = alg;

}


OpenSSLCryptoHash::~OpenSSLCryptoHash() {}



// Hashing Activities
void OpenSSLCryptoHash::reset(void) {

	EVP_DigestInit(&m_mdctx, mp_md);

}

void OpenSSLCryptoHash::hash(unsigned char * data, 
								 unsigned int length) {

	EVP_DigestUpdate(&m_mdctx, data, length);

}
unsigned int OpenSSLCryptoHash::finish(unsigned char * hash,
									   unsigned int maxLength) {

	unsigned int retLen;

	// Finish up and copy out hash, returning the length

	EVP_DigestFinal(&m_mdctx, m_mdValue, &m_mdLen);

	// Copy to output buffer
	
	retLen = (maxLength > m_mdLen ? m_mdLen : maxLength);
	memcpy(hash, m_mdValue, retLen);

	return retLen;

}

// Get information

XSECCryptoHash::HashType OpenSSLCryptoHash::getHashType(void) {

	return m_hashType;			// This could be any kind of hash

}

