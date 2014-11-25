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
#include "redirect.h"

void RedirectServlet::service(servlet::HttpServletRequest& req, servlet::HttpServletResponse& resp)
{
	std::ostream &out=resp.getOutputStream();
	std::string doRedir=req.getParameter("doRedir");
	if(doRedir=="yes") {
	    std::string url=req.getRequestURL();
	    std::string::size_type namePos=url.rfind("/");
	    url.erase(namePos+1);
	    url+="FileUploadServlet";
	    resp.sendRedirect(url);
	} else {
	    renderHeader(out);
	    out<<"<A HREF=\""<<req.getRequestURL()<<"?doRedir=yes\">Test Redirect</A>";
	    renderFooter(out);
	}
}

void RedirectServlet::renderHeader(std::ostream& out)
{
	out<<"<html>\n <head>\n  <title>Sample Redirect Servlet for CPPSERV</title>\n </head>\n <body>";
}

void RedirectServlet::renderFooter(std::ostream& out)
{
	out<<"\n </body>\n</html>";
}

EXPORT_SERVLET(RedirectServlet)
