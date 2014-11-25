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

#include <librpn/Program.h>
#include <librpn/Error.h>
#include <librpn/Simplifier.h>
#include <librpn/Compiler.h>
#include <librpn/Operator.h>
#include <assert.h>
#include <iostream>
using namespace rpn;

Program::Program() : _compiled(false), _numVariables(0)
{
}

Program::Program(const std::string& source) : Expression(source), _compiled(false), _numVariables(0)
{
}

void Program::compile()
{
	// First step: simplification
	// The problem is to find, within the expression, the set of the sub-expressions of maximal 
	// length that do not contain variables; those can be precomputed (i.e. reduced) and 
	// inserted back into the original expression.
	if (!_simplified)
		simplify();

	// Second step: compilation. 
	// Create a flat list of low-level operations (e.g. PUSH, POP)
	if (!_compiled)
	{
		Compiler compiler(_bytecode, _variables);
		for (std::vector<tokenPtr>::iterator it=_tokens.begin();it!=_tokens.end();it++)
		{
			compiler.push((*it).get());
			if ((*it)->getClass() == Token::VARIABLE)
				_numVariables++;
		}
		_compiled = true;
	}
}

Bytecode::doubleArrayPtr Program::execute(const Bytecode::variableArrayPtr& input)
{
	assert(input->size() == (size_t)_numVariables);
#ifdef _DEBUG
	size_t size=0;
	for (Bytecode::VariableArray::const_iterator it=input->begin();it!=input->end();it++)
		if (size == 0)
			size = (*it)->size();
		else
			assert((*it)->size() == size);
	if (input->size())
		assert((*input)[0]->size() > 0);
#endif
	
	if (!_compiled)
		compile();

	Bytecode::doubleArrayPtr result = RPN_NEW Bytecode::DoubleArray();
	
	std::vector<double> stack(_tokens.size());
	int stackptr = -1;

	Bytecode::DoubleArray::const_iterator *vars = NULL;
	if (_numVariables)
		vars = RPN_NEW Bytecode::DoubleArray::const_iterator[_numVariables];

	for (int i=0;i<_numVariables;i++)
		vars[i] = (*input)[i]->begin();
	
	size_t steps = input->size() ? (*input)[0]->size() : 1;
	for (size_t step=0;step<steps;step++)
	{
		for (std::vector<Bytecode::Instruction>::const_iterator it=_bytecode.begin();it!=_bytecode.end();it++)
		{
			/*assert((*it)->size() == _numVariables);
			Bytecode::Instruction& instr = (*it)*/
			switch (it->opcode)
			{
			case Bytecode::PUSHI:
				stack[++stackptr] = it->value;
				break;
			case Bytecode::PUSHR:
				stack[++stackptr] = *vars[it->varpos];
				break;
			case Bytecode::OP:
				switch (it->card)
				{
				case 1:
					stack[stackptr] = Operator::Apply(it->op, stack[stackptr]);
					break;				
				case 2:
					stack[stackptr-1] = Operator::Apply(it->op, stack[stackptr-1], stack[stackptr]);
					stackptr-=1;
					break;				
				case 3:
					stack[stackptr-2] = Operator::Apply(it->op, stack[stackptr-2], stack[stackptr-1], stack[stackptr]);
					stackptr-=2;
					break;	
				default:
					assert(false);
					break;
				}
				break;
			default:
				assert(false);
				break;
			}

		}
	
		if (_numVariables)
			for (int i=0;i<_numVariables;i++)
				vars[i]++;

		result->push_back(stack[stackptr--]);
		assert(stackptr == -1);
	}

	return result;
}


double Program::executeOnceSingle()
{
	if (_numVariables != 0)
		throw Error("Cannot execute program : missing input variable(s)");
	Bytecode::variableArrayPtr vars = RPN_NEW Bytecode::VariableArray;
	Bytecode::doubleArrayPtr val = RPN_NEW Bytecode::DoubleArray;
	Bytecode::doubleArrayPtr ret = execute(vars);
	assert(ret->size() == 1);
	return (*ret)[0];
}

double Program::executeOnceSingle(double value)
{
	if (_numVariables != 1)
		throw Error("Cannot execute program : wrong number of input variables");
	Bytecode::variableArrayPtr vars = RPN_NEW Bytecode::VariableArray;
	Bytecode::doubleArrayPtr val = RPN_NEW Bytecode::DoubleArray;
	val->push_back(value);
	vars->push_back(val);
	Bytecode::doubleArrayPtr ret = execute(vars);
	assert(ret->size() == 1);
	return (*ret)[0];
}

double Program::executeOnceMulti(const Bytecode::doubleArrayPtr& values)
{
	if (_numVariables != 1)
		throw Error("Cannot execute program : wrong number of input variables");
	Bytecode::variableArrayPtr vars = RPN_NEW Bytecode::VariableArray;
	for (Bytecode::DoubleArray::const_iterator it=values->begin();it!=values->end();it++)
	{
		Bytecode::doubleArrayPtr val = RPN_NEW Bytecode::DoubleArray;
		val->push_back(*it);
		vars->push_back(val);
	}
	Bytecode::doubleArrayPtr ret = execute(vars);
	assert(ret->size() == 1);
	return (*ret)[0];
}

Bytecode::doubleArrayPtr Program::executeMultiSingle(const Bytecode::doubleArrayPtr& values)
{
	if (_numVariables != 1)
		throw Error("Cannot execute program : wrong number of input variables");
	Bytecode::variableArrayPtr vars = RPN_NEW Bytecode::VariableArray;
	Bytecode::doubleArrayPtr val = RPN_NEW Bytecode::DoubleArray;
	for (Bytecode::DoubleArray::const_iterator it=values->begin();it!=values->end();it++)
	{
		val->push_back(*it);
	}
	vars->push_back(val);
	return execute(vars);
}

void Program::dump()
{
	for (std::vector<Bytecode::Instruction>::iterator it=_bytecode.begin();it!=_bytecode.end();it++)
	{
			switch (it->opcode)
			{
			case Bytecode::PUSHI:
				std::cout << "NUM " << it->value << std::endl;
				break;
			case Bytecode::PUSHR:
				std::cout << "VAR " << it->varpos << std::endl;
				break;
			case Bytecode::OP:
				std::cout << "OPERATOR " << it->op << std::endl;
				break;
			}
	}
}

