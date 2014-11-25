/*

CHERMITEPOLYFUNCTION.CPP: 
IMPLEMENTS calculus::unary_operators::polynomials::CHermitePolyFunction

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

calculus::unary_operators::polynomials::CHermitePolyFunction::CHermitePolyFunction(unsigned int uiNumPoints,DATA<double,3>* pPoints) : calculus::unary_operators::polynomials::polynomial() {
	this->m_epoly_function_type = Interpolatory;
	this->__load_interpolant(CHermitePolyFunction::HermiteInterpolatoryCoefficientsFromPoints(uiNumPoints,pPoints));
}

calculus::DATA<double,2> *calculus::unary_operators::polynomials::CHermitePolyFunction::HermiteInterpolatoryCoefficientsFromPoints(unsigned int uiNumPoints,DATA<double,3>* pPoints) {
//TRANSFER THE DATA FROM DATA OBJECTS TO TABLES
	calculus::DATA<double,3>* tempPTR = pPoints;
	double* lpXPoints = (double*)malloc(uiNumPoints*sizeof(double));
	double* lpYPoints = (double*)malloc(uiNumPoints*sizeof(double));
	double* lpY_prime_Points = (double*)malloc(uiNumPoints*sizeof(double));

	unsigned int a;
	for(a = 0;a < uiNumPoints;a++) {
		lpXPoints[a] = tempPTR->m_tDataSet[0];
		lpYPoints[a] = tempPTR->m_tDataSet[1];
		lpY_prime_Points[a] = tempPTR->m_tDataSet[2];
		tempPTR = tempPTR->m_pNext;
	}

//HERMITE POLYNOMIAL SCHEME THAT USES DIVIDED-DIFFERENCES AND THREE ARRAYS
	unsigned int numEntries = 0;
	
	for(a = 0;a <= 2*uiNumPoints;a++)
        numEntries += a;
	double *dividedDifferences = (double*)malloc(numEntries*sizeof(double));
	double *localDifference = dividedDifferences;
	double *lastDifference = localDifference;

	unsigned int numDifferences = 2*uiNumPoints;
	//LOAD THE ZEROTH STAGE (ALL Y VALUES)
	for (a = 0;a < numDifferences;a++)
		*(localDifference++) = lpYPoints[a/2];
	numDifferences--;
	lastDifference = localDifference;
	//LOAD THE FIRST STAGE (ALL Y' VALUES, AND FIRST DIFFERENCES)
	for (a = 0;a < numDifferences;a++) {
		if (*(localDifference - numDifferences) == *(localDifference - numDifferences - 1)) {
			*(localDifference++) = lpY_prime_Points[a/2];
		}
		else {
			double upperDivider = lpXPoints[a/2+1];
			double lowerDivider = lpXPoints[a/2];
			double upperDivided = *(localDifference - numDifferences);
			double lowerDivided = *(localDifference - numDifferences - 1);
			*(localDifference++) = ( upperDivided - lowerDivided)/( upperDivider - lowerDivider);
		}
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
			double upperDivider = lpXPoints[((lpUpperDDDivider - dividedDifferences)/2)];
			double lowerDivider = lpXPoints[((lpLowerDDDivider - dividedDifferences)/2)];
			*localDifference =	(*(localDifference - numDifferences) - *(localDifference - numDifferences - 1))/(upperDivider - lowerDivider);
			localDifference++;
		}
		numDifferences--;
		lastDifference = localDifference;
	}

//TRANSFER BACK TO DATA STRUCTURES
	calculus::DATA<double,2>* destPTR = AllocateInlinedDataSets<double,2>(2*uiNumPoints);
	localDifference = dividedDifferences;
	for(a = 0;a < 2*uiNumPoints; a++) {
		destPTR[a].m_tDataSet[0] = *localDifference;
		localDifference += (2*uiNumPoints-a);

		destPTR[a].m_tDataSet[1] = lpXPoints[a];
	}
	free(dividedDifferences);

//FREE THE ARRAYS NEEDED TO FIT INTO THE SCHEME
	free(lpXPoints);
	free(lpYPoints);
	free(lpY_prime_Points);

	return destPTR;
}
