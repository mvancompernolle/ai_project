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
#include <ServletConfigImpl.h>

namespace container {

ServletConfigImpl::ServletConfigImpl(servlet::ServletContext* ctx, const std::string& name)
	: ServletConfig()
	, m_ctx(ctx)
	, m_name(name)
{
}


ServletConfigImpl::~ServletConfigImpl()
{
}

/*!
    \fn container::ServletConfigImpl::addParam(const std::string& name, const std::string& val)
 */
void ServletConfigImpl::addParam(const std::string& name, const std::string& val)
{
	m_parameters[name]=val;
}

std::string ServletConfigImpl::getServletName() const
{
	return m_name;
}
servlet::ServletContext&  ServletConfigImpl::getServletContext() const
{
	return *m_ctx;
}
std::string  ServletConfigImpl::getInitParameter(const std::string& name) const
{
	util::param_t::const_iterator it = m_parameters.find(name);
	if(m_parameters.end() == it)
	    return "";
	return it->second;
}
std::auto_ptr< std::vector<std::string> > ServletConfigImpl::getInitParameterNames() const
{
	return util::getMapKeyNames(m_parameters);
}

}
