/*

CBINARYOPERATOR.CPP: 
IMPLEMENTS calculus::binary_operators::binary_operator

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

bool calculus::binary_operators::binary_operator::UseConstantOptimizations = true;
bool calculus::binary_operators::binary_operator::UseDisorderedOptimizations = true;
bool calculus::binary_operators::binary_operator::UseFlipOptimizations = true;

double calculus::binary_operators::binary_operator::eval(double* pVars) {
	double a = 0,b = 0;
	if (m_pao_left_operand) {
		int i_num_left_vars = m_pao_left_operand->get_number_of_variables();
		calculus::variable** ppv_left_vars = m_pao_left_operand->get_variables();
		double* pd_left_vars = new double[i_num_left_vars];
		for(int i = 0;i < i_num_left_vars;i++) {
            int j;
			for(j = 0;j < m_i_number_of_variables;j++)
				if (m_ppv_variables[j] == ppv_left_vars[i])
					break;
			if (m_ppv_variables[j] == ppv_left_vars[i]) {
				pd_left_vars[i] = pVars[j];
			}
			else pd_left_vars[i] = 0;
		}
        a = m_pao_left_operand->eval(pd_left_vars);
		delete [] pd_left_vars;
	}
	if (m_pao_right_operand) {
		int i_num_right_vars = m_pao_right_operand->get_number_of_variables();
		calculus::variable** ppv_right_vars = m_pao_right_operand->get_variables();
		double* pd_right_vars = new double[i_num_right_vars];
		for(int i = 0;i < i_num_right_vars;i++) {
            int j;
			for(j = 0;j < m_i_number_of_variables;j++) {
				if (m_ppv_variables[j] == ppv_right_vars[i])
					break;
			}
			if (m_ppv_variables[j] == ppv_right_vars[i]) {
				pd_right_vars[i] = pVars[j];
			}
			else pd_right_vars[i] = 0;
		}
        b = m_pao_right_operand->eval(pd_right_vars);
		delete [] pd_right_vars;
	}
	return eval_binary(a,b);
}

calculus::variable** calculus::binary_operators::binary_operator::identify_variables() {
	unsigned int numLeftVars = m_pao_left_operand->get_number_of_variables();
	unsigned int numRightVars = m_pao_right_operand->get_number_of_variables();
	m_i_number_of_variables = numLeftVars+numRightVars;
	if (m_ppv_variables)
		delete [] m_ppv_variables;
    
    if (!m_i_number_of_variables)
        return m_ppv_variables = NULL;

	m_ppv_variables = new calculus::variable*[m_i_number_of_variables+1];

	calculus::variable** ppv_left_vars = m_pao_left_operand->get_variables();
	calculus::variable** ppv_right_vars = m_pao_right_operand->get_variables();
	
    { for(unsigned int i = 0;i < numLeftVars;i++)
        m_ppv_variables[i] = ppv_left_vars[i]; }
    { for(unsigned int i = 0;i < numRightVars;i++)
        m_ppv_variables[i+numLeftVars] = ppv_right_vars[i]; }


    //LETS PRUNE THE LIST TO REMOVE DUPLICATES
    for(int i = 0;i < m_i_number_of_variables;i++) {
        //IF THIS REFERENCE IS NULL, THEN LEAVE
        if (!m_ppv_variables[i])
            break;
        //THIS WILL PUT ALL LIKE REFERENCES TO NULL
        for(int j = i+1;j < m_i_number_of_variables;j++)
            if (m_ppv_variables[j])
                if (!strcmp(m_ppv_variables[i]->get_variable_name(),m_ppv_variables[j]->get_variable_name()))
                    m_ppv_variables[j] = NULL;
        //THIS WILL COMPRESS THE ARRAY
        int i_next_var = i+1;
        { for(int j = i+1;j < m_i_number_of_variables;j++) {
            //FIND THE NEXT NON-NULL VARIABLE REFERENCE
            for(;i_next_var < m_i_number_of_variables;i_next_var++)
                if (m_ppv_variables[i_next_var])
                    break;
            //IF WE REACHED THE END OF THE LIST THEN LETS DEPART
            if (i_next_var == m_i_number_of_variables)
                break;
            //SET THE CURRENT VAR TO THE NEXT-VAR
            m_ppv_variables[j] = m_ppv_variables[i_next_var++];
        } }
    }
    //WE NEED TO COUNT THE NUMBER OF NON-NULL VARIABLES AT THIS POINT
    {int i;
    for(i = 0;i < m_i_number_of_variables;i++)
        if (!m_ppv_variables[i])
            break;
    m_i_number_of_variables = i;}
    //LETS SORT THE ARRAY NOW
    { for(int i = 0;i < m_i_number_of_variables-1;i++)
    {
        //CURRENTLY LOOKING FOR I-TH CANDIDATE
        int i_candidate = i;
        for(int j = i+1;j < m_i_number_of_variables;j++)
            if (strcmp(m_ppv_variables[i]->get_variable_name(),m_ppv_variables[j]->get_variable_name()) > 0)
                i_candidate = j;
        calculus::variable* temp = m_ppv_variables[i];
        m_ppv_variables[i] = m_ppv_variables[i_candidate];
        m_ppv_variables[i_candidate] = temp;
    } }
	mass_addref<calculus::variable>(m_ppv_variables,m_i_number_of_variables);
    m_b_variables_identified = true;
	return m_ppv_variables;
}
