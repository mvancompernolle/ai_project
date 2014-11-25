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
 * \fn container::ServletContext::ServletContext(Context *parent, const std::string& path, const std::string &dso, bool hidden)
 * @param parent Parent context
 * @param path Virtual path this servlet is mapped to
 * @param dso path to shared library that contains this servlet
 * @param hidden Whether to enable access to this servlet through HTTP
 */
ServletContext::ServletContext(Context* parent, const std::string& path, const std::string& dso, bool hidden)
	: Context(parent->getServerConfig(), parent)
	, m_dso(dso)
	, m_hidden(hidden)
	, m_path(path)
	, m_maxRequestSize(-1)
	, m_maxFileSize(-1)
{
	m_paramregistry.getParamList(getUnsetParams());
}

/*!
 * \fn virtual bool container::ServletContext::onSetParam(const ConfigNode& node)
 */
bool ServletContext::onSetParam(const ConfigNode& node)
{
	return m_paramregistry.setParam(node, this);
}

void ServletContext::registerContexts(ContextRegistry&)
{
	//No subcontexts for servlet.
}

void ServletContext::registerParams(ParamRegistry<ServletContext>& reg)
{
	reg.registerParam("parameter",&ServletContext::addInitParam,0);
	reg.registerParam("max_request_size",&ServletContext::setMaxRequestSize,PARAM_INHERITABLE|PARAM_SINGLE_OF_TYPE);
	reg.registerParam("max_file_size",&ServletContext::setMaxFileSize,PARAM_INHERITABLE|PARAM_SINGLE_OF_TYPE);
}

bool ServletContext::addInitParam(const ConfigNode& val)
{
	util::param_t::const_iterator name=val.getAttrs().find("name");
	util::param_t::const_iterator value=val.getAttrs().find("value");
	if(value == val.getAttrs().end()) {
		std::cerr<<"Servlet parameter has no name"<<std::endl;
		return false;
	}
	if(name == val.getAttrs().end()) {
		std::cerr<<"Servlet parameter has no value";
		return false;
	}
	m_params.push_back(std::pair<std::string,std::string>(name->second,value->second));
	return true;
}

bool ServletContext::setMaxRequestSize(const ConfigNode& val)
{
	util::param_t::const_iterator value=val.getAttrs().find("value");
	if(value == val.getAttrs().end()) {
		std::cerr<<"Maximum request size parameter has no value"<<std::endl;
		return false;
	}
	m_maxRequestSize = atol(value->second.c_str());
	return true;
}

bool ServletContext::setMaxFileSize(const ConfigNode& val)
{
	util::param_t::const_iterator value=val.getAttrs().find("value");
	if(value == val.getAttrs().end()) {
		std::cerr<<"Maximum file size parameter has no value"<<std::endl;
		return false;
	}
	m_maxFileSize = atol(value->second.c_str());
	return true;
}

Context* ServletContext::contextCreator(const ConfigNode& n, Context* parent)
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
	return new ServletContext(parent, path, dso->second, hidden);
}

bool ServletContext::onPostComplete()
{
	AppContext* app=static_cast<AppContext*>(getParent());
	if(m_path.empty())
		m_path = getName();
	ServletConfigImpl* conf=app->addServlet(m_path, getServletName(), m_dso, m_hidden, m_maxRequestSize, m_maxFileSize);
	if(!conf){
		std::cerr<<"Unable to add servlet "<<m_path<<" "<<getServletName()<<std::endl;
		return false;
	}
	for(util::pairlist_t::iterator it=m_params.begin(); it!=m_params.end(); it++){
		conf->addParam(it->first,it->second);
	}
	return true;
}

std::string ServletContext::getServletName()
{
	return getName();
}

ParamRegistry<ServletContext> ServletContext::m_paramregistry(ServletContext::registerParams);
ContextRegistry ServletContext::m_contextregistry(ServletContext::registerContexts);

}
}
