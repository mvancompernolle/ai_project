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

#include <serverconfig/serverconfig.h>

namespace container {
namespace serverconfig {

/*!
 * \fn container::CSPContext::CSPContext(Context *parent, const std::string& path, const std::string &dso, bool hidden)
 * This context defines CSP (C++ Server Page). It is very similar to ServletContext, except it does some mangling on servlet
 * name before actually trying to create it.
 * @param parent Parent context
 * @param path Virtual path this CSP is mapped to
 * @param dso path to shared library that contains this CSP
 * @param hidden Whether to enable access to this CSP through HTTP
 */
CSPContext::CSPContext(Context* parent, const std::string& path, const std::string& dso, bool hidden)
	: ServletContext(parent, path, dso, hidden)
{
}

Context* CSPContext::contextCreator(const ConfigNode& n, Context* parent)
{
	util::param_t::const_iterator dso = n.getAttrs().find("dso");
	if(dso == n.getAttrs().end())
		ServerConfig::fart("app=>servlets[]->dso");
	util::param_t::const_iterator ithidden = n.getAttrs().find("hidden");
	bool hidden = false;
	if(ithidden != n.getAttrs().end() && ithidden->second == "true")
	    hidden = true;
	util::param_t::const_iterator pathit = n.getAttrs().find("path");
	std::string path;
	if(pathit != n.getAttrs().end())
		path = pathit->second;
	return new CSPContext(parent, path, dso->second, hidden);
}

std::string CSPContext::getServletName()
{
	std::string name = getName();
        std::string class_name = "CspServlet";
        for(std::string::size_type i=0;i<name.size();++i){
            if(!::isalnum(name[i]))
                class_name += '_';
            else
                class_name += name[i];
        }
        return class_name;
}

}
}
