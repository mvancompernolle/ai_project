/*
librpn - A cross-platform Reverse Polish Notation C++ library
Copyright (C) 2005 Thibault Genessay
http://librpn.sourceforge.net/

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <librpn/Expression.h>
#include <librpn/Error.h>
#include <librpn/Operator.h>
#include <librpn/Number.h>
#include <librpn/Evaluator.h>
#include <librpn/Simplifier.h>
#include <assert.h>
using namespace rpn;

Expression::Expression() 
	: _simplified(false), _dirty(false), _separator(',')
{
}

Expression::Expression(const std::string& source) 
	: _simplified(false), _dirty(false), _source(source), _separator(',')
{
	tokenize(source);
}

Expression::~Expression()
{
}

void Expression::tokenize(const std::string& source)
{
	std::string token;
	for (size_t i=0;i<source.size();i++)
	{
		if (source[i] == _separator)
		{
			if (token.size() != 0)
			{
				Token* tok;
				_tokens.push_back(tok = Token::FromString(token));
				assert(tok);
			}
			token = "";
		}else{
			token += source[i];
		}
	}
	if (token.size() != 0)
	{
		Token* tok;
		_tokens.push_back(tok = Token::FromString(token));
		assert(tok);
	}
}

void Expression::append(const std::string& source)
{
	if (_source[_source.size()-1] != _separator && source[0] != _separator)
		_source += _separator;
	_source += source;
	tokenize(source);
}

void Expression::append(Token* token)
{
	assert(token);
	if (_source.size())
		if (_source[_source.size()-1] != _separator)
			_source += _separator;
	_source += token->toString();
	_tokens.push_back(token);
}

double Expression::evaluate()
{
	Evaluator stack;
	for (std::vector<tokenPtr>::iterator it=_tokens.begin();it!=_tokens.end();it++)
	{
		stack.push((*it).get());
	}
	numberPtr result = stack.pop();
	if (stack.size())
		throw Error("Non-empty final stack");

	return result->val();
}

void Expression::simplify()
{
	Simplifier simplifier;
	for (std::vector<tokenPtr>::iterator it=_tokens.begin();it!=_tokens.end();it++)
	{
		simplifier.push((*it).get());
	}
	_tokens = simplifier.getTokens();
	_simplified = true;
	_dirty = true;
}

std::ostream& rpn::operator<<(std::ostream& stream, rpn::Expression& expression)
{
	if (expression._dirty)
	{
		expression._source = "";
		int i=0;
		for (std::vector<tokenPtr>::iterator it=expression._tokens.begin();it!=expression._tokens.end();it++)
		{
			if (i++ > 0)
				expression._source += expression._separator;
			expression._source += (*it)->toString();
		}
		expression._dirty = false;

	}
	stream << expression._source;
	return stream;
}

