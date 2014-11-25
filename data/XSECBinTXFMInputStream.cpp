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
 * XSECBinTXFMInputStream := Implement the BinInputStream around Transforms.
 *					 
 *
 */


#include <xsec/utils/XSECBinTXFMInputStream.hpp>
#include <xsec/transformers/TXFMBase.hpp>
#include <xsec/framework/XSECError.hpp>

// ---------------------------------------------------------------------------
//  Constructors/Destructors
// ---------------------------------------------------------------------------


XSECBinTXFMInputStream::XSECBinTXFMInputStream(TXFMBase * lst, bool deleteWhenDone) :
mp_txfm(lst),
m_deleteWhenDone(deleteWhenDone),
m_deleted(false),
m_done(false),
m_currentIndex(0) {

	if (mp_txfm->getOutputType() != TXFMBase::BYTE_STREAM) {

		throw XSECException(XSECException::TransformError,
			"Cannot wrapper a non BYTE_STREAM TXFM with XSECBinTXFMInputStream");

	}

}

XSECBinTXFMInputStream::~XSECBinTXFMInputStream() {

	if (m_deleteWhenDone == true && m_deleted == false) {

		deleteTXFMChain(mp_txfm);
		m_deleted = false;

	}

}

// ---------------------------------------------------------------------------
//  Stream methods
// ---------------------------------------------------------------------------

void XSECBinTXFMInputStream::reset(void) {}

unsigned int XSECBinTXFMInputStream::curPos() const {

	return m_currentIndex;

}

unsigned int XSECBinTXFMInputStream::readBytes(XMLByte* const  toFill, 
					   const unsigned int maxToRead) {

	if (m_done == true)
		return 0;

	unsigned int bytesRead;

	bytesRead = mp_txfm->readBytes(toFill, maxToRead);

	if (bytesRead == 0) {

		if (m_deleteWhenDone) {

			deleteTXFMChain(mp_txfm);
			mp_txfm = 0;
			m_deleted = true;

		}

		m_done = true;

	}

	m_currentIndex += bytesRead;

	return bytesRead;

}

