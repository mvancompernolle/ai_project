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

#include <librpn/Simplifier.h>
#include <librpn/Operator.h>
#include <librpn/Number.h>
#include <librpn/Error.h>
#include <librpn/Variable.h>
#include <assert.h>
#include <iostream>
using namespace rpn;

Simplifier::Simplifier()
{
}

void Simplifier::flushTokens()
{
	for (std::vector<numberPtr>::iterator it=_constantTokens.begin();it!=_constantTokens.end();it++)
		_tokens.push_back((*it).get());
	_constantTokens.clear();
}

void Simplifier::accept(Constant* constant)
{
	assert(constant);

	_constantTokens.push_back(constant);

}

void Simplifier::accept(Variable* variable)
{
	assert(variable);

	flushTokens();

	_tokens.push_back(variable);

}


void Simplifier::accept(Operator* op)
{
	assert(op);
	rpn::operatorPtr opPtr(op);

	unsigned int operands = (unsigned int) opPtr->card();
	if (_constantTokens.size() >= operands)
	{
		std::vector<numberPtr> args;
		for (unsigned int i=0;i<operands;i++)
		{
			assert((*(_constantTokens.rbegin()))->getClass() == Token::CONSTANT);
			args.push_back(*(_constantTokens.rbegin()));
			_constantTokens.pop_back();
		}
		Number *cst = op->apply(args);
		_constantTokens.push_back(cst);
	}else{
		_tokens.push_back(op);
	}
}

