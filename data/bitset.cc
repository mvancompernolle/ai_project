//
//* \file  bitset.cc
//* \brief Bitset.
//
// This file is part of the libecc package.
// Copyright (C) 2002, by
//
// Carlo Wood, Run on IRC <carlo@alinoe.com>
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "sys.h"
#include "debug.h"
#include <iostream>
#include <libecc/bitset.h>

namespace libecc {

/**
 * \brief Print contents of the index, for debugging purposes.
 */
std::ostream& operator<<(std::ostream& os, bitset_index const& index)
{
#ifdef __i386__
  os << "[bit index:" << index.M_index << ']';
#else
  os << "[digit:" << std::dec << index.M_digit << "; bitmask:" << std::hex << index.M_mask << ']';
#endif
  return os;
}

int
subtract(bitset_index const& i1, bitset_index const& i2)
{
#ifdef __i386__
  return (i1.M_index - i2.M_index);
#else
#if ECC_BITS == 128
#error Not implemented yet, mail me.
#elif ECC_BITS == 64
  register bitset_digit_t const c6 = 0xffffffff00000000;
  register bitset_digit_t const c5 = 0xffff0000ffff0000;
  register bitset_digit_t const c4 = 0xff00ff00ff00ff00;
  register bitset_digit_t const c3 = 0xf0f0f0f0f0f0f0f0;
  register bitset_digit_t const c2 = 0xcccccccccccccccc;
  register bitset_digit_t const c1 = 0xaaaaaaaaaaaaaaaa;
#else
  register bitset_digit_t const c5 = 0xffff0000;
  register bitset_digit_t const c4 = 0xff00ff00;
  register bitset_digit_t const c3 = 0xf0f0f0f0;
  register bitset_digit_t const c2 = 0xcccccccc;
  register bitset_digit_t const c1 = 0xaaaaaaaa;
#endif

  unsigned char n1 = 0, n2 = 0;

#if ECC_BITS == 64
  if ((i1.M_mask & c6))
    n1 |= 32;
  if ((i2.M_mask & c6))
    n2 |= 32;
#endif
  if ((i1.M_mask & c5))
    n1 |= 16;
  if ((i2.M_mask & c5))
    n2 |= 16;
  if ((i1.M_mask & c4))
    n1 |= 8;
  if ((i2.M_mask & c4))
    n2 |= 8;
  if ((i1.M_mask & c3))
    n1 |= 4;
  if ((i2.M_mask & c3))
    n2 |= 4;
  if ((i1.M_mask & c2))
    n1 |= 2;
  if ((i2.M_mask & c2))
    n2 |= 2;
  if ((i1.M_mask & c1))
    n1 |= 1;
  if ((i2.M_mask & c1))
    n2 |= 1;

  return (i1.M_digit - i2.M_digit) * bitset_digit_bits + n1 - n2;
#endif // __i386__
}

#if LIBECC_TRACK_EXPR_TEMPORARIES
namespace Operator {
std::multiset<void const*> bitsetExpression_bitset_invertible_pointers;
}
#endif

} // namespace libecc
