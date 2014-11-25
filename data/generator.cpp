/*
 *  Copyright (C) 2008-2009 Ilya A. Volynets-Evenbakh <ilya@total-knowledge.com>
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

#include "generator.h"
#include <stdexcept>
#include <fstream>
#include <boost/tokenizer.hpp>
#include "tokenizer.h"
#include <servlet/taglib/Generator.h>
#include <utils/text.h>

namespace cxxsp
{

using namespace std;

inline string generate_class_name(const string& filename)
{
	string class_name = "CspServlet";
	for (string::size_type i=0;i<filename.size();++i) {
		if (!isalnum(filename[i]))
			class_name += '_';
		else
			class_name += filename[i];
	}
	return class_name;
}

generator::generator(const string& fileout, const string& filein, const std::string& class_name)
		: class_name(class_name)
		, filename(fileout)
		, filein(filein)
		, m_abstract(false)
		, session(false)
		, isErrorPage(false)
		, contentType("text/html")
		, characterEncoding("utf8")
		, extends("servlet::HttpServlet")
{
	generators.head() << "#include <servlet/HttpServlet.h>" << endl;
}

string generator::escape(const string& s)
{
	string res;
	res.reserve(s.size() + s.size()/3);
	for (unsigned i=0;i<s.size();++i) {
		switch (s[i]) {
		case '\\':
			res += "\\\\";
			break;
		case '\r':
			res += "\\r";
			break;
		case '\n':
			res += "\\n\"\n\t\"";
			break;
		case '"':
			res += "\\\"";
			break;
		default:
			res += s[i];
		}
	}
	return res;
}

void generator::data(const string& data, int line, const std::string& fname)
{
	if (!data.empty())
		generators.body() << "#line " << line << " \"" << fname << "\"\n";
	generators.body() << "\tout.write(\"" << escape(data) << "\", " << data.size() << ");\n";
}

void generator::decl(const string& data, int line, const std::string& fname)
{
	generators.head() << "#line " << line << " \"" << fname << "\"\n";
	generators.head() << data << endl;
}

void generator::code(const string& data, int line, const std::string& fname)
{
	generators.body() << "#line " << line << " \"" << fname << "\"\n";
	generators.body() << data << endl;
}

void generator::expr(const string& data, int line, const std::string& fname)
{
	generators.body() << "#line " << line << " \"" << fname << "\"\n";
	generators.body() << "out << ( " << data << ");\n";
}

void generator::header(const string& data, int line, const std::string& fname)
{
	generators.head() << "#line " << line << " \"" << fname << "\"\n";
	generators.head() << data << endl;
}

inline void fill(string &templ, string name, string val)
{
	string::size_type pos;
	const string needle = "{" + name + "}";
	for (;;) {
		pos = templ.find(needle);
		if (pos == string::npos)
			break;
		templ.replace(pos, needle.size(), val);
	}
}

void generator::libtag_start(int line, const std::string& fname, const string& prefix, const string& name, const attribs_t& attribs)
{
	servlet::taglib::Generator& gen = generators.get(prefix,name);
	generators.body() << endl << "#line " << line << " \"" << fname << "\"\n";
	gen.doStartTag(attribs);
	generators.body() << "#line " << line << " \"" << fname << "\"\n";
}

void generator::libtag_end(int line, const std::string& fname, const string& prefix, const string& name)
{
	generators.body() << "#line " << line << " \"" << fname << "\"\n";
	std::string fullname(prefix);
	fullname+=':';
	fullname.append(name);
	servlet::taglib::Generator& gen = generators.pop(fullname);
	generators.body() << "#line " << line << " \"" << fname << "\"\n";
}


void generator::directive(const string& name, const attribs_t& attribs)
{
	/* include directive is processed by lexer*/
	if (name == "page") {
		attribs_t::const_iterator attrib;

		attrib = attribs.find("language");
		if (attrib != attribs.end())
			if (attrib->second != "c++")
				throw runtime_error("Invalid language. Only accepted value is \"c++\"");

		attrib = attribs.find("import");
		if (attrib != attribs.end()) {
			tokenizer tok(attrib->second,',');
			while (!tok.end())
				generators.head() << "#include <" << tok.next() << ">" << endl;
		}

		attrib = attribs.find("session");
		if (attrib != attribs.end())
			session = (attrib->second == "true");

		attrib = attribs.find("isErrorPage");
		if (attrib != attribs.end())
			isErrorPage = (attrib->second == "true");

		attrib = attribs.find("errorPage");
		if (attrib != attribs.end()) {
			errorPage = attrib->second;
			if (errorPage == "this")
				errorPage = filename;
		}
		attrib = attribs.find("contentType");
		if (attrib != attribs.end())
			contentType = attrib->second;
		attrib = attribs.find("characterEncoding");
		if (attrib != attribs.end())
			characterEncoding = attrib->second;

		attrib = attribs.find("extends");
		if (attrib != attribs.end())
			extends = attrib->second;

		attrib = attribs.find("flags");
		if (attrib != attribs.end())
			split_to(attrib->second, flags_);

		attrib = attribs.find("class");
		if (attrib != attribs.end())
			if(class_name.empty()) // Make sure it wasn't set on command line
				class_name=attrib->second;

		// Allows CSP page class to be in a namespace
		// It's helpful, if, for example, you want to have
		// Identical class names for multiple CSPs (think themes/skins)
		attrib = attribs.find("namespace");
		if (attrib != attribs.end())
			m_namespace = attrib->second;

		// If abstract is set to "true"
		// No function is generated, and body stream is discarded
		attrib = attribs.find("abstract");
		if (attrib != attribs.end() && attrib->second=="true")
			m_abstract = true;
	} else if (name == "taglib") {
		attribs_t::const_iterator attrib;
		string prefix,uri;

		attrib = attribs.find("uri");
		if (attrib != attribs.end()) {
			uri = attrib->second;
		} else {
			throw runtime_error("uri attribute missing from taglib directive");
		}

		attrib = attribs.find("prefix");
		if (attrib != attribs.end()) {
			prefix = attrib->second;
		} else {
			throw runtime_error("prefix attribute missing from taglib directive");
		}

		// If compiletime attribute is set,
		// we have a preprocessor taglib
		// Otherwise we have "runtime" taglib,
		// and container code is generated with
		// a cppserv-provided generator.
		attrib = attribs.find("compiletime");
		if (attrib == attribs.end() || attrib->second!="true") {
			prefix="cxxsp_runtime_taglib";
			uri="cxxsp_runtime_taglib.so";
		}

		try {
			generators.load(prefix, uri, attribs);
		} catch (std::exception& e) {
			std::cerr<<"Unable to load tag lib (prefix="<<prefix<<", uri="<<uri<<")"<<std::endl;
			throw;
		}
	} else {
		throw runtime_error("Unknown directive: " + name);
	}
}

