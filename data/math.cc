//  Copyright (C) 2002--2009  Petter Urkedal
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
#include <stdexcept>

namespace more {
namespace math {

  template<typename T, int LBit, int Width>
    struct lubit_helper
    {
	static int const lbit = LBit;
	static int const mbit = LBit + Width/2;
	static int const hbit = LBit + Width;
	static int const lmask = (T(1) << mbit) - (T(1) << lbit);
	static int const hmask
	    = ((T(1) << (hbit - 1)) - (T(1) << mbit)) | (T(1) << (hbit - 1));

	static int
	lower_bit(T n)
	{
	    if (n & lmask)
		return lubit_helper<T, lbit, Width/2>::lower_bit(n);
	    else
		return lubit_helper<T, mbit, Width/2>::lower_bit(n);
	}

	static int
	upper_bit(T n)
	{
	    if (n & hmask)
		return lubit_helper<T, mbit, Width/2>::upper_bit(n);
	    else
		return lubit_helper<T, lbit, Width/2>::upper_bit(n);
	}
    };

  template<typename T, int LBit>
    struct lubit_helper<T, LBit, 1>
    {
	static int
	lower_bit(T n)
	{
	    return LBit;
	}

	static int
	upper_bit(T n)
	{
	    return LBit;
	}
    };

  unsigned int
  lower_bit(unsigned int n)
  {
      if (n == 0)
	  throw std::domain_error("more::math::lbit: Invalid zero argument.");
      return lubit_helper<unsigned int, 0, sizeof(unsigned int)*8>
	  ::lower_bit(n);
  }

  unsigned int
  upper_bit(unsigned int n)
  {
      if (n == 0)
	  throw std::domain_error("more::math::hbit: Invalid zero argument.");
      return lubit_helper<unsigned int, 0, sizeof(unsigned int)*8>
	  ::upper_bit(n);
  }

}}
