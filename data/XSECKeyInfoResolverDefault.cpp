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
 * XSECKeyInfoResolverDefault := Default (very basic) class for applications
 *						 to map KeyInfo to keys
 *
 */

#include <xsec/enc/XSECKeyInfoResolverDefault.hpp>
#include <xsec/dsig/DSIGKeyInfoX509.hpp>
#include <xsec/dsig/DSIGKeyInfoValue.hpp>

// --------------------------------------------------------------------------------
//           Interface Methods
// --------------------------------------------------------------------------------


XSECCryptoKey * XSECKeyInfoResolverDefault::resolveKey(DSIGKeyInfoList * lst) {

	// Try to find a key from the KeyInfo list as best we can
	// NOTE: No validation is performed (i.e. no cert/CRL checks etc.)

	XSECCryptoKey * ret = NULL;

	DSIGKeyInfoList::size_type sz = lst->getSize();

	for (DSIGKeyInfoList::size_type i = 0; i < sz; ++i) {

		switch (lst->item(i)->getKeyInfoType()) {

		case (DSIGKeyInfo::KEYINFO_X509) :
		{
			ret = NULL;
			const char * x509Str;
			XSECCryptoX509 * x509 = XSECPlatformUtils::g_cryptoProvider->X509();
			x509Str = ((DSIGKeyInfoX509 *) lst->item(i))->getCertificateItem(0);
			
			if (x509Str != 0) {
				x509->loadX509Base64Bin(x509Str, strlen(x509Str));
				ret = x509->clonePublicKey();
			}

			delete x509;

			if (ret != NULL)
				return ret;
		
		}
			break;

		case (DSIGKeyInfo::KEYINFO_VALUE_DSA) :
		{

			XSECCryptoKeyDSA * dsa = XSECPlatformUtils::g_cryptoProvider->keyDSA();

			dsa->loadPBase64BigNums(((DSIGKeyInfoValue *) lst->item(i))->getDSAP(),
				strlen(((DSIGKeyInfoValue *) lst->item(i))->getDSAP()));
			dsa->loadQBase64BigNums(((DSIGKeyInfoValue *) lst->item(i))->getDSAQ(),
				strlen(((DSIGKeyInfoValue *) lst->item(i))->getDSAQ()));
			dsa->loadGBase64BigNums(((DSIGKeyInfoValue *) lst->item(i))->getDSAG(),
				strlen(((DSIGKeyInfoValue *) lst->item(i))->getDSAG()));
			dsa->loadYBase64BigNums(((DSIGKeyInfoValue *) lst->item(i))->getDSAY(),
				strlen(((DSIGKeyInfoValue *) lst->item(i))->getDSAY()));

			return dsa;
		}
			break;

		case (DSIGKeyInfo::KEYINFO_VALUE_RSA) :
		{

			XSECCryptoKeyRSA * rsa = XSECPlatformUtils::g_cryptoProvider->keyRSA();

			rsa->loadPublicModulusBase64BigNums(((DSIGKeyInfoValue *) lst->item(i))->getRSAModulus(),
				strlen(((DSIGKeyInfoValue *) lst->item(i))->getRSAModulus()));
			rsa->loadPublicExponentBase64BigNums(((DSIGKeyInfoValue *) lst->item(i))->getRSAExponent(),
				strlen(((DSIGKeyInfoValue *) lst->item(i))->getRSAExponent()));

			return rsa;

		}

		default :
			break;

		}
	}

	return NULL;

}


XSECKeyInfoResolver * XSECKeyInfoResolverDefault::clone(void) {

	return new XSECKeyInfoResolverDefault();

}