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
#include "reset.h"
#include "servlet/ServletResponse.h"

void ResetServlet::service(servlet::HttpServletRequest& req, servlet::HttpServletResponse& resp)
{
	std::ostream &out=resp.getOutputStream();
	std::auto_ptr<servlet::HttpServletRequest::cookielist_t> cookies=req.getCookies();
	servlet::HttpServletRequest::cookielist_t::iterator it=cookies->find("TestResetCookie");
	std::string reset=req.getParameter("reset");
	servlet::Cookie cookie("TestResetCookie","bla-bla");
	cookie.setMaxAge(3600);
	resp.addCookie(cookie);
	long seconds = time(NULL) + 3600;
	resp.setDateHeader("expires", seconds);
	if(reset!="false") {
		resp.reset();
	}
	renderHeader(out);
	if(it==cookies->end()) {
		out<<"TestResetCookie and Expires response header are not set because of work of reset().<br>";
		out<<"<a href=\""<<req.getRequestURI()<<"?reset=false\">Turn off reset</a>\n";
	} else {
		out<<"Reset is turned off.<br>";
		out<<"TestResetCookie = "<<it->second->getValue()<<" is set\n";
	}
	renderFooter(out);
}

void ResetServlet::renderHeader(std::ostream& out)
{
	out<<"<html>\n <head>\n  <title>Sample Reset Servlet for CPPSERV</title>\n </head>\n <body>";
}

void ResetServlet::renderFooter(std::ostream& out)
{
	out<<"\n </body>\n</html>";
}

EXPORT_SERVLET(ResetServlet)
