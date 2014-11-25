/*
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

#include "lexer.h"
#include "parser_error.h"
#include <utils/dir.h>

namespace cxxsp
{

using namespace std;

void lexer::save_text(int start_strip, int end_strip)
{
	text.assign(
	    YYText() + start_strip,
	    YYLeng() - start_strip - end_strip
	);
}

lexer::lexer(const std::string& fname, bool eat_spaces)
		: CxxspFlexLexer(open(fname))
		, fname(fname)
		, m_eat_spaces(eat_spaces)
{
	next_token(); // initialize text & current
}
lexer::~lexer()
{
	delete in;
}
std::istream* lexer::open(const std::string& fname)
{
	in = new ifstream();
	in->exceptions(ios::badbit);
	in->open(fname.c_str());
	if (!in->is_open()) {
		delete in;
		throw not_found_error("Cannot open file: " + fname);
	}
	return in;
}

lexer::token lexer::next_token()
{
	token prev = current;
	last = text;
	text.clear();
	current = static_cast<token>(yylex());
	return prev;
}

std::string token_string(lexer::token token)
{
	switch (token) {
	case lexer::CSP_EOF: return "CSP_EOF";
	case lexer::CSP_CODE_START: return "CSP_CODE_START";
	case lexer::CSP_EXPR_START: return "CSP_EXPR_START";
	case lexer::CSP_DECL_START: return "CSP_DECL_START";
	case lexer::CSP_HEADER_START: return "CSP_HEADER_START";
	case lexer::CSP_DIRECTIVE_START: return "CSP_DIRECTIVE_START";
	case lexer::CSP_LIBTAG_START: return "CSP_LIBTAG_START";
	case lexer::CSP_LIBTAG_STOP: return "CSP_LIBTAG_STOP";
	case lexer::CSP_SPECIAL_TAG_CLOSED: return "CSP_SPECIAL_TAG_CLOSED";
	case lexer::CSP_DATA: return "CSP_DATA";
	case lexer::CSP_CODE_END: return "CSP_CODE_END";
	case lexer::CSP_VALUE: return "CSP_VALUE";
	case lexer::CSP_ID: return "CSP_ID";
	default: return boost::lexical_cast<string>(token);
	}
}
} //namespace

