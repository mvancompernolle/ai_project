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

#include <librpn/Operator.h>
#include <librpn/Error.h>
#include <librpn/Stack.h>
#include <assert.h>
#include <cmath>
using namespace rpn;

#ifdef _WIN32
#include <float.h>
#define isnan _isnan
#define isfinite _finite
#else
#define isfinite finitel
#endif //_WIN32


Operator::Operator(int type) : Token(OPERATOR), _type(type)
{
	_card = GetNumOperands(_type);
}

std::map<std::string,int> Operator::_s_reprs;
Operator::Initializer Operator::_s_initializer;
Operator::Initializer::Initializer()
{
	Operator::_s_reprs["+"] = Operator::PLUS;
	Operator::_s_reprs["-"] = Operator::MINUS;
	Operator::_s_reprs["*"] = Operator::MULT;
	Operator::_s_reprs["/"] = Operator::DIV;
	Operator::_s_reprs["%"] = Operator::MOD;
/*	Operator::_s_reprs["PLUS"] = Operator::PLUS;
	Operator::_s_reprs["MINUS"] = Operator::MINUS;
	Operator::_s_reprs["MULT"] = Operator::MULT;
	Operator::_s_reprs["DIV"] = Operator::DIV;
	Operator::_s_reprs["MOD"] = Operator::MOD;*/

	Operator::_s_reprs["<"] = Operator::LT;
	Operator::_s_reprs["<="] = Operator::LE;
	Operator::_s_reprs[">"] = Operator::GT;
	Operator::_s_reprs[">="] = Operator::GE;
	Operator::_s_reprs["=="] = Operator::EQ;
	Operator::_s_reprs["!="] = Operator::NE;
	Operator::_s_reprs["LT"] = Operator::LT;
	Operator::_s_reprs["LE"] = Operator::LE;
	Operator::_s_reprs["GT"] = Operator::GT;
	Operator::_s_reprs["GE"] = Operator::GE;
	Operator::_s_reprs["EQ"] = Operator::EQ;
	Operator::_s_reprs["NE"] = Operator::NE;

	Operator::_s_reprs["UN"] = Operator::UN;
//	Operator::_s_reprs["un"] = Operator::UN;
	Operator::_s_reprs["ISINF"] = Operator::ISINF;
//	Operator::_s_reprs["isinf"] = Operator::ISINF;
	Operator::_s_reprs["IF"] = Operator::IF;
//	Operator::_s_reprs["if"] = Operator::IF;

	Operator::_s_reprs["COS"] = Operator::COS;
	Operator::_s_reprs["SIN"] = Operator::SIN;
	Operator::_s_reprs["LOG"] = Operator::LOG;
	Operator::_s_reprs["EXP"] = Operator::EXP;
	Operator::_s_reprs["SQRT"] = Operator::SQRT;
	Operator::_s_reprs["ATAN"] = Operator::ATAN;
	Operator::_s_reprs["ATAN2"] = Operator::ATAN2;
	Operator::_s_reprs["FLOOR"] = Operator::FLOOR;
	Operator::_s_reprs["CEIL"] = Operator::CEIL;
/*	Operator::_s_reprs["cos"] = Operator::COS;
	Operator::_s_reprs["sin"] = Operator::SIN;
	Operator::_s_reprs["log"] = Operator::LOG;
	Operator::_s_reprs["exp"] = Operator::EXP;
	Operator::_s_reprs["sqrt"] = Operator::SQRT;
	Operator::_s_reprs["atan"] = Operator::ATAN;
	Operator::_s_reprs["atan2"] = Operator::ATAN2;
	Operator::_s_reprs["floor"] = Operator::FLOOR;
	Operator::_s_reprs["ceil"] = Operator::CEIL;*/
}

Operator* Operator::FromString(const std::string& repr)
{
	assert(repr.size() != 0);

	std::map<std::string,int>::const_iterator found = _s_reprs.find(repr);
	if (found == _s_reprs.end())
		throw Error("Unknown operator : "+repr);
	else
		return RPN_NEW Operator(found->second);
}

