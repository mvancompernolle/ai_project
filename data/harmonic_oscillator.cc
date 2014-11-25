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


#include <more/phys/harmonic_oscillator.h>
#include <more/math/complex.h>
#include <more/math/spinor.h>
#include <more/math/qtypes.h>
#include <more/math/special_functions.h>
#include <more/num/tensin.h>
#include <assert.h>

namespace more {
namespace phys {

  template<typename T>
    std::complex<T>
    harmonic_oscillator(int n, int l, int ml,
			more::num::vectin<T, 3> const& r) {
 	T r2 = num::norm(r);
 	T phi = math::atan2(r(1), r(0));
 	T theta = math::atan2(std::sqrt(math::pow2(r(0)) +
					math::pow2(r(1))), r(2));
 	std::complex<T> res
	    = math::pow(r2, l*math::half)
	    * math::lagL(n, l + math::half, r2)*math::exp(-T(.5)*r2)
	    * math::sphY(l, ml, theta, phi);
	assert(math::isfinite(res));
	return res;
    }

//   template
//     std::complex<float>
//     harmonic_oscillator(int, int, int, more::vectin<float, 3> const&);
  template
    std::complex<double>
    harmonic_oscillator(int, int, int,
			more::num::vectin<double, 3> const&);
//   template
//     std::complex<long double>
//     harmonic_oscillator(int, int, int, more::vectin<long double, 3> const&);

}}
