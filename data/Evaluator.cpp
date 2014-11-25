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

#include <librpn/Evaluator.h>
#include <librpn/Operator.h>
#include <librpn/Constant.h>
#include <librpn/Variable.h>
#include <librpn/Error.h>
#include <assert.h>
using namespace rpn;

Evaluator::Evaluator()
{
}

void Evaluator::accept(Constant* constant)
{
	assert(constant);

	push_back(constant);
}

void Evaluator::accept(Variable* variable)
{
	assert(variable);

	push_back(variable);
}

void Evaluator::accept(Operator* op)
{
	assert(op);

	// Make sure the pointer gets referenced at least once so it is deleted
	// if no further reference is made to it outside this function
	// Note, however, that passing a raw Operator* pointer to this function
	// and then reusing it later will trigger a segfault. If you plan to reuse
	// an operator, create it with an operatorPtr and pass it here with .get()
	rpn::operatorPtr opPtr(op);
	int operands = opPtr->card();
	std::vector<numberPtr> args;
	for (int i=0;i<operands;i++)
	{
		if (size() == 0)
			throw Error("Not enough arguments");
		args.push_back(pop());
	}
	push_back(opPtr->apply(args));
}

