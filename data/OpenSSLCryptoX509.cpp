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
 * OpenSSLCryptoX509:= OpenSSL based class for handling X509 (V3) certificates
 *
 */

#include <xsec/enc/OpenSSL/OpenSSLCryptoX509.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyDSA.hpp>
#include <xsec/enc/OpenSSL/OpenSSLCryptoKeyRSA.hpp>
#include <xsec/enc/XSECCryptoException.hpp>

#include <openssl/evp.h>

OpenSSLCryptoX509::OpenSSLCryptoX509() :
m_DERX509("") { 

	mp_X509 = NULL;

}

OpenSSLCryptoX509::~OpenSSLCryptoX509() {

	if (mp_X509 != NULL)
		X509_free(mp_X509);

}

OpenSSLCryptoX509::OpenSSLCryptoX509(X509 * x) {

	// Build this from an existing X509 structure

	mp_X509 = X509_dup(x);
	
	// Now need to create the DER encoding

	BIO * b64 = BIO_new(BIO_f_base64());
	BIO * bmem = BIO_new(BIO_s_mem());

	BIO_set_mem_eof_return(bmem, 0);
	b64 = BIO_push(b64, bmem);

	// Translate X509 to Base64

	int sz = i2d_X509_bio(b64, x);

	//BIO_write(b64, encryptBuf, encryptLen);
	BIO_flush(b64);

	char buf[1024];
	unsigned int l;
	
	m_DERX509.sbStrcpyIn("");

	while ((l = BIO_read(bmem, buf, 1023)) > 0) {
		buf[l] = '\0';
		m_DERX509.sbStrcatIn(buf);
	}

	BIO_free_all(b64);

}

// load functions

void OpenSSLCryptoX509::loadX509Base64Bin(const char * buf, unsigned int len) {

	// Have a Base64 buffer with a binary (DER) encoded certificate

	BIO * b64 = BIO_new(BIO_f_base64());
	BIO * bmem = BIO_new(BIO_s_mem());

	// BIO_set_mem_eof_return(bmem, 1);
	b64 = BIO_push(b64, bmem);

	// Now push the encoded X509

	BIO_write(bmem, buf, len);

	// Translate to a true X509
	mp_X509 = d2i_X509_bio(b64, NULL);

	// Free the IO structures
	BIO_free_all(b64);

	// Check to see if we have a certificate....
	if (mp_X509 == NULL) {

		throw XSECCryptoException(XSECCryptoException::X509Error,
		"OpenSSL:X509 - Error transating Base64 DER encoding into OpenSSL X509 structure");

	}

	m_DERX509.sbStrcpyIn(buf);

}

// Info functions

XSECCryptoKey::KeyType OpenSSLCryptoX509::getPublicKeyType() {

	if (mp_X509 == NULL) {
		throw XSECCryptoException(XSECCryptoException::X509Error,
			"OpenSSL:X509 - getSigningKeyType called before X509 loaded");
	}

	EVP_PKEY *pkey;

	pkey = X509_get_pubkey(mp_X509);

	if (pkey == NULL) {
		throw XSECCryptoException(XSECCryptoException::X509Error,
			"OpenSSL:X509 - cannot retrieve public key from cert");
	}

	XSECCryptoKey::KeyType ret;

	switch (pkey->type) {

	case EVP_PKEY_DSA :

		ret = XSECCryptoKey::KEY_DSA_PUBLIC;
		break;

	case EVP_PKEY_RSA :

		ret = XSECCryptoKey::KEY_RSA_PUBLIC;
		break;

	default :

		ret = XSECCryptoKey::KEY_NONE;

	}

	EVP_PKEY_free (pkey);

	return ret;

}
		

// Get functions
XSECCryptoKey * OpenSSLCryptoX509::clonePublicKey() {

	if (mp_X509 == NULL) {
		throw XSECCryptoException(XSECCryptoException::X509Error,
			"OpenSSL:X509 - getSigningKey called before X509 loaded");
	}

	EVP_PKEY *pkey;
	XSECCryptoKey * ret;

	pkey = X509_get_pubkey(mp_X509);

	if (pkey == NULL) {
		throw XSECCryptoException(XSECCryptoException::X509Error,
			"OpenSSL:X509 - cannot retrieve public key from cert");
	}

	switch (pkey->type) {

	case EVP_PKEY_DSA :

		ret = new OpenSSLCryptoKeyDSA(pkey);
		break;

	case EVP_PKEY_RSA :

		ret = new OpenSSLCryptoKeyRSA(pkey);
		break;

	default :

		ret = NULL;

	}

	EVP_PKEY_free (pkey);

	return ret;

}


