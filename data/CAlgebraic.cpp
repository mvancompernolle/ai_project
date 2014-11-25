/*

CALGEBRAIC.CPP: IMPLEMENTS calculus::algebraic_operator

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

#ifdef _MSC_VER
#include <crtdbg.h>
#endif
#include "Calculus_cpp.h"
#include <limits.h>

unsigned int calculus::algebraic_operator::s_ui_compilation_deferral = UINT_MAX;
calculus::IA32_binary* calculus::IA32_binary::s_pia32b_first = NULL;
calculus::IA32_binary* calculus::IA32_binary::s_pia32b_last = NULL;
unsigned short calculus::algebraic_operator::s_us_compile_flags = COMPILER_ADAPT;

calculus::algebraic_operator::algebraic_operator() {
    m_b_variables_identified = false;
	m_ul_refcount = 0;
	m_ui_call_count = 0;
    m_pia32_binary = NULL;
	m_i_number_of_variables = 0;
	m_ppao_partial_derivatives = NULL;
	m_i_number_of_variables = 0;
	m_ppv_variables = NULL;
}

calculus::algebraic_operator::~algebraic_operator() {
	if (m_ppao_partial_derivatives != NULL) {
		unsigned int num_vars = get_number_of_variables();
		for(;num_vars;) {
			num_vars--;
			if (m_ppao_partial_derivatives[num_vars])
				m_ppao_partial_derivatives[num_vars]->release();
		}
		delete [] m_ppao_partial_derivatives;
		this->m_ppao_partial_derivatives = NULL;
	}

	if (m_ppv_variables != NULL) {
		mass_release<calculus::variable>(m_ppv_variables,m_i_number_of_variables);
		delete [] m_ppv_variables;
	}
	this->m_ppv_variables = NULL;
}

double calculus::algebraic_operator::eval(double* pVars) {
	UNREFERENCED_PARAMETER(pVars);
	_ASSERT(NULL); //YOU MUST OVERRIDE THIS IN YOUR DERIVED CLASS
	return 0;
}

FUNCTION calculus::algebraic_operator::compile() {
	if (m_pia32_binary == NULL)
		m_pia32_binary = to_IA32_binary();
	return (FUNCTION)(m_pia32_binary->m_pus_binary);
};

int calculus::algebraic_operator::to_string(char* pBuffer) {
	if (!pBuffer)
		return 6;
	return sprintf(pBuffer,"#ERROR");
}

calculus::algebraic_operator* calculus::algebraic_operator::get_partial_derivative(calculus::variable * pVar) {
	//IF IT'S NOT A FUNCTION OF THAT VARIABLE THEN THERE IS NO POINT IN GOING FORWARD
	if (!is_function_of(pVar))
		return calculus::_cst(0);
	variable ** ppv_vars = get_variables();
	int num_vars = this->get_number_of_variables();
	//LOCATE THE ENTRY IN THE VARS ARRAY TO RELATE TO THE DERIVATIVE ARRAY
    int i;
    for(i = 0;i < num_vars;i++)
		if (ppv_vars[i] == pVar)
            break;
	//IF DERIVATIVE ARRAY IS NULL
	if (m_ppao_partial_derivatives == NULL) {
		//ALLOCATE A NEW ARRAY
		m_ppao_partial_derivatives = new algebraic_operator*[num_vars];
		//ZERO MEMORY OF NEW ARRAY
        for(int i = 0;i < num_vars;i++)
            m_ppao_partial_derivatives[i] = NULL;
	}
    if (m_ppao_partial_derivatives[i] == NULL)
        (m_ppao_partial_derivatives[i] = partial_derivative(pVar))->addref();
	return m_ppao_partial_derivatives[i];
}

void calculus::algebraic_operator::set_partial_derivative(calculus::variable * pVar,calculus::algebraic_operator * ppartial_derivative) {
	//IF IT'S NOT A FUNCTION OF THAT VARIABLE THEN THERE IS NO POINT IN GOING FORWARD
	if (!is_function_of(pVar))
		return;
	//LOCATE THE ENTRY IN THE VARS ARRAY TO RELATE TO THE DERIVATIVE ARRAY
	variable ** ppv_vars = get_variables();
	int num_vars = get_number_of_variables();
    int i;
	for(i = 0;i < num_vars;i++)
		if (ppv_vars[i] == pVar)
			break;
	//IF DERIVATIVE ARRAY IS NULL
	if (m_ppao_partial_derivatives == NULL) {
		//ALLOCATE A NEW ARRAY
		m_ppao_partial_derivatives = new algebraic_operator*[num_vars];
		//ZERO MEMORY OF NEW ARRAY
		for(int i = 0;i < num_vars;i++)
			m_ppao_partial_derivatives[i] = NULL;
	}
	(m_ppao_partial_derivatives[i] = ppartial_derivative)->addref();
}

calculus::algebraic_operator* calculus::algebraic_operator::create_copy() {
	_ASSERT(NULL);
	return NULL;
}

calculus::algebraic_operator* calculus::algebraic_operator::partial_derivative(calculus::variable * pVar) {
	return calculus::unary_operators::derivative_operators::__d5pc(pVar,this->create_copy());
}

void calculus::algebraic_operator::annotate(PPT_INFO pparse_info)
//YOU MUST OVERRIDE THIS VIRTUAL MEMBER FUNCTION IN YOUR DERIVED CLASS IN ORDER TO SUPPORT THE RUN-TIME COMPILER
{
	UNREFERENCED_PARAMETER(pparse_info);
	_ASSERT(0);
};

void calculus::algebraic_operator::to_IA32_binary(PCT_INFO pInfo)
//YOU MUST OVERRIDE THIS VIRTUAL MEMBER FUNCTION IN YOUR DERIVED CLASS IN ORDER TO SUPPORT THE RUN-TIME COMPILER
{
	UNREFERENCED_PARAMETER(pInfo);
	_ASSERT(0);
};

//#define INSERT_BREAK

#pragma warning(disable: 4189)

calculus::IA32_binary* calculus::algebraic_operator::to_IA32_binary()
{
	char * sc_name_buffer = new char[this->to_string(NULL)];
	this->to_string(sc_name_buffer);

	PT_INFO parse_info;
	parse_info.st_size = sizeof(parse_info);
	parse_info.i_aux_features_needed = FEAT_AUX_NEED_NONE;
	parse_info.i_features_needed = FEAT_NEED_EAX;
	parse_info.st_global_storage_size = 0;
	parse_info.i_instruction_count = 0;
	parse_info.st_instruction_storage_size = 0;
	parse_info.st_local_storage_size = 0;
	parse_info.st_pmap_size = 1;	//INC dword_type PTR
	parse_info.i_clock_count = 0;
	parse_info.i_operator_count = 0;
	parse_info.i_fpu_stack_offset = 0;
	
	annotate(&parse_info);

	dword_type InstructionLengthCheck = parse_info.st_instruction_storage_size;
	parse_info.i_instruction_count  +=
#ifdef INSERT_BREAK
										  1
#endif
										+ (2)
										* (4 
										+ (parse_info.i_features_needed & FEAT_NEED_EAX)?1:0
										+ (parse_info.i_features_needed & FEAT_NEED_EBX)?1:0
										+ (parse_info.i_features_needed & FEAT_NEED_ECX)?1:0
										+ (parse_info.i_features_needed & FEAT_NEED_EDX)?1:0);
	parse_info.st_instruction_storage_size +=
#ifdef INSERT_BREAK
										  CompilerSizeOfBREAK()+
#endif
										  2*CompilerSizeOfMOV_EXX_EYX()
										+ (CompilerSizeOfPOP_EXX()+ CompilerSizeOfPUSH_EXX())
										* (1 
										+ ((parse_info.i_features_needed & FEAT_NEED_EAX)?1:0)
										+ ((parse_info.i_features_needed & FEAT_NEED_EBX)?1:0)
										+ ((parse_info.i_features_needed & FEAT_NEED_ECX)?1:0)
										+ ((parse_info.i_features_needed & FEAT_NEED_EDX)?1:0))
										+ CompilerSizeOfMOV_EXX_IMM32()
										+ sizeof(dword_type)
										+ CompilerSizeOfINC_dword_typePTREXX()
										+ CompilerSizeOfFCLEX()
										+ CompilerSizeOfRET();
	dword_type dwMemRequired = parse_info.st_global_storage_size
						+ parse_info.st_instruction_storage_size
						+ parse_info.st_local_storage_size
						+ parse_info.st_pmap_size*sizeof(dword_type*)
						+ (parse_info.i_aux_features_needed)*AUX_FEATURE_SIZE
						+ sizeof(COMPILER_HEADER) + (strlen(sc_name_buffer)+1)*sizeof(char) + sizeof(void*);

	PCOMPILER_HEADER pHead = (PCOMPILER_HEADER)malloc(dwMemRequired);

	pHead->st_size					= sizeof(COMPILER_HEADER);
	pHead->pSelf					= pHead;
	pHead->i_f_flags					= COMPILER_FLAG_NOFLAGS;
	pHead->i_call_count				= this->m_ui_call_count;
	pHead->i_clock_count				= parse_info.i_clock_count;
	pHead->i_operator_count			= parse_info.i_operator_count;
	pHead->i_instruction_count		= parse_info.i_instruction_count;
	pHead->i_num_vars				= this->get_number_of_variables();
	pHead->st_mem_size				= dwMemRequired;
	pHead->psc_name					= ((unsigned char*)pHead)+(sizeof(COMPILER_HEADER)/sizeof(unsigned char));
	strcpy((char*)pHead->psc_name,sc_name_buffer);
	pHead->ppv_pmap					= (unsigned char**)(pHead->psc_name + strlen((char*)pHead->psc_name) + 1);
	pHead->pv_global_storage	    = (unsigned char*)(pHead->ppv_pmap + parse_info.st_pmap_size);
	pHead->pv_local_storage			=  pHead->pv_global_storage + parse_info.st_global_storage_size;
	pHead->ppv_auxiliary_storage_toc	= (unsigned char**)(pHead->pv_local_storage + parse_info.st_local_storage_size);
	pHead->pv_auxiliary_storage		= (unsigned char*)(pHead->ppv_auxiliary_storage_toc + AUX_FEATURE_TABLE_SIZE);
	*((void**)(pHead->pv_auxiliary_storage + AUX_FEATURE_SIZE)) = pHead;
	pHead->pInstructions			= (FUNCTION)(pHead->pv_auxiliary_storage + AUX_FEATURE_SIZE + sizeof(void*));

	CT_INFO info;	PCT_INFO pInfo = &info;
	pInfo->pHeader				= pHead;
	pInfo->pv_instruction_storage_pos = (byte_type*)(pHead->pInstructions);
	pInfo->pv_local_storage_pos		= pHead->pv_local_storage;
	pInfo->ppv_pmapPos			= pHead->ppv_pmap;
	pInfo->pv_global_storage_pos		= pHead->pv_global_storage;
	pInfo->pv_aux_storage_pos			= pHead->pv_auxiliary_storage;
	pInfo->ppv_vars				= this->m_ppv_variables;
	pInfo->i_stack_offset		= 0;
	//BEGIN OUTPUT TO OPCODE STREAM
#ifdef INSERT_BREAK
	CompilerWriteBREAK(pInfo);
#endif
	//OUTPUT CALL COUNT INCREMENTING PROCEDURE
	CompilerWritePUSH_EXX(pInfo,REG_EBP);
	pInfo->i_stack_offset -= sizeof(dword_type);				//THIS IS A HACK FOR THE FALSE STACK OFFSET REGISTERED WHEN PUSHING EBP
	CompilerWriteMOV_EXX_EYX(pInfo,REG_EBP,REG_ESP);
	if (parse_info.i_features_needed & FEAT_NEED_EAX)
		CompilerWritePUSH_EXX(pInfo,REG_EAX);
	if (parse_info.i_features_needed & FEAT_NEED_EBX)
		CompilerWritePUSH_EXX(pInfo,REG_EBX);
	if (parse_info.i_features_needed & FEAT_NEED_ECX)
		CompilerWritePUSH_EXX(pInfo,REG_ECX);
	if (parse_info.i_features_needed & FEAT_NEED_EDX)
		CompilerWritePUSH_EXX(pInfo,REG_EDX);
	CompilerWriteMOV_EXX_IMM32(pInfo,REG_EAX);
		CompilerWritePTR_ENTRY(pInfo,(unsigned char*)pInfo->pv_instruction_storage_pos);
		CompilerWriteIMM32(pInfo,(dword_type)&(pHead->i_call_count));
	CompilerWriteINC_dword_typePTREXX(pInfo,REG_EAX);
	CompilerWriteFCLEX(pInfo);
	//RECURSE OUTPUT TO OPCODES
	byte_type * pPreEncodePos = pInfo->pv_instruction_storage_pos;
	
    to_IA32_binary(pInfo);
	
    //CHECK IF THE REQUESTED SIZE DIDN'T MATCH THE USED SIZE
    _ASSERT(pPreEncodePos+InstructionLengthCheck == pInfo->pv_instruction_storage_pos);
	if (parse_info.i_features_needed & FEAT_NEED_EAX)
		CompilerWritePOP_EXX(pInfo,REG_EAX);
	if (parse_info.i_features_needed & FEAT_NEED_EBX)
		CompilerWritePOP_EXX(pInfo,REG_EBX);
	if (parse_info.i_features_needed & FEAT_NEED_ECX)
		CompilerWritePOP_EXX(pInfo,REG_ECX);
	if (parse_info.i_features_needed & FEAT_NEED_EDX)
		CompilerWritePOP_EXX(pInfo,REG_EDX);
	CompilerWriteMOV_EXX_EYX(pInfo,REG_ESP,REG_EBP);
	pInfo->i_stack_offset += sizeof(dword_type);//THIS IS A HACK FOR THE FALSE STACK OFFSET REGISTERED WHEN PUSHING EBP
	CompilerWritePOP_EXX(pInfo,REG_EBP);
	CompilerWriteRET(pInfo);

	_ASSERT(pInfo->i_stack_offset == 0);													//THERE WAS A STACK LEAK
	_ASSERT((byte_type*)pInfo->ppv_pmapPos == (byte_type*)pHead->pv_global_storage);					//THERE WERE MORE PTR ENTRIES THEN PLANNED
	_ASSERT((byte_type*)pInfo->pv_global_storage_pos == (byte_type*)pHead->pv_local_storage);				//TOO MUCH GLOBAL INFO WAS WRITTEN
	_ASSERT((byte_type*)pInfo->pv_local_storage_pos == (byte_type*)pHead->ppv_auxiliary_storage_toc);		//TOO MUCH LOCAL INFO WAS WRITTEN
	_ASSERT((byte_type*)pInfo->pv_aux_storage_pos == (byte_type*)pHead->pInstructions - sizeof(dword_type));	//TOO MUCH AUX INFO WAS WRITTEN
	byte_type * pEND = ((byte_type*)pHead->pInstructions)+parse_info.st_instruction_storage_size;
	_ASSERT(pEND == (byte_type*)pHead + pHead->st_mem_size);
	_ASSERT(pInfo->pv_instruction_storage_pos == pEND);							//THE ALLOCATED SIZE DIDN'T MATCH THE USED SIZE

    delete [] sc_name_buffer;

	return (new IA32_binary((unsigned short*)(pHead->pInstructions)));
};

bool calculus::algebraic_operator::is_function_of(calculus::variable* a) {
	variable ** ppv_vars = get_variables();
	int i = get_number_of_variables();
    while(i)
		if (ppv_vars[--i] == a)
            return true;
	return false;
}

calculus::variable** calculus::algebraic_operator::identify_variables() {
	_ASSERT(0);
    return NULL;
}

calculus::variable** calculus::algebraic_operator::get_variables() {
	return (m_ppv_variables == NULL)?identify_variables():m_ppv_variables;
}

int calculus::algebraic_operator::get_number_of_variables() {
	if (!m_b_variables_identified)
		identify_variables();
	return m_i_number_of_variables;
}

unsigned int calculus::algebraic_operator::get_call_count() {
	return m_ui_call_count;
}
