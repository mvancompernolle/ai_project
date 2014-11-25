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
#include "cookies.h"
#include <servlet/Cookie.h>

void CookiesServlet::service(servlet::HttpServletRequest& req, servlet::HttpServletResponse& resp)
{
	std::ostream &out=resp.getOutputStream();
	std::auto_ptr<servlet::HttpServletRequest::cookielist_t> cookies=req.getCookies();
	servlet::HttpServletRequest::cookielist_t::iterator it=cookies->find("MyCookie");
	std::string unset=req.getParameter("unset");
	if(unset=="true") {
	    if(it!=cookies->end()) {
		it->second->setMaxAge(0);
		resp.addCookie(*(it->second));
	    }
	} else {
	    servlet::Cookie cookie("MyCookie","Bubu-gaga");
	    cookie.setMaxAge(3600);
	    resp.addCookie(cookie);
	}
	if(req.getParameter("setdouble") == "1") {
		servlet::Cookie cookie1("doublecookie", "first one");
		servlet::Cookie cookie2("doublecookie", "second one");
		cookie1.setPath("/");
		cookie2.setPath("/cserv");
		//cookie1.setVersion(0);
		//cookie2.setVersion(0);
		resp.addCookie(cookie1);
		resp.addCookie(cookie2);
	}
	std::string usercookie=req.getParameter("usercookie");
	if(!usercookie.empty())
	{
		std::string ucomment=req.getParameter("ucomment");
		servlet::Cookie cookie("UserCookie", usercookie);
		cookie.setComment(ucomment);
		resp.addCookie(cookie);
	}
	renderHeader(out);
	if(it==cookies->end()) {
		out<<"Cookie is not set<br>\n";
	} else {
		out<<it->second->getValue()<<"<br>\n";
		out<<"<a href=\""<<req.getRequestURI()<<"?unset=true\">Unset</a><br>\n";
	}
	it=cookies->find("UserCookie");
	if(it==cookies->end()) {
		out<<"<br>User cookie is not set<br>\n";
	} else {
		out<<"<br>User cookie: \""<<it->second->getValue()<<"\"<br>\n";
	}
	out<<"<BR><FORM method=\"post\" action=\""<<req.getRequestURI()<<"\">"
		"User cookie value: <input type=\"text\" name=\"usercookie\"><br>"
		"User cookie comment: <input type=\"text\" name=\"ucomment\"><br>"
		"<input type=\"submit\" value=\"Set User Cookie\">"
		"</FORM>"<<std::endl;
	out<<"<br><a href=\""<<req.getRequestURI()<<"?setdouble=1\">Set Double Cookie</a>\n";
	out<<"<br><pre>\nFull cookie list:\n";
	for(it=cookies->begin(); it != cookies->end(); it++) {
		out<<it->first<<'='<<it->second->getValue()<<"; Path=";
		out<<it->second->getPath()<<"; Domain="<<it->second->getDomain()<<std::endl;
		//out<<"; Port="<<it->second->getPort()<<std::endl;
	}
	renderFooter(out);
}

void CookiesServlet::renderHeader(std::ostream& out)
{
	out<<"<html>\n <head>\n  <title>Sample Cookies Servlet for CPPSERV</title>\n </head>\n <body>";
}

void CookiesServlet::renderFooter(std::ostream& out)
{
	out<<"\n </body>\n</html>";
}

EXPORT_SERVLET(CookiesServlet)
