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
 * XSECSafeBuffer := a class for storing expanding amounts of information.
 *					 
 *
 */

// XSEC includes

#include <xsec/utils/XSECSafeBuffer.hpp>

// Standard includes

#include <stdlib.h>
#include <string.h>



void safeBuffer::checkAndExpand(unsigned int size) {

	// For a given size, check it will fit (with one byte spare) 
	// and expand if necessary

	if (size + 1 < bufferSize)
		return;

	// Make the new size twice the size of the new string requirement
	int newBufferSize = size * 2;

	unsigned char * newBuffer = new unsigned char[newBufferSize];
	memcpy(newBuffer, buffer, bufferSize);

	// clean up
	bufferSize = newBufferSize;
	delete[] buffer;
	buffer = newBuffer;

}

void safeBuffer::resize(unsigned int sz) {

	checkAndExpand(sz);

}

safeBuffer::safeBuffer(int initialSize) {

	// Initialise the buffer with a set size string

	bufferSize = initialSize;
	buffer = new unsigned char[initialSize];
	mp_XMLCh = NULL;

}

safeBuffer::safeBuffer() {

	bufferSize = DEFAULT_SAFE_BUFFER_SIZE;
	buffer = new unsigned char[bufferSize];
	mp_XMLCh = NULL;

}

safeBuffer::safeBuffer(char * inStr, unsigned int initialSize) {

	// Initialise with a string

	bufferSize = (strlen(inStr) > initialSize ? (strlen(inStr) * 2) : initialSize);
	buffer = new unsigned char[bufferSize];
	strcpy((char *) buffer, inStr);
	mp_XMLCh = NULL;

}

safeBuffer::safeBuffer(const safeBuffer & other) {

	// Copy constructor

	bufferSize = other.bufferSize;
	buffer = new unsigned char [bufferSize];

	memcpy(buffer, other.buffer, bufferSize);

	if (other.mp_XMLCh != NULL) {

		mp_XMLCh = XMLString::replicate(other.mp_XMLCh);

	}
	else {

		mp_XMLCh = NULL;

	}

}
	
safeBuffer::~safeBuffer() {

	if (buffer != NULL)
		delete[] buffer;

	if (mp_XMLCh != NULL)
		delete[] mp_XMLCh;

}


// "IN" functions - these read in information to the buffer

void safeBuffer::sbStrcpyIn(const char * inStr) {

	// Copy a string into the safe buffer
	checkAndExpand(strlen(inStr));

	strcpy((char *) buffer, inStr);

}

void safeBuffer::sbStrcpyIn(const safeBuffer & inStr) {

	checkAndExpand(strlen((char *) inStr.buffer));

	strcpy((char *) buffer, (char *) inStr.buffer);

}


void safeBuffer::sbStrncpyIn(char * inStr, int n) {

	int len = strlen(inStr);

	checkAndExpand((n < len) ? n : len);

	strncpy((char *) buffer, inStr, n);

}

void safeBuffer::sbStrncpyIn(const safeBuffer & inStr, int n) {

	checkAndExpand(n);

	strncpy((char *) buffer, (char *) inStr.buffer, n);
	buffer[n] = '\0';


}


void safeBuffer::sbStrcatIn(char * inStr) {

	checkAndExpand(strlen((char *) buffer) + strlen(inStr));

	strcat((char *) buffer, inStr);

}

void safeBuffer::sbStrcatIn(const safeBuffer & inStr) {

	checkAndExpand(strlen((char *) buffer) + strlen((char *) inStr.buffer));

	strcat((char *) buffer, (char *) inStr.buffer);

}

void safeBuffer::sbStrncatIn(char * inStr, int n) {


	int len = strlen(inStr);

	checkAndExpand(((n < len) ? n : len) + strlen((char *) buffer));

	strncat((char *) buffer, inStr, n);

}

void safeBuffer::sbMemcpyIn(void * inBuf, int n) {

	checkAndExpand(n);
	memcpy(buffer, inBuf, n);

}

