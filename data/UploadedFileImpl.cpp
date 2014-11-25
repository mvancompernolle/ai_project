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
#include <UploadedFileImpl.h>
#include <unistd.h>
#include <stdio.h>

namespace container {

UploadedFileImpl::UploadedFileImpl(const std::string& path,
                                   bool empty,
                                   const std::string& mime,
                                   const std::string& name)
	: servlet::UploadedFile()
	, m_fpath(path)
	, m_empty(empty)
	, m_mimeType(mime)
	, m_name(name)
{
}


UploadedFileImpl::~UploadedFileImpl()
{
	if(m_stream.is_open()) {
		m_stream.close();
	}
	if(!m_moved) {
		killTmp();
	}
}

/*!
 * \fn UploadedFileImpl::isEmpty() const
 * @return true if file was empty
 * This could be i.e. if user chose not to upload a file,
 * so user agent sent multi-part section, but no actual data
 */
bool UploadedFileImpl::isEmpty() const
{
	return m_empty;
}

/*!
 * \fn UploadedFileImpl::getName() const
 * @return file name, as supplied by user agent.
 * Note: this name bears no relationship to any names
 * on server file system. Furthermore - using it for
 * saving files on file system is a security risk.
 */
const std::string& UploadedFileImpl::getName() const
{
	return m_name;
}
const std::string& UploadedFileImpl::mimeType() const
{
	return m_mimeType;
}
/*!
 * \fn UploadedFileImpl::getStream() throw(IOError)
 * returns stream refering to downloaded file. It may not
 * nessesarily be file
 */

std::iostream& UploadedFileImpl::getStream() throw(servlet::IOError)
{
	if(!m_stream.is_open()) {
		m_stream.open(m_fpath.c_str());
		if(!m_stream.is_open()) {
			throw servlet::IOError();
		}
	}
	return m_stream;
}

}


/*!
 *   \fn container::UploadedFileImpl::killTmp()
 *   Unlinks temporary file
 */
void container::UploadedFileImpl::killTmp()
{
	::unlink(m_fpath.c_str());
}


/*!
 * \fn container::UploadedFileImpl::doMoveTo(const std::string& path) throw(IOError)
 * @param path Where to put saved file
 * This function takes data from internal stream and puts into file named by path
 */
void container::UploadedFileImpl::doMoveTo(const std::string& path)
	throw(servlet::IOError)
{
	long pos;
	{
		std::iostream& in = getStream();
		std::ofstream out(path.c_str());
		if(!out)
			throw servlet::IOError("Moving temporary file");
		pos=in.tellg();
		in.seekg(0);
		out<<in.rdbuf();
		killTmp();
		out.close();
		m_stream.close();
	}
	m_fpath=path;
	{
		std::iostream& in=getStream();
		in.seekg(pos);
	}
}
