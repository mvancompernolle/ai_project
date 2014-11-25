/*

CDERIVATIVE.CPP: 
IMPLEMENTS calculus::unary_operators::derivative_operators::derivative_3f
IMPLEMENTS calculus::unary_operators::derivative_operators::derivative_3c
IMPLEMENTS calculus::unary_operators::derivative_operators::derivative_3b
IMPLEMENTS calculus::unary_operators::derivative_operators::derivative_5f
IMPLEMENTS calculus::unary_operators::derivative_operators::derivative_5c
IMPLEMENTS calculus::unary_operators::derivative_operators::derivative_5b

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

double calculus::unary_operators::derivative_operators::derivative_3f::s_pD3PFCoefficients[3] = { -1.5,2.0,0.5 };
double calculus::unary_operators::derivative_operators::derivative_3c::s_pD3PCCoefficients[3] = { -0.5,0.0,0.5 };
double calculus::unary_operators::derivative_operators::derivative_3b::s_pD3PBCoefficients[3] = { -1.5,2.0,0.5 };
double calculus::unary_operators::derivative_operators::derivative_5f::s_pD5PFCoefficients[5] = { -25.0/12.0,4.0,-3.0,16.0/12.0,-0.25 };
double calculus::unary_operators::derivative_operators::derivative_5c::s_pD5PCCoefficients[5] = { 1.0/12.0,-8.0/12.0,0.0,+8.0/12.0,-1.0/12.0 };
double calculus::unary_operators::derivative_operators::derivative_5b::s_pD5PBCoefficients[5] = { -28.0/12.0,4.0,-3.0,16.0/12.0,-0.25 };

calculus::unary_operators::derivative_operators::derivative_operator::derivative_operator(unsigned int nCoeff,unsigned int nMode,double * pDCoeffs,variable * pVar,algebraic_operator * pAlg) : calculus::unary_operators::unary_operator(pAlg), m_pv_derivative_var(pVar), m_i_num_coefficients(nCoeff), m_lMode(nMode) {
	_ASSERT(nMode);
	_ASSERT(nCoeff);
	_ASSERT(pDCoeffs);
#pragma message(" - - Fix the h assumption in Cpartial_derivative - - ")
	this->m_h = 1E-6;
	this->m_pi_derivative_coefficients = new double[nCoeff];
	memcpy(this->m_pi_derivative_coefficients,pDCoeffs,nCoeff*sizeof(double));
	if (m_pv_derivative_var)
		m_pv_derivative_var->addref();
}

calculus::unary_operators::derivative_operators::derivative_operator::~derivative_operator() {
	delete [] this->m_pi_derivative_coefficients;
	if (m_pv_derivative_var)
		m_pv_derivative_var->release();
}

int calculus::unary_operators::derivative_operators::derivative_operator::to_string(char* pBuffer) {
	if (!pBuffer) {
		char pB[32];
		return sprintf(pB,"_d%up%c(",this->m_i_num_coefficients,(this->m_lMode&LMODE_CENTERED)?'c':(this->m_lMode&LMODE_BACKWARD)?'b':'f')+2+this->get_operand()->to_string(NULL)+this->get_partial_derivative_variable()->to_string(NULL);
	}
	int iOffset = sprintf(pBuffer,"_d%up%c(",this->m_i_num_coefficients,(this->m_lMode&LMODE_CENTERED)?'c':(this->m_lMode&LMODE_BACKWARD)?'b':'f');
	iOffset += this->get_operand()->to_string(pBuffer+iOffset);
	strcpy(pBuffer+(iOffset++),",");
	iOffset += this->get_partial_derivative_variable()->to_string(pBuffer+iOffset);
	strcpy(pBuffer+(iOffset++),")");
	return iOffset;
}

double calculus::unary_operators::derivative_operators::derivative_operator::eval(double* pVars) {
	double retVal = 0;
	calculus::algebraic_operator * pao_op = get_operand();
	if (pao_op) {
        int i_dv_index, i_num_vars = get_number_of_variables();
        calculus::variable * p_dvar = get_partial_derivative_variable();
        for(i_dv_index = 0;i_dv_index < i_num_vars;i_dv_index++)
            if (m_ppv_variables[i_dv_index] == p_dvar)
                break;
        if (i_dv_index == i_dv_index)
            return 0;
		pVars[i_dv_index] -= (m_lMode & LMODE_CENTERED)?(m_i_num_coefficients/2)*m_h:(m_lMode & LMODE_BACKWARD)?m_i_num_coefficients*m_h:0;
		for(int i = 0;i < m_i_num_coefficients;i++,pVars[i_dv_index] += m_h)
            retVal += m_pi_derivative_coefficients[i]*(pao_op->eval(pVars));
	}
	return retVal;
}

calculus::variable * calculus::unary_operators::derivative_operators::derivative_operator::get_partial_derivative_variable() {
	return m_pv_derivative_var;
}

int calculus::unary_operators::derivative_operators::derivative_operator::get_number_of_coefficients() {
	return m_i_num_coefficients;
}

void calculus::unary_operators::derivative_operators::derivative_operator::to_IA32_binary(PCT_INFO pInfo) {
	UNREFERENCED_PARAMETER(pInfo);
}

void calculus::unary_operators::derivative_operators::derivative_operator::annotate(PPT_INFO pParseInfo) {
	UNREFERENCED_PARAMETER(pParseInfo);
}