void safeBuffer::sbMemcpyIn(int offset, const void * inBuf, int n) {

	checkAndExpand(n + offset);
	memcpy(&buffer[offset], inBuf, n);

}

void safeBuffer::sbStrinsIn(const char * inStr, unsigned int offset) {

	unsigned int bl = strlen((char *) buffer);
	unsigned int il = strlen((char *) inStr);

	checkAndExpand(bl + il);

	memmove(&buffer[offset + il], &buffer[offset], bl - offset + 1);
	memcpy(&buffer[offset], inStr, il);

}



void safeBuffer::sbMemcpyOut(void *outBuf, int n) const {

	// WARNING - JUST ASSUMES OUTPUT BUFFER LONG ENOUGH
	memcpy(outBuf, buffer, n);

}

// Comparisons

int safeBuffer::sbStrncmp(char *inStr, int n) {

	return (strncmp((char *) buffer, inStr, n));

}

int safeBuffer::sbStrcmp(char *inStr) const {

	return (strcmp((char *) buffer, inStr));

}

int safeBuffer::sbStrcmp(const safeBuffer & inStr) const {

	return (strcmp((char *) buffer, (char *) inStr.buffer));

}

int safeBuffer::sbOffsetStrcmp(char * inStr, unsigned int offset) {

	if (offset > bufferSize)
		return -1;

	return (strcmp((char *) &buffer[offset], inStr));

}

int safeBuffer::sbOffsetStrncmp(char * inStr, unsigned int offset, int n) {

	if (offset > bufferSize)
		return -1;

	return (strncmp((char *) &buffer[offset], inStr, n));

}

int safeBuffer::sbStrstr(char * inStr) {

	char * p;
	int d;

	p = strstr((char *) buffer, inStr);

	if (p == NULL)
		return -1;

	d = (unsigned int) p - (unsigned int) buffer;

	if (d < 0 || (unsigned int) d > bufferSize)
		return -1;

	return d;

}	

int safeBuffer::sbOffsetStrstr(const char * inStr, unsigned int offset) {

	char * p;
	int d;

	if (offset > bufferSize)
		return -1;

	p = strstr((char *) &buffer[offset], inStr);

	if (p == NULL)
		return -1;

	d = (unsigned int) p - (unsigned int) buffer;

	if (d < 0 || (unsigned int) d > bufferSize)
		return -1;

	return d;

}

// Operators

unsigned char & safeBuffer::operator[](int n) {

	// If the character is outside our range (but +ve), then simply increase
	// the buffer size - NOTE: it is not our problem if the caller does
	// not realise they are outside the buffer, we are simply trying to ensure
	// the call is "safe"

	if (n < 0)
		return buffer[0];  // Should raise exception

	checkAndExpand(n);

	return buffer[n];

}

safeBuffer & safeBuffer::operator= (const safeBuffer & cpy) {

	if (bufferSize != cpy.bufferSize) {

		if (bufferSize != 0)
			delete [] buffer;

		buffer = new unsigned char [cpy.bufferSize];
		bufferSize = cpy.bufferSize;

	}

	memcpy(buffer, cpy.buffer, bufferSize);

	return *this;
}

// Unicode Functions

const XMLCh * safeBuffer::sbStrToXMLCh(void) {

	if (mp_XMLCh != NULL)
		delete mp_XMLCh;

	mp_XMLCh = XMLString::transcode((char *) buffer);

	return mp_XMLCh;

}

// Get functions

int safeBuffer::sbStrlen(void) const {

	return (strlen ((char *) buffer));

}

unsigned int safeBuffer::sbRawBufferSize(void) const {

	return bufferSize;

}


// raw buffer manipulation

const unsigned char * safeBuffer::rawBuffer() const {

	return buffer;

}

const char * safeBuffer::rawCharBuffer() const {

	return (char *) buffer;

}
