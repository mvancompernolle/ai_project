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
#include <serverconfig/serverconfig.h>
#include <ServletConfigImpl.h>
#include <iostream>
#include <functional>
#include <algorithm>
#include <dlfcn.h>

using namespace container;

namespace container {
namespace serverconfig {

ServerConfig::ServerConfig()
	: m_root(0)
	, m_globalContext(*this)
{
}


ServerConfig::~ServerConfig()
{
}

/*!
    \fn container::ServerConfig::processConfig()
	This function is called by main appserver object in order
	to create all the applications, servlets, etc.
	In a way this one can be viewed as real entry point,
	and main &Co are just a bootstraps.
 */
void ServerConfig::processConfig()
{
	if(!m_globalContext.process(*this, m_root) ||
	   !m_globalContext.complete())
	{
		fart("Unable to process configuration");
	}
}

/*!
    \fn container::ServerConfig::fart(const char*) throw (servlet::CPPServException)
 */
void ServerConfig::fart(const char* msg) throw (servlet::ServletException)
{
	std::cerr<<"error during configuration: "<<msg<<'\n';
	throw servlet::ServletException();
}

/*!
 * \fn container::ServerConfig::setGlobalParam(const std::string& type, const std::string& value)
 */
bool ServerConfig::setGlobalParam(const std::string& type, const std::string& value)
{
	return m_globalContext.setGlobalParam(type, value);
}

}
}
