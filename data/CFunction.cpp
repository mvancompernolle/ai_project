/*

CFUNCTION.CPP: IMPLEMENTS calculus::function_adapter

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

calculus::function_adapter::function_adapter(void * p_function_adapter,char * psc_function_adapter_name,int i_num_vars,variable **ppVars) : algebraic_operator() {
	m_pv_function_adapter = p_function_adapter;
	strcpy(m_sc_function_adapter_name,psc_function_adapter_name);
	mass_addref<variable>(ppVars,i_num_vars);
	this->m_i_number_of_variables = i_num_vars;
	if (i_num_vars) {
		_ASSERT(ppVars);
		this->m_ppv_variables = new variable*[i_num_vars];
		i_num_vars--;
		this->m_ppv_variables[i_num_vars] = ppVars[i_num_vars];
	}
	while(i_num_vars) {
		i_num_vars--;
		this->m_ppv_variables[i_num_vars] = ppVars[i_num_vars];
	}
}

calculus::algebraic_operator * calculus::function_adapter::create(void * pv_function_adapter,char * psc_function_adapter_name,int i_num_vars,calculus::variable ** ppv_vars) {
	return new calculus::function_adapter(pv_function_adapter,psc_function_adapter_name,i_num_vars,ppv_vars);
}

void calculus::function_adapter::to_IA32_binary(PCT_INFO pInfo) {
	int i = m_i_number_of_variables;
	while(--i >= 0) {
        int j;
		for(j = 0;j < pInfo->pHeader->i_num_vars;j++) {
			if (pInfo->ppv_vars[j] == m_ppv_variables[i]) {
#pragma warning(disable:4244)
				CompilerWriteFLD_EBPX_IMM32(pInfo);
				CompilerWriteIMM32(pInfo,COMPILER_INFO_V(j));
				CompilerWriteFSTP_EBPX_IMM32(pInfo);
				CompilerWriteIMM32(pInfo,pInfo->i_stack_offset+sizeof(double)*(this->m_i_number_of_variables-i-1));
				break;
			}
		}
		if (j == pInfo->pHeader->i_num_vars)
			_ASSERT(NULL); //ILLEGAL STATE
	}
	CompilerWriteSUB_EXX_IMM32(pInfo,REG_ESP);
	CompilerWriteIMM32(pInfo,0x8u*(this->m_i_number_of_variables-1));
	CompilerWriteMOV_EXX_IMM32(pInfo,REG_EAX);
	CompilerWriteIMM32(pInfo,(dword_type)m_pv_function_adapter);
	CompilerWriteCALL_EXX(pInfo,REG_EAX);
	pInfo->pHeader->i_f_flags |= COMPILER_FLAG_FUNCTION_NOT_REMOTABLE;
}

void calculus::function_adapter::annotate(PPT_INFO pParseInfo) {
	pParseInfo->i_instruction_count += this->m_i_number_of_variables*2 + 3;
	pParseInfo->st_instruction_storage_size += this->m_i_number_of_variables*(
												  CompilerSizeOfFLD_EBPX_IMM32() 
												+ CompilerSizeOfFSTP_EBPX_IMM32()
												+ 2*sizeof(dword_type)) 
										 +	CompilerSizeOfMOV_EXX_IMM32()
										 +  CompilerSizeOfSUB_EXX_IMM32()
										 +	CompilerSizeOfCALL_EXX()
										 +  2*sizeof(dword_type);
	pParseInfo->i_operator_count++;
}

double calculus::function_adapter::eval(double *pVars) {
    return ((REAL_FUNCTION)m_pv_function_adapter)(pVars);
}

int calculus::function_adapter::to_string(char* pBuffer) {
	if (!pBuffer)
		return 1024;
	int iOffset = sprintf(pBuffer,"%s[0x%X](",m_sc_function_adapter_name,(unsigned long)this->m_pv_function_adapter);
	for(int i = 0;i < m_i_number_of_variables;i++) {
		iOffset += this->m_ppv_variables[i]->to_string(pBuffer+iOffset);
		if (i < m_i_number_of_variables-1)
			strcpy(pBuffer+(iOffset++),",");
	}
	strcpy(pBuffer+(iOffset++),")");
	return iOffset;
}

calculus::variable** calculus::function_adapter::identify_variables() {
	return m_ppv_variables;
}

int calculus::function_adapter::get_number_of_variables() {
	return m_i_number_of_variables;
}

calculus::algebraic_operator* calculus::function_adapter::partial_derivative(calculus::variable * pVar) {
	//IF IT'S NOT A function_adapter OF THAT VARIABLE THEN THERE IS NO POINT IN GOING FORWARD
	if (!this->is_function_of(pVar))
		return calculus::_cst(0);
	return calculus::unary_operators::derivative_operators::__d5pc(pVar,function_adapter::create(this->m_pv_function_adapter,this->m_sc_function_adapter_name,this->m_i_number_of_variables,this->m_ppv_variables));
}
