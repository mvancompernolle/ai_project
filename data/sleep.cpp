/***************************************************************************
 *   Copyright (C) 2006 by Ilya A. Volynets-Evenbakh                       *
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
#include "sleep.h"
#include <iostream>
void SleepServlet::service(servlet::HttpServletRequest&, servlet::HttpServletResponse& resp)
{
	std::cerr<<__PRETTY_FUNCTION__<<std::endl;
	std::ostream &out=resp.getOutputStream();
	sleep(10);
	out<<"<HTML><HEAD><TITLE>Sleep CPPSERV Servlet</TITLE></HEAD>"
	     "<BODY><H1>I just had a nice nap</H1>"
	     "<P>I am sample servlet running in "
	     "<A HREF=\"http://www.total-knowledge.com/progs/cppserv\">CPPSERV</A>"
	     "</P></BODY></HTML>";
}

EXPORT_SERVLET(SleepServlet)
