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

#include <librpn/Bytecode.h>
#include <assert.h>
#include <iostream>
using namespace rpn;

Bytecode::DoubleArray::DoubleArray()
{
}
Bytecode::DoubleArray::DoubleArray(int size) : std::vector<double>(size)
{
}
Bytecode::VariableArray::VariableArray()
{
}

Bytecode::Instruction::Instruction() : opcode(NOOP)
{
}

Bytecode::Instruction::Instruction(double value) : opcode(PUSHI), value(value)
{
}

Bytecode::Instruction::Instruction(int varpos) : opcode(PUSHR), varpos(varpos)
{
}

Bytecode::Instruction::Instruction(int op, int card) : opcode(OP), op(op), card(card)
{
}
