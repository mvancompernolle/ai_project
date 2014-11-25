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
#include <iostream>

namespace container {
namespace serverconfig {

/*!
 * \fn ContextRegistry::getContext(const ConfigNode& node, ServerConfig::Context* parent)
 * @param node Node that caused creation of this context (node creator may look at attributes)
 * @param parent Parent context to pass to context factory
 * @return pointer to new Context object registered under type or NULL
 * if no such object is available
 */
Context* ContextRegistry::getContext(const ConfigNode& node, 
	Context* parent)
{
	registry_t::iterator it=m_registry.find(node.getType());
	if(it==m_registry.end())
		return 0;
	return it->second.create(node, parent);
}

/*!
 * \fn ContextRegistry::registerContext(const std::string& type, createcontext_f creator)
 */
void ContextRegistry::registerContext(const std::string& type, ContextRegistry::createcontext_f creator, int flags)
{
	ContextInfo cinfo(flags, creator);
	m_registry[type]=cinfo;
}

Context* ContextRegistry::ContextInfo::create(const ConfigNode& node,
	Context* parent)
{
	if(m_flags&PARAM_SINGLE_OF_TYPE && m_type_done) {
		std::cerr<<"Error while registering context "<<node.getType()
			<<" (name="<<node.getName()
			<<"): another one of same type already exists"<<std::endl;
		return 0;
	}
	if(m_flags&PARAM_SINGLE_OF_NAME && m_names.find(node.getName())!=m_names.end()) {
		std::cerr<<"Error while registering context "<<node.getType()
			<<" (name="<<node.getName()
			<<"): another one with same name already exists"<<std::endl;
		return 0;
	}
	m_type_done = true;
	m_names.insert(node.getName());
	return m_f(node, parent);
}

}
}
