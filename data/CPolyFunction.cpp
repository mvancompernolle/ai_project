/*

CPOLYFUNCTION.CPP: 
IMPLEMENTS calculus::unary_operators::polynomials::polynomial

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

#include <stdio.h>
#include "Calculus_cpp.h"

calculus::unary_operators::polynomials::polynomial::polynomial(unsigned int uiOrder,poly_function_type pft,double * pAis,calculus::algebraic_operator* pF) : calculus::unary_operators::unary_operator(pF) {
	_ASSERT((pft == Standard)||(pft == Optimized));
	this->m_epoly_function_type = pft;
	this->m_uiOrder = uiOrder;
	this->m_ppi_coefficients = new double*[1];
	this->m_ppi_coefficients[0] = new double[this->m_uiOrder+1];
	for(unsigned int i = 0; i < (this->m_uiOrder+1); i++)
		this->m_ppi_coefficients[0][i] = pAis[i];
}

calculus::unary_operators::polynomials::polynomial::polynomial(unsigned int uiOrder,double * pXs,double * pAis,calculus::algebraic_operator* pF) : calculus::unary_operators::unary_operator(pF) {
	_ASSERT((pAis != NULL)&& (pXs != NULL) && (uiOrder != 0));
	this->m_epoly_function_type = Interpolatory;
	this->m_uiOrder = uiOrder;
	this->m_ppi_coefficients = new double*[2];
	this->m_ppi_coefficients[0] = new double[this->m_uiOrder+1];
	this->m_ppi_coefficients[1] = new double[this->m_uiOrder+1];
	for(unsigned int i = 0;i < (this->m_uiOrder+1);i++)	{
		this->m_ppi_coefficients[0][i] = pAis[i];
		this->m_ppi_coefficients[1][i] = pXs[i];
	}
}

void calculus::unary_operators::polynomials::polynomial::__load_interpolant(unsigned int uiOrder,double * pIC_1,double * pIC_2) {
	this->m_uiOrder = uiOrder;
	this->m_epoly_function_type = Interpolatory;
	if (this->m_ppi_coefficients) {
		if (this->m_ppi_coefficients[0])
			delete [] this->m_ppi_coefficients[0];
		if (this->m_epoly_function_type == Interpolatory)
			if (this->m_ppi_coefficients[1])
				delete [] this->m_ppi_coefficients[1];
		delete [] this->m_ppi_coefficients;
	}
	this->m_ppi_coefficients = new double*[2];
	this->m_ppi_coefficients[0] = new double[m_uiOrder+1];
	this->m_ppi_coefficients[1] = new double[m_uiOrder+1];
	for(unsigned int i = 0;i < m_uiOrder;i++) {
		this->m_ppi_coefficients[0][i] = pIC_1[i];
		this->m_ppi_coefficients[1][i] = pIC_2[i];
	}
}

calculus::unary_operators::polynomials::polynomial::~polynomial() {
	if (this->m_ppi_coefficients) {
		if (this->m_ppi_coefficients[0])
			delete [] this->m_ppi_coefficients[0];
		if (this->m_epoly_function_type == Interpolatory)
			if (this->m_ppi_coefficients[1])
				delete [] this->m_ppi_coefficients[1];
		delete [] this->m_ppi_coefficients;
	}
}

double calculus::unary_operators::polynomials::polynomial::eval_unary(double a) {
	double retVal = 0,temp = 1;
	unsigned int i;
	calculus::algebraic_operator * pOperand = this->get_operand();
	_ASSERT(pOperand != NULL);
    a = pOperand->eval(&a); //ASSUMES THIS IS A FUNCVTION OF ONLY ONE VARIABLES
	switch(this->m_epoly_function_type) {
	case Standard :
//		STANDARD ALGEBRAIC REPRESENTATION OF POLYNOMIALS
//		a + bx + cx^2 + dx^3 + ...
		for(i = 0;i < (this->m_uiOrder+1);i++) {
			retVal += temp*this->m_ppi_coefficients[0][i];
			temp *= a;
		}
		break;
	case Optimized :
//		COMPUTATIONALLY OPTIMIZED POLYNOMIAL FORMS
//		( (((((a)x+b)x+c)x+d)x+e) ...  )
		i = this->m_uiOrder+1;
		while(i > 1) {
			i--;
			retVal += this->m_ppi_coefficients[0][i];
			retVal *= a;
		}
		retVal += this->m_ppi_coefficients[0][0];
		break;
	case Interpolatory :
//		NEWTON'S INTERPOLATORY DIVIDED-DIFFERENCE POLYNOMIALS
//		f(x0) + f(x0,x1)(x-x0) + f(x0,x1,x2)(x-x0)(x-x1) + ...
		for(i = 0;i < (this->m_uiOrder+1);i++) {
			retVal += temp*this->m_ppi_coefficients[0][i];
			temp *= (a-this->m_ppi_coefficients[1][i]);
		}
		break;
	}
	return retVal;
}

calculus::algebraic_operator* calculus::unary_operators::polynomials::polynomial::partial_derivative(variable * pVar) {
	if (!this->is_function_of(pVar))
		return calculus::_cst(0);
	double* pAis;
	double* pXs;
	unsigned int i;
	calculus::algebraic_operator* ppartial_derivative = this->get_operand()->get_partial_derivative(pVar);
	if (ppartial_derivative) {
		switch(this->m_epoly_function_type) {
		case Standard :
	//		STANDARD ALGEBRAIC REPRESENTATION OF POLYNOMIALS
	//		a + bx + cx^2 + dx^3 + ...
			pAis = new double[this->m_uiOrder];
			for(i = 0;i < this->m_uiOrder;i++)
			{
				pAis[i] = (i+1)*this->m_ppi_coefficients[0][i+1];
			};
			ppartial_derivative = binary_operators::intrinsic_operators::_multiply(calculus::unary_operators::polynomials::polynomial::create(this->m_uiOrder-1,Standard,pAis,this->get_operand()),ppartial_derivative);
			delete [] pAis;
			break;
		case Optimized :
	//		COMPUTATIONALLY OPTIMIZED POLYNOMIAL FORMS
	//		( (((((a)x+b)x+c)x+d)x+e) ...  )
			pAis = new double[this->m_uiOrder];
			for(i = 0;i < this->m_uiOrder;i++)
			{
				pAis[i] = this->m_ppi_coefficients[0][i+1];
			};
			ppartial_derivative = binary_operators::intrinsic_operators::_multiply(calculus::unary_operators::polynomials::polynomial::create(this->m_uiOrder-1,Optimized,pAis,this->get_operand()),ppartial_derivative);
			delete [] pAis;
			break;
		case Interpolatory :
	//		NEWTON'S INTERPOLATORY DIVIDED-DIFFERENCE POLYNOMIALS
	//		f(x0) + f(x0,x1)(x-x0) + f(x0,x1,x2)(x-x0)(x-x1) + ...
			pAis = new double[this->m_uiOrder];
			pXs = new double[this->m_uiOrder];
			for(i = 0;i < this->m_uiOrder;i++)
			{
				pAis[i] = this->m_ppi_coefficients[0][i+1];
				pXs[i] = this->m_ppi_coefficients[1][i+1];
			};
			ppartial_derivative = binary_operators::intrinsic_operators::_multiply(calculus::unary_operators::polynomials::polynomial::create(this->m_uiOrder-1,pXs,pAis,this->get_operand()),ppartial_derivative);
			delete [] pAis;
			delete [] pXs;
			break;
		default :
			ppartial_derivative = this->get_partial_derivative(pVar);
			break;
		}
	}
	return ppartial_derivative;
}

int calculus::unary_operators::polynomials::polynomial::to_string(char* pBuffer) {
	if (!pBuffer)
		return 1024;
	char scNameBuffer[4096] = "";
	char scNamePartBuffer[4096] = "";
	this->get_operand()->to_string(scNameBuffer);
	strcpy(pBuffer,"POLY(");
	for(unsigned int i = 0;i < this->m_uiOrder+1;i++) {
		if (i) {
			switch(this->m_epoly_function_type) {
//	-Standard : a0 + a1x + a2x^2 + a3x^3 + ...
			case Standard :
				sprintf(scNamePartBuffer,(i>1)?"+%g*(%s)^%u":"+%g*(%s)",this->m_ppi_coefficients[0][i],scNameBuffer,i);
				break;
//	-Optimized : a0 + x(a1 + x( a2 + x(...)))
			case Optimized :
				sprintf(scNamePartBuffer,"+(%s)*(%g",scNameBuffer,this->m_ppi_coefficients[0][i]);
				break;
//	-Interpolatory : a0 + a1(x-x0) + a2(x-x0)(x-x1) + ...
			case Interpolatory :
				sprintf(scNamePartBuffer+strlen(scNamePartBuffer),"*(%s-%g)",scNameBuffer,this->m_ppi_coefficients[1][i]);
				sprintf(pBuffer+strlen(pBuffer),"+%g%s",this->m_ppi_coefficients[0][i],scNamePartBuffer);
				continue;
			}
			strcat(pBuffer,scNamePartBuffer);
		}
		else
			sprintf(pBuffer+strlen(pBuffer),"%g",this->m_ppi_coefficients[0][i]);

	}
	if (this->m_epoly_function_type == Optimized)
		for(unsigned int i = 0;i < this->m_uiOrder;i++)
			strcat(pBuffer,")");
	strcat(pBuffer,")");	
	return 1024;
}

void calculus::unary_operators::polynomials::polynomial::to_IA32_binary(PCT_INFO pInfo) {
	this->get_operand()->to_IA32_binary(pInfo);
	unsigned int i = 0;
	switch(this->m_epoly_function_type) {
	case Standard :
		CompilerWriteFLD_IMM32PTR64(pInfo);
				CompilerWritePTR_ENTRY(pInfo,(unsigned char*)pInfo->pv_instruction_storage_pos);
				CompilerWriteIMM32(pInfo,(dword_type)pInfo->pv_local_storage_pos);
				CompilerWriteLocalInfo(pInfo,(unsigned char*)&this->m_ppi_coefficients[0][i],sizeof(double));
		CompilerWriteFLD1(pInfo);
		for(;i < this->m_uiOrder;i++)
		{
			CompilerWriteFMUL_STX(pInfo,REG_STX(2));
			CompilerWriteFLD_IMM32PTR64(pInfo);
				CompilerWritePTR_ENTRY(pInfo,(unsigned char*)pInfo->pv_instruction_storage_pos);
				CompilerWriteIMM32(pInfo,(dword_type)pInfo->pv_local_storage_pos);
				CompilerWriteLocalInfo(pInfo,(unsigned char*)&this->m_ppi_coefficients[0][i+1],sizeof(double));
			CompilerWriteFMUL_STX(pInfo,REG_STX(1));	
			CompilerWriteFRADDP_STX(pInfo,REG_STX(2));
		}
		CompilerWriteFINCSTP(pInfo);
		CompilerWriteFXCH_STX(pInfo,REG_STX(1));
		CompilerWriteFINCSTP(pInfo);
		break;
	case Optimized :
		i = this->m_uiOrder;
		CompilerWriteFLD_IMM32PTR64(pInfo);
				CompilerWritePTR_ENTRY(pInfo,(unsigned char*)pInfo->pv_instruction_storage_pos);
				CompilerWriteIMM32(pInfo,(dword_type)pInfo->pv_local_storage_pos);
				CompilerWriteLocalInfo(pInfo,(unsigned char*)(&(this->m_ppi_coefficients[0][i])),sizeof(double));
		for(i--;i != 0xffffffffu;i--)
		{
			CompilerWriteFMUL_STX(pInfo,REG_STX(1));
			CompilerWriteFLD_IMM32PTR64(pInfo);
					CompilerWritePTR_ENTRY(pInfo,(unsigned char*)pInfo->pv_instruction_storage_pos);
					CompilerWriteIMM32(pInfo,(dword_type)pInfo->pv_local_storage_pos);
					CompilerWriteLocalInfo(pInfo,(unsigned char*)(&(this->m_ppi_coefficients[0][i])),sizeof(double));
			CompilerWriteFRADDP_STX(pInfo,REG_STX(1));
		}
		CompilerWriteFXCH_STX(pInfo,REG_STX(1));
		CompilerWriteFINCSTP(pInfo);
		break;
	case Interpolatory :
		_ASSERT(0);
		break;
	default : 
		_ASSERT(0);
	}
}

void calculus::unary_operators::polynomials::polynomial::annotate(PPT_INFO pParseInfo) {
	this->get_operand()->annotate(pParseInfo);

    pParseInfo->i_operator_count++;

	switch(this->m_epoly_function_type) {
	case Standard :
		pParseInfo->st_instruction_storage_size	+= CompilerSizeOfFLD_IMM32PTR64()
												+ sizeof(dword_type)
												+ CompilerSizeOfFLD1() 
												+ this->m_uiOrder*(
													  2*CompilerSizeOfFMUL_STX() 
													+ CompilerSizeOfFLD_IMM32PTR64()
													+ sizeof(dword_type)
													+ CompilerSizeOfFRADDP_STX()) 
												+ 2*CompilerSizeOfFINCSTP() 
												+ CompilerSizeOfFXCH_STX();
        pParseInfo->i_instruction_count			+= 5 + this->m_uiOrder*4;
		pParseInfo->st_pmap_size				+= this->m_uiOrder+1;
		pParseInfo->st_local_storage_size			+= sizeof(double)*(this->m_uiOrder+1);
		pParseInfo->i_fpu_stack_offset			+= 4;
		_ASSERT(pParseInfo->i_fpu_stack_offset < COMPILER_FPU_MAX_STACK_USE);
		break;
	case Optimized :
		pParseInfo->st_instruction_storage_size	+= CompilerSizeOfFLD_IMM32PTR64()
												+ sizeof(dword_type)
												+ (this->m_uiOrder)*(
													  CompilerSizeOfFMUL_STX()
													+ CompilerSizeOfFLD_IMM32PTR64()
													+ sizeof(dword_type)
													+ CompilerSizeOfFRADDP_STX())
												+ CompilerSizeOfFINCSTP()
												+ CompilerSizeOfFXCH_STX();
        pParseInfo->i_instruction_count			+= 3 + this->m_uiOrder*3;
		pParseInfo->st_pmap_size				+= this->m_uiOrder+1;
		pParseInfo->st_local_storage_size			+= sizeof(double)*(this->m_uiOrder+1);
		pParseInfo->i_fpu_stack_offset			+= 3;
		_ASSERT((pParseInfo->i_fpu_stack_offset < COMPILER_FPU_MAX_STACK_USE));
		break;
	case Interpolatory :
		_ASSERT(0);
		break;
	default : 
		_ASSERT(0);
	}
}

calculus::unary_operators::polynomials::polynomial * calculus::unary_operators::polynomials::_poly(unsigned int uiOrder,poly_function_type pft,double * pAis,calculus::algebraic_operator* pF) {
	return calculus::unary_operators::polynomials::polynomial::create(uiOrder,pft,pAis,pF);
}

calculus::unary_operators::polynomials::polynomial * calculus::unary_operators::polynomials::_poly(unsigned int uiOrder,double * pXs,double * pAis,calculus::algebraic_operator* pF) {
	return calculus::unary_operators::polynomials::polynomial::create(uiOrder,pXs,pAis,pF);
}

void calculus::unary_operators::polynomials::polynomial::get_lagrange_interpolatory_coefficients_from_function(unsigned int uiOrder,double a,double b,algebraic_operator* pF,double ** ppXs,double ** ppAis) {
	unsigned int uiNumPoints = uiOrder+1;
	*ppXs = new double[uiNumPoints];
	double * pYs = new double[uiNumPoints];
	double h = (b-a)/uiNumPoints;
	(*ppXs)[0] = a;
	for(unsigned int i = 1;i < uiNumPoints;i++) {
		(*ppXs)[i] = (*ppXs)[i-1] + h;
		pYs[i] = pF->eval(*ppXs+i);  //THIS ASSUMES pF IS A FUNCTION IN ONE VARIABLE ONLY !!!!
	}
	calculus::unary_operators::polynomials::polynomial::get_lagrange_interpolatory_coefficients_from_points(uiNumPoints,*ppXs,pYs,ppAis);
	delete [] pYs;
}

void calculus::unary_operators::polynomials::polynomial::get_lagrange_interpolatory_coefficients_from_points(unsigned int uiNumPoints,double * lpXPoints,double * lpYPoints,double ** ppAis) {
//USE DIVIDED-DIFFERENCES AND THREE ARRAYS TO FIND THE Ai's
	unsigned int numEntries = 0,a;
	for(a = 0;a <= uiNumPoints;a++)
        numEntries += a;
	double *dividedDifferences = (double*)malloc(numEntries*sizeof(double));
	double *localDifference = dividedDifferences;
	double *lastDifference = localDifference;

	unsigned int numDifferences = uiNumPoints;
	//LOAD THE ZEROTH STAGE (ALL Y VALUES)
	for (a = 0;a < numDifferences;a++) {
		*localDifference = lpYPoints[a];
		localDifference++;
	}
	numDifferences--;
	lastDifference = localDifference;
	//PERFORM ALL THE OTHER DIFFERENCES
	while (numDifferences) {
		while(localDifference < (lastDifference + numDifferences)) {
			unsigned int tempNumDif = numDifferences;
			double *lpUpperDDDivider = localDifference;
			double *lpLowerDDDivider = localDifference;
			while((lpUpperDDDivider - tempNumDif - 1) >= dividedDifferences) {
				lpUpperDDDivider -= tempNumDif;
				lpLowerDDDivider -= (tempNumDif + 1);
				tempNumDif++;
			}
			double upperDivider = lpXPoints[((lpUpperDDDivider - dividedDifferences))];
			double lowerDivider = lpXPoints[((lpLowerDDDivider - dividedDifferences))];
			*localDifference =	(*(localDifference - numDifferences) - *(localDifference - numDifferences - 1))/(upperDivider - lowerDivider);
			localDifference++;
		}
		numDifferences--;
		lastDifference = localDifference;
	}

//TRANSFER BACK TO DATA STRUCTURES
	*ppAis = new double[uiNumPoints];
	localDifference = dividedDifferences;
	for(a = 0;a < uiNumPoints; a++) {
		(*ppAis)[a] = *localDifference;
		localDifference += (uiNumPoints-a);
	}
	free(dividedDifferences);
}

calculus::unary_operators::polynomials::polynomial * calculus::unary_operators::polynomials::_lagrange_poly(unsigned int uiNumPoints,double * pXs,double * pYs,calculus::algebraic_operator* pF) {
    return static_cast<calculus::unary_operators::polynomials::polynomial*>(calculus::unary_operators::polynomials::polynomial::create_lagrange_polynomial(uiNumPoints,pXs,pYs,pF));
}

calculus::unary_operators::polynomials::polynomial * calculus::unary_operators::polynomials::_lagrange_poly(unsigned int uiOrder,double a,double b,algebraic_operator* pF) {
	return static_cast<calculus::unary_operators::polynomials::polynomial*>(calculus::unary_operators::polynomials::polynomial::create_lagrange_polynomial(uiOrder,a,b,pF));
}
