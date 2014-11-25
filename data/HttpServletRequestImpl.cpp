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
#include <HttpServletRequestImpl.h>
#include <HttpServletResponseImpl.h>
#include <util.h>
#include <servlet/Util.h>
#include <UploadedFileImpl.h>
#include <servlet/ServletException.h>
#include <servlet/NumberFormatException.h>
#include <servlet/IllegalArgumentException.h>
#include <servlet/IllegalStateException.h>
#include <serverconfig/serverconfig.h>
#include <servlet/ServletException.h>

#include <sstream>
#include <iterator>
#include <algorithm>

#include <ctype.h>

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <boost/tokenizer.hpp>

namespace container {

HttpServletRequestImpl::HttpServletRequestImpl(Connection& con,serverconfig::AppContext* ctx,
			const std::string& servletPath, size_t maxRequestSize, size_t maxFileSize)
	: m_localPort(0)
	, m_remotePort(0)
	, m_contentLength(-1)
	, m_servletPath(servletPath)
	, m_session(0)
	, m_maxRequestSize(maxRequestSize)
	, m_maxFileSize(maxFileSize)
	, m_inputStreamTaken(false)
	, m_con(con)
	, m_ctx(ctx)
	, m_resp(0)
	, m_inputStream(&con)
{
}

void HttpServletRequestImpl::load()
{
	m_con.setLimit(m_maxRequestSize);
	getHeaders();
	getParams();
	m_con.setLimit(-1);
	if(m_characterEncoding.empty())
		m_characterEncoding="ISO-8859-1";
}

HttpServletRequestImpl::~HttpServletRequestImpl()
{
	//delete inputStream;
	for(filelist_t::iterator it=m_files.begin(); it!=m_files.end();it++)
		delete it->second;
	for(cookielist_t::iterator it = m_cookies.begin(); it!= m_cookies.end(); it++)
		if(it->second)
			delete it->second;
}

void HttpServletRequestImpl::parseAddr(const std::string& addr, std::string& name, unsigned short& port)
{
	std::string::size_type colon=addr.find(':');
	if(colon!=std::string::npos){
		name=addr.substr(0,colon);
		port=atoi(addr.substr(colon+1).c_str());
	} else {
		name=addr;
	}
}

void HttpServletRequestImpl::addr2host(const std::string& addr, std::string& name)
{
	if(addr.empty())
		return;
	struct in_addr a;
	if(inet_pton(AF_INET, addr.c_str(), &a)>0)
	{
		struct hostent he, *phe;
		char buf[1024];
		int err;
		gethostbyaddr_r(&a, sizeof(a), AF_INET, &he, buf, sizeof(buf), &phe, &err);
		if(phe)
		{
			name = he.h_name;
		}
	}
}

/*!
 * \fn container::HttpServletRequestImpl::getHeaders()
 * Parses request line and headers of the request.
 * Fills out \c method, \c uri, \c contentType, \c contentLength,
 * \c serverName, \c serverPort, and calls ServletRequestImpl::parseCookieHeader
 * if it encounters Cookie header
 */
void HttpServletRequestImpl::getHeaders()
{
//Use stringstream here to parse...
	std::string line;
	getline(m_inputStream,line);
	std::string::size_type i=line.find(' ');
	if(i==std::string::npos)
		return;
	m_method.assign(line,0,i);
	std::transform(m_method.begin(),m_method.end(),m_method.begin(),::toupper);
	std::string::size_type j=++i;
	i=line.find(' ',i);
	if(i==std::string::npos)
		return;
	m_uri.assign(line, j, i-j);
	m_protocol.assign(line, i+1, line.length()-i-1);
	i=m_uri.find("://");
	if(i==std::string::npos) {
		m_scheme="http";
		i = 0;
	} else {
		m_scheme.assign(m_uri,0,i-1);
		m_uri=m_uri.substr(i+3);
	}
	i=m_uri.find('?');
	if(i!=std::string::npos){
		m_queryString.assign(m_uri.substr(i+1));
		m_uri.erase(i);
	}
	std::string servPath(getServletPath() + '/');
	i=m_uri.find(servPath);
	if(i!=std::string::npos){
		m_pathInfo=m_uri.substr(i + servPath.length());
	}
	std::transform(m_scheme.begin(),m_scheme.end(),m_scheme.begin(),::tolower);
	m_serverPort=80;
	if(m_scheme == "https")
		m_serverPort=443;
	while(getline(m_inputStream,line)){
		if(line.empty())
			return;
		i=line.find(':');
		std::string header(line.substr(0,i));
		while(line[++i]==' ') i++; // skip white spaces
		std::string value(line.substr(i));
		std::transform(header.begin(),header.end(),header.begin(),::tolower);
		m_headers.insert(util::nvpair(header,value));
		if(header=="content-type"){
			m_contentType=value;
		}else if (header=="content-length"){
			m_contentLength=::strtoul(value.c_str(),0,0);
		}else if (header=="host" || header=="hostname"){
			parseAddr(value, m_serverName, m_serverPort);
		}else if(header=="cookie") {
			parseCookieHeader(value);
		}else if(header=="localaddr") {
			parseAddr(value, m_localAddr, m_localPort);
		}else if(header=="remoteaddr") {
			parseAddr(value, m_remoteAddr, m_remotePort);
		}else if(header=="localhost") {
			m_localName=value;
		}else if(header=="remotehost") {
			m_remoteHost=value;
		}else if(header=="remoteport") {
			m_remotePort=::atoi(value.c_str());
		}else if(header=="localport") {
			m_localPort=::atoi(value.c_str());
		}
	}
}


/*!
 * \fn HttpServletRequestImpl::getParams()
 * Process request and get request parameters if any
 *
 * Parses query string from GET request, url-encoded POST data,
 * and multi-part data, in case of file uploads
 */
void HttpServletRequestImpl::getParams()
{
	if(!getQueryString().empty()){
		std::stringstream ss(getQueryString());
		parseQueryString(ss);
	}
	if(m_contentLength!=-1 && m_method=="POST"){
		std::string transfer_encoding=
			getHeader("content-type");
		size_t rs = ((size_t)m_contentLength>m_maxRequestSize)?m_maxRequestSize:m_contentLength;
		if(transfer_encoding.find("multipart/form-data")!=std::string::npos) {
			parseMultipartData(m_inputStream);
		} else {
			parseQueryString(m_inputStream);
		}
	}
}

/*!
 * \fn HttpServletRequestImpl::parseQueryString(const std::string& queryString)
 */
void HttpServletRequestImpl::parseQueryString(std::istream& queryString)
{
	std::istream_iterator<char> begin(queryString);
	boost::char_separator<char> amp("&","",boost::keep_empty_tokens);
	boost::char_separator<char> eq("=","",boost::keep_empty_tokens);
	boost::tokenizer<boost::char_separator<char>, std::istream_iterator<char> > t(begin, std::istream_iterator<char>(), amp);
	for(boost::tokenizer<boost::char_separator<char>, std::istream_iterator<char> >::iterator token=t.begin();
		token!=t.end(); token++)
	{
		boost::tokenizer<boost::char_separator<char> > nv(*token, eq);
		boost::tokenizer<boost::char_separator<char> >::const_iterator it=nv.begin();
		std::string name, value;
		if(it!=nv.end()) {
			name=*it;
			it++;
		}
		if(it!=nv.end())
			value=*it;
		servlet::util::urlInPlaceDecode(name);
		servlet::util::urlInPlaceDecode(value);
		m_parameters.insert(util::nvpair(name,value));
	}
}


void HttpServletRequestImpl::CookieSeparator::do_escape(std::string::const_iterator next,std::string::const_iterator end,std::string& tok)
{
	if (++next == end)
		throw servlet::IllegalArgumentException("error while parsing cookie: cannot end with escape");
	tok+=*next;
}

bool HttpServletRequestImpl::CookieSeparator::operator()(std::string::const_iterator& next,std::string::const_iterator end,std::string& tok)
{
	//bool bInQuote = false;
	tok.clear();
	if (next == end)
	{
		return false;
	}
	// We just came here, so we are
	// at the beginning of a token
	// (IOW, not in quote, not in escape, etc
	// Check if char is one of return delimiters
	if(is_sep(*next, m_sep_return))
	{
		tok+=*next;
		++next;
		return true;
	}
	for (;next != end;++next)
	{
		if (is_escape(*next))
		{
			do_escape(next,end,tok);
		}
		else if(!m_cur_quote)
		{
			if(is_sep(*next,m_sep_drop))
			{
				if(tok.empty())
					continue;
				++next;
				return true;
			}
			else if (is_sep(*next,m_sep_return))
			{
				// We are at one of returned separators
				// Stop, without advancing
				if(tok.empty())
					continue;
				return true;
			}
			else if (is_quote(*next))
			{
				m_cur_quote = *next;
			}
			else
				tok += *next;
		}
		else
		{
			if(is_quote(*next))
				m_cur_quote=0;
			else
				tok += *next;
		}
	}
	return !tok.empty();
}

/*!
 * \fn HttpServletRequestImpl::parseCookieHeader(const std::string& data)
 * Parses Cookie header and adds cookies to request cookie list
 */
void HttpServletRequestImpl::parseCookieHeader(const std::string& data)
{
	if(!data.empty())
	{
		CookieSeparator sep;
		typedef boost::tokenizer<CookieSeparator> tokenizer;
		tokenizer t(data, sep);
		enum {C_VERSION, C_CONTENT, C_ATTRS} state = C_VERSION;
		char version;
		std::string name;
		std::string value;
		servlet::Cookie *cookie;
		for(tokenizer::iterator token=t.begin(); token!=t.end(); token++)
		{
			std::string nm;
			std::string val;
			nm=*token;
			if(++token == t.end() || *token!="=") // Invalid cookie token. Ignore for now.
			{
				std::cerr<<"Invalid cookie token: "<<*token<<" (full string: \""<<data<<"\")"<<std::endl;
				break;
			}
			if(++token != t.end())
				val=*token;
			if(nm[0]=='$')
				std::transform(nm.begin(),nm.end(),nm.begin(),::tolower);
			switch(state)
			{
			case C_ATTRS:
				if(nm[0]=='$' && nm!="$version")
				{
					if(nm=="$path")
						cookie->setPath(val);
					else if(nm=="$domain")
						cookie->setDomain(val);
					else
						std::cerr<<"Unknown attribute \""<<nm
							<<"\" for cookie \""<<name
							<<"\" (full header value: \""
							<<data<<"\")"<<std::endl;
					break;
				}
			case C_VERSION:
				name.clear();
				value.clear();
				if(nm=="$version")
				{
					version=::atoi(val.c_str());
					state = C_CONTENT;
					break;
				}
				else if(nm[0]=='$') // Can't start with attribute unless it's version...
				{
					throw servlet::IllegalArgumentException("Invalid cookie header: "+data);
				}
				else
				{
					version=0;
				}
			case C_CONTENT:
				name=nm;
				value=val;
				cookie = new servlet::Cookie(name, value);
				cookie->setVersion(version);
				m_cookies[name]=cookie;
				if(name==m_ctx->getSessionCookieName())
				{
					m_sessionId=value;
				}
				state=C_ATTRS;
				break;
			}
		}
	}
}


/*!
 * \fn HttpServletRequestImpl::getSession(bool create)
 * Gets session associated with current request.
 *
 * @param create Whether to create a new session if it doesn't yet exist
 * The session is associated with current request through session ID
 * passed as cookie. Thus, if session is retrieved after request is committed
 * and session is new (cookie needs to be set), an InvalidStateException will
 * be thrown
 */
servlet::HttpSession* HttpServletRequestImpl::getSession(bool create)
//	throw (servlet::IllegalStateException)
{
	if(m_session)
		return m_session;
	m_session=m_ctx->getSession(getRequestedSessionId(), create);
	if(m_session && m_session->isNew()){
		servlet::Cookie sc(m_ctx->getSessionCookieName(),m_session->getId());
		sc.setMaxAge(-1);
		sc.setPath(m_ctx->getUriBase()+m_ctx->getServletContextName());
		m_resp->addCookie(sc);
	}
	return m_session;
}


/*!
 * \fn HttpServletRequestImpl::setRespRef(HttpServletResponseImpl* resp)
 * Sets reference to responce in the request
 *
 * Request processing may need to do some things to responce.
 * This includes handling SessionID cookie, and probably some other stuff.
 * So, when ServletContainer creates request and response, it calls this
 * function before passing the request to Servlet::service
 */
void HttpServletRequestImpl::setRespRef(HttpServletResponseImpl* resp)
{
	this->m_resp=resp;
	// If client sent session cookie and session is valid, touch session
	if((!getRequestedSessionId().empty())&&isRequestedSessionIdValid()){
		getSession();
	}
}


/*!
 * \fn HttpServletRequestImpl::isRequestedSessionIdValid()
 */
bool HttpServletRequestImpl::isRequestedSessionIdValid()
{
	return m_ctx->isSessionValid(this->getRequestedSessionId());
}


/*!
 * \fn HttpServletRequestImpl::getUploadedFile(const std::string& name)
 * @param name parameter name, as defined by request. _not_ the name of file
 * @return Pointer for UploadedFile object or NULL, if file with such name
 * wasn't uploaded.
 * Note - it is possible to have empty file object (i.e. file element was there,
 * but no file was sent).
 */
servlet::UploadedFile* HttpServletRequestImpl::getUploadedFile(const std::string& name)
{
	filelist_t::const_iterator it=m_files.find(name);
	if(it==m_files.end())
		return 0;
	return it->second;
}

class CharSink
{
public:
	virtual void consume(char* data, int maxind) = 0;
	virtual ~CharSink(){}
};

static bool find_boundary(const std::string& boundary, std::istream& data, CharSink& sink)
{
	char buf[boundary.length()];
	std::string::size_type ptr=0;
	while(!data.eof()) {
		data.read(buf+ptr,1);
		if(boundary[ptr]!=buf[ptr]) {
			sink.consume(buf, ptr);
			ptr = 0;
		} else {
			if(++ptr == boundary.length()) // Found a match
				return true;
		}
	}
	return false; // Didn't find boundary match before EOF
}

class NullCharSink: public CharSink
{
public:
	virtual void consume(char* data, int maxind){}
	virtual ~NullCharSink(){}
};

class MimePartCharSink: public CharSink
{
private:
	enum state
	{
		HEADER_NAME,
		HEADER_VALUE_BEGIN,
		HEADER_VALUE,
		HEADER_VALUE_END,
		DATA
	} state;
	HttpServletRequestImpl& req;
	std::string headerName;
	std::string headerValue;
	std::string fileName;
	std::string filePath;
	std::string contentType;
	std::ostream *dataout;
	std::string partName;
	bool empty;
	bool killFile;
public:
	virtual void consume(char* data, int maxind)
	{
		for(int i=0; i<=maxind; i++)
		{
			switch(state)
			{
			case HEADER_NAME:
				switch(data[i])
				{
				case ' ':
					break;
				case ':':
					state=HEADER_VALUE_BEGIN;
					break;
				case '\r':
					state=HEADER_VALUE_END;
					break;
				default:
					headerName.append(data+i,1);
					break;
				}
				break;
			case HEADER_VALUE_BEGIN:
				if(data[i]==' ') {
					break;
				} else if(data[i]=='\r') {
					state=HEADER_VALUE_END;
				} else {
					state=HEADER_VALUE;
					headerValue.assign(data+i,1);
				}
				break;
			case HEADER_VALUE:
				if(data[i]=='\r') {
					state=HEADER_VALUE_END;
				} else {
					headerValue.append(data+i,1);
				}
				break;
			case HEADER_VALUE_END:
				if(data[i]!='\n')
					throw servlet::ServletException("Invalid header in multi-part/form-data post");
				processHeader();
				break;
			case DATA:
				dataout->write(data+i, maxind-i+1);
				if(dataout->bad())
				{
					throw servlet::IOError("Unable to write all of data");
				}
				i=maxind+1;
				empty=false;
				break;
			}
		}
	}
	void partDone()
	{
		if(partName.empty())
			return;
		if(filePath.empty()) {
			req.addParam(partName,static_cast<std::stringstream*>(dataout)->str());
		} else {
			req.addUploadedFile(partName,new UploadedFileImpl(filePath,empty,contentType,fileName));
			killFile=false;
		}
	}
private:
	void processHeader()
	{
		if(headerName.empty()) {
			if(headerValue.empty()) {
				if(fileName.empty()) {
					dataout = new std::stringstream;
				} else {
					char tmpPath[PATH_MAX];
					strncpy(tmpPath,req.getFileSaveTemplate().c_str(),PATH_MAX);
					int fd=::mkstemp(tmpPath); // Just how portable is this?
					if(fd==-1)
						throw servlet::IOError("creating temporary file");
					::close(fd); // Should we just use fd-based stream?
					filePath=tmpPath;
					dataout = new std::ofstream(tmpPath, std::ofstream::out|std::ofstream::trunc);
				}
				state = DATA;
				return;
			}
		} else {
			std::transform(headerName.begin(),headerName.end(),headerName.begin(),::tolower);
			if(headerName=="content-type") {
				contentType = headerValue;
			} else if(headerName=="content-disposition") {
				std::string::size_type col=headerValue.find("name=");
				if(col!=std::string::npos) {
					col+=sizeof("name=")-1;
					partName=getHeaderPart(headerValue,col);
				}
				col=headerValue.find("filename=");
				if(col!=std::string::npos) {
					col+=sizeof("filename=")-1;
					fileName=getHeaderPart(headerValue,col);
				}
			}
			state=HEADER_NAME;
			headerName.clear();
			headerValue.clear();
		}
	}
	/*!
	 * \fn HttpServletRequestImpl::getHeaderPart(const std::string& header, std::string::size_type pos)
	 * @param header The header string
	 * @param pos Position where part value starts (next character after = sign)
	 */
	std::string getHeaderPart(std::string header, std::string::size_type start)
	{
		start=header.find_first_not_of(" \t",start, 2);
		if(start==std::string::npos)
			return std::string();
		std::string::size_type end=0;
		if(header[start]=='"') {
			start++;
			end=header.find('"', start);
			if(end==std::string::npos)
				throw servlet::ServletException("Invalid header value (unterminated quote)");
			header.erase(end);
		}
		end=header.find(';',end);
		if(end==std::string::npos)// last element
			end=header.length();
		end--;
		end=header.find_last_not_of(" \t", end, 2);
		if(header[end]=='"')end--;
		return header.substr(start, end-start+1);
	}
public:
	MimePartCharSink(HttpServletRequestImpl& req)
		: state(HEADER_NAME)
		, req(req)
		, dataout(0)
		, empty(true)
		, killFile(true)
	{}
	~MimePartCharSink()
	{
		if(dataout)
			delete dataout;
		if(!filePath.empty() && killFile)
			::unlink(filePath.c_str());
	}
};

/*!
 * \fn HttpServletRequestImpl::parseMultipartData(const std::string& data)
 * @param data String containing data sent with post request
 * Parses multi-part/form-data POSTed data. Allows file uploads from forms.
 * Shamelessly stolen from cgicc - GNU CGI C++ library by Stephen F. Booth
 * (http://www.cgicc.org)
 */
void HttpServletRequestImpl::parseMultipartData(std::istream& data)
{
	std::string::size_type pos=getContentType().find("boundary=")+sizeof("boundary=")-1;
	std::string boundary("--");
	boundary.append(getContentType().substr(pos));
	NullCharSink nullSink;
	if(!find_boundary(boundary, data, nullSink))
		throw servlet::ServletException("Unable to parse multipart/form-data request: could not find boundary"); // Probably didn't get all of the request
	char c[2];
	data.read(c,2);
	if(c[0] != '\r' || c[1]!='\n') {
		throw servlet::ServletException("Unable to parse multipart/form-data request: could not find boundary"); // Probably didn't get all of the request
	}
	boundary.insert(0,"\r\n",2);
	while(true) {
		MimePartCharSink sink(*this);
		if(!find_boundary(boundary, data, sink))
			throw servlet::ServletException("Unable to parse multipart/form-data request: could not find boundary"); // Probably didn't get all of the request
		sink.partDone();
		data.read(c,2);
		if(c[0] == '\r' && c[1]=='\n') {
			continue;
		} else if (c[0] == '-' && c[1] == '-') {
			data.read(c,2);
			if(c[0]=='\r' && c[1]=='\n') {
				break; // This was the last mime part
			}
		}
		// If we got here, we didn't get propper mime message termination
		throw servlet::ServletException("Unable to parse multipart/form-data request: message is not terminated properly");
	}
}

void HttpServletRequestImpl::addUploadedFile(const std::string& name, UploadedFileImpl* file)
{
	filelist_t::iterator it=m_files.find(name);
	if(it!=m_files.end())
		delete it->second;
	m_files[name]=file;
}

void HttpServletRequestImpl::addParam(const std::string& name, const std::string& value)
{
	m_parameters.insert(util::nvpair(name, value));
}

boost::shared_ptr<void> HttpServletRequestImpl::getAttribute(const std::string& name)
{
	util::attr_t::iterator it = m_attributes.find(name);
	if(it == m_attributes.end())
		return boost::shared_ptr<void>();
	return it->second;
}
bool HttpServletRequestImpl::hasAttribute(const std::string& name) const
{
	util::attr_t::const_iterator it = m_attributes.find(name);
	return (it != m_attributes.end());
}
std::auto_ptr< std::vector<std::string> > HttpServletRequestImpl::getAttributeNames() const
{
	return util::getMapKeyNames(m_attributes);
}
void  HttpServletRequestImpl::setAttribute(const std::string& name, boost::shared_ptr<void> o)
{
	m_attributes[name]=o;
}
void  HttpServletRequestImpl::removeAttribute(const std::string& name)
{
	util::attr_t::iterator it = m_attributes.find(name);
	if(it != m_attributes.end())
		m_attributes.erase(it);
}
std::string  HttpServletRequestImpl::getCharacterEncoding() const
{
	return m_characterEncoding;
}
void  HttpServletRequestImpl::setCharacterEncoding(const std::string& enc) 
{
	m_characterEncoding = enc;
}
int  HttpServletRequestImpl::getContentLength() const
{
	return m_contentLength;
}
std::string  HttpServletRequestImpl::getContentType() const
{
	return m_contentType;
}
std::istream&  HttpServletRequestImpl::getInputStream()
{
	if(m_inputStreamTaken)
		throw servlet::IllegalStateException();
	m_inputStreamTaken = true;
	return m_inputStream;
}
std::string  HttpServletRequestImpl::getParameter(const std::string& name) const
{
	util::mparam_t::const_iterator it = m_parameters.find(name);
	if(it == m_parameters.end())
		return std::string();
	return it->second;
}
std::auto_ptr< std::vector<std::string> > HttpServletRequestImpl::getParameterNames() const
{
	return util::getMapKeyNames(m_parameters);
}
std::auto_ptr< std::vector<std::string> > HttpServletRequestImpl::getParameterValues(const std::string& name) const
{
	return util::getMapValues(m_parameters, name);
}
std::auto_ptr< std::multimap<std::string,std::string> > HttpServletRequestImpl::getParameterMap() const
{
	return std::auto_ptr<util::mparam_t>(new util::mparam_t(m_parameters));
}
std::string  HttpServletRequestImpl::getProtocol() const
{
	return m_protocol;
}
std::string  HttpServletRequestImpl::getScheme() const
{
	return m_scheme;
}
std::string  HttpServletRequestImpl::getServerName() const
{
	return m_serverName;
}
int  HttpServletRequestImpl::getServerPort() const
{
	return m_serverPort;
}
std::string  HttpServletRequestImpl::getRemoteAddr() const
{
	return m_remoteAddr;
}
std::string  HttpServletRequestImpl::getRemoteHost() const
{
	if(m_remoteHost.empty())
		addr2host(m_remoteAddr, m_remoteHost);
	return m_remoteHost;
}
bool  HttpServletRequestImpl::isSecure() const
{
	return m_isSecure;
}
servlet::RequestDispatcher* HttpServletRequestImpl::getRequestDispatcher(const std::string& path)
{
	std::string lpath(path);
	if(path[0]!='/')
	    lpath.insert(0,"/");
	return m_ctx->getRequestDispatcher(lpath);
}
int  HttpServletRequestImpl::getRemotePort() const
{
	return m_remotePort;
}
std::string  HttpServletRequestImpl::getLocalName() const
{
	if(m_localName.empty())
		addr2host(m_localAddr, m_localName);
	return m_localName;
}
std::string  HttpServletRequestImpl::getLocalAddr() const
{
	return m_localAddr;
}
int  HttpServletRequestImpl::getLocalPort() const
{
	return m_localPort;
}
std::string  HttpServletRequestImpl::getAuthType() const 
{
	throw servlet::ServletException("Method unimplemented");
}
std::auto_ptr< servlet::HttpServletRequest::cookielist_t > HttpServletRequestImpl::getCookies() const
{
	return std::auto_ptr<cookielist_t>(new cookielist_t(m_cookies));
}
long  HttpServletRequestImpl::getDateHeader(const std::string& name) const
{
	std::string hval = getHeader(name);
	if(hval.empty())
		return -1;
	struct tm out;
	if(strptime(hval.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &out) ||
	   strptime(hval.c_str(), "%A, %d-%m-%y %H:%M:%S GMT", &out) ||
	   strptime(hval.c_str(), "%a %m %d %H:%M:%S %Y GMT", &out))
	    return mktime(&out);
	throw servlet::ServletException("IllegalArgumentException");
}
std::string  HttpServletRequestImpl::getHeader(const std::string& name) const 
{
	util::mparam_t::const_iterator it=m_headers.find(name);
	if(it == m_headers.end())
		return "";
	return it->second;
}
std::auto_ptr< std::vector<std::string> > HttpServletRequestImpl::getHeaders(const std::string& name) const 
{
	return util::getMapValues(m_headers, name);
}
std::auto_ptr< std::vector<std::string> > HttpServletRequestImpl::getHeaderNames() const 
{
	return util::getMapKeyNames(m_headers);
}
int  HttpServletRequestImpl::getIntHeader(const std::string& name) const 
{
	std::string hval = getHeader(name);
	if(hval.empty())
		return -1;
	int ret, len;
	if(sscanf(hval.c_str(), "%d%n", &ret, &len)==EOF)
		throw servlet::NumberFormatException();
	if((unsigned)len != hval.length())
		throw servlet::NumberFormatException();
	return ret;
}
std::string  HttpServletRequestImpl::getMethod() const 
{
	return m_method;
}
std::string  HttpServletRequestImpl::getPathInfo() const 
{
	return m_pathInfo;
}
std::string  HttpServletRequestImpl::getPathTranslated() const 
{
	return m_ctx->getRealPath(getPathInfo());
}
std::string  HttpServletRequestImpl::getContextPath() const 
{
	//TODO: verify that my interpretation of HttpServletRequest::getContextPath() is correct
	// Right now I think it means '/' + context (app) name
	// for us, since we simply add app name right after URI base.
	// exception is root app (one with an empty name), where
	// return value must be an empty string
	std::string ret = m_ctx->getServletContextName();
	if(!ret.empty())
		ret.insert(0,"/");
	return ret;
}
std::string  HttpServletRequestImpl::getQueryString() const 
{
	return m_queryString;
}
std::string  HttpServletRequestImpl::getRemoteUser() const 
{
	return m_remoteUser;
}
bool HttpServletRequestImpl::isUserInRole(const std::string& role) const 
{
	throw servlet::ServletException("Method unimplemented");
}
std::string  HttpServletRequestImpl::getRequestedSessionId() const 
{
	return m_sessionId;
}
std::string  HttpServletRequestImpl::getRequestURI() const 
{
	return m_uri;
}
std::string  HttpServletRequestImpl::getRequestURL() const 
{
    	std::stringstream url;
	url<<getScheme()<<"://"<<getServerName()<<':'<<getServerPort()<<getRequestURI();
	return url.str();
}
std::string  HttpServletRequestImpl::getServletPath() const 
{
	return m_servletPath;
}
bool  HttpServletRequestImpl::isRequestedSessionIdValid() const 
{
	HttpSessionImpl* session = m_ctx->getSession(m_sessionId, false);
	return session && session->validP();
}
bool  HttpServletRequestImpl::isRequestedSessionIdFromCookie() const 
{
	return true;
}
bool  HttpServletRequestImpl::isRequestedSessionIdFromURL() const 
{
	return false;
}

}
