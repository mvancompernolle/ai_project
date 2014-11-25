/*

CARCCOSINE.CPP: 
IMPLEMENTS calculus::unary_operators::trigonometric_operators::arccosine

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
#include <math.h>

#pragma warning(disable:4244)

double calculus::unary_operators::trigonometric_operators::arccosine::eval_unary(double a) {
	return (double)::acos(a);
}

int calculus::unary_operators::trigonometric_operators::arccosine::to_string(char* pBuffer) {
	if (!pBuffer)
		return 6+this->get_operand()->to_string(NULL);
	int iOffset = 4;
	strcpy(pBuffer,"acos(");
	iOffset += this->get_operand()->to_string(pBuffer+iOffset);
	strcpy(pBuffer+iOffset,")");
	return iOffset+1;
}

void calculus::unary_operators::trigonometric_operators::arccosine::to_IA32_binary(PCT_INFO pInfo) {
	double (__cdecl* pacos)(double) = ::acos;
	this->get_operand()->to_IA32_binary(pInfo);
	CompilerWriteSUB_EXX_IMM32(pInfo,REG_ESP);
		CompilerWriteIMM32(pInfo,sizeof(double));
		pInfo->i_stack_offset += sizeof(double);
	CompilerWriteFSTP_EBPX_IMM32(pInfo);
		CompilerWriteIMM32(pInfo,-int(pInfo->i_stack_offset));
	CompilerWriteMOV_EXX_IMM32(pInfo,REG_EAX);
		CompilerWriteIMM32(pInfo,(dword_type)pacos);
	CompilerWriteCALL_EXX(pInfo,REG_EAX);
	CompilerWriteADD_EXX_IMM32(pInfo,REG_ESP);
		CompilerWriteIMM32(pInfo,sizeof(double));
		pInfo->i_stack_offset -= sizeof(double);
	pInfo->pHeader->i_f_flags |= COMPILER_FLAG_FUNCTION_NOT_REMOTABLE;
}

void calculus::unary_operators::trigonometric_operators::arccosine::annotate(PPT_INFO pParseInfo) {
	this->get_operand()->annotate(pParseInfo);
	pParseInfo->st_instruction_storage_size	+= CompilerSizeOfSUB_EXX_IMM32()
											+  4*sizeof(dword_type)
											+  CompilerSizeOfFSTP_EBPX_IMM32()
											+  CompilerSizeOfMOV_EXX_IMM32()
											+  CompilerSizeOfCALL_EXX()
											+  CompilerSizeOfADD_EXX_IMM32();
	pParseInfo->i_instruction_count			+=	5;
	pParseInfo->i_operator_count++;
	pParseInfo->i_features_needed			|= FEAT_NEED_EAX;
}

calculus::algebraic_operator* calculus::unary_operators::trigonometric_operators::arccosine::partial_derivative(variable * pVar) {
	if (!this->is_function_of(pVar))
		return calculus::_cst(0);
	calculus::algebraic_operator* pOpD = this->get_operand()->get_partial_derivative(pVar);
	calculus::algebraic_operator* pD = NULL;
	_ASSERT(pOpD);
	if (pOpD != NULL) {
		using namespace calculus::binary_operators::intrinsic_operators;
		using namespace calculus::unary_operators::intrinsic_operators;
		pOpD->addref();
		pD = _neg(_divide(pOpD,_sqrt(_subtract(_cst(1.0),_INT_POW(2,this->get_operand())))));
		pOpD->release();
	}
	return pD;
}
