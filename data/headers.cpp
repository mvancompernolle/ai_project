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
#include "headers.h"
#include <iostream>
#include <sstream>
#include <servlet/ServletException.h>

void HeadersServlet::service(servlet::HttpServletRequest& req, servlet::HttpServletResponse& resp)
{
	std::ostream &out = resp.getOutputStream();
	std::auto_ptr< std::vector<std::string> > headerNames = req.getHeaderNames();
	resp.setHeader("X-Test-Header", "MyHeader");
	long seconds = time(NULL) + 3600;
	resp.setDateHeader("expires", seconds);
	renderHeader(out);
	out<<"<PRE>";
	for(std::vector<std::string>::iterator it = headerNames->begin();
	    it!=headerNames->end(); it++)
	{
		std::auto_ptr< std::vector<std::string> > headerValues = req.getHeaders(*it);
		for(std::vector<std::string>::const_iterator vit = headerValues->begin();
		    vit != headerValues->end(); vit++)
		    out<<*it<<':'<<*vit<<std::endl;
	}
	out<<"============\nProtocol="<<req.getProtocol()<<std::endl;
	out << "\n==========\nHTTP Method = "+req.getMethod();
	std::string testHeaderName = "if-modified-since";
	out<<"\n"<<testHeaderName+":"+req.getHeader(testHeaderName);
	try {
	        long l = req.getDateHeader(testHeaderName);
		std::stringstream s;
		s<<l;
		std::string dateHeader = s.str();
		out<<"\n"<<testHeaderName+" value = "+dateHeader;
	} catch (servlet::ServletException e) {
	        out<<"\n"<<testHeaderName+" is not formatted correctly";
	}
	std::string respTestHeaderName = "X-Test-Header";
	if(resp.containsHeader(respTestHeaderName)) {
	         out<<"\n"<<respTestHeaderName<<" is set";
	} else {
	         out<<"\n"<<respTestHeaderName<<" is not set";
	}
	out<<"</PRE>";
	renderFooter(out);
}

void HeadersServlet::renderHeader(std::ostream& out)
{
	out<<"<html>\n <head>\n  <title>Sample Headers Servlet for CPPSERV</title>\n </head>\n <body>";
}

void HeadersServlet::renderFooter(std::ostream& out)
{
	out<<"\n </body>\n</html>";
}

EXPORT_SERVLET(HeadersServlet)
