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
#include <servletcontainer.h>

#include <servlet/Servlet.h>
#include <HttpServletRequestImpl.h>
#include <HttpServletResponseImpl.h>
#include <ServletOutputStream.h>
#include <servlet/IllegalStateException.h>
#include <IncludedResponseWrapper.h>
#include <ForwardedRequestWrapper.h>
#include <serverconfig/appcontext.h>

namespace container {

ServletContainer::ServletContainer(servlet::Servlet* servlet,
		container::ServletConfigImpl* conf,
		const std::string& path,
		size_t maxRequestSize, size_t maxFileSize,
		const std::string& mime, const std::string& enc, bool cache)
	: m_servlet(servlet)
	, m_conf(conf)
	, m_path(path)
	, m_maxRequestSize(maxRequestSize)
	, m_maxFileSize(maxFileSize)
	, m_mime(mime)
	, m_enc(enc)
	, m_cache(cache)
{
}

ServletContainer::~ServletContainer()
{
	//FIXME:  should this be done elsewhere, since we didn't create this servlet?
	// Unlikely though - servlets can't be allocated on stack (not yet at least)
	delete m_servlet;
}

void ServletContainer::forward(servlet::ServletRequest& request, servlet::ServletResponse& response) const
{
	if(response.isCommitted())
		throw servlet::IllegalStateException();
	response.getOutputStream().clear();
	serverconfig::AppContext& ctx = static_cast<serverconfig::AppContext&>(getConfig()->getServletContext());
	std::string uri = ctx.getUriBase()+ctx.getServletContextName()+getName();
	ForwardedRequestWrapper freq(dynamic_cast<servlet::HttpServletRequest*>(&request),
				     ctx, uri, getPath());
	m_servlet->service(freq, response);
}
void ServletContainer::include(servlet::ServletRequest& request, servlet::ServletResponse& response) const
{
	IncludedResponseWrapper iresp(dynamic_cast<servlet::HttpServletResponse*>(&response));	
	m_servlet->service(request, iresp);
}

void ServletContainer::invalidRequest(std::ostream& out, servlet::Traceable& e)
{
	out<<"Status: "<<servlet::HttpServletResponse::SC_INTERNAL_SERVER_ERROR<<"\n";
	out<<"Content-type: text/html\n\n";
	out<<"<HTML><HEAD><TITLE>Internal Error</TITLE></HEAD><BODY><B>Exception thrown while processing the request:</B><BR><PRE>\n";
	e.printStackTrace(out);
	out<<"\n</PRE></BODY>"<<std::endl;
}
void ServletContainer::invalidRequest(std::ostream& out)
{
	out<<"Status: "<<servlet::HttpServletResponse::SC_INTERNAL_SERVER_ERROR<<"\n";
	out<<"Content-type: text/html\n\n";
	out<<"<HTML><HEAD><TITLE>Internal Error</TITLE></HEAD><BODY><B>Unknown error occured while processing the request</B></BODY>"<<std::endl;
}

/*!
    \fn container::ServletContainer::service(Connection& con)
	Sets up request and response objects and calls servlet's service method
 */
void ServletContainer::service(Connection& con)
{
	container::HttpServletRequestImpl req(con,(serverconfig::AppContext*)&m_conf->getServletContext(),
		getPath(), m_maxRequestSize, m_maxFileSize);
	container::HttpServletResponseImpl resp(con, m_mime, m_enc);
	resp.setReqRef(&req);
	req.setRespRef(&resp);
	try {
		req.load();
	}
	catch (servlet::Traceable& e) {
		std::ostream out(&con);
		invalidRequest(out, e);
		invalidRequest(std::cerr, e);
		return;
	}
	catch (...) {
		std::ostream out(&con);
		invalidRequest(out);
		invalidRequest(std::cerr);
		return;
	}
	std::ostream& out=resp.getOutputStream();
	try {
		// If caching isn't allowed, set cache-control, expires, and
		// no-cache pragma headers
		if(!m_cache) {
			resp.setHeader("pragma", "no-cache");
			resp.setHeader("cache-control", "no-cache");
			resp.setDateHeader("expires", -1);
		}
		m_servlet->service(static_cast<servlet::ServletRequest&>(req),
		    static_cast<servlet::ServletResponse&>(resp));
	} catch (servlet::IllegalStateException& e) {
		std::cerr<<"Trying to set headers in committed servlet?\n";
		e.printStackTrace(std::cerr);
		out<<"<pre>Cannot set headers after output has been committed\n";
		e.printStackTrace(out);
		out<<"\n</pre>\n";
	} catch (servlet::Traceable& e) {
		std::cerr<<"Caught an exception\n";
		out<<"<pre>Caught an exception:\n";
		e.printStackTrace(std::cerr);
		out<<"</pre></body></html>";
	} catch (...){
		std::cerr<<"Caught an unknown exception\n";
		out<<"Caught an unknown exception\n";
	}
}

}
