//  Copyright (C) 2000--2009  Petter Urkedal
//
//  This file is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  As a special exception, you may use this file as part of a free
//  software library without restriction.  Specifically, if other files
//  instantiate templates or use macros or inline functions from this
//  file, or you compile this file and link it with other files to
//  produce an executable, this file does not by itself cause the
//  resulting executable to be covered by the GNU General Public
//  License.  This exception does not however invalidate any other
//  reasons why the executable file might be covered by the GNU General
//  Public License.


#define PRECISE
//#define DEBUG_FINITY
#ifdef DEBUG_FINITY
#  include <more/diag/debug.h>
#  include <iostream>
#  include <iomanip>
#endif

namespace more {
namespace math {

  template<typename T>
    spinopr<T>
    FUNC(spinopr<T> const& op)
    {
	typedef typename norm_type_<T>::eval real_type;

	T c1 = T(.5)*trace(op);
	T c2 = sqrt(c1*c1 - det(op));
	T lambda0 = c1 + c2;
	T lambda1 = c1 - c2;

	spinor<T> chi0, chi1;
#ifdef PRECISE
	real_type nrm01;
	spinor<T> chi2;
	real_type nrm2;
#endif

	chi0(half) = op(half, -half);
	chi0(-half) = lambda0 - op(half, half);
#ifdef PRECISE
	chi2(half) = lambda0 - op(-half, -half);
	chi2(-half) = op(-half, half);
	nrm01 = norm(chi0);
	nrm2 = norm(chi2);
	if (nrm2 > nrm01) {
	    chi0 = chi2;
	    nrm01 = nrm2;
	}
	if (nrm01 != 0) chi0 /= sqrt(nrm01);
	else chi0 = alpha;
#else
	chi0 /= sqrt(norm(chi0));
#endif

	chi1(half) = op(half, -half);
	chi1(-half) = lambda1 - op(half, half);
#ifdef PRECISE
	chi2(half) = lambda1 - op(-half, -half);
	chi2(-half) = op(-half, half);
	nrm01 = norm(chi1);
	nrm2 = norm(chi2);
	if (nrm2 > nrm01) {
	    chi1 = chi2;
	    nrm01 = nrm2;
	}
	if (nrm01 != 0) chi1 /= sqrt(nrm01);
	else chi1 = beta;
#else
	chi1 /= sqrt(norm(chi1));
#endif

#if 1
	T c3 = dot(chi0, chi1);
	T c4 = T(1)/(T(1) - norm(c3));
	T c5 = -c3*c4;

// 	spinopr<T> res
// 	    = chi0*FUNC(lambda0)*adj(c4*chi0 + conj(c5)*chi1)
// 	    + chi1*FUNC(lambda1)*adj(c4*chi1 + c5*chi0);
	spinopr<T> res
	  = outer_prod(chi0*FUNC(lambda0), c4*chi0 + conj(c5)*chi1)
	  + outer_prod(chi1*FUNC(lambda1), c4*chi1 + c5*chi0);
#else
	assert(abs(dot(chi0, chi1)) < 1e-6);
	spinopr<T> res
	  = outer_prod(chi0*FUNC(lambda0), chi0)
	  + outer_prod(chi1*FUNC(lambda1), chi1);
#endif
#ifdef DEBUG_FINITY
	if (!isfinite(res)) {
	    std::cerr << "\n*********** DEBUG_FINITY\n";
	    MORE_SHOW(op);
	    MORE_SHOW(c1);
	    MORE_SHOW(c2);
	    MORE_SHOW(lambda0);
	    MORE_SHOW(lambda1);
	    MORE_SHOW(chi0);
	    MORE_SHOW(chi1);
	    MORE_SHOW(c3);
	    MORE_SHOW(c4);
	    MORE_SHOW(c5);
	    MORE_SHOW(res);
	    std::cerr << "\n" << std::endl;
	}
#endif
	return res;
    }

//   template
//     spinopr<float>
//     FUNC(spinopr<float> const&);
//   template
//     spinopr< complex<float> >
//     FUNC(spinopr< complex<float> > const&);
  template
    spinopr<double>
    FUNC(spinopr<double> const&);
  template
    spinopr< complex<double> >
    FUNC(spinopr< complex<double> > const&);
//   template
//     spinopr<long double>
//     FUNC(spinopr<long double> const&);
//   template
//     spinopr< complex<long double> >
//     FUNC(spinopr< complex<long double> > const&);
}}
