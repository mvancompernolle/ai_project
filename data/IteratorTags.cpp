/***************************************************************************
 *   Copyright (C) 2009 by Ilya A. Volynets-Evenbakh                  *
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


/// @class AttrIteratorTag represents <csp:foreach_attr>
/// Tag synopsys <csp:foreach_attr attribute="<attribute name>" var="<iterator>" type="<collection type>" [source="{request|session|servlet}"] [required="{true|false}">...</csp:foreach_attr>
/// @param attribute Attribute name
/// @param var Iterator variable name. It can be referenced only inside the block
/// @param type Collection type (e.g. std::vector<std::string>)
/// @param source Where the attribute comes from (request, session, or servlet; request it the default)
/// @param required Whether attribute must be present (boolean, default false)
/// Note: currently no data type checking is performed.
/// If "required" is set to true and attribute is missing, MissingAttributeException is thrown
/// If @a source is "servlet", the attribute is retrieved from servlet context
class AttrIteratorTag: public StdTagBase
{
public:
	string m_attribute, m_var, m_type, m_source, m_attrvar;
	bool m_required;
public:
	AttrIteratorTag(const string& name)
		: StdTagBase(name)
	{}
	virtual void doStartTag(const attribs_t& attribs);
	virtual void doEndTag();
};

void AttrIteratorTag::doStartTag(const Generator::attribs_t& attribs)
{
	string constP;
	m_required = get_bool_attr(attribs, "required", "csp:foreach_attr");
	get_attr(attribs, "attribute", m_attribute, "csp:foreach_attr");
	get_attr(attribs, "var", m_var, "csp:foreach_attr");
	get_attr(attribs, "type", m_type, "csp:foreach_attr");
	get_attr(attribs, "source", m_source, "csp:foreach_attr", "request");
	get_attr(attribs, "const", constP, "csp:foreach_attr", "false");
	if(constP == "true") // We'll use const_iterator in this case
		constP="const_";
	else
		constP.clear();
	if(m_source!="request" && m_source!="session" && m_source!="servlet")
		throw runtime_error("csp:foreach_attr: valid values for \"source\": \"request\", \"session\", \"servlet\"");
	if(m_source=="servlet")
		m_source="this->getServletContext()";
	else if(m_source=="session")
		m_source="(*request.getSession())";
	if(!common_headers_included) {
		*header<<"#include <servlet/taglib/MissingAttributeException.h>"<<endl;
		// Hmm... should I really do this?
		*header<<"using namespace std;"<<endl;
		*header<<"using namespace boost;"<<endl;
		common_headers_included=true;
	}
	stringstream s;
	m_attrvar = "_csp_attr_"+m_attribute;
	transform(m_attrvar.begin(), m_attrvar.end(), m_attrvar.begin(), to_cxx_ident);
	s<<"do {\n\t";
	s<<	"boost::shared_ptr<"<<m_type<<" > "<<m_attrvar<<" =\n\t\t";
	s<<		"static_pointer_cast<"<<m_type<<" >("<<m_source<<".getAttribute(\""<<m_attribute<<"\"));\n\t";
	s<<	"if(!"<<m_attrvar<<") {\n\t\t";
	if(m_required) {
		s<<"throw MissingAttributeException(\""<<m_attribute<<"\");\n\t";
	} else {
		stringstream ss;
		ss<<"<!-- attribute "<<m_attribute<<" is not set -->\\n";
		s<<"out.write(\""<<ss.str()<<"\", "<<ss.str().length()<<");\n\t\t";
		s<<"break;\n\t";
	}
	s<<	"}\n\t";
	s<<	"for("<<m_type<<"::"<<constP<<"iterator "<<m_var<<"=(*"<<m_attrvar<<").begin();";
	s<<		m_var<<"!=(*"<<m_attrvar<<").end(); "<<m_var<<"++) {\n\t\t";
	*body<<s.str()<<endl;
}

void AttrIteratorTag::doEndTag()
{
	stringstream s;
	s<<"\t}\n";
	s<<"} while(0);\n";
	s<<"/* end csp:foreach_attr: "<<m_attribute<<" */"<<endl;
	*body<<s.str();
}


/// @class ListIteratorTag represents <csp:foreach_list>
/// Tag synopsys <csp:foreach_list list="<list>" var="<iterator>" type="<collection type>">...</csp:foreach_list>
/// @param list List name. Must be existing variable of type (or reference to type)
/// @param var Iterator variable name. It can be referenced only inside the block
/// @param type Collection type (e.g. std::vector<std::string>)

class ListIteratorTag: public StdTagBase
{
public:
	string m_list, m_var, m_type;
public:
	ListIteratorTag(const string& name)
		: StdTagBase(name)
	{}
	virtual void doStartTag(const attribs_t& attribs);
	virtual void doEndTag();
};

void ListIteratorTag::doStartTag(const Generator::attribs_t& attribs)
{
	string constP;
	get_attr(attribs, "list", m_list, "csp:foreach_list");
	get_attr(attribs, "var", m_var, "csp:foreach_list");
	get_attr(attribs, "type", m_type, "csp:foreach_list");
	get_attr(attribs, "const", constP, "csp:foreach_list", "false");
	if(constP == "true") // We'll use const_iterator in this case
		constP="const_";
	else
		constP.clear();
	stringstream s;
	s<<"for("<<m_type<<"::"<<constP<<"iterator "<<m_var<<"=("<<m_list<<").begin();";
	s<<		m_var<<"!=("<<m_list<<").end(); "<<m_var<<"++) {\n\t";
	*body<<s.str()<<endl;
}

void ListIteratorTag::doEndTag()
{
	stringstream s;
	s<<"}\n";
	s<<"/* end csp:foreach_list: "<<m_list<<" */"<<endl;
	*body<<s.str();
}

}
}
DECLARE_COMPILE_TIME_TAGLIB(csp)
EXPORT_COMPILE_TIME_TAG(csp, foreach_attr, csp::tags::AttrIteratorTag)
EXPORT_COMPILE_TIME_TAG(csp, foreach_list, csp::tags::ListIteratorTag)
