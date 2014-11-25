/***************************************************************************
 *   Copyright (C) 2007 by Ilya A. Volynets-Evenbakh                       *
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
#include "initparams.h"
#include <iostream>
#include <servlet/Util.h>

void InitParamsServlet::service(servlet::HttpServletRequest& req, servlet::HttpServletResponse& resp)
{
	std::ostream &out=resp.getOutputStream();
	renderHeader(out);
	testInitParams(req,resp);
	renderFooter(out);
}

void InitParamsServlet::testInitParams(servlet::HttpServletRequest& req, servlet::HttpServletResponse& resp)
{
	std::ostream &out=resp.getOutputStream();
	std::string paramNow = getServletConfig().getInitParameter("initparam");
	out<<"current_value = \""<<paramNow<<"\"<br>"<<std::endl;
	out<<"init_value = \""<<paramInit<<"\"<br>"<<std::endl;
}

void InitParamsServlet::renderHeader(std::ostream& out)
{
	out<<"<html>\n <head>\n  <title>Parameter Test Servlet for CPPSERV</title>\n </head>\n <body>";
}

void InitParamsServlet::renderFooter(std::ostream& out)
{
	out<<"\n </body>\n</html>";
}

void InitParamsServlet::init()
{
	paramInit = getServletConfig().getInitParameter("initparam");
	std::cerr<<"Got init param: "<<paramInit<<std::endl;
}

void InitParamsServlet::destroy()
{
	std::cerr<<"Well, hello there! I'm destroyed"<<std::endl;
}

EXPORT_SERVLET(InitParamsServlet)
