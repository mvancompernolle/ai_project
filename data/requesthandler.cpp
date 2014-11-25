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

#include "requesthandler.h"
#include <servlet/HttpServletResponse.h> // Needed for SC_NOTFOUND constant
#include <servlet/IllegalStateException.h>
#include <iostream>
#include <fstream>
#include <dlfcn.h>

namespace container {

RequestHandler::urlmap_t RequestHandler::m_urlMap;
sptk::CRWLock RequestHandler::m_servletMapLock;

RequestHandler::RequestHandler(Connection* con)
	: m_con(con)
{
}


RequestHandler::~RequestHandler()
{
	try {
		delete m_con;//This may need to be done elsewhere..
	} catch (sockerr& e) {
		std::cerr<<"Error while closing connection: "<<e.errstr()<<std::endl;
	}
}
//TODO: make this configurable
//      (perhaps per-app and with some sort of substitution syntax)
std::string RequestHandler::m_noSuchServletText(
	"<HTML>"
		"<HEAD>"
			"<TITLE>404 - resource not found</TITLE>"
		"</HEAD>"
		"<BODY>"
			"<H1>404 - resource not found</H1>"
		"</BODY>"
	"</HTML>");
}



/*!
 * \fn container::RequestHandler::noSuchServlet(container::Connection& con)
 */
void container::RequestHandler::noSuchServlet()
{
	std::ostream out(m_con);
	out<<"Status: "<<servlet::HttpServletResponse::SC_NOT_FOUND<<"\n"
		"Content-type: text/html\n\n";
	out<<m_noSuchServletText<<std::endl;
}


/*!
	\fn container::RequestHandler::addServlet(const std::string& name, ServletContainer *cont)
 */
bool container::RequestHandler::addServlet(const std::string& name,ServletContainer *cont)
{
	m_servletMapLock.lockRW();
	if(m_urlMap.getServletDesc(name)) { // Such servlet already mapped
		m_servletMapLock.unlock();
		return false;
	}
	ServDesc* desc=new ServDesc(cont);
	m_urlMap[name]=desc;
	m_servletMapLock.unlock();
	return true;
}
bool container::RequestHandler::removeServlet(const std::string& name)
{
	ServDesc *desc=m_urlMap.getServletDesc(name);
	if(desc == 0)
		return true; // Hmm... Should we return false?
	m_servletMapLock.lockRW();
	m_urlMap.removeDesc(name);
	m_servletMapLock.unlock();
	//Once the following lock/unlock is done we can be
	//sure this servlet is no longer used, since we already removed
	//it from the map
	desc->m_servletLock.lockRW();
	desc->m_servletLock.unlock();
	delete desc;
	return true;
}


/*!
    \fn container::RequestHandler::run()
 */
void container::RequestHandler::taskFunction()
{
	std::string line;
	std::istream icon(m_con);
	std::getline(icon,line);
	if(line[line.size()-1]=='\r')
		line.erase(line.size()-1,1);
	m_servletMapLock.lockR();
	ServDesc *desc=m_urlMap.getServletDesc(line);
	if(desc == 0){
		urlmap_t* sm = m_urlMap.getPath(line, false);
		if(sm)
			desc = sm->getServletDesc("IndexServlet");
	}
	if(desc == 0){
		m_servletMapLock.unlock();
		std::cerr<<"Unable to map \""<<line<<"\" to anything"<<std::endl;
		while(getline(icon,line))
			std::cerr<<"Line: "<<line<<std::endl;
		noSuchServlet();
		return;
	} else {
		std::cerr<<"Found mapping for "<<line<<std::endl;
	}
	desc->m_servletLock.lockR();
	m_servletMapLock.unlock();
	try {
		desc->m_cont->service(*m_con);
	} catch (servlet::Traceable& e) {
		e.printStackTrace(std::cerr);
	} catch (std::exception& e) {
		std::cerr<<"Run-time error occured: "<<e.what()<<std::endl;
	} catch (...){
		std::cerr<<"Exception cought while processing request\n";
	}
	desc->m_servletLock.unlock();
}
