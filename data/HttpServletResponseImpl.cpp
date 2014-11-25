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
#include <HttpServletResponseImpl.h>
#include <servlet/IllegalStateException.h>
#include <servlet/ServletException.h>

#include <sstream>

namespace container {

HttpServletResponseImpl::HttpServletResponseImpl(Connection& con, const std::string& mimeType, const std::string& charEnc)
	: m_req(0)
	, m_contentLength(-1)
	, m_contentType(mimeType)
	, m_characterEncoding(charEnc)
	, m_outputStream(con,*this)
	, m_committed(false)
	, m_statusCode(SC_OK)
{
}


HttpServletResponseImpl::~HttpServletResponseImpl()
{
}

/*!
 * \fn container::HttpServletResponseImpl::sendHeaders(container::Connection& c)
 * This function is called by container::ServletOutputStreamBuf before request
 * body data is sent for the first time (or whenever response is committed in some
 * other manner)
 */
void HttpServletResponseImpl::sendHeaders(Connection& c)
{
	std::ostream con(&c);
	std::string ct = getContentType();
	if(!getCharacterEncoding().empty())
		ct+="; charset="+getCharacterEncoding();
	setHeader(std::string("content-type"),ct);
	//con<<"HTTP/1.1 "<<statusCode<<' '<<getStatusMessage()<<'\n';
	con<<"Status: "<<m_statusCode<<'\n';
	if(m_contentLength!=-1)
		con<<"Content-length: "<<m_contentLength<<'\n';
	for(util::mparam_t::iterator cur=m_headers.begin(); cur!=m_headers.end(); cur++)
		con<<cur->first<<": "<<cur->second<<'\n';
	//Send cookies now
	for(cookielist_t::iterator curC=m_cookies.begin(); curC!=m_cookies.end(); curC++){
		std::stringstream cookieStr;
		time_t maxAge=curC->getMaxAge();
		cookieStr<<curC->getName()<<'='<<quoteString(curC->getValue());
		if(!curC->getDomain().empty())
			cookieStr<<"; Domain="<<quoteString(curC->getDomain());
		if(!curC->getPath().empty())
			cookieStr<<"; Path="<<quoteString(curC->getPath());
		if(curC->getSecure())
			cookieStr<<"; Secure;";
		if(curC->getVersion()!=0){
			cookieStr<<"; Version="<<curC->getVersion();
			if(!curC->getComment().empty())
				cookieStr<<"; Comment="<<quoteString(curC->getComment());
			if(maxAge>=0)
				cookieStr<<"; Max-Age="<<maxAge;
		}else{
			if(maxAge>=0) {
				char date[32];
				struct tm exp;
				maxAge=maxAge?maxAge+::time(0):1;
				::gmtime_r(&maxAge, &exp);
				::strftime(date, sizeof(date), "\"%a, %d-%m-%Y %H:%M:%S %Z\"", &exp);
				cookieStr<<"; Expires="<<date;
			}
		}
		con<<"Set-Cookie: "<<cookieStr.str()<<'\n';
		std::cerr<<"Set-Cookie: "<<cookieStr.str()<<std::endl;
	}
	con<<'\n';
	m_committed=true;
}

std::string HttpServletResponseImpl::quoteString(const std::string& in)
{
	std::string out;
	if(in.empty())
		out="\"\"";
	else if(in.find_first_of(",; \"")!=std::string::npos)
	{
		out.reserve(in.length()+2);
		out+='"';
		for(std::string::size_type i=0; i<in.length(); i++)
		{
			if(in[i]=='"')
				out+="\\\"";
			else
				out+=in[i];
		}
		out+='"';
	}
	else
		out.assign(in);
	return out;
}

}

/*!
    \fn container::HttpServletResponseImpl::flushBuffer()
 */
void container::HttpServletResponseImpl::flushBuffer()
{
	getOutputStream().flush();
}


/*!
    \fn container::HttpServletResponseImpl::reset()
 */
void container::HttpServletResponseImpl::reset()
{
	resetBuffer();// Will throw servlet::IllegalStateException if isCommitted()
	m_headers.clear();
	m_cookies.clear();
	setContentType("text/html");
	/// @todo make sure everything that needs to be reset is in fact reset
}


/*!
    \fn container::HttpServletResponseImpl::resetBuffer()
	This function is supposed to clear internal buffer,
	but in current implementation any output will cause
	isCommitted() to becom true, so we look like we are "unbuffered", even
	though in reality there is some buffering going on.
 */
void container::HttpServletResponseImpl::resetBuffer()
{
	if(isCommitted())
		throw servlet::IllegalStateException();
    /// @todo find a way to clear internal stream buffer..
}


/*!
    \fn container::HttpServletResponseImpl::addHeader(const std::string& name, const std::string& value)
 */
void container::HttpServletResponseImpl::addHeader(const std::string& name, const std::string& value)
{
	if(isCommitted())
		throw servlet::IllegalStateException();

	std::string lname(name);
	std::transform(lname.begin(),lname.end(),lname.begin(),::tolower);
	m_headers.insert(util::nvpair(lname,value));
}

