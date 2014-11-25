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

#include <librpn/Stack.h>
#include <librpn/Operator.h>
#include <librpn/Number.h>
#include <librpn/Error.h>
#include <assert.h>
using namespace rpn;

Stack::Stack()
{
}

Stack::~Stack()
{
}

void Stack::push(Token* token)
{
	assert(token);
	token->visit(this);
}

numberPtr Stack::pop()
{
	if (size() == 0)
		throw Error("Cannot pop from an empty stack");

	numberPtr num = at(size()-1);
	pop_back();
	return num;
}

size_t Stack::size() const
{
	return std::vector<numberPtr>::size();
}
