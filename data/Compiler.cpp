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

#include <librpn/Compiler.h>
#include <librpn/Operator.h>
#include <librpn/Number.h>
#include <librpn/Error.h>
#include <librpn/Variable.h>
#include <librpn/Bytecode.h>
#include <assert.h>
#include <iostream>
using namespace rpn;

Compiler::Compiler(std::vector<Bytecode::Instruction>& bytecode, Bytecode::variableArrayPtr& variables) 
	: _bytecode(bytecode), _variables(variables), _nextVariableIndex(0)
{
}

void Compiler::accept(Constant* constant)
{
	assert(constant);

	_bytecode.push_back(Bytecode::Instruction(constant->val()));
}

void Compiler::accept(Variable* variable)
{
	assert(variable);

	_bytecode.push_back(Bytecode::Instruction(_nextVariableIndex++));
}

void Compiler::accept(Operator* op)
{
	assert(op);

	_bytecode.push_back(Bytecode::Instruction(op->type(), op->card()));
}
