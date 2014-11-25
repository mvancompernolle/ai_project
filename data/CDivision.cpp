/*

CDIVISION.CPP: 
IMPLEMENTS calculus::binary_operators::intrinsic_operators::division

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

double calculus::binary_operators::intrinsic_operators::division::eval_binary(double a,double b) {
	return a/b;
}

int calculus::binary_operators::intrinsic_operators::division::to_string(char* pBuffer) {
	if (!pBuffer)
		return 3+this->GetLeftOperand()->to_string(NULL)+this->GetRightOperand()->to_string(NULL);
	int iOffset = 1;
	strcpy(pBuffer,"(");
	iOffset += this->GetLeftOperand()->to_string(pBuffer+iOffset);
	strcpy(pBuffer+iOffset,"/");
	iOffset += this->GetRightOperand()->to_string(pBuffer+(++iOffset));
	strcpy(pBuffer+iOffset,")");
	return iOffset+1;
}

void calculus::binary_operators::intrinsic_operators::division::to_IA32_binary(PCT_INFO pInfo) {
	double* pLocalPos = (double*)pInfo->pv_local_storage_pos;
	double d = 0;
	CompilerWriteLocalInfo(pInfo,(byte_type*)&d,sizeof(double));
	this->GetRightOperand()->to_IA32_binary(pInfo);
	CompilerWriteFSTP_IMM32PTR64(pInfo);
		CompilerWritePTR_ENTRY(pInfo,(unsigned char*)pInfo->pv_instruction_storage_pos);
		CompilerWriteIMM32(pInfo,(dword_type)pLocalPos);
	this->GetLeftOperand()->to_IA32_binary(pInfo);
	CompilerWriteFLD_IMM32PTR64(pInfo);
		CompilerWritePTR_ENTRY(pInfo,(unsigned char*)pInfo->pv_instruction_storage_pos);
		CompilerWriteIMM32(pInfo,(dword_type)pLocalPos);
	CompilerWriteFDIVP_STX(pInfo,REG_STX(1));
}

void calculus::binary_operators::intrinsic_operators::division::annotate(PPT_INFO pParseInfo) {
	pParseInfo->i_instruction_count;
	pParseInfo->st_instruction_storage_size	+=	CompilerSizeOfFSTP_IMM32PTR64() 
											+	CompilerSizeOfFLD_IMM32PTR64() 
											+	CompilerSizeOfFRDIVP_STX()
											+	2*sizeof(dword_type);
	pParseInfo->i_instruction_count			+=	3;
	pParseInfo->st_local_storage_size			+= sizeof(double);
	pParseInfo->i_operator_count++;
	pParseInfo->st_pmap_size				+=2;
	this->GetRightOperand()->annotate(pParseInfo);
	this->GetLeftOperand()->annotate(pParseInfo);
}

calculus::algebraic_operator* calculus::binary_operators::intrinsic_operators::division::partial_derivative(variable * pVar) {
	if (!this->is_function_of(pVar))
		return calculus::_cst(0.0);
	calculus::algebraic_operator* pOpDR = this->GetRightOperand()->get_partial_derivative(pVar);
	calculus::algebraic_operator* pOpDL = this->GetLeftOperand()->get_partial_derivative(pVar);
	calculus::algebraic_operator* pD = NULL;
	_ASSERT(pOpDL);
	_ASSERT(pOpDR);
	if (pOpDL)
		pOpDL->addref();
	if (pOpDR)
		pOpDR->addref();
	if ( pOpDR && pOpDL ) {
		using namespace calculus::binary_operators::intrinsic_operators;
		using namespace calculus::unary_operators::intrinsic_operators;
		pD = _divide(_subtract(_multiply(pOpDL,this->GetRightOperand()), _multiply(pOpDR,this->GetLeftOperand())),_INT_POW(2,this->GetRightOperand()));
	}
	if (pOpDL)
		pOpDL->release();
	if (pOpDR)
		pOpDR->release();
	return pD;
}

calculus::algebraic_operator * calculus::binary_operators::intrinsic_operators::_divide(calculus::algebraic_operator * arg1,calculus::algebraic_operator * arg2)  { 
	_ASSERT(arg1);
	_ASSERT(arg2);
	calculus::algebraic_operator * pRet = NULL;
	if (calculus::binary_operators::binary_operator::IsUsingConstantOptimizations()) {
		if (typeid(*arg1) == typeid(calculus::constant)) {
			calculus::constant* pC1 = dynamic_cast<calculus::constant*>(arg1);
			if (typeid(*arg2) == typeid(calculus::constant))	{
				pRet = calculus::constant::create(pC1->GetValue()/dynamic_cast<calculus::constant*>(arg2)->GetValue());	}
			else if (pC1->GetValue() == 0)	{
				pRet = arg1;	}
			else
				pRet = calculus::binary_operators::intrinsic_operators::division::create(arg1,arg2);
		}
		else if (typeid(*arg2) == typeid(calculus::constant)) {
			calculus::constant* pC2 = dynamic_cast<calculus::constant*>(arg2);
			if (pC2->GetValue() == 1)	{
				pRet = arg1;	}
			else
				pRet = calculus::binary_operators::intrinsic_operators::division::create(arg1,arg2);
		}
		else
			pRet = calculus::binary_operators::intrinsic_operators::division::create(arg1,arg2);
	}
	else
		pRet = calculus::binary_operators::intrinsic_operators::division::create(arg1,arg2);
	return pRet;
}
