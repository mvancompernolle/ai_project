/*

CLIENTAPP.CPP: SAMPLE OF USAGE FOR CALCULUS-CPP

This file includes <calculus.h> and demonstrates the use of most of the
interesting functionality the package is capable of.  It implements
a method for n-dimensional minimization using a simplified version
the Newton-Raphson minimization method as a sample of a functionally-
enhanced algorithm.  You may use this method to solve FEA problems 
using energy formulations in pure quadratic form by way of the TNT
or any other templated matrix toolkit.

Use this file as a springboard to get you started using calculus-cpp.

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

#include <iostream>
#include <stdio.h>
#include <math.h>
#include <Calculus.h>

#define MATH_UTILS_H
#include "tnt/tnt.h"

using namespace std;
using namespace TNT;

#pragma warning(disable:4239)

void gradmin(
           Function f, 
           double * p, 
           double tolerance, 
           double & value, 
           unsigned int & iterations
           );

void set_up_report_info(
                        int num_vars,
                        Function f,
                        Function * gradient
                        );

void report_info(
                 int num_vars,
                 int iterations,
                 double value,
                 double * p,
                 double * grad_value,
                 double * grad_div_value,
                 double * delta,
                 double delta_value
                 );

int main()
{
//THIS IS ALWAYS HANDY...
//	_CrtSetBreakAlloc(226);

    //INITIALIZATION OF THE CALCULUS LAYER
	initialize_calculus(0);

    //DECLARATIONS FOR THE VARIABLES USED IN THE SAMPLES
    Variable x = "x",y = "y",z = "z",w = "w";

	//SIMPLE DEMO FIRST... LETS JUST DECLARE A FEW FUNCTIONS
	char * pBuffer;

	Function t = INT_POW(6,x)*y*z+y*z+x*z;
    t->to_string(pBuffer = new char[t->to_string(NULL)+1]);
	printf("t : %s\n\n",pBuffer);
    delete [] pBuffer;

    Function dtdx = t->get_partial_derivative(x);
    dtdx->to_string(pBuffer = new char[dtdx->to_string(NULL)+1]);
	printf("dtdx : %s\n\n",pBuffer);
    delete [] pBuffer;

	Function ddtdxdx = dtdx->get_partial_derivative(x);
    ddtdxdx->to_string(pBuffer = new char[ddtdxdx->to_string(NULL)+1]);
	printf("ddtdxdx : %s\n\n",pBuffer);
    delete [] pBuffer;

	Function dddtdxdxdx = ddtdxdx->get_partial_derivative(x);
    dddtdxdxdx->to_string(pBuffer = new char[dddtdxdxdx->to_string(NULL)+1]);
	printf("dddtdxdxdx : %s\n\n",pBuffer);
    delete [] pBuffer;

	Function ddddtdxdxdxdx = dddtdxdxdx->get_partial_derivative(x);
    ddddtdxdxdxdx->to_string(pBuffer = new char[ddddtdxdxdxdx->to_string(NULL)+1]);
	printf("ddddtdxdxdxdx : %s\n\n",pBuffer);
    delete [] pBuffer;
	//MORE COMPLICATED TEST... AN ENGINEERING PROBLEM TO BE 
    //SOLVED USING A GRADIENT METHOD
	//Function potential = INT_POW(4,x)+INT_POW(4,y);//+z*z*z*z+w*w*w*w;
	Function potential = INT_POW(2,x-1.0)+(x-3.0)*(y+0.5)+INT_POW(2,y+2.0);
	double p[] = { 10, 10 };
	double value;
	unsigned int iterations;
	gradmin(potential,p,1.0e-5,value,iterations);
	printf("Final values after %i iterations for the minimization approximation were (%g,%g).\n",iterations,p[0],p[1]);
    //THE FINAL TOUCH: VECTOR OPERATIONS USING TNT
    pBuffer = new char[4096];
    Function f = x*y*z;
    Function dfdz = f->get_partial_derivative(z);
    Function farr1[] = {(cst(1)*z),(cst(1)*x),(cst(1)*y)};

    Vector<Function> v1(3,farr1);
    Function farr2[] = { f->get_partial_derivative(x),f->get_partial_derivative(y),dfdz };
    Vector<Function> v2(3,farr2);
    Vector<Function> result = v1*v2;

    //std::cout << f << "\n";
    f->to_string(pBuffer);
    printf("The original Function was %s\n",pBuffer);
    dfdz->to_string(pBuffer);
    printf("The dfdz Function was %s\n",pBuffer);
    for(int i = 0;i < 3;i++)
    {
        result[i]->to_string(pBuffer);
        printf("The vector component %i is %s\n",i,pBuffer);
    }
    delete [] pBuffer;
    return uninitialize_calculus();
}

void gradmin(
           Function f, 
           double * p, 
           double tolerance, 
           double & value, 
           unsigned int & iterations
           )
{
    //Get the number of Variables in the Function given
    int num_vars = f->get_number_of_variables();
    //Obtain all Variables into a convenient array
    calculus::variable ** variables = f->get_variables();
    //Build the gradient vector for this Function
    Function * grad_f = new Function[num_vars];
    Function * grad_div_f = new Function[num_vars];
    {for(int i = 0;i<num_vars;i++) {
        grad_f[i] = f->get_partial_derivative(variables[i]);
        grad_div_f[i] = grad_f[i]->get_partial_derivative(variables[i]);
    }}

    //Gradient components may be Functions of fewer (but no more) Variables 
    //than the original Function so we need to rebuild local Variable/value 
    //sets for each gradient vector component.
    int *gradient_num_vars = new int[num_vars],*gradient_div_num_vars = new int[num_vars];
    int **gradient_var_mask = new int*[num_vars],**gradient_div_var_mask = new int*[num_vars];
    {for(int i = 0;i < num_vars;i++) {
        gradient_num_vars[i] = grad_f[i]->get_number_of_variables();
        gradient_div_num_vars[i] = grad_div_f[i]->get_number_of_variables();
        calculus::variable **ppGradientVars = grad_f[i]->get_variables(),**ppGradientDivVars = grad_div_f[i]->get_variables();
        gradient_var_mask[i] = new int[gradient_num_vars[i]];
        gradient_div_var_mask[i] = new int[gradient_div_num_vars[i]];
        {for(int j = 0,k = 0;(j < num_vars) && (k < gradient_num_vars[i]);j++) {
            if (ppGradientVars[k] == variables[j])
                gradient_var_mask[i][k++] = j;
        }}
        {for(int j = 0,k = 0;(j < num_vars) && (k < gradient_div_num_vars[i]);j++) {
            if (ppGradientDivVars[k] == variables[j])
                gradient_div_var_mask[i][k++] = j;
        }}
    }}

    //More variable allocations which I will need in the iteration
    double delta_value = 0,
        *local_p = new double[num_vars],
        *grad_value = new double[num_vars],
        *delta = new double[num_vars],
        *grad_div_value = new double[num_vars];

    {for(int i = 0;i < num_vars;i++)
        local_p[i] = grad_div_value[i] = grad_value[i] = delta[i] = 0;}
    //Get the local Function value
    value = f(p);
    iterations = 0;
    //Iterate this section to get progressively closer to the minimum
    set_up_report_info(num_vars,f,grad_f);
    do
    {
        report_info(num_vars,iterations,value,p,grad_value,grad_div_value,delta,delta_value);
        //Get the local gradient value
        {for(int i = 0;i < num_vars;i++) {
            {for(int j = 0;j < gradient_num_vars[i];j++)
                local_p[j] = p[gradient_var_mask[i][j]];}
            grad_value[i] = grad_f[i](local_p);
            {for(int j = 0;j < gradient_div_num_vars[i];j++)
                local_p[j] = p[gradient_div_var_mask[i][j]];
            grad_div_value[i] = grad_div_f[i](local_p);}
        }}
        //Get the various directional deltas
        {for(int i = 0;i < num_vars;i++)
            delta[i] = -grad_value[i]/fabs(grad_div_value[i]);}
        //Get the next point value guess and determine how much motion resulted
        delta_value = 0;
        {for(int i = 0;i < num_vars;i++) {
            p[i] += delta[i];
            delta_value += delta[i]*delta[i];
        }}
        delta_value = sqrt(delta_value);
        //Get the local Function value
        value = f(p);
        //Increment iteration count
        if (++iterations > 1000)
            break;
    }
    while(fabs(delta_value) > tolerance);
    
    //Free all memory allocated
    {for(int i = 0;i < num_vars;i++) {
        delete [] gradient_div_var_mask[i];
        delete [] gradient_var_mask[i];
    }}
    delete [] local_p;
    delete [] gradient_div_var_mask;
    delete [] gradient_var_mask;
    delete [] gradient_div_num_vars;
    delete [] gradient_num_vars;
    delete [] delta;
    delete [] grad_div_value;
    delete [] grad_value;
    delete [] grad_div_f;
    delete [] grad_f;
}

void set_up_report_info(
                        int num_vars,
                        Function f,
                        Function * gradient
                        )
{
    char * pBuffer = new char[4096];
    f->to_string(pBuffer);
    printf("The Function at study is %s\n",pBuffer);
    delete [] pBuffer;

    {for(int i = 0;i < num_vars;i++) {
        gradient[i]->to_string(pBuffer = new char[gradient[i]->to_string(NULL)+1]);
        printf("Gradient component %i is %s.\n",i,pBuffer);
        delete [] pBuffer;
    }}

    printf("iterations,value");
    {for(int i = 0;i < num_vars;i++)
        printf(",var%i",i);}
#ifdef BIGPRINT
    {for(int i = 0;i < num_vars;i++)
        printf(",grad%i",i);}
    {for(int i = 0;i < num_vars;i++)
        printf(",grad_div%i",i);}
    {for(int i = 0;i < num_vars;i++)
        printf(",delta%i",i);}
    printf(",delta_value\n");
#else
    printf("\n");
#endif
}

void report_info(
                 int num_vars,
                 int iterations,
                 double value,
                 double * p,
                 double * grad_value,
                 double * grad_div_value,
                 double * delta,
                 double delta_value
                 )
{
    printf("%i,%g",iterations,value);
    for(int i = 0;i < num_vars;i++)
        printf(",%g",p[i]);
#ifdef BIGPRINT
    for(int i = 0;i < num_vars;i++)
        printf(",%g",grad_value[i]);
    for(int i = 0;i < num_vars;i++)
        printf(",%g",grad_div_value[i]);
    for(int i = 0;i < num_vars;i++)
        printf(",%g",delta[i]);
    printf(",%g\n",delta_value);
#else
    UNREFERENCED_PARAMETER(delta_value);
    UNREFERENCED_PARAMETER(delta);
    UNREFERENCED_PARAMETER(grad_div_value);
    UNREFERENCED_PARAMETER(grad_value);
    printf("\n");
#endif
}
