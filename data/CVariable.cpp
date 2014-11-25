/*

CVARIABLE.CPP: IMPLEMENTS calculus::variable

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

unsigned int calculus::variable::uiNextAutoVarName = 0;
unsigned int calculus::variable::nums[27] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
calculus::variable ** calculus::variable::ppVars[27] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void calculus::variable::FreeRegistry() {
	for(unsigned int uiCatNum = 0;uiCatNum < 27;uiCatNum++) {
		if (nums[uiCatNum]) {
			for(unsigned int ui = nums[uiCatNum];ui;) {
				ui--;
				ppVars[uiCatNum][ui]->release();
			}
			free(ppVars[uiCatNum]);
		}
	}
}

calculus::algebraic_operator * calculus::variable::create(const char * psc_name) { 
	if (strlen(psc_name) > (MAX_VARIABLE_NAME_LENGTH-1))
		return NULL;
	int iCatNum = toupper(*psc_name)-'A';
    if ((iCatNum < 0)||(iCatNum > 26))
        iCatNum = 26;
	if (nums[iCatNum]) {
		for(unsigned int ui = nums[iCatNum];ui;) {
			ui--;
			if (!strcmp(ppVars[iCatNum][ui]->get_variable_name(),psc_name))
				return ppVars[iCatNum][ui];
		}
	}
	nums[iCatNum]++;
	if (ppVars[iCatNum]) {
		ppVars[iCatNum] = (calculus::variable**)realloc(ppVars[iCatNum],nums[iCatNum]*sizeof(calculus::variable*));
	}
	else {
		ppVars[iCatNum] = (calculus::variable**)malloc(sizeof(calculus::variable*));
	}
	ppVars[iCatNum][nums[iCatNum]-1] = new calculus::variable(psc_name);
	ppVars[iCatNum][nums[iCatNum]-1]->addref();
	return ppVars[iCatNum][nums[iCatNum]-1];
}

calculus::variable::variable(const char *pVarName) : calculus::algebraic_operator() {
	pThis = this;
	m_ppv_variables = &pThis;
	m_i_number_of_variables = 1;
	::strcpy(this->m_scVarName,pVarName);
    m_b_variables_identified = true;
    m_i_number_of_variables = 1;
}

int calculus::variable::to_string(char* pBuffer) {
	if (!pBuffer)
		return strlen(this->m_scVarName);
	return sprintf(pBuffer,this->m_scVarName);
}

double calculus::variable::eval(double* pVars) {
	return *pVars;
}

void calculus::variable::to_IA32_binary(PCT_INFO pInfo) {	
    unsigned short varNumber;
	for(varNumber = 0;varNumber < pInfo->pHeader->i_num_vars;varNumber++) {
        if (pInfo->ppv_vars[varNumber] == this)
			break;
	}
	if (pInfo->ppv_vars[varNumber] == this) {
		CompilerWriteFLD_EBPX_IMM32(pInfo);
		CompilerWriteIMM32(pInfo,(dword_type)COMPILER_INFO_V(varNumber));
	}
	else _ASSERT(0);	//THIS IS AN ILLEGAL PROGRAM STATE
}

void calculus::variable::annotate(PPT_INFO pParseInfo) {
	pParseInfo->st_instruction_storage_size += CompilerSizeOfFLD_EBPX_IMM32() + sizeof(dword_type);
	pParseInfo->i_operator_count++;
	pParseInfo->i_instruction_count++;
}

calculus::algebraic_operator* calculus::variable::partial_derivative(calculus::variable * pVar) {
	return (pVar==this)?calculus::_cst(1):calculus::_cst(0);
}

calculus::variable** calculus::variable::identify_variables() {
	return this->m_ppv_variables;
}

int calculus::variable::get_number_of_variables() {
	return 1;
}

bool calculus::variable::is_function_of(calculus::variable* a) {
	return (a == this);
}
