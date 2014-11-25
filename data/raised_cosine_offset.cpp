// 
// Copyright(c) 1993-1996 Tony Kirke
// author="Tony Kirke" *
/*
 * SPUC - Signal processing using C++ - A DSP library
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
//
#ifndef RAISEDCOS
#define RAISEDCOS
#include <math.h>
#include <complex.h>
#include <fir.h>
namespace SPUC {
//using namespace SPUC;
void root_raised_cosine(fir<complex<double> >& rcfir, double alpha, int rate,
						double offset) {
  extern double root_raised_cosine_imp(double alpha, double xin, double rate, long num_taps);
  int i;
  int num_taps = rcfir.num_taps;
  for (i=0;i<num_taps;i++) {
	rcfir.coeff[i] = root_raised_cosine_imp(alpha,double(i)+offset, (double)rate, num_taps);
  } 
}
} // namespace SPUC 
#endif
