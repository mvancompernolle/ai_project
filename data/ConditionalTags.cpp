/***************************************************************************
 *   Copyright (C) 2011 by Ilya A. Volynets-Evenbakh                       *
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

#include "StdTagBase.h"
#include <ctype.h>
#include <sstream>
#include <list>

using namespace servlet::taglib;
using namespace std;

namespace csp
{
namespace tags
{


/// @class IfTag represents <csp:if>
/// Tag synopsys <csp:if test="[condition]">...</csp:if>
/// @param test Boolean C++ expression
class IfTag: public StdTagBase
{
private:
	string m_cond;
public:
	IfTag(const string& name)
		: StdTagBase(name)
	{}
	virtual void doStartTag(const attribs_t& attribs);
	virtual void doEndTag();
};

void IfTag::doStartTag(const Generator::attribs_t& attribs)
{
	get_attr(attribs, "test", m_cond, "csp:if");
	*body << "if(" << m_cond << ") {"<<endl;
	//TODO: indent generated code in output?
}

void IfTag::doEndTag()
{
	*body<<endl<<"} /* end csp:foreach_attr: "<<m_cond<<" */"<<endl;
}

/// @class ChoiceTag represents <csp:choice>
/// Tag synopsys
/// <csp:choice>
///  <csp:when test="[condition]">...</csp:when>
///  <csp:when test="[condition]">...</csp:when>
///  <csp:otherwise>...</csp:otherwise>
/// </csp:choice>
/// @param test Boolean C++ expression
/// This tag generates if..else if..else construct.
/// Currently only minimal checking is done to ensure
/// no invalid constructs are present
class ChoiceTag: public StdTagBase
{
private:
	stringstream m_my_body;
	struct condition
	{
		string name;
		string cond; // ignored by "otherwise"
		string body;
	};
	list<condition> m_conditions;
	condition* m_my_condition;
public:
	ChoiceTag(const string& name)
		: StdTagBase(name)
		, m_my_condition(0)
	{}
	virtual void doStartTag(const attribs_t& attribs);
	virtual void doEndTag();
	virtual void initChildBuffers();
private:
	void doWhenStartTag(const attribs_t& attribs);
	void doTopEndTag();
	void doChildEndTag();
};

void ChoiceTag::initChildBuffers()
{
	StdTagBase::initChildBuffers();
	m_child_body = &m_my_body;
	if(getName()!="csp:choice") {
		ChoiceTag* parent = (ChoiceTag*)this->parent;
		condition c;
		parent->m_conditions.push_back(c);
		m_my_condition = &parent->m_conditions.back();
		m_my_condition->name = getName();
	}
}

void ChoiceTag::doWhenStartTag(const Generator::attribs_t& attribs)
{
	ChoiceTag* parent = (ChoiceTag*)this->parent;
	get_attr(attribs, "test", m_my_condition->cond, "csp:when");
}

void ChoiceTag::doStartTag(const Generator::attribs_t& attribs)
{
	if(getName() != "csp:choice") {
		if(this->parent->getName()!="csp:choice")
			throw runtime_error("<"+getName()+"> must be enclosed in <csp:choice> tag.");
	}
	if(getName() == "csp:when")
		doWhenStartTag(attribs);
}

void ChoiceTag::doEndTag()
{
	if(getName() == "csp:choice")
		doTopEndTag();
	else
		doChildEndTag();
}
void ChoiceTag::doChildEndTag()
{
	m_my_condition->body.assign(m_my_body.str());
}
void ChoiceTag::doTopEndTag()
{
	///FIXME: provide some checking for <csp:choice> body
	/*string cb = m_my_body.str();
	if(cb.find_first_not_of(" \t\n") != string::npos)
	throw runtime_error("<csp:choice> body contains something other then <csp:when> or <csp:otherwise>: '"+cb+"'");*/
	bool otherwise = false;
	bool first = true;
	for(list<condition>::iterator it = m_conditions.begin(); it!=m_conditions.end(); it++) {
		if(otherwise) {
			throw runtime_error("<csp:otherwise> can only be the last child of <csp:choice>");
		}
		if(it->name == "csp:when") {
			*body<<((first)?"":"else ")<<"if("<<it->cond<<") {\n" << it->body << "\n}\n";
			first=false;
		} else if(it->name == "csp:otherwise") {
			otherwise = true;
			*body<<"else {\n" << it->body << "\n}\n";
		} else {
			throw runtime_error("Invalid child of <csp:choice>: "+it->name);
		}
	}
}
/// @class SwitchTag represents <csp:switch>
/// Tag synopsys
/// <csp:switch test="int expr">
///  <csp:case value="[int]" [fallthrough="true"]>...</csp:case>
///  <csp:default>...</csp:default>
/// </csp:switch>
/// @param test Expression whose result to test.
/// @param value integer or enum value
/// @param fallthrough true or false. If true, no break
///        will be generated at the end of case statement
/// This tag generates switch statement construct.
/// Currently only minimal checking is done to ensure
/// no invalid constructs are present
class SwitchTag: public StdTagBase
{
private:
	stringstream m_my_body;
	string m_test;
	struct condition
	{
		string name;
		string cond; // ignored by "default"
		string body;
		bool   fallthrough;
	};
	list<condition> m_conditions;
	condition* m_my_condition;
public:
	SwitchTag(const string& name)
		: StdTagBase(name)
		, m_my_condition(0)
	{}
	virtual void doStartTag(const attribs_t& attribs);
	virtual void doEndTag();
	virtual void initChildBuffers();
private:
	void doTopEndTag();
	void doChildEndTag();
};

