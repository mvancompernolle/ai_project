/***************************************************************************
 *   Copyright (C) 2004 by Ilya A. Volynets-Evenbakh                       *
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
#include <servlet/IOError.h>
#include <string.h>

namespace servlet {

IOError::IOError()
	: std::ios_base::failure(geterrmsg())
	, Traceable(errbuf)
	, m_errno(errno)
{
}
IOError::IOError(const std::string& msg)
	: std::ios_base::failure((msg+": "+geterrmsg()).c_str())
	, Traceable(msg+": "+errbuf)
	, m_errno(errno)
{
}


IOError::~IOError() throw()
{
}

const char* IOError::geterrmsg() throw()
{
	const char* err = ::strerror_r(errno, errbuf, sizeof(errbuf));
	if(err!=errbuf) // GNU strerror_r may return errbuf or some other, static string
		::strncpy(errbuf, err, sizeof(errbuf));
	return errbuf;
}

}