int Operator::GetNumOperands(int type)
{
	switch (type)
	{
	case LT:
	case LE:
	case GT:
	case GE:
	case EQ:
	case NE:
		return 2;
	case UN:
	case ISINF:
		return 1;
	case IF:
		return 3;

	case PLUS:
	case MINUS:
	case MULT:
	case DIV:
	case MOD:
		return 2;

	case COS:
	case SIN:
	case LOG:
	case EXP:
	case SQRT:
	case ATAN:
	case FLOOR:
	case CEIL:
		return 1;
	case ATAN2:
		return 2;
/*
		// II) Comparing values
		MIN, MAX, LIMIT,

		// III) Arithmetics
		// 2) Maths functions
		// 3) Conversion functions
		DEG2RAD, RAD2DEG,

		// IV) Sort operations
		SORT, REV, TREND,

		// V) Stack manipulation
		DUP, POP, EXC*/
	default:
		throw Error("Unknown operand type");
	}
}

Number* Operator::apply(const std::vector<numberPtr>& args) const
{
	assert ((int)args.size() == card());

	switch (args.size())
	{
	case 1:
		switch (_type)
		{
		case SQRT:
			if (args[0]->val() < 0.0)
				throw Error("Complex square root");
		}
		return RPN_NEW Constant(Apply(_type, args[0]->val()));
	case 2:
		switch (_type)
		{
		case ATAN2:
		case DIV:
			if (args[0]->val() == 0.0)
				throw Error("Division by zero");
		case MOD:
//			if (args[0]->type() != Number::INTEGER || args[1]->type() != Number::INTEGER)
//				throw Error("Modulo operation on non-integer values");
			if (args[0]->val() == 0.0)
				throw Error("Modulo zero");
		}
		return RPN_NEW Constant(Apply(_type, args[1]->val(), args[0]->val()));
	case 3:
		return RPN_NEW Constant(Apply(_type, args[2]->val(), args[1]->val(), args[0]->val()));
	default:
		throw Error("Operator has wrong number of operands");
	}
}

double Operator::Apply(int unaryOp, const double arg)
{
	switch (unaryOp)
	{
	case UN:
		return isnan(arg) != 0;
	case ISINF:
		return isfinite(arg) == 0;
	case COS:
		return cos(arg);
	case SIN:
		return sin(arg);
	case LOG:
		return log(arg);
	case EXP:
		return exp(arg);
	case SQRT:
		return sqrt(arg);
	case ATAN:
		return atan(arg);
	case FLOOR:
		return floor(arg);
	case CEIL:
		return ceil(arg);
	default:
		throw Error("Operator not supported in compiled mode");
	}
}

double Operator::Apply(int binaryOp, double arg1, double arg2)
{
	switch (binaryOp)
	{
	case LT:
		return arg1 < arg2;
	case LE:
		return arg1 <= arg2;
	case GT:
		return arg1 > arg2;
	case GE:
		return arg1 >= arg2;
	case EQ:
		return arg1 == arg2;
	case NE:
		return arg1 != arg2;
	case PLUS:
		return arg1 + arg2;
	case MINUS:
		return arg1 - arg2;
	case MULT:
		return arg1 * arg2;
	case DIV:
		return arg1 / arg2;
	case MOD:
		return (double) ((int)arg1 % (int)arg2);
	case ATAN2:
		return atan(arg1 / arg2);
	default:
		throw Error("Operator not supported in compiled mode");
	}
}

double Operator::Apply(int ternaryOp, double arg1, double arg2, double arg3)
{
	switch (ternaryOp)
	{
	case IF:
		if (arg1) 
			return arg2; 
		else 
			return arg3;
	default:
		throw Error("Operator not supported in compiled mode");
	}
}

void Operator::visit(Stack* visitor)
{
	visitor->accept(this);
}

std::string Operator::toString() const
{
	for (std::map<std::string,int>::const_iterator it=_s_reprs.begin();it!=_s_reprs.end();it++)
	{
		if (it->second == _type)
			return it->first;
	}
	return "<OP-NOTXTREPR-FOUND>";
}
