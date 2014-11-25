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
#include <HttpSessionImpl.h>
#include <iostream>
#include <util.h>

namespace container {

HttpSessionImpl::HttpSessionImpl(servlet::ServletContext& ctx, const std::string& id, time_t timeout)
 : m_maxInactiveInterval(timeout)
 , m_creationTime (time(0))
 , m_servletContext(ctx)
 , m_id(id)
 , m_isNew(true)
 , m_isValid(true)
{
	this->m_lastAccessTime = this->m_creationTime;
}


HttpSessionImpl::~HttpSessionImpl()
{
}

long HttpSessionImpl::getCreationTime() const
{
	return m_creationTime;
}

long HttpSessionImpl::getLastAccessedTime() const
{
	return m_lastAccessTime;
}

const servlet::ServletContext& HttpSessionImpl::getServletContext() const
{
	return m_servletContext;
}

int HttpSessionImpl::getMaxInactiveInterval() const
{
	return m_maxInactiveInterval;
}

boost::shared_ptr<void> HttpSessionImpl::getAttribute(const std::string& name)
{
	attr_t::iterator iRet = m_attrs.find(name);
	if(iRet == m_attrs.end())
		return boost::shared_ptr<void>();
	return iRet->second;
}

std::auto_ptr<std::vector<std::string> > HttpSessionImpl::getAttributeNames() const
{
	return container::util::getMapKeyNames(m_attrs);
}

void HttpSessionImpl::setAttribute(const std::string& name, boost::shared_ptr<void> value)
{
	m_attrs[name] = value;
}

void HttpSessionImpl::removeAttribute(const std::string& name)
{
	attr_t::iterator it = m_attrs.find(name);
	if(it == m_attrs.end())
	    return;
	m_attrs.erase(it);
}

void HttpSessionImpl::invalidate()
{
	m_isValid = false;
}

bool HttpSessionImpl::isNew() const
{
	return m_isNew;
}

std::string HttpSessionImpl::getId() const
{
	return m_id;
}

void  HttpSessionImpl::setMaxInactiveInterval(int interval)
{
	m_maxInactiveInterval = interval;
}

bool HttpSessionImpl::hasAttribute(const std::string& name) const
{
	return m_attrs.find(name) != m_attrs.end();
}

}