void SwitchTag::initChildBuffers()
{
	StdTagBase::initChildBuffers();
	m_child_body = &m_my_body;
	if(getName()!="csp:switch") {
		SwitchTag* parent = (SwitchTag*)this->parent;
		condition c;
		parent->m_conditions.push_back(c);
		m_my_condition = &parent->m_conditions.back();
		m_my_condition->name = getName();
		m_my_condition->fallthrough = false;
	}
}

void SwitchTag::doStartTag(const Generator::attribs_t& attribs)
{
	if(getName() != "csp:switch") {
		if(this->parent->getName()!="csp:switch")
			throw runtime_error("<"+getName()+"> must be enclosed in <csp:switch> tag.");
		SwitchTag* parent = (SwitchTag*)this->parent;
		if(getName() == "csp:case")
			get_attr(attribs, "value", m_my_condition->cond, "csp:case");
		string ft;
		get_attr(attribs, "fallthrough", ft, getName(), "false");
		m_my_condition->fallthrough = (ft == "true");
	} else {
		get_attr(attribs, "test", m_test, "csp:switch");
	}
}

void SwitchTag::doEndTag()
{
	if(getName() == "csp:switch")
		doTopEndTag();
	else
		doChildEndTag();
}
void SwitchTag::doChildEndTag()
{
	m_my_condition->body.assign(m_my_body.str());
}
void SwitchTag::doTopEndTag()
{
	///FIXME: provide some checking for <csp:switch> body
	/*string cb = m_my_body.str();
	if(cb.find_first_not_of(" \t\n") != string::npos)
	throw runtime_error("<csp:switch> body contains something other then <csp:case> or <csp:default>: '"+cb+"'");*/
	int otherwise = 0;
	*body<<"switch ("<<m_test<<") {\n";
	for(list<condition>::iterator it = m_conditions.begin(); it!=m_conditions.end(); it++) {
		if(it->name == "csp:case") {
			*body<<"\tcase "<<it->cond<<":\n\t{" << it->body << "\n\t}\n";
			if(!it->fallthrough)
				*body<<"\tbreak;\n";
		} else if(it->name == "csp:default") {
			otherwise++;
			*body<<"default:\n\t{\n" << it->body << "\n\t}\n";
			if(!it->fallthrough)
				*body<<"\tbreak;\n";
		} else {
			throw runtime_error("Invalid child of <csp:switch>: "+it->name);
		}
		if(otherwise>1) {
			throw runtime_error("<csp:default> can only appear once in <csp:switch>");
		}
	}
	*body<<"} /* end of switch("<<m_test<<") */\n";
}

} //namespace tags
} //namespace csp
DECLARE_COMPILE_TIME_TAGLIB(csp)
EXPORT_COMPILE_TIME_TAG(csp, if, csp::tags::IfTag)
EXPORT_COMPILE_TIME_TAG(csp, choice, csp::tags::ChoiceTag)
EXPORT_COMPILE_TIME_TAG(csp, when, csp::tags::ChoiceTag)
EXPORT_COMPILE_TIME_TAG(csp, otherwise, csp::tags::ChoiceTag)
EXPORT_COMPILE_TIME_TAG(csp, switch, csp::tags::SwitchTag)
EXPORT_COMPILE_TIME_TAG(csp, case, csp::tags::SwitchTag)
EXPORT_COMPILE_TIME_TAG(csp, default, csp::tags::SwitchTag)
