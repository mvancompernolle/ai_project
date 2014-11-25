/***************************************************************************
 *   Copyright (C) 2004-2006 by Ilya A. Volynets-Evenbakh                  *
 *   ilya@total-knowledge.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <ServletOutputStream.h>
#include <HttpServletResponseImpl.h>

namespace container {

ServletOutputStream::ServletOutputStream(Connection& con, HttpServletResponseImpl& resp)
 : std::ostream(&m_buf)
 , m_buf(con,resp)
{
}


ServletOutputStream::~ServletOutputStream()
{
}

void ServletOutputStream::setBufferSize(size_t)
{
	///@todo make this function actually set the size of buffer
	///Do we need to free()/delete[] previous buffer and do pubsetbuf
	///or will previous one be freed automatically?
	return;
}
size_t ServletOutputStream::getBufferSize()
{
	return m_buf.getBufSize();
}

}


/*!
    \fn container::ServletOutputStreamBuf::setbuf(char* s,std::streamsize n)
 */
std::streambuf* container::ServletOutputStreamBuf::setbuf(char* s,std::streamsize n)
{
	m_bufSz=n;
	return m_con.setbuf(s,n);
}

/*!
 * \fn container::ServletOutputStreamBuf::sync()
 */
int container::ServletOutputStreamBuf::sync()
{
	if(!m_resp.isCommitted())
		m_resp.sendHeaders(m_con);
	return m_con.sync();
}

int container::ServletOutputStreamBuf::pbackfail()
{
	if(!m_resp.isCommitted())
		m_resp.sendHeaders(m_con);
	return m_con.pbackfail();
}

std::streamsize container::ServletOutputStreamBuf::xsputn(const char* s,std::streamsize n)
{
	if(!m_resp.isCommitted())
		m_resp.sendHeaders(m_con);
	return m_con.xsputn(s,n);
}

int container::ServletOutputStreamBuf::overflow(int c)
{
	if(!m_resp.isCommitted())
		m_resp.sendHeaders(m_con);
	return m_con.overflow(c);
}
