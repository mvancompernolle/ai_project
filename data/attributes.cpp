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
#include "attributes.h"
#include <iostream>
#include <sstream>

void AttributesServlet::service(servlet::HttpServletRequest& req, servlet::HttpServletResponse& resp)
{
	std::ostream &out = resp.getOutputStream();
	renderHeader(out);
	out<<"<PRE>";
	out<<"\nRequest URI:  "<<req.getRequestURI();
	out<<"\nRequest URL:  "<<req.getRequestURL();
	out<<"\nContext Path: "<<req.getContextPath();
	out<<"\nServletPath:  "<<req.getServletPath();
	out<<"\nQuery String: "<<req.getQueryString();
	out<<"\nPath info:    "<<req.getPathInfo();
	out<<"\nLocal Address:"<<req.getLocalAddr();
	out<<"\nRemote Address:"<<req.getRemoteAddr();
	out<<"\nLocal Name   :"<<req.getLocalName();
	out<<"\nRemote Host  :"<<req.getRemoteHost();
	out<<"\nServlet Name :"<<getServletName();
	out<<"\nPathInfo Translated: "<<req.getPathTranslated();
	out<<"\nMIME type (text): "<<getServletContext().getMimeType("tests/attributes/attributes.cpp");
	out<<"\nMIME type (html): "<<getServletContext().getMimeType("tests/multiple/multiple.csp");
	out<<"\n</PRE>";
	renderFooter(out);
}

void AttributesServlet::renderHeader(std::ostream& out)
{
	out<<"<html>\n <head>\n  <title>Sample Attributes Servlet for CPPSERV</title>\n </head>\n <body>";
}

void AttributesServlet::renderFooter(std::ostream& out)
{
	out<<"\n </body>\n</html>";
}

EXPORT_SERVLET(AttributesServlet)
