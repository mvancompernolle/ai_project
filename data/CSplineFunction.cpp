/*

CVARIABLE.CPP: IMPLEMENTS calculus::unary_operators::hyperbolic_operators::tanh

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

/////////////////////////////////////////////////////////////////////////
///Polynomial functions in one variable support//////////////////////////
/////////////////////////////////////////////////////////////////////////
/*
template class CSplineFunction;
template class CSplineFunction;
template class CSplineFunction;


CSplineFunction::CSplineFunction() : constant()
{
	this->m_eFunctionType = Polynomial;
	this->m_epoly_function_type = Null;
	this->m_uiOrder = 0;
	this->lplpCoefficients = NULL;
};


CSplineFunction::CSplineFunction(unsigned int uiOrder,DATA<double,1>* pAi,algebraic_operator* pF,bool bOptimized) : constant()
{
	this->m_eFunctionType = Polynomial;
	this->m_epoly_function_type = (bOptimized)?Optimized:Standard;
	this->m_uiOrder = uiOrder;
	this->lplpCoefficients = (double**)malloc(sizeof(double*));
	this->lplpCoefficients[0] = (double*)malloc((this->m_uiOrder+1)*sizeof(double));
	for(unsigned int i = 0; i < (this->m_uiOrder+1); i++)
	{
		this->lplpCoefficients[0][i] = pAi->m_tDataSet[0];
		pAi = pAi->m_pNext;
	};
};


CSplineFunction::CSplineFunction(unsigned int uiOrder,DATA<double,2>* pAi,algebraic_operator* pF) : constant()
{
	this->m_eFunctionType = Polynomial;
	this->m_epoly_function_type = Null;
	this->m_uiOrder = 0;
	this->lplpCoefficients = NULL;
};


CSplineFunction::CSplineFunction(CSplineFunction& ppf) : constant(ppf)
{
	this->m_eFunctionType = Polynomial;
	this->m_epoly_function_type = ppf.m_epoly_function_type;
	this->m_uiOrder = ppf.m_uiOrder;
	unsigned int i;
	switch(this->m_epoly_function_type)
	{
	case Standard :
	case Optimized :
		this->lplpCoefficients = (double**)malloc(sizeof(double*));
		this->lplpCoefficients[0] = (double*)malloc((this->m_uiOrder+1)*sizeof(double));
		for(i=0;i<(this->m_uiOrder+1);i++)
		{
			this->lplpCoefficients[0][i] = ppf.lplpCoefficients[0][i];
		};
		break;
	case Interpolatory :
		this->lplpCoefficients = (double**)malloc(2*sizeof(double*));
		this->lplpCoefficients[0] = (double*)malloc((this->m_uiOrder+1)*sizeof(double));
		this->lplpCoefficients[1] = (double*)malloc((this->m_uiOrder+1)*sizeof(double));
		for(i=0;i<(this->m_uiOrder+1);i++)
		{
			this->lplpCoefficients[0][i] = ppf.lplpCoefficients[0][i];
			this->lplpCoefficients[1][i] = ppf.lplpCoefficients[1][i];
		};
		break;
	default :
		this->lplpCoefficients = NULL;
		break;
	};
};


CSplineFunction::CSplineFunction(FUNCTION f) : constant(f)
{
	m_eFunctionType = Polynomial;
	m_epoly_function_type = Idiomatic;
	m_uiOrder = 0;
	lplpCoefficients = NULL;
};


CSplineFunction::~CSplineFunction()
{
	switch(this->m_epoly_function_type)
	{
	case Interpolatory :
		free(this->lplpCoefficients[1]);
	case Optimized :
	case Standard :
		free(this->lplpCoefficients[0]);
		free(this->lplpCoefficients);
	default :
		this->lplpCoefficients = NULL;
		break;
	};
	constant::~constant();
};


algebraic_operator* CSplineFunction::simplify()
{
	algebraic_operator* retVal = NULL;	
	unsigned int i;
	switch(this->m_epoly_function_type)
	{
	case Standard :
//		STANDARD ALGEBRAIC REPRESENTATION OF POLYNOMIALS
//		a + bx + cx^2 + dx^3 + ...
		retVal = &cst(this->lplpCoefficients[0][0]);
		for(i = 1;i < (this->m_uiOrder+1);i++)
		{
			retVal = (*retVal + INT_POW(i,constant(__x))*cst(this->lplpCoefficients[0][i])).Mirror();
		};
		break;
	case Optimized :
//		COMPUTATIONALLY OPTIMIZED POLYNOMIAL FORMS
//		( (((((a)x+b)x+c)x+d)x+e) ...  )
//		for(i = this->m_i_num_coefficients-1;i >= 0;i++)
//		{
//			retVal = point*(retVal + this->lplpCoefficients[0][i]);
//		};
		break;
	case Interpolatory :
//		NEWTON'S INTERPOLATORY DIVIDED-DIFFERENCE POLYNOMIALS
//		f(x0) + f(x0,x1)(x-x0) + f(x0,x1,x2)(x-x0)(x-x1) + ...
//		for(i = 0;i < this->m_i_num_coefficients;i++)
//		{
//			retVal += temp*this->lplpCoefficients[0][i];
//			temp *= (point-this->lplpCoefficients[1][i]);
//		};
		break;
	case Idiomatic :
//		SIMPLY A FUNCTION OF UNKNOWN POLYNOMIAL FORM
//		return (this->m_pv_function)(point);
		break;
	};
	return retVal->simplify();
};


algebraic_operator* CSplineFunction::Mirror()
{
	return new CSplineFunction(*this);	
};


double CSplineFunction::eval(double point,...)
{
	double retVal = 0,temp = 1;
	unsigned int i;
	if (this->m_pAlgebra != NULL)
	{
		point = (*(this->m_pAlgebra))(point);
	};
	switch(this->m_epoly_function_type)
	{
	case Standard :
//		STANDARD ALGEBRAIC REPRESENTATION OF POLYNOMIALS
//		a + bx + cx^2 + dx^3 + ...
		for(i = 0;i < (this->m_uiOrder+1);i++)
		{
			retVal += temp*this->lplpCoefficients[0][i];
			temp *= point;
		};
		break;
	case Optimized :
//		COMPUTATIONALLY OPTIMIZED POLYNOMIAL FORMS
//		( (((((a)x+b)x+c)x+d)x+e) ...  )
		i = this->m_uiOrder+1;
		while(i > 1)
		{
			i--;
			retVal += this->lplpCoefficients[0][i];
			retVal *= point;
		};
		retVal += this->lplpCoefficients[0][0];
		break;
	case Interpolatory :
//		NEWTON'S INTERPOLATORY DIVIDED-DIFFERENCE POLYNOMIALS
//		f(x0) + f(x0,x1)(x-x0) + f(x0,x1,x2)(x-x0)(x-x1) + ...
		for(i = 0;i < (this->m_uiOrder+1);i++)
		{
			retVal += temp*this->lplpCoefficients[0][i];
			temp *= (point-this->lplpCoefficients[1][i]);
		};
		break;
	case Idiomatic :
//		SIMPLY A FUNCTION OF UNKNOWN POLYNOMIAL FORM
		return (this->m_pv_function)(point);
	};
	return retVal;
};


algebraic_operator* CSplineFunction::partial_derivative()
{
	DATA<double,1>* pAi;
	unsigned int i;
	switch(this->m_epoly_function_type)
	{
	case Standard :
//		STANDARD ALGEBRAIC REPRESENTATION OF POLYNOMIALS
//		a + bx + cx^2 + dx^3 + ...
		pAi = AllocateInlinedDataSets<double,1>(this->m_uiOrder);
		for(i = 0;i < this->m_uiOrder;i++)
		{
			pAi[i].m_tDataSet[0] = this->lplpCoefficients[0][i+1];
		};
		this->m_ppartial_derivative = new CSplineFunction(this->m_uiOrder-1,pAi,this->m_pAlgebra,false);
		FreeInlinedDataSets(pAi);
		break;
	case Optimized :
//		COMPUTATIONALLY OPTIMIZED POLYNOMIAL FORMS
//		( (((((a)x+b)x+c)x+d)x+e) ...  )
		pAi = AllocateInlinedDataSets<double,1>(this->m_uiOrder);
		for(i = 0;i < this->m_uiOrder;i++)
		{
			pAi[i].m_tDataSet[0] = this->lplpCoefficients[0][i+1];
		};
		this->m_ppartial_derivative = new CSplineFunction(this->m_uiOrder-1,pAi,this->m_pAlgebra,true);
		FreeInlinedDataSets(pAi);
		break;
	case Interpolatory :
//		NEWTON'S INTERPOLATORY DIVIDED-DIFFERENCE POLYNOMIALS
//		f(x0) + f(x0,x1)(x-x0) + f(x0,x1,x2)(x-x0)(x-x1) + ...
		this->m_ppartial_derivative = _d5pc(*this).Mirror();
		break;
	case Idiomatic :
//		SIMPLY A FUNCTION OF UNKNOWN POLYNOMIAL FORM
		this->m_ppartial_derivative = _d5pc(*this).Mirror();
		break;		
	default :
		this->m_ppartial_derivative = NULL;
	};
	return this->m_ppartial_derivative;
};


algebraic_operator* CSplineFunction::Integral()
{	
	DATA<double,1>* pAi;
	unsigned int i;
	switch(this->m_epoly_function_type)
	{
	case Standard :
//		STANDARD ALGEBRAIC REPRESENTATION OF POLYNOMIALS
//		a + bx + cx^2 + dx^3 + ...
		pAi = AllocateInlinedDataSets<double,1>(this->m_uiOrder+1);
		pAi[0].m_tDataSet[0] = 0;
		for(i = 0;i < (this->m_uiOrder+1);i++)
		{
			pAi[i+1].m_tDataSet[0] = this->lplpCoefficients[0][i];
		};
		this->m_pIntegral = new CSplineFunction(this->m_uiOrder+1,pAi,this->m_pAlgebra,false);
		FreeInlinedDataSets(pAi);
		break;
	case Optimized :
//		COMPUTATIONALLY OPTIMIZED POLYNOMIAL FORMS
//		( (((((a)x+b)x+c)x+d)x+e) ...  )
		pAi = AllocateInlinedDataSets<double,1>(this->m_uiOrder+1);
		pAi[0].m_tDataSet[0] = 0;
		for(i = 0;i < (this->m_uiOrder+1);i++)
		{
			pAi[i+1].m_tDataSet[0] = this->lplpCoefficients[0][i];
		};
		this->m_pIntegral = new CSplineFunction(this->m_uiOrder+1,pAi,this->m_pAlgebra,true);
		FreeInlinedDataSets(pAi);
		break;
	case Interpolatory :
//		NEWTON'S INTERPOLATORY DIVIDED-DIFFERENCE POLYNOMIALS
//		f(x0) + f(x0,x1)(x-x0) + f(x0,x1,x2)(x-x0)(x-x1) + ...
		return NULL;
	case Idiomatic :
//		SIMPLY A FUNCTION OF UNKNOWN POLYNOMIAL FORM
		break;		
	};
	return this->m_pIntegral;
};


void CSplineFunction::to_string(char* pBuffer)
{
	strcpy(pBuffer,"");
};


byte_type* CSplineFunction::to_IA32_binary(byte_type *lpInstructionField,unsigned int uiCurrentDepht,unsigned int uiFLSDepht)
{
	return lpInstructionField;
};


void CSplineFunction::annotate(COMPILE_TIME_INFO* pCompileTimeInfo)
{

};*/