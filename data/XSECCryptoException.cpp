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
 * XSECCryptoException:= How we throw exceptions in XSEC
 *
 */

#include <xsec/enc/XSECCryptoException.hpp>

#include <stdlib.h>
#include <string.h>

extern const char * XSECCryptoExceptionStrings[] = {

	"No Error",
	"Error Creating SHA1 MD",
	"Error in Base64",
	"Memory allocation error",
	"X509 Error",
	"DSA Error",
	"RSA Error"

};

XSECCryptoException::XSECCryptoException(XSECCryptoExceptionType eNum, char * inMsg) {

	if (eNum > UnknownError)
		type = UnknownError;
	else
		type = eNum;

	if (inMsg != NULL) {
		msg = new char[strlen(inMsg) + 1];
		strcpy(msg, inMsg);
	}
	else {
		msg = new char[strlen(XSECCryptoExceptionStrings[type]) + 1];
		strcpy(msg, XSECCryptoExceptionStrings[type]);
	}

}

XSECCryptoException::XSECCryptoException(XSECCryptoExceptionType eNum, safeBuffer &inMsg) {

	if (eNum > UnknownError)
		type = UnknownError;
	else
		type = eNum;

	
	msg = new char[strlen((char *) inMsg.rawBuffer()) + 1];
	strcpy(msg, (char *) inMsg.rawBuffer());

}

XSECCryptoException::XSECCryptoException(const XSECCryptoException &toCopy) {

	// Copy Constructor

	type = toCopy.type;
	if (toCopy.msg == NULL)
		msg = NULL;
	else {

		msg = new char[strlen(toCopy.msg) + 1];
		strcpy(msg, toCopy.msg);
	}
}

XSECCryptoException::~XSECCryptoException() {

	if (msg != NULL)
		delete[] msg;

}

const char * XSECCryptoException::getMsg(void) {

	return msg;

}

XSECCryptoException::XSECCryptoExceptionType XSECCryptoException::getType(void) {

	return type;

}