void container::HttpServletResponseImpl::setHeader(const std::string& name, const std::string& value)
{
	if(isCommitted())
		throw servlet::IllegalStateException();

	std::string lname(name);
	std::transform(lname.begin(),lname.end(),lname.begin(),::tolower);
	m_headers.erase(m_headers.lower_bound(lname),m_headers.upper_bound(lname));
	m_headers.insert(util::nvpair(name,value));
}

/*!
    \fn container::HttpServletResponseImpl::containsHeader(const std::string& name) const
 */
bool container::HttpServletResponseImpl::containsHeader(const std::string& name) const
{
	std::string lname(name);
	std::transform(lname.begin(),lname.end(),lname.begin(),::tolower);
	return m_headers.find(lname)!=m_headers.end();
}


/*!
 * \fn void container::HttpServletResponseImpl::setStatus(int sc)
 */
void container::HttpServletResponseImpl::setStatus(int sc)
{
	if(isCommitted())
		throw servlet::IllegalStateException();

	m_statusCode=sc;
}


/*!
    \fn container::HttpServletResponseImpl::sendRedirect(const std::string& location)
 */
void container::HttpServletResponseImpl::sendRedirect(const std::string& location)
{
	if(isCommitted())
		throw servlet::IllegalStateException();
	// Send location header
	sendError(SC_SEE_OTHER);
	addHeader("Location",location);
	//Commit...
	getOutputStream()<<"\n";
}


/*!
 * \fn container::HttpServletResponseImpl::sendError(int sc)
 */
void container::HttpServletResponseImpl::sendError(int sc)
{
	if(isCommitted())
		throw servlet::IllegalStateException();

	m_statusMsg.clear();
	m_statusCode=sc;
}


/*!
 * \fn container::HttpServletResponseImpl::sendError(int sc, const std::string& msg)
 */
void container::HttpServletResponseImpl::sendError(int sc, const std::string& msg)
{
	if(isCommitted())
		throw servlet::IllegalStateException();

	m_statusCode=sc;
	m_statusMsg=msg;
}


/*!
    \fn container::HttpServletResponseImpl::getStatusMessage()
 */
std::string container::HttpServletResponseImpl::getStatusMessage()
{
	if(!m_statusMsg.empty())
		return m_statusMsg;
	switch(m_statusCode){
	case SC_OK:
		return std::string("OK");
	case SC_NOT_FOUND:
		return std::string("Servlet is not defined");
	default:
		return std::string("No message");
	}
}


/*!
    \fn container::HttpServletResponseImpl::addCookie(const servlet::Cookie& cookie)
 */
void container::HttpServletResponseImpl::addCookie(const servlet::Cookie& cookie)
{
	if(isCommitted()){
		throw servlet::IllegalStateException();
	}
	m_cookies.push_back(cookie);
}

namespace container
{
std::string  HttpServletResponseImpl::getCharacterEncoding()
{
	return m_characterEncoding;
}
std::string  HttpServletResponseImpl::getContentType()
{
	return m_contentType;
}
std::ostream&  HttpServletResponseImpl::getOutputStream()
{
	return m_outputStream;
}
void  HttpServletResponseImpl::setCharacterEncoding(const std::string& charset)
{
	if(isCommitted()){
		throw servlet::IllegalStateException();
	}
	m_characterEncoding = charset;
}
void  HttpServletResponseImpl::setContentLength(int len)
{
	if(isCommitted()){
		throw servlet::IllegalStateException();
	}
	m_contentLength = len;
}
void  HttpServletResponseImpl::setContentType(const std::string& type)
{
	if(isCommitted()){
		throw servlet::IllegalStateException();
	}
	m_contentType = type;
}
void  HttpServletResponseImpl::setBufferSize(int size)
{
	m_outputStream.setBufferSize(size);
}
int  HttpServletResponseImpl::getBufferSize()
{
	return m_outputStream.getBufferSize();
}
bool  HttpServletResponseImpl::isCommitted()
{
	return m_committed;
}

///////////////////HttpServletResponse implementation
std::string  HttpServletResponseImpl::encodeURL(const std::string& url)
{
	//FIXME: implement this for real...
	return url;
}
std::string  HttpServletResponseImpl::encodeRedirectURL(const std::string& url)
{
	//FIXME: implement this for real...
	return url;
}
void  HttpServletResponseImpl::setDateHeader(const std::string& name, long date)
{
	struct tm out;
	gmtime_r(&date, &out);
	char str[32];
	strftime(str, sizeof(str), "%a, %d %b %Y %H:%M:%S GMT", &out);
	setHeader(name, str);
}
void  HttpServletResponseImpl::addDateHeader(const std::string& name, long date)
{
	struct tm out;
	gmtime_r(&date, &out);
	char str[32];
	strftime(str, sizeof(str), "%a, %d %b %Y %H:%M:%S GMT", &out);
	addHeader(name, str);
}
void  HttpServletResponseImpl::setIntHeader(const std::string& name, int value)
{
	std::stringstream val;
	val<<value;
	setHeader(name, val.str());
}
void  HttpServletResponseImpl::addIntHeader(const std::string& name, int value)
{
	std::stringstream val;
	val<<value;
	addHeader(name, val.str());
}
}