void generator::generate(bool split_def)
{
	if(class_name.empty())
		class_name=generate_class_name(filein);
	ofstream out_decl, out_def;
	out_decl.exceptions(ios::badbit | ios::failbit);
	out_def.exceptions(ios::badbit | ios::failbit);
	if(split_def)
	{
		out_decl.open((filename+".h").c_str());
		out_def.open((filename+".cpp").c_str());
	}
	else
	{
		out_decl.open(filename.c_str());
	}
	generators.head()<<endl;
	generators.head().seekg(0);
	generators.member()<< endl;
	generators.member().seekg(0);
	generators.body()<<endl;
	generators.body().seekg(0);

	// Set file name
	out_decl<<"#line 1 \"" << filein << "\"\n";
	if(split_def)
	{
		out_def<<"#line 1\"" << filein << "\"\n";
		//FIXME: use filesystem function, when TR1 is ready
		string base_name;
		string::size_type sep = filename.rfind('/');
		if(sep!=string::npos)
			base_name=filename.substr(sep+1);
		else
			base_name=filename;
		out_def<<"#include \"" << base_name << ".h\""<<std::endl;
		out_def<<"#line 1\"" << filein << "\"\n";
	}

	if (session)
		out_decl << "#include <memory>\n";
	out_decl << generators.head().rdbuf() << endl;
	if(!m_namespace.empty())
		out_decl << "namespace " << m_namespace << "{"<<std::endl;
	out_decl << "class " << class_name << ": public " << extends << " {" << endl;
	out_decl << generators.member().rdbuf() << endl;
	std::ostream* out;
	if(split_def)
		out = &out_def;
	else
		out = &out_decl;
	if(!m_abstract)
	{
		out_decl << "public:\n";
		out_decl << "	void doPost(servlet::HttpServletRequest& request, servlet::HttpServletResponse& response) {\n";
		out_decl << "		doGet(request, response);\n";
		out_decl << "	}\n";
		if(split_def)
		{
			out_decl << "	void doGet(servlet::HttpServletRequest& request, servlet::HttpServletResponse& response);\n";
			if(!m_namespace.empty())
				out_def << "namespace " << m_namespace <<" {" << endl;
			out_def <<"void " << class_name << "::doGet(servlet::HttpServletRequest& request, servlet::HttpServletResponse& response) {\n";
		}
		else
		{
			out_decl << "	void doGet(servlet::HttpServletRequest& request, servlet::HttpServletResponse& response) {\n";
		}
		if (isErrorPage) {
			*out << "		const std::exception* exception = request.hasAttribute(\"csp.exception\") ? \n"
				 << "			&request.getAttribute<std::exception>(\"csp.exception\") : \n"
				 << "			0;\n";
		}
		if (!errorPage.empty())
			*out << "	try { \n";
		*out << "		std::ostream& out = response.getOutputStream();\n"
			 << "		response.setContentType(\"" << escape(contentType) <<"\");\n"
			 << "		response.setCharacterEncoding(\"" << escape(characterEncoding) <<"\");\n";
		if (session)
			*out << "		servlet::HttpSession* session = request.getSession();\n";
		*out << endl << generators.body().rdbuf() << endl;
		if (!errorPage.empty()) {
			*out << "	}catch(const std::exception& e) { \n"
				 << "		request.setAttribute(\"csp.exception\", e);\n"
				 << "		if(exception == 0) /* avoid exception loop */ { "
				 << "			request.getRequestDispatcher(\"" << errorPage << "\")->forward(request,response); \n"
				 << "		} else {\n"
				 << "			throw;"
				 << "		}\n"
				 << "	}\n";
		}
		*out << "	}\n";
	}
	out_decl << "};\n";
	if(!m_abstract)
		*out << "EXPORT_SERVLET(" << class_name << ");" << endl;
	if(!m_namespace.empty())
	{
		out_decl<<'}'<<endl;
		if(split_def && !m_abstract)
			out_def<<'}'<<endl;
	}
	if(split_def)
		out_def.close();
	out_decl.close();
}
}
