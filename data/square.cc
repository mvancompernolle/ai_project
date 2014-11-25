//
// This file is part of the libecc package.
//
//* \file square.cc
//* \brief Implementation of internal polynomial functions in order to efficiently calculate the square.
//
// This code has been derived from the program 'irred' version 1.50.
//
// Copyright (C) 2000 R. P. Brent.
//
// Author: Richard Brent
//         with assistance and contributions from
//         Samuli Larvala and Paul Zimmermann
//
// Dates:  20000528..20000817
// See http://web.comlab.ox.ac.uk/oucl/work/richard.brent/irred.html.
//
//
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
#include <libecc/bitset.h>

namespace libecc {

#if ECC_BITS == 32

#if (defined(__i386) || defined(__sgi))

static bitset_digit_t tabw[256] = {
  0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015,
  0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055,
  0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115,
  0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155,
  0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415,
  0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455,
  0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515,
  0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555,
  0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015,
  0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055,
  0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115,
  0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155,
  0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415,
  0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455,
  0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515,
  0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555,
  0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015,
  0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055,
  0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115,
  0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155,
  0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415,
  0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455,
  0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515,
  0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555,
  0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015,
  0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055,
  0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115,
  0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155,
  0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415,
  0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455,
  0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515,
  0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555,
};

// square using 8-bit -> 16-bit table lookup for 32-bit machines.
// (works on both big-endian and little-endian machines).
//
void square(bitset_digit_t const* input, bitset_digit_t* output, unsigned int digits)
{
  for (int j = digits - 1; j >= 0; --j)
  {
    bitset_digit_t tmp = input[j];
    output[2 * j] = tabw[(unsigned char)tmp] | (tabw[(unsigned char)(tmp >> 8)] << 16);
    output[2 * j + 1] = tabw[(unsigned char)(tmp >> 16)] | (tabw[(unsigned char)(tmp >> 24)] << 16);
  }
}

#else

#ifdef __i386		// Don't use unrolled version of cpu with only a few registers.

// 32-bit version of square.
//
// Squares polynomial a of degree r - 1 (mod 2), without table lookup, assuming 32-bit words.
// (1.load + 2.store).q1 memory ops. and about 24.q1 REGISTER ops. */
//
void square(bitset_digit_t const* input, bitset_digit_t* output, unsigned int digits)
{
  register bitset_digit_t t, u, next;
  register bitset_digit_t const c1 = 0x0000FFFFL;		// Some 32-bit constants.
  register bitset_digit_t const c2 = 0x00FF00FFL;
  register bitset_digit_t const c3 = 0x0F0F0F0FL;
  register bitset_digit_t const c4 = 0x33333333L;
  register bitset_digit_t const c5 = 0x22222222L;

  next = input[--digits];

  for (register unsigned int j = digits; j >= 0; --j)
  {
    u = next >> 16;		// High order 16 bits of input[j].
    t = next & c1;		// Low order 16 bits.

    u = (u | u << 8)  & c2;
    t = (t | t << 8)  & c2;

    next = input[j - 1];	// Load in advance to overcome latency.  May access input[-1].

    u = (u | u << 4)  & c3;
    t = (t | t << 4)  & c3;
    u = (u | u << 2)  & c4;
    t = (t | t << 2)  & c4;

    output[2 * j + 1] = u + (u & c5);
    output[2 * j]   = t + (t & c5);
  }
}

#else

// 32-bit unrolled version of square.
//
// Squares polynomial a of degree r-1 (mod 2), result -> a,
// without table lookup, assuming 64-bit words.
// (1.load + 2.store).q1 memory ops. and about 24.q1 register ops.
// 
// May be faster than "un-unrolled" version if more than two register ops can
// be performed concurrently. Faster than table lookup on word-oriented
// machines (e.g. SPARC) but not on IBM PC. 
// 
// Because of a loop optimisation, it should be valid to access input[-2].
//
void square(bitset_digit_t const* input, bitset_digit_t* output, unsigned int digits)
{
  register int j, low;
  register bitset_digit_t t, u, v, w, next1, next2;
  
  register bitset_digit_t const c1 = 0x0000FFFFL;	// Some 32-bit constants.
  register bitset_digit_t const c2 = 0x00FF00FFL;
  register bitset_digit_t const c3 = 0x0F0F0F0FL;
  register bitset_digit_t const c4 = 0x33333333L;
  register bitset_digit_t const c5 = 0x22222222L;
  
  low = 2 * (digits / 2) - 1;
    
  if ((digits & 1))			// Do an iteration of loop if q1 even.
  {
    t = input[--digits];
    u = t >> 16;			// High order 16 bits of input[q1].
    t &= c1;				// Low order 16 bits.

    u = (u | u << 8)  & c2;	
    t = (t | t << 8)  & c2;	
    u = (u | u << 4)  & c3;
    t = (t | t << 4)  & c3;
    u = (u | u << 2)  & c4;
    t = (t | t << 2)  & c4;
    output[2 * digits + 1] = u + (u & c5);
    output[2 * digits] = t + (t & c5);
  }

  next1 = input[low];
  next2 = input[low - 1];
  
  for (j = low; j >= 0; j -= 2)		// Here low and j are odd.
  {
    u = next1 >> 16;			// High order 16 bits of input[j].
    w = next2 >> 16;			// Ditto of input[j - 1].
    t = next1 & c1;			// Low order 16 bits of input[j].
    v = next2 & c1;			// Ditto of input[j - 1].

    u = (u | u << 8) & c2;	
    t = (t | t << 8) & c2;	

    next1 = input[j - 2];		// Perform loads in advance to overcome latency.

    w = (w | w << 8) & c2;	
    v = (v | v << 8) & c2;	

    next2 = input[j - 3];		// Ditto (may access input[-2]).
    
    u = (u | u << 4) & c3;
    t = (t | t << 4) & c3;
    w = (w | w << 4) & c3;
    v = (v | v << 4) & c3;

    u = (u | u << 2) & c4;
    t = (t | t << 2) & c4;
    w = (w | w << 2) & c4;
    v = (v | v << 2) & c4;

    output[2 * j + 1] = u + (u & c5);
    output[2 * j]   = t + (t & c5);
    output[2 * j - 1] = w + (w & c5);
    output[2 * j - 2] = v + (v & c5);
  }
}

#endif // !__i386
#endif

#elif ECC_BITS == 64

#ifdef ECC_USEUNUNROLLED	// not defined

// 64-bit version of square.
//
// Squares polynomial input of degree r - 1 (mod 2).
// (1.load + 2.store).q1 memory ops. and about 30.q1 REGISTER ops.
// NB: Because of an optimisation in the loop, it should be valid to read input[-2]. */
//
void square(bitset_digit_t const* input, bitset_digit_t* output, unsigned int digits)
{
  register bitset_digit_t const c0 = 0x00000000FFFFFFFFL;		// Some 64-bit constants.
  register bitset_digit_t const c1 = 0x0000FFFF0000FFFFL;
  register bitset_digit_t const c2 = 0x00FF00FF00FF00FFL;
  register bitset_digit_t const c3 = 0x0F0F0F0F0F0F0F0FL;
  register bitset_digit_t const c4 = 0x3333333333333333L;
  register bitset_digit_t const c5 = 0x2222222222222222L;
  register bitset_digit_t t, u, next;

  next = input[--digits];

  for (register int j = digits; j >= 0; --j)
  {
    u = next >> 32;			// High order 32 bits of input[j].
    t = next & c0;			// Low order 32 bits.

    u = (u | u << 16) & c1;		// Operations on t and u can be
    t = (t | t << 16) & c1;		// done in parallel.

    next = input[j - 1];		// Do load early to overcome memory latency.  May access input[-1]!

    u = (u | u << 8)  & c2;
    t = (t | t << 8)  & c2;
    u = (u | u << 4)  & c3;
    t = (t | t << 4)  & c3;
    u = (u | u << 2)  & c4;
    t = (t | t << 2)  & c4;

    output[2 * j + 1] = u + (u & c5);
    output[2 * j] = t + (t & c5);
  }
}

#else

// 64-bit unrolled version of square.
//
// Squares polynomial a of degree r-1 (mod 2), result -> a,
// without table lookup, assuming 64-bit words.
// (1.load + 2.store).q1 memory ops. and about 30.q1 REGISTER ops.
//
// May be faster than "un-unrolled" version because more than two 
// register ops can be performed concurrently on machines such as
// SGI R10000 and DEC alpha.
//
// Because of a loop optimisation, it should be valid to access input[-2].
//   
// squarem() is the IBM PC assembler version of this using MMX instruction.
//
void square(bitset_digit_t const* input, bitset_digit_t* output, unsigned int digits)
{
  register int j, low;
  register bitset_digit_t t, u, v, w, next1, next2;
  register bitset_digit_t c0, c1, c2, c3, c4, c5;

  c0 = 0x00000000FFFFFFFFL;		/* Some 64-bit constants */
  c1 = 0x0000FFFF0000FFFFL;
  c2 = 0x00FF00FF00FF00FFL;
  c3 = 0x0F0F0F0F0F0F0F0FL;
  c4 = 0x3333333333333333L;
  c5 = 0x2222222222222222L;

  low = 2 * (digits / 2) - 1;
    
  if ((digits & 1))			/* Do an iteration of loop if q1 even */
  {
    t = input[--digits];
    u = t >> 32;			/* High order 32 bits of input[q1] */
    t &= c0;				/* Low order 32 bits */

    u = (u | u << 16) & c1;		/* Operations on t and u can be */
    t = (t | t << 16) & c1;		/* done in parallel */
    u = (u | u << 8)  & c2;	
    t = (t | t << 8)  & c2;	
    u = (u | u << 4)  & c3;
    t = (t | t << 4)  & c3;
    u = (u | u << 2)  & c4;
    t = (t | t << 2)  & c4;
    output[2 * digits + 1] = u + (u & c5);
    output[2 * digits] = t + (t & c5);
  }

  next1 = input[low];
  next2 = input[low - 1];

  for (j = low; j >= 0; j -= 2)		// Here low and j are odd.
  {
    u = next1 >> 32;			// High order 32 bits of input[j].
    w = next2 >> 32;			// Ditto of input[j - 1].
    t = next1 & c0;			// Low order 32 bits of input[j].
    v = next2 & c0;			// Ditto of input[j - 1].

    u = (u | u << 16) & c1;		 /* Operations on t,u,v,w   */
    t = (t | t << 16) & c1;		 /* can be done in parallel */
    w = (w | w << 16) & c1;
    v = (v | v << 16) & c1;

    next1 = input[j - 2];		// Perform loads in advance 
					// to overcome latency.
    u = (u | u << 8) & c2;	
    t = (t | t << 8) & c2;	
    w = (w | w << 8) & c2;	
    v = (v | v << 8) & c2;	

    next2 = input[j - 3];		// Ditto (may access input[-2]).

    u = (u | u << 4) & c3;
    t = (t | t << 4) & c3;
    w = (w | w << 4) & c3;
    v = (v | v << 4) & c3;

    u = (u | u << 2) & c4;
    t = (t | t << 2) & c4;
    w = (w | w << 2) & c4;
    v = (v | v << 2) & c4;

    output[2 * j + 1] = u + (u & c5);
    output[2 * j] = t + (t & c5);
    output[2 * j - 1] = w + (w & c5);
    output[2 * j - 2] = v + (v & c5);
  }
}

#endif

#else

#error No support for non- 32 or 64 bits machines.  If you have a 128 bits machine then please contact the author.

#endif // ECC_BITS

} // namespace libecc
