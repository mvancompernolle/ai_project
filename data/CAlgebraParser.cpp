/*

CALGEBRAPARSER.CPP: IMPLEMENTS calculus::algebra_parser

* calculus-cpp: Scientific "Functional" Library
*
* This software was developed at McGill University (Montreal, 2002) by
* Olivier Giroux in the course of his studies in Mechanical Engineering.
* It was presented, along with an accompanying paper, for credit in the fall
* of 2002.
*
* Calculus-cpp was not designed to prove a point or to serve as a formal
* framework within which exact solutions can be derived.  Instead it was
* created to fill the need for run-time functional constructions and to
* accomplish very real and tangible goals.  It remains your responsibility
* to use it properly - as much more sophisticated <math.h>, which allows
* functions to be treated as first-class objects.
*
* You are welcome to make any additions you feel are necessary.

COPYRIGHT AND PERMISSION NOTICE

Copyright (c) 2002, Olivier Giroux, <oliver@canada.com>.

All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without any restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
provided that the copyright notice(s) and this permission notice appear
in all copies of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN
NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS NOTICE BE
LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall not
be used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization of the copyright holder.

THIS SOFTWARE INCLUDES THE NIST'S TNT PACKAGE FOR USE WITH THE EXAMPLES FURNISHED.

THE FOLLOWING NOTICE APPLIES SOLELY TO THE TNT-->
* Template Numerical Toolkit (TNT): Linear Algebra Module
*
* Mathematical and Computational Sciences Division
* National Institute of Technology,
* Gaithersburg, MD USA
*
*
* This software was developed at the National Institute of Standards and
* Technology (NIST) by employees of the Federal Government in the course
* of their official duties. Pursuant to title 17 Section 105 of the
* United States Code, this software is not subject to copyright protection
* and is in the public domain. NIST assumes no responsibility whatsoever for
* its use by other parties, and makes no guarantees, expressed or implied,
* about its quality, reliability, or any other characteristic.
<--END NOTICE

THE FOLLOWING NOTICE APPLIES SOLELY TO LEMON-->
** Copyright (c) 1991, 1994, 1997, 1998 D. Richard Hipp
**
** This file contains all sources (including headers) to the LEMON
** LALR(1) parser generator.  The sources have been combined into a
** single file to make it easy to include LEMON as part of another
** program.
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
** 
** You should have received a copy of the GNU General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
**
** Author contact information:
**   drh@acm.org
**   http://www.hwaci.com/drh/
<--END NOTICE

*/

#include "Calculus_cpp.h"

#pragma warning(disable:4244)
#pragma warning(disable:4267)

#ifdef _MSC_VER
#include <tchar.h>
#endif

calculus::algebra_parser* calculus::algebra_parser::_running_service = NULL;

calculus::algebra_parser::algebra_parser()
{
	this->punary_create_functions = NULL;
	psc_unary_operator_string = NULL;
	pi_unary_operator_string_positions = NULL;
	i_num_unary_operators = 0;

	punary_create_extended_functions = NULL;
	psc_extended_unary_operator_string = NULL;
	pi_extended_unary_operator_string_positions = NULL;
	i_num_extended_unary_operators = 0;

	pbinary_create_functions = NULL;
	psc_binary_operator_string = NULL;
	pi_binary_operator_string_positions = NULL;
	i_num_binary_operators = 0;

	if (_running_service == NULL)
		_running_service = this;
};


calculus::algebra_parser::~algebra_parser()
{
	if (punary_create_functions)
		delete [] punary_create_functions;
	if (psc_unary_operator_string)
		delete [] psc_unary_operator_string;
	if (pi_unary_operator_string_positions)
		delete [] pi_unary_operator_string_positions;

	if (punary_create_extended_functions)
		delete [] punary_create_extended_functions;
	if (psc_extended_unary_operator_string)
		delete [] psc_extended_unary_operator_string;
	if (pi_extended_unary_operator_string_positions)
		delete [] pi_extended_unary_operator_string_positions;

	if (pbinary_create_functions)
		delete [] pbinary_create_functions;
	if (psc_binary_operator_string)
		delete [] psc_binary_operator_string;
	if (pi_binary_operator_string_positions)
		delete [] pi_binary_operator_string_positions;

	if (_running_service == this)
		_running_service = NULL;
};

calculus::algebra_parser*  calculus::algebra_parser::get_service()
{
	if (!_running_service)
	{
		_running_service = new calculus::algebra_parser();
		_running_service->initialize();
	}
	return _running_service;
}

void calculus::algebra_parser::kill_service()
{
	if (_running_service)
		delete _running_service;
}

