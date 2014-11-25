/*

CALCULUS.CPP: IMPLEMENTS THE BASIC INTERFACE

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

static bool bcalculusinitialized = false;

void initialize_calculus(unsigned int cMode) {
    
    UNREFERENCED_PARAMETER(cMode);

	if (bcalculusinitialized)
		return;

	//INITIALIZE TRIVIAL COMPILER
	calculus::algebra_parser * pService = calculus::algebra_parser::get_service();

	calculus::unary_operators::intrinsic_operators::exponential::Register(pService);
	calculus::unary_operators::intrinsic_operators::integer_power::Register(pService);
	calculus::unary_operators::intrinsic_operators::ln::Register(pService);
	calculus::unary_operators::intrinsic_operators::log::Register(pService);
	calculus::unary_operators::intrinsic_operators::negate::Register(pService);
	calculus::unary_operators::intrinsic_operators::nop::Register(pService);
	calculus::unary_operators::intrinsic_operators::square_root::Register(pService);
	calculus::unary_operators::trigonometric_operators::sine::Register(pService);
	calculus::unary_operators::trigonometric_operators::cosine::Register(pService);
	calculus::unary_operators::trigonometric_operators::tangent::Register(pService);
	calculus::unary_operators::trigonometric_operators::arcsine::Register(pService);
	calculus::unary_operators::trigonometric_operators::arccosine::Register(pService);
	calculus::unary_operators::trigonometric_operators::arctangent::Register(pService);
	calculus::unary_operators::hyperbolic_operators::sinh::Register(pService);
	calculus::unary_operators::hyperbolic_operators::cosh::Register(pService);
	calculus::unary_operators::hyperbolic_operators::tanh::Register(pService);
	calculus::unary_operators::derivative_operators::derivative_3b::Register(pService);
	calculus::unary_operators::derivative_operators::derivative_3c::Register(pService);
	calculus::unary_operators::derivative_operators::derivative_3f::Register(pService);
	calculus::unary_operators::derivative_operators::derivative_5b::Register(pService);
	calculus::unary_operators::derivative_operators::derivative_5c::Register(pService);
	calculus::unary_operators::derivative_operators::derivative_5f::Register(pService);

	calculus::binary_operators::intrinsic_operators::addition::Register(pService);
	calculus::binary_operators::intrinsic_operators::division::Register(pService);
	calculus::binary_operators::intrinsic_operators::exponentiation::Register(pService);
	calculus::binary_operators::intrinsic_operators::multiplication::Register(pService);
	calculus::binary_operators::intrinsic_operators::subtraction::Register(pService);

	bcalculusinitialized = true;
}

int uninitialize_calculus() {
	if (!bcalculusinitialized)
		return -1;
	//KILL THE TRIVIAL COMPILER
	calculus::algebra_parser::kill_service();
	//FREE COMPILED STRUCTURES
	calculus::IA32_binary::free_all_IA32_binaries();
	//FREE THE VARIABLE REGISTRY
	calculus::variable::FreeRegistry();

#ifdef _DEBUG
#ifdef _MSC_VER
    _RPT0(_CRT_WARN,"\nChecking for leaks...\n\n");
	_CrtDumpMemoryLeaks();
#endif //_MSC_VER
#endif //_DEBUG

    bcalculusinitialized = false;

    return 0;
}
