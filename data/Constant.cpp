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

#include <librpn/Constant.h>
#include <librpn/Error.h>
#include <librpn/Stack.h>
#include <assert.h>
using namespace rpn;

Constant::Constant() : Number(CONSTANT)
{
	_value = 0.0;
}
	
Constant::Constant(double floatingpoint) : Number(CONSTANT)
{
	_value = floatingpoint;
}

Constant* Constant::FromString(const std::string& repr)
{
	assert(repr.size() != 0);

	return RPN_NEW Constant(atof(repr.c_str()));
}

double Constant::val() const
{
	return _value;
}

void Constant::set(double value)
{
	throw Error("Cannot modify the value of a constant");
}

void Constant::visit(Stack* visitor)
{
	visitor->accept(this);
}