void calculus::algebra_parser::initialize()
{
	calculus::unary_operators::intrinsic_operators::exponential::Register(this);
	calculus::unary_operators::intrinsic_operators::integer_power::Register(this);
	calculus::unary_operators::intrinsic_operators::ln::Register(this);
	calculus::unary_operators::intrinsic_operators::log::Register(this);
	calculus::unary_operators::intrinsic_operators::negate::Register(this);
	calculus::unary_operators::intrinsic_operators::nop::Register(this);
	calculus::unary_operators::intrinsic_operators::square_root::Register(this);
	calculus::unary_operators::trigonometric_operators::sine::Register(this);
	calculus::unary_operators::trigonometric_operators::cosine::Register(this);
	calculus::unary_operators::trigonometric_operators::tangent::Register(this);
	calculus::unary_operators::trigonometric_operators::arcsine::Register(this);
	calculus::unary_operators::trigonometric_operators::arccosine::Register(this);
	calculus::unary_operators::trigonometric_operators::arctangent::Register(this);
	calculus::unary_operators::hyperbolic_operators::sinh::Register(this);
	calculus::unary_operators::hyperbolic_operators::cosh::Register(this);
	calculus::unary_operators::hyperbolic_operators::tanh::Register(this);
	calculus::unary_operators::derivative_operators::derivative_3b::Register(this);
	calculus::unary_operators::derivative_operators::derivative_3c::Register(this);
	calculus::unary_operators::derivative_operators::derivative_3f::Register(this);
	calculus::unary_operators::derivative_operators::derivative_5b::Register(this);
	calculus::unary_operators::derivative_operators::derivative_5c::Register(this);
	calculus::unary_operators::derivative_operators::derivative_5f::Register(this);

	calculus::binary_operators::intrinsic_operators::addition::Register(this);
	calculus::binary_operators::intrinsic_operators::division::Register(this);
	calculus::binary_operators::intrinsic_operators::exponentiation::Register(this);
	calculus::binary_operators::intrinsic_operators::multiplication::Register(this);
	calculus::binary_operators::intrinsic_operators::subtraction::Register(this);
}

inline void _add_class_registry_info(int * pNumRegEntries,dword_type ** ppFuncReg,char ** ppsc_nameReg,int ** ppPosReg,dword_type pFunc,char * psc_name)
{
	unsigned int uiLen;
	if (*ppsc_nameReg)
	{
		*ppsc_nameReg = (char*)realloc(*ppsc_nameReg,(strlen(psc_name)+ 2 + (uiLen = strlen(*ppsc_nameReg)))*(unsigned int)sizeof(char));
		strcat(*ppsc_nameReg,psc_name);
	}
	else
	{
		*ppsc_nameReg = (char*)malloc((strlen(psc_name) + 2 + (uiLen = 1))*sizeof(char));
		strcpy(*ppsc_nameReg,";");
		strcat(*ppsc_nameReg,psc_name);
	}
	strcat(*ppsc_nameReg,";");
	if (*ppPosReg)
	{
		*ppPosReg = (int *)realloc(*ppPosReg,(*pNumRegEntries+1)*sizeof(unsigned int));
	}
	else
	{
		*ppPosReg = (int *)malloc((*pNumRegEntries+1)*sizeof(unsigned int));
	}
	(*ppPosReg)[*pNumRegEntries] = uiLen-1;
	if (*ppFuncReg)
	{
		*ppFuncReg = (dword_type*)realloc(*ppFuncReg,(*pNumRegEntries+1)*sizeof(void*));
	}
	else
	{
		*ppFuncReg = (dword_type*)malloc((*pNumRegEntries+1)*sizeof(void*));
	}
	(*ppFuncReg)[*pNumRegEntries] = pFunc;
	(*pNumRegEntries)++;
};

void calculus::algebra_parser::register_class(dword_type dwOperatorType,dword_type pcreateFunc,char * pOperatorString )
{
	switch(dwOperatorType)
	{
	case CLASS_UNARY_NORMAL :
		_add_class_registry_info(&this->i_num_unary_operators,(dword_type**)&this->punary_create_functions,&this->psc_unary_operator_string,&this->pi_unary_operator_string_positions,pcreateFunc,pOperatorString);
		break;
	case CLASS_UNARY_EXTENDED :
		_add_class_registry_info(&this->i_num_extended_unary_operators,(dword_type**)&this->punary_create_extended_functions,&this->psc_extended_unary_operator_string,&this->pi_extended_unary_operator_string_positions,pcreateFunc,pOperatorString);
		break;
	case CLASS_BINARY_NORMAL :
		_add_class_registry_info(&this->i_num_binary_operators,(dword_type**)&this->pbinary_create_functions,&this->psc_binary_operator_string,&this->pi_binary_operator_string_positions,pcreateFunc,pOperatorString);
		break;
	}
};

inline void remove_all_spaces(char * pscString)
{
	char * pscCur = pscString;
	while(*pscCur)
	{
		if (isspace(*pscCur))
		{
			char *pscSwap = pscCur;
			while(*pscCur)
			{
				pscCur[0] = pscCur[1];
				pscCur++;
			}
			pscCur = pscSwap;
		}
		pscCur++;
	}
};

