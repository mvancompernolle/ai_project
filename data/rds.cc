//
//* \file  rds.cc
//* \brief Random Digits Stream.
//
// This file is part of the libecc package.
// Copyright (C) 2004, by
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
#include <libecc/rds.h>

namespace libecc {

/**\class rds rds.h libecc/rds.h
 *
 * \brief Buffered Pseudo Random Number Generator.
 */

/**
 * \brief Read random digits.
 *
 * Write \a digits digits of random data into the array pointed to by \a outptr.
 */
void rds::read(bitset_digit_t* outptr, unsigned int digits)
{
  while(digits)
  {
    if (M_available == 0)
    {
      M_rng.generate_512_bits();
      bitset<512> const& bits(M_rng.get_512_bits());
      if (M_use_SHA1)
      {
        M_hash.process_msg(bits, 512);
        M_bufptr = M_hash.digest().digits_ptr();
	M_available = bitset_base<160>::digits - bitset_base<160>::has_excess_bits ? 1 : 0;
      }
      else
      {
	M_bufptr = bits.digits_ptr();
	M_available = bitset_base<512>::digits;
      }
    }
    unsigned int n = std::min(digits, M_available);
    std::memcpy(outptr, M_bufptr, n * sizeof(bitset_digit_t));
    outptr += n;
    digits -= n;
    M_bufptr += n;
    M_available -= n;
  }
}

}

