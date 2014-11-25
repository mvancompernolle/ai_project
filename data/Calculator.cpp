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

#include <librpn/Calculator.h>
#include <librpn/Error.h>
#include <librpn/Expression.h>
#include <assert.h>
#include <iostream>
using namespace rpn;

Calculator::Calculator()
{
	_output = &std::cout;
}

bool Calculator::Compute(const std::string& expression, double* result) const
{
	assert(result);

	rpn::Expression expr;
	
	// Compile the expression
	try
	{
		expr.append(expression);
	}
	catch (rpn::Error& err)
	{
		*_output << "Syntax error : " << err.getMessage() << std::endl;
		return false;
	}

	// And evaluate it
	double res;
	try
	{
		res = expr.evaluate();
	}
	catch (rpn::Error& err)
	{
		*_output << "Evaluation error : " << err.getMessage() << std::endl;
		return false;
	}

	*result = res;
	return true;
}

expressionPtr Calculator::Parse(const std::string& expression) const
{
	// Compile the expression
	try
	{
		expressionPtr expr(RPN_NEW Expression(expression));
		return expr;
	}
	catch (rpn::Error& err)
	{
		*_output << "Syntax error : " << err.getMessage() << std::endl;
		return NULL;
	}
}

