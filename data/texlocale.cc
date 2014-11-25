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


#include <more/math/math.h>
#include <more/io/texlocale.h>

namespace more {
namespace io {

  template<typename Char, typename OutIter>
    OutIter
    tex_num_put<Char, OutIter>
    ::put_num_impl(OutIter it, std::ios_base& ios, Char fill,
		   long double x) const
    {
	// Full reimplementation, since we want to
	//   -- show all significant digits, even if the last are zeroes, and
	//   -- gives exponents as $\cdot10^{...}$.

	// Don't care about field width in TeX output, only precision.
	int prec = ios.precision();

	// Extract and output sign
	if (x < 0.0L) {
	    *it = '-', ++it;
	    x = -x;
	}

	// Extract exponent
	const int xpt_mod = 1;
	int xpt;
	int xpt2;
	if (x != 0.0L) {
	    xpt2 = more::math::ifloor(std::log10(x));
	    x += 5.0L*std::pow(10.0L, xpt2-prec);
	    xpt2 = more::math::ifloor(std::log10(x));
	    if (xpt2 < -3 || xpt2 >= prec) {
		xpt = (xpt2/xpt_mod)*xpt_mod;
		x *= std::pow(.1L, xpt);
		xpt2 = 0;
	    }
	    else xpt = 0;
	}
	else xpt = xpt2 = 0;

	// Output the number less exponent
	int xpt1 = std::max(0, xpt2);
	prec -= std::min(0, xpt2);
	x *= std::pow(.1L, xpt1);
	while (prec > 0 || xpt1 >= 0) {
	    if (xpt1 == -1)
		*it = '.', ++it;
	    double xi;
	    x = modf(x, &xi)*10.0;
	    *it = '0' + (int)xi, ++it;
	    --prec;
	    --xpt1;
	}

	// Exponent
	if (xpt != 0) {
	    std::ostringstream oss;
	    oss << "$\\cdot10^{" << xpt << "}$";
	    std::string s = oss.str();
	    std::copy(s.begin(), s.end(), it);
	}
	return it;
    }

  template class tex_num_put< char, std::ostreambuf_iterator<char> >;

}
}
