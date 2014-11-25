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
 * XSECCanon := Base (virtual) class for canonicalisation objects
 *
 */

#include <xsec/canon/XSECCanon.hpp>
#include <xsec/utils/XSECDOMUtils.hpp>

#include <memory.h>

// --------------------------------------------------------------------------------
//           XSECCanon Virtual Class implementation
// --------------------------------------------------------------------------------


// Constructors

XSECCanon::XSECCanon() {};
	
XSECCanon::XSECCanon(DOMDocument *newDoc) : m_buffer() {
		
	
	mp_doc = newDoc;
	mp_startNode = mp_nextNode = newDoc;		// By default, start from startNode
	m_bufferLength = m_bufferPoint = 0; 	// Start with an empty buffer
	m_allNodesDone = false;
	
};

XSECCanon::XSECCanon(DOMDocument *newDoc, DOMNode *newStartNode) {
	
	mp_doc = newDoc;
	mp_startNode = mp_nextNode = newStartNode;
	m_bufferLength = m_bufferPoint = 0; 	// Start with an empty buffer
	m_allNodesDone = false;
	
};

// Destructors

XSECCanon::~XSECCanon() {};

// Public Methods

int XSECCanon::outputBuffer(unsigned char *outBuffer, int numBytes) {

	// numBytes of data are required to be placed in outBuffer.

	// Calculate amount left in buffer

	int remaining = m_bufferLength - m_bufferPoint;
	int bytesToGo = numBytes;
	int i = 0;					// current point in outBuffer


	// While we don't have enough, and have not completed - 

	while (!m_allNodesDone && (remaining < bytesToGo)) {

		// Copy what we have and get some more in the buffer
		memcpy(&outBuffer[i], &m_buffer[m_bufferPoint], remaining);
		i += remaining;
		m_bufferPoint += remaining;
		bytesToGo -= remaining;

		// Get more

		processNextNode();

		remaining = m_bufferLength - m_bufferPoint;		// This will be reset by processNextElement
													// "-bufferPoint" is just in case.

	}

	if (m_allNodesDone) {

		// Was not enough data to fill everything up
		memcpy (&outBuffer[i], &m_buffer[m_bufferPoint], remaining);
		m_bufferPoint += remaining;
		return i + remaining;
	}
	
	// Copy the tail of the buffer

	memcpy(&outBuffer[i], &m_buffer[m_bufferPoint], bytesToGo);
	m_bufferPoint += bytesToGo;
	return (bytesToGo + i);
	
}

// setStartNode sets the starting point for the output if it is a sub-document 
// that needs canonicalisation and we want to re-start

bool XSECCanon::setStartNode(DOMNode *newStartNode) {

	mp_startNode = newStartNode;
	m_bufferPoint = 0;
	m_bufferLength = 0;
	mp_nextNode = mp_startNode;

	m_allNodesDone = false;			// Restart

	return true;			// Should check to ensure that the StartNode is part of the doc.

}


