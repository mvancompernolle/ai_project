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

#include <stdexcept>
#include "parser.h"
#include "parser_error.h"
#include <utils/dir.h> //directory(...)
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>

namespace cxxsp
{

using namespace std;

	parser::parser(const string& a_file_in, const string& a_file_out, const std::string& class_name, bool eat_spaces, bool gen_deps, bool split_def)
		: lex(0)
		, generate(a_file_out, a_file_in, class_name)
		, file_in(a_file_in)
		, m_eat_spaces(eat_spaces)
		, m_gen_deps(gen_deps)
		, m_split_def(split_def)
{
}

parser::~parser()
{
	if(lex)
		delete lex;
}

string parser::get_data()
{
	string data=lex->last_text();
	while (lex->peek_token() == lexer::CSP_DATA) {
		lex->next_token();
		data += lex->last_text();
	}
	return data;
}

void parser::get_token(lexer::token expected)
{
	lexer::token got = lex->next_token();
	if (got != expected)
		throw parser_error("Unexpected token:"+lex->last_text(),lex->lineno(), got,expected);
}

void parser::get_token(lexer::token expected1, lexer::token expected2)
{
	lexer::token got = lex->next_token();
	if (got != expected1 && got != expected2)
		throw parser_error("Unexpected token",lex->lineno(), got,expected1,expected2);
}

void parser::get_data(const string& expected)
{
	string got = get_data();
	if (expected != got)
		throw parser_error("Unexpected data. \nGot: '"+ got+"'\nExpected: '"+expected+"'",lex->lineno());
}

string parser::get_code()
{
	stringstream data;

	data << lex->last_text();
	while (lex->peek_token() != lexer::CSP_CODE_END && lex->peek_token() != lexer::CSP_EOF) {
		lex->next_token();
		data << lex->last_text();
	}
	lex->next_token(); //eat CSP_CODE_END
	return data.str();
}

generator::attribs_t parser::get_tag_attribs()
{
	generator::attribs_t attribs;
	string name,value;
	lexer::token t;
	while ((t = lex->peek_token()) != lexer::CSP_SPECIAL_TAG_CLOSED &&
		   t != lexer::CSP_LIBTAG_NOBODY_STOP && t != lexer::CSP_EOF) {
		get_token(lexer::CSP_ID);
		name = lex->last_text();

		get_token(lexer::CSP_DATA);
		get_data("=");

		get_token(lexer::CSP_VALUE,lexer::CSP_ID);
		value = lex->last_text();

		attribs.insert(std::pair<std::string,std::string>(name,value));
	}
	if(t!=lexer::CSP_LIBTAG_NOBODY_STOP)
		lex->next_token();
	return attribs;
}

string parser::relative_path(const string& filename)
{
	if (filename.empty())
		return filename;
	if (filename[0] == '/') {
		throw runtime_error("TODO: including relative to web aplication root");
	} else {
		//FIXME: do we want to implement some sort of search path
		// that would include path of _current_ parse file, not
		// just top-level one?
		struct stat st;
		for(list<string>::iterator it = m_search_path.begin(); it!=m_search_path.end(); it++) {
			string fname = *it+"/"+filename;
			if(::stat(fname.c_str(), &st)==0) {
				if(st.st_mode&S_IFREG)
					return fname;
			}
		}
	}
	throw runtime_error("File "+filename+" is not found in search path");
}

bool parser::parse()
{
	lex = new lexer(file_in, m_eat_spaces);
	m_search_path.push_front(directory(file_in));
	lex_stack.push(lex);
	try {
		for (;;) {
			lexer::token token = lex->next_token();
			switch (token) {
			case lexer::CSP_EOF:
				delete lex;
				lex=0;
				lex_stack.pop();
				m_search_path.pop_front();
				if (lex_stack.empty()) {
					if(m_gen_deps) {
						std::cout<<outFileName()<<':';
						for(std::set<std::string>::iterator it = m_deps.begin(); it!=m_deps.end(); it++) {
							std::cout<<" \\"<<std::endl<<'\t'<<*it;
						}
						std::cout<<std::endl;
					} else {
						generate.generate(m_split_def);
					}
					return true;
				}
				lex=lex_stack.top();
				break;
			case lexer::CSP_DATA:
				generate.data(get_data(), lex->lineno(), lex->curfile());
				break;
			case lexer::CSP_CODE_START:
				generate.code(get_code(), lex->lineno(), lex->curfile());
				break;
			case lexer::CSP_EXPR_START:
				generate.expr(get_code(), lex->lineno(), lex->curfile());
				break;
			case lexer::CSP_DECL_START:
				generate.decl(get_code(), lex->lineno(), lex->curfile());
				break;
			case lexer::CSP_HEADER_START:
				generate.header(get_code(), lex->lineno(), lex->curfile());
				break;
			case lexer::CSP_DIRECTIVE_START: {
				get_token(lexer::CSP_ID);
				string name = lex->last_text();
				generator::attribs_t attribs = get_tag_attribs();

				if (name == "include") {
					generator::attribs_t::const_iterator attrib
						= attribs.find("file");
					if (attrib != attribs.end()) {
						std::string path = relative_path(attrib->second);
						lex=new lexer(path, m_eat_spaces);
						lex_stack.push(lex);
						m_search_path.push_front(directory(path));
						add_dep(path);
					} else
						throw parser_error("include directive without file attribute", lex->lineno());
				} else if (name == "spacing") {
					generator::attribs_t::const_iterator attrib = attribs.find("preserve");
					if(attrib == attribs.end())
						throw parser_error("missing attribute \"preserve\" in spacing directive", lex->lineno());
					if(attrib->second == "true")
						lex->eat_spaces(false);
					else if(attrib->second == "restore")
						lex->eat_spaces(m_eat_spaces);
					else
						lex->eat_spaces(true);
				} else {
					generate.directive(name, attribs);
				}
				break;
			}
			case lexer::CSP_LIBTAG_START: {
				m_cur_libtag_prefix = lex->last_text();
				get_token(lexer::CSP_ID);
				m_cur_libtag_name = lex->last_text();
				generate.libtag_start(lex->lineno(), lex->curfile(), m_cur_libtag_prefix, m_cur_libtag_name, get_tag_attribs());
				break;
			}
			case lexer::CSP_LIBTAG_STOP: {
				string prefix = lex->last_text();
				get_token(lexer::CSP_ID);
				string name = lex->last_text();
				get_token(lexer::CSP_SPECIAL_TAG_CLOSED);
				generate.libtag_end(lex->lineno(), lex->curfile(), prefix, name);
				break;
			}
			case lexer::CSP_LIBTAG_NOBODY_STOP: {
				generate.libtag_end(lex->lineno(), lex->curfile(), m_cur_libtag_prefix, m_cur_libtag_name);
				break;
			}
			default:
				throw parser_error("Internal parser error",lex->lineno(), token);
			}
		}
	} catch (std::exception& e) {
		if(lex)
		{
			lex_stack.pop();
			std::cerr<<lex->curfile()<<": "<<lex->lineno()<<": ";
		}
		std::cerr<<"Error: "<<e.what()<<std::endl;
		while(!lex_stack.empty()) {
			lex=lex_stack.top();
			std::cerr<<"In file included from "<<lex->curfile()<<": "<<lex->lineno()<<std::endl; 
			lex_stack.pop();
		}
		return false;
	}
	return true;
}

}//namespace
