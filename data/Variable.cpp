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

#include <librpn/Variable.h>
#include <librpn/Error.h>
#include <librpn/Stack.h>
#include <assert.h>
using namespace rpn;

Variable::Variable(const std::string& name) : Number(VARIABLE), _name(name), _initialized(false)
{
}

Variable::Variable(const std::string& name, double floatingpoint) : Number(VARIABLE), _name(name),
	_value(floatingpoint), _initialized(true)
{
	_value = floatingpoint;
}

double Variable::val() const
{
	if (!_initialized)
		throw Error("Uninitalized variable : " + _name);
	return _value;
}

double* Variable::ptr()
{
	return &_value;
}

void Variable::set(double value)
{
	_value = value;
}

void Variable::visit(Stack* visitor)
{
	visitor->accept(this);
}

Variable* Variable::FromString(const std::string& repr)
{
	assert(repr.size() != 0);

	return RPN_NEW Variable(repr);
}

std::string Variable::toString() const
{
	return _name;
}

