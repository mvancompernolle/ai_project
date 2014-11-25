/*
 *  Copyright (C) 2008-2009, 2011 Ilya A. Volynets-Evenbakh <ilya@total-knowledge.com>
 *  Copyright (C) 2005 Krzysztof Rzymkowski <rzymek@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "generator_manager.h"
#include <iostream>
#include <sstream>
using namespace std;

void generator_manager::load(const string& prefix, const string& uri, const generator_manager::attribs_t& attribs)
{
	if (registry.find(prefix) == registry.end()) {
		entry e;
		e.dso = new dso_code(uri);
		e.taglib = e.dso->func<taglibfactory_t>("create_taglib_"+prefix)(attribs);
		registry[e.taglib->getPrefix()] = e;
	}
}

servlet::taglib::Generator& generator_manager::get(const string& prefix, const string& name)
{
	registry_t::iterator r = registry.find(prefix);
	if (r == registry.end())
		throw runtime_error("Prefix "+prefix+" not registered");
	entry& e = r->second;
	e.dso->reload_if_needed();
	servlet::taglib::Generator* generator = e.taglib->getTag(name);
	if(!generators.empty())
		generator->setParent(generators.top().first);
	generator->setBuffers(*m_body,*m_head,*m_member);
	generator->initChildBuffers();
	m_head = generator->child_header();
	m_member = generator->child_member();
	m_body = generator->child_body();
	generators.push(std::pair<servlet::taglib::Generator*, entry*>(generator, &e));
	return *generator;
}

generator_manager::generator_manager()
	: m_head(&top_head)
	, m_member(&top_member)
	, m_body(&top_body)
{
}

generator_manager::~generator_manager()
{
	if (!generators.empty()) {
		std::cerr<<"There are still unclosed custom tags: "<<std::endl;
		while(!generators.empty()) {
			generators_t::value_type top = generators.top();
			std::cerr<<"\t"<<top.first->getName()<<std::endl;
			top.second->taglib->releaseTag(top.first);
			generators.pop();
		}
	}
			
	for (registry_t::iterator lib = registry.begin(); lib != registry.end(); ++lib) {
		entry& e = lib->second;
		delete e.dso;
		e.dso = NULL;
	}
}

servlet::taglib::Generator& generator_manager::pop(const std::string& name)
{
	if(generators.empty())
		throw runtime_error("Closing tag \""+name+"\" which wasn't open");
	std::pair<servlet::taglib::Generator*, entry*> top = generators.top();
	if(!top.first)
		throw runtime_error("Closing tag \""+name+"\" which wasn't open");
	std::string topname = top.first->getName();
	if(topname != name) {
		std::stringstream errmsg;
		errmsg<< "Closing tag \""<<name<<"\" which wasn't open (current tag: \""<<topname<<"\")";
		throw std::runtime_error(errmsg.str());
	}
	top.first->doEndTag();
	top.second->taglib->releaseTag(top.first);
	generators.pop();
	if(generators.empty())
	{
		m_head = &top_head;
		m_body = &top_body;
		m_member = &top_member;
	}
	else
	{
		m_head = generators.top().first->child_header();
		m_body = generators.top().first->child_body();
		m_member = generators.top().first->child_member();
	}
	return *top.first;
}