calculus::algebra_parser::unary_create_function calculus::algebra_parser::get_unary_create_function(char * pString)
{
	int iOffset = dword_type(strstr(this->psc_unary_operator_string,pString) - (char*)this->psc_unary_operator_string);
	for(int i = 0;i < this->i_num_unary_operators;i++)
	{
		if (iOffset == this->pi_unary_operator_string_positions[i])
			return this->punary_create_functions[i];
	}
	return NULL;
};

calculus::algebra_parser::binary_create_function calculus::algebra_parser::get_binary_create_function(char * pString)
{
	int iOffset = strstr(this->psc_binary_operator_string,pString) - this->psc_binary_operator_string;
	for(int i = 0;i < this->i_num_binary_operators;i++)
	{
		if (iOffset == this->pi_binary_operator_string_positions[i])
			return this->pbinary_create_functions[i];
	}
	return NULL;
};

calculus::algebra_parser::unary_create_extended_function calculus::algebra_parser::get_extended_unary_create_function(char * pString)
{
	int iOffset = strstr(this->psc_extended_unary_operator_string,pString) - this->psc_extended_unary_operator_string;
	for(int i = 0;i < this->i_num_extended_unary_operators;i++)
	{
		if (iOffset == this->pi_extended_unary_operator_string_positions[i])
			return this->punary_create_extended_functions[i];
	}
	return NULL;
};

#include "calculus_grammar.h"

#pragma warning(push)
#pragma warning(disable : 4100 4065)
#include "calculus_grammar.c"
#pragma warning(pop)

calculus::algebraic_operator * calculus::algebra_parser::parse_to_algebra(char * psc_algebraic_string)
{
	calculus::algebraic_operator * palg = NULL;

	void * pParser = ParseAlloc(malloc);
	if (pParser)
	{
		char * pscCurrentBuffer = new char[strlen(psc_algebraic_string)+1];
		if (pscCurrentBuffer)
		{
			strcpy(pscCurrentBuffer,psc_algebraic_string);
			remove_all_spaces(pscCurrentBuffer);
			char * pscLocalPos = pscCurrentBuffer;

			Parse(pParser,BEGIN,0,&palg);
			while(*pscLocalPos)
			{
				//Let's see if the current location is a simple terminal symbol
				switch(*pscLocalPos)
				{
				case '(' :	Parse(pParser,LPAREN,0,&palg);	break;
				case ')' :	Parse(pParser,RPAREN,0,&palg);	break;
				case '+' :	Parse(pParser,PLUS,0,&palg);	break;
				case '-' :	Parse(pParser,MINUS,0,&palg);	break;
				case '*' :	Parse(pParser,TIMES,0,&palg);	break;
				case '/' :	Parse(pParser,DIVIDE,0,&palg);	break;
				case '^' :	Parse(pParser,EXP,0,&palg);		break;
				case ',' :	Parse(pParser,COMMA,0,&palg);	break;
				default :	goto not_a_symbol;
				}
				pscLocalPos++;
				continue;
not_a_symbol:	//If we get here then this is a function/variable/constant

				//Variables and terminals must start with a letter or an underscore
				if (isalpha(*pscLocalPos) || *pscLocalPos == '_')
				{
					char *pscEndOfLocalPos = pscLocalPos;
					for(;isalnum(*pscEndOfLocalPos) || *pscEndOfLocalPos == '_';pscEndOfLocalPos++);
					char c = *pscEndOfLocalPos;
					*pscEndOfLocalPos = 0;

					char * pscNameBuffer = new char[strlen(pscLocalPos)+3];
					if (pscNameBuffer)
					{
						sprintf(pscNameBuffer,";%s;",pscLocalPos);

						void * pFunc = (void*)get_unary_create_function(pscNameBuffer);
						if (pFunc) {
							Parse(pParser,UNARY_OPERATOR,pFunc,&palg);
						}
						else {
							pFunc = (void*)get_extended_unary_create_function(pscNameBuffer);
							if (pFunc) {
								//Parse(pParser,BINARY_OPERATOR,pFunc,&palg);
							}
							else {
								pFunc = (void*)get_binary_create_function(pscNameBuffer);
								if (pFunc) {
									Parse(pParser,BINARY_OPERATOR,pFunc,&palg);
								}
								else {
									Parse(pParser,VARIABLE,_var(pscLocalPos),&palg);
								}
							}
						}
						delete [] pscNameBuffer;
					}

					*pscEndOfLocalPos = c;
					pscLocalPos = pscEndOfLocalPos;
				}
				//Constants must start with a digit
				else if (isdigit(*pscLocalPos))
				{
					
				}
                else{ //ILLEGAL CHARACTER SEQUENCE
                }
			}
			Parse(pParser,END,0,&palg);
			Parse(pParser, 0, 0,&palg);
		}
		delete [] pscCurrentBuffer;
		ParseFree(pParser,free);
	}
	return palg;
};
