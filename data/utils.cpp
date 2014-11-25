/***************************************************************************
 *   Copyright (C) 2006 by Sergey Jukov                                    *
 *   sergey@total-knowledge.com                                            *
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
#include "utils.h"
#include <servlet/Util.h>

void UtilServlet::service(servlet::HttpServletRequest& req, servlet::HttpServletResponse& resp)
{
	std::ostream &out=resp.getOutputStream();
	std::string myUrl=req.getParameter("param");

	renderHeader(out);

	out << "<br><form name=\"testurl\" action=\"#\"><b>Please enter URL:</b>";

	out << "&nbsp;&nbsp;<input type=\"text\" name=\"param\">" << "&nbsp;&nbsp;<input type=\"submit\" name=\"submit\"><br><br>";

	if(myUrl != ""){

	  std::string dest = "";
	  std::string myEncodedUrl = servlet::util::urlEncode(myUrl, dest);
	  std::string myDecodedUrl = servlet::util::urlDecode(myUrl);
	  servlet::util::urlInPlaceDecode(myDecodedUrl);

	  out << "<b>Original URL</b> = " << myUrl << "<br><br>";
	  out << "<b>Decoded URL</b> = " << myDecodedUrl << "<br><br>";
	  out << "<b>Encoded URL</b> = " << myEncodedUrl << "\n";

	}

	out << "</form>";

	renderFooter(out);
}

void UtilServlet::renderHeader(std::ostream& out)
{
	out << "<html>\n <head>\n  <title>Sample Util Servlet for CPPSERV</title>\n </head>\n <body>";
}

void UtilServlet::renderFooter(std::ostream& out)
{
	out << "\n </body>\n</html>";
}

EXPORT_SERVLET(UtilServlet)
