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
#include "params.h"
#include <iostream>
#include <servlet/Util.h>

#define ARGNPRINT_VAL "Nonprint: &quot;!@#$%^&*()-+=&quot;"

void ParamsServlet::service(servlet::HttpServletRequest& req, servlet::HttpServletResponse& resp)
{
	std::ostream &out=resp.getOutputStream();
	renderHeader(out);
	testParams(req,resp);
	renderForm(req,resp);
	renderFooter(out);
}

void ParamsServlet::testParams(servlet::HttpServletRequest& req, servlet::HttpServletResponse& resp)
{
	std::ostream &out=resp.getOutputStream();
	std::string arg=req.getParameter("arg");
	std::string argNPrint=req.getParameter("argNPrint");
	out<<"arg=\""<<arg<<"\" (this should be whatever you typed in)<br>";
	out<<"argNPrint='"<<argNPrint<<"' should be '"<<ARGNPRINT_VAL<<"'... ";//<<
	//    ((argNPrint==ARGNPRINT_VAL)?"OK":"Failed");
	std::auto_ptr< std::vector<std::string> > multiArg= req.getParameterValues("multiArg");
	out<<"\n<BR>MultiArg:<br>\n";
	for(std::vector<std::string>::iterator it=multiArg->begin();
	    it!=multiArg->end(); it++)
	{
		out<<"&nbsp;"<<*it<<"<br>\n";
	}
}

void ParamsServlet::renderForm(servlet::HttpServletRequest& req, servlet::HttpServletResponse& resp)
{
	std::ostream &out=resp.getOutputStream();
	std::string arg=req.getParameter("arg");
	std::string argEnc;
	servlet::util::urlEncode(arg,argEnc);
	out<<"Click on get or post button blow to switch kind of test performed<br>"<<std::endl;
	out<<"<FORM ACTION=\""<<req.getRequestURI()<<"\" METHOD=\"GET\">"
	     "<INPUT TYPE=\"SUBMIT\" VALUE=\"Do GET Tests\"></FORM>"
	     "<FORM ACTION=\""<<req.getRequestURI()<<"\" METHOD=\"POST\">"
	     "<INPUT TYPE=\"SUBMIT\" VALUE=\"Do POST Tests\"></FORM><BR>";
	out<<"<FORM ACTION=\""<<req.getRequestURI()<<"\" METHOD=\""<<req.getMethod()<<"\">"
	     "<INPUT TYPE=\"text\" name=\"arg\" value=\""<<arg<<"\">"
	     "<input type=\"submit\" value=\"Simple Arg Test\"></FORM><br>";
	out<<"<FORM ACTION=\""<<req.getRequestURI()<<"\" METHOD=\""<<req.getMethod()<<"\">"
	     "<INPUT TYPE=\"hidden\" name=\"argNPrint\" value=\""ARGNPRINT_VAL"\">"
	     "<input type=\"submit\" value=\"Non-printable Test\"></FORM><br>";
	out<<"<FORM ACTION=\""<<req.getRequestURI()<<"\" METHOD=\""<<req.getMethod()<<"\">"
	     "<INPUT TYPE=\"hidden\" name=\"multiArg\" value=\"val1\">"
	     "<INPUT TYPE=\"hidden\" name=\"multiArg\" value=\"val2\">"
	     "<INPUT TYPE=\"hidden\" name=\"multiArg\" value=\"val3\">"
	     "<INPUT TYPE=\"hidden\" name=\"multiArg\" value=\"val4\">"
	     "<INPUT TYPE=\"SUBMIT\" VALUE=\"Multiple Arguments Test\"></FORM><BR>";
	if(req.getMethod()=="get") {
	    out<<"<A HREF=\""<<req.getRequestURI()<<"?=moo\">Invalid Arg Test1</A><br>";
	    out<<"<A HREF=\""<<req.getRequestURI()<<"?inval2\">Invalid Arg Test1</A><br>";
	}
}

void ParamsServlet::renderHeader(std::ostream& out)
{
	out<<"<html>\n <head>\n  <title>Parameter Test Servlet for CPPSERV</title>\n </head>\n <body>";
}

void ParamsServlet::renderFooter(std::ostream& out)
{
	out<<"\n </body>\n</html>";
}

void ParamsServlet::init()
{
	std::cerr<<__PRETTY_FUNCTION__<<std::endl;
}

EXPORT_SERVLET(ParamsServlet)
