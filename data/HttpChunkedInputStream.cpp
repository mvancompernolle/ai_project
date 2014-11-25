//==============================================================================
// This file is part of the OpenTop C++ Library.
// (c) Copyright 2000-2002 ElCel Technology Limited
//
// This file may be distributed and/or modified under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL included in the
// packaging of this file.
//
// Licensees holding valid OpenTop Commercial licences may use this file
// in accordance with the OpenTop Commercial License Agreement included
// with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// See http://www.elcel.com/products/opentop/pricing.html or email
// sales@elcel.com for information about OpenTop Commercial License Agreements.
//
//==============================================================================
//
// $Revision: 1.1.1.1 $
// $Date: 2002/12/14 00:25:27 $
//
//==============================================================================

#include "HttpChunkedInputStream.h"

#include "ot/base/SystemUtils.h"
#include "ot/base/UnicodeCharacterType.h"
#include "ot/io/IOException.h"

#include <ctype.h>
#include <stdlib.h>

OT_NET_NAMESPACE_BEGIN

using io::IOException;

HttpChunkedInputStream::HttpChunkedInputStream(InputStream* pInputStream) :
	FilterInputStream(pInputStream),
	m_chunkSize(0),
	m_chunkRead(0),
	m_eof(false)
{
}

//==============================================================================
// HttpChunkedInputStream::read
//
//==============================================================================
long HttpChunkedInputStream::read(Byte* pBuffer, size_t bufLen)
{
	SystemUtils::TestBufferIsValid(pBuffer, bufLen);

	if(m_chunkRead == m_chunkSize && !m_eof)
	{
		readChunkHeader();
	}

	if(m_eof)
	{
		return EndOfFile;
	}
	else
	{
		OT_DBG_ASSERT(m_chunkRead < m_chunkSize);
		size_t chunkLeft = m_chunkSize - m_chunkRead;
		size_t maxBytes = (chunkLeft < bufLen) ? chunkLeft : bufLen;
		long numBytesRead = FilterInputStream::read(pBuffer, maxBytes);
		OT_DBG_ASSERT(numBytesRead > 0);
		m_chunkRead += numBytesRead;
		return numBytesRead;
	}
}

//==============================================================================
// HttpChunkedInputStream::readChunkHeader
//
// When m_chunkRead == m_chunkSize we expect to read a chunk header
// which comprises a hexadecimal size plus CRLF.
//
// The first header we read will start with the chunk size, but subsequent
// ones will have a leading CRLF.
//==============================================================================
void HttpChunkedInputStream::readChunkHeader()
{
	//
	// First extract the hex chunk size
	// (perhaps with leading white space)
	//
	bool bSkipWhitespace = true;
	ByteString strChunkSize;
	int x;

	while(true)
	{
		x = FilterInputStream::read();
		if(x == EndOfFile)
		{
			throw IOException(OT_T("HTTP Chunked encoding exception"));
		}
		if(isxdigit(x))
		{
			strChunkSize += char(x);
			bSkipWhitespace = false;
		}
		else if(UnicodeCharacterType::IsSpace(x) && bSkipWhitespace)
		{
			// carry on camping;
		}
		else
		{
			break;
		}
	}

	//
	// Now discard the rest of the line
	//
	while(x != EndOfFile && x != '\n')
	{
		x = FilterInputStream::read();
	}

	//
	// Decode the hexadecimal chunk size into an understandable number
	//
	m_chunkSize = strtol(strChunkSize.c_str(), 0, 16);

	//
	// The last chunk is followed by zero or more trailers
	// (in our case it should always be zero because we don't give permission
	// for trailers to be present), followed by a blank line.
	//
	if(m_chunkSize == 0)
	{
		m_eof = true;
		x = 0;
		size_t lineSize=0;
		while(x != EndOfFile && !(x == '\n' && lineSize == 0))
		{
			x = FilterInputStream::read();
			if(x != '\r' && x != '\n')
			{
				lineSize++;
			}
			else if(x == '\n')
			{
				lineSize=0;
			}
		}
	}

	m_chunkRead = 0;
}

OT_NET_NAMESPACE_END
