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
 * DSIGTransformList := Class for Loading and storing a list of references
 *					 
 *
 */

// XSEC Includes
#include <xsec/dsig/DSIGTransformList.hpp>
#include <xsec/dsig/DSIGTransform.hpp>

DSIGTransformList::DSIGTransformList(void){

}

DSIGTransformList::~DSIGTransformList() {

	// Delete all the transforms contained in the list

	DSIGTransformList::TransformListVectorType::iterator it = m_transformList.begin();

	if (it != m_transformList.end()) {
		
		delete *it;
		it++;

	}

}


void DSIGTransformList::addTransform(DSIGTransform * ref) {

	m_transformList.push_back(ref);

}

DSIGTransformList::size_type DSIGTransformList::getSize() {

	return m_transformList.size();

}

void DSIGTransformList::removeTransform(size_type index) {

	if (index < m_transformList.size()) {

		TransformListVectorType::iterator i = m_transformList.begin();
		for (size_type j = 0; j < index; ++j)
			i++;
		
		if (i != m_transformList.end()) {
			delete *i;
			m_transformList.erase(i);
		}
	}
}

DSIGTransform * DSIGTransformList::item(DSIGTransformList::size_type index) {

	if (index < m_transformList.size())
		return m_transformList[index];

	return NULL;

}


bool DSIGTransformList::empty() {

	// Clear out the list - note we do NOT delete the transform elements

	return m_transformList.empty();

}
