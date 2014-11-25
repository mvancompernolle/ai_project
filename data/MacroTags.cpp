/***************************************************************************
 *   Copyright (C) 2009, 2010 by Ilya A. Volynets-Evenbakh                  *
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

using namespace servlet::taglib;
using namespace std;

namespace csp
{
namespace tags
{

/// @class DefineTag represents <csp:define>
/// Tag synopsys: <csp:define name="<name>" params="<C++ argument definitions>">
/// Tag so defined, can later on be used with <csp:call> construct
/// or as C++ function (<% myDef(a, b, c); %> )
/// Note1: current implementation does no param syntax checking
/// Note2: current implementation doesn't allow referencing variables from
///        the caller function
/// Note3: argument names MUST be unique
class DefineTag: public StdTagBase
{
protected:
	typedef set<string> deflist_t;
	static deflist_t m_defines;
	string m_defname;
	stringstream func_body;
	bool with_servlet;
public:
	DefineTag(const string& name)
		: StdTagBase(name)
	{}
	virtual void doStartTag(const attribs_t& attribs);
	virtual void doEndTag();
	virtual void initChildBuffers();
};

DefineTag::deflist_t DefineTag::m_defines;

void DefineTag::initChildBuffers()
{
	StdTagBase::initChildBuffers();
	m_child_body=&func_body;
}

void DefineTag::doStartTag(const Generator::attribs_t& attribs)
{
	get_attr(attribs, "name", m_defname, "csp:define");
	if(m_defines.find(m_defname)!=m_defines.end())
		throw runtime_error(m_defname+" is already defined");
	string params;
	get_attr(attribs, "params", params, "csp:define");
	with_servlet = get_bool_attr(attribs, "with_servlet", "csp:define");
	stringstream s;
	s<<"protected:"<<endl;
	s<<"struct "<<m_defname<<"_t"<<endl;
	s<<"{"<<endl;
	s<<"	servlet::HttpServletRequest& request;"<<endl;
	s<<"	servlet::HttpServletResponse& response;"<<endl;
	s<<"	std::ostream& out;"<<endl;
	if(with_servlet)
		s<<"	servlet::HttpServlet& servlet;"<<endl;
	s<<"	"<<m_defname<<"_t (servlet::HttpServletRequest& req, servlet::HttpServletResponse& resp, std::ostream& o";
	if(with_servlet)
		s<<", servlet::HttpServlet& s";
	s<<")"<<endl;
	s<<"		: request(req)"<<endl;
	s<<"		, response(resp)"<<endl;
	s<<"		, out(o)"<<endl;
	if(with_servlet)
		s<<"		, servlet(s)"<<endl;
	s<<"	{}"<<endl;
	s<<"	void operator()("<<params<<")"<<endl;
	s<<"	{"<<endl;
	*member<<s.str();
}

void DefineTag::doEndTag()
{
	*member<<func_body.rdbuf()<<endl;
	*member<<"	}"<<endl;
	*member<<"};";
	*member<<"private:"<<endl;
	*body<<m_defname<<"_t "<<m_defname<<"(request, response, out";
	if(with_servlet)
		*body<<", *this";
	*body<<");"<<endl;
	*body<<"/* end csp:define "<<m_defname<<" */"<<endl;
	m_defines.insert(m_defname);
}

/// @class UseDefTag represents <csp:usedef>
/// Tag synopsys: <csp:usedef name="<name>"/>
/// This tag allows use of "define" inherited from parent class
class UseDefTag: public DefineTag
{
private:
	bool with_servlet;
public:
	UseDefTag(const string& name)
		: DefineTag(name)
	{}
	virtual void doStartTag(const attribs_t& attribs);
	virtual void doEndTag();
	virtual void initChildBuffers();
};

void UseDefTag::initChildBuffers()
{
	m_child_header = m_child_member = m_child_body=&func_body;
}

void UseDefTag::doStartTag(const Generator::attribs_t& attribs)
{
	get_attr(attribs, "name", m_defname, "csp:usedef");
	if(m_defines.find(m_defname)!=m_defines.end())
		throw runtime_error(m_defname+" is already imported (usedefed)");
	with_servlet = get_bool_attr(attribs, "with_servlet", "csp:define");
}

void UseDefTag::doEndTag()
{
	/*if(!func_body.str().empty())
	  throw(runtime_error("<csp:usedef name=\""+m_defname+"\"> tag cannot have a body. Body contents:\"\n"+func_body.str()+"\n"));*/
	*body<<m_defname<<"_t "<<m_defname<<"(request, response, out";
	if(with_servlet)
		*body<<", *this";
	*body<<");"<<endl;
	m_defines.insert(m_defname);
}

/// @class DefunTag represents <csp:defun>
/// Tag synopsys: <csp:defun name="<name>" params="<C++ argument definitions>">
/// Tag so defined, can later on be used with <csp:call> construct
/// Note1: current implementation does no param syntax checking
/// Note2: current implementation doesn't allow referencing variables from
///        the caller function
class DefunTag: public DefineTag
{
public:
	DefunTag(const string& name)
		: DefineTag(name)
	{}
	virtual void doStartTag(const attribs_t& attribs);
	virtual void doEndTag();
};

void DefunTag::doStartTag(const Generator::attribs_t& attribs)
{
	get_attr(attribs, "name", m_defname, "csp:defun");
	if(m_defines.find(m_defname)!=m_defines.end())
		throw runtime_error(m_defname+" is already defined");
	string params;
	get_attr(attribs, "params", params, "csp:defun");
	stringstream s;
	s<<"protected:"<<endl;
	s<<"	void "<<m_defname<<"(servlet::HttpServletRequest& request, servlet::HttpServletResponse& response, std::ostream& out";
	if(!params.empty()) {s<<","<<params;}
	s<<")";
	s<<"	{"<<endl;
	*member<<s.str();
}

void DefunTag::doEndTag()
{
	*member<<func_body.rdbuf()<<endl;
	*member<<"	} /* end csp:defun "<<m_defname<<" */"<<endl;
	m_defines.insert(m_defname);
}
/// @class CallTag represents <csp:call>
/// Tag synopsys: <csp:call name="<name>" params="<C++ rvalues to pass as arguments>">
/// Generates a call to function generated with <csp:defun>
/// Note1: current implementation does no param syntax checking
class CallTag: public DefineTag
{
public:
	CallTag(const string& name)
		: DefineTag(name)
	{}
	virtual void doStartTag(const attribs_t& attribs);
	virtual void doEndTag(){}
};

void CallTag::doStartTag(const Generator::attribs_t& attribs)
{
	get_attr(attribs, "name", m_defname, "csp:call");
	string params;
	get_attr(attribs, "params", params, "csp:call");
	stringstream s;
	*body<<"	"<<m_defname<<"(request, response, out";
	if(!params.empty()) {*body<<","<<params;}
	*body<<");"<<endl;
}

}
}

DECLARE_COMPILE_TIME_TAGLIB(csp)
EXPORT_COMPILE_TIME_TAG(csp, define, csp::tags::DefineTag)
EXPORT_COMPILE_TIME_TAG(csp, usedef, csp::tags::UseDefTag)
EXPORT_COMPILE_TIME_TAG(csp, defun,  csp::tags::DefunTag)
EXPORT_COMPILE_TIME_TAG(csp, call,   csp::tags::CallTag)
