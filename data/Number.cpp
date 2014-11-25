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

#include <librpn/Number.h>
#include <librpn/Error.h>
#include <librpn/Stack.h>
#include <sstream>
#include <assert.h>
using namespace rpn;

Number::Number(int __class) : Token(__class)
{
}

Number::Initializer Number::_s_initializer;

/*double Number::SNAN;
double Number::QNAN;
double Number::INFINITY;*/

Number::Initializer::Initializer()
{
//	assert(sizeof(double) == 2*sizeof(unsigned int) == 8);

/*	unsigned char snan[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	memcpy(&Number::SNAN, snan, sizeof(double));
	unsigned char qnan[8] = { 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
	memcpy(&Number::QNAN, qnan, sizeof(double));
	unsigned char infinity[8] = { 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	memcpy(&Number::INFINITY, infinity, sizeof(double));*/
}

void Number::visit(Stack* visitor)
{
	visitor->accept(this);
}

std::string Number::toString() const
{
	std::stringstream ss;
	ss << val();
	return ss.str();
}

