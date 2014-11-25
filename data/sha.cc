//
//* \file sha.cc
//* \brief SHA-1 implementation.
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
#include <libecc/sha.h>

namespace libecc {

bitset<160>
sha1::digest(void) const
{
  bitset<160> result;
  if (bitset_digit_bits == 16)
  {
    result.rawdigit(9) = H0 >> 16;
    result.rawdigit(8) = H0 & 0xffff;
    result.rawdigit(7) = H1 >> 16;
    result.rawdigit(6) = H1 & 0xffff;
    result.rawdigit(5) = H2 >> 16;
    result.rawdigit(4) = H2 & 0xffff;
    result.rawdigit(3) = H3 >> 16;
    result.rawdigit(2) = H3 & 0xffff;
    result.rawdigit(1) = H4 >> 16;
    result.rawdigit(0) = H4 & 0xffff;
  }
  else if (bitset_digit_bits == 32)
  {
    result.rawdigit(4) = H0;
    result.rawdigit(3) = H1;
    result.rawdigit(2) = H2;
    result.rawdigit(1) = H3;
    result.rawdigit(0) = H4;
  }
  else if (bitset_digit_bits == 64)
  {
    result.rawdigit(2) = H0;
    result.rawdigit(1) = (static_cast<bitset_digit_t>(H1) << (bitset_digit_bits / 2)) | static_cast<bitset_digit_t>(H2);
    result.rawdigit(0) = (static_cast<bitset_digit_t>(H3) << (bitset_digit_bits / 2)) | static_cast<bitset_digit_t>(H4);
  }
  else
  {
    result.rawdigit(1) = H0;
    result.rawdigit(0) = (static_cast<bitset_digit_t>(H1) << (3 * bitset_digit_bits / 4)) |
                       (static_cast<bitset_digit_t>(H2) << (bitset_digit_bits / 2)) |
		       (static_cast<bitset_digit_t>(H3) << (bitset_digit_bits / 4)) |
		       static_cast<bitset_digit_t>(H4);
  }
  return result;
}

sha1::sha1(void) : H0(0x67452301), H1(0xEFCDAB89), H2(0x98BADCFE), H3(0x10325476), H4(0xC3D2E1F0)
{
}

void sha1::reset(void)
{
  H0 = 0x67452301;
  H1 = 0xEFCDAB89;
  H2 = 0x98BADCFE;
  H3 = 0x10325476;
  H4 = 0xC3D2E1F0;
}

void
sha1::process_block(bitset_digit_t const* block)
{
  if (sizeof(bitset_digit_t) == 2)
  {
    W[0] = static_cast<uint32_t>(block[31]) << 16 | static_cast<uint32_t>(block[30]);
    W[1] = static_cast<uint32_t>(block[29]) << 16 | static_cast<uint32_t>(block[28]);
    W[2] = static_cast<uint32_t>(block[27]) << 16 | static_cast<uint32_t>(block[26]);
    W[3] = static_cast<uint32_t>(block[25]) << 16 | static_cast<uint32_t>(block[24]);
    W[4] = static_cast<uint32_t>(block[23]) << 16 | static_cast<uint32_t>(block[22]);
    W[5] = static_cast<uint32_t>(block[21]) << 16 | static_cast<uint32_t>(block[20]);
    W[6] = static_cast<uint32_t>(block[19]) << 16 | static_cast<uint32_t>(block[18]);
    W[7] = static_cast<uint32_t>(block[17]) << 16 | static_cast<uint32_t>(block[16]);
    W[8] = static_cast<uint32_t>(block[15]) << 16 | static_cast<uint32_t>(block[14]);
    W[9] = static_cast<uint32_t>(block[13]) << 16 | static_cast<uint32_t>(block[12]);
    W[10] = static_cast<uint32_t>(block[11]) << 16 | static_cast<uint32_t>(block[10]);
    W[11] = static_cast<uint32_t>(block[9]) << 16 | static_cast<uint32_t>(block[8]);
    W[12] = static_cast<uint32_t>(block[7]) << 16 | static_cast<uint32_t>(block[6]);
    W[13] = static_cast<uint32_t>(block[5]) << 16 | static_cast<uint32_t>(block[4]);
    W[14] = static_cast<uint32_t>(block[3]) << 16 | static_cast<uint32_t>(block[2]);
    W[15] = static_cast<uint32_t>(block[1]) << 16 | static_cast<uint32_t>(block[0]);
  }
  else if (sizeof(bitset_digit_t) == 4)
  {
    W[0] = block[15];
    W[1] = block[14];
    W[2] = block[13];
    W[3] = block[12];
    W[4] = block[11];
    W[5] = block[10];
    W[6] = block[9];
    W[7] = block[8];
    W[8] = block[7];
    W[9] = block[6];
    W[10] = block[5];
    W[11] = block[4];
    W[12] = block[3];
    W[13] = block[2];
    W[14] = block[1];
    W[15] = block[0];
  }
  else if (sizeof(bitset_digit_t) == 8)
  {
    W[0] = block[7] >> (bitset_digit_bits / 2);
    W[1] = block[7] & 0xffffffff;
    W[2] = block[6] >> (bitset_digit_bits / 2);
    W[3] = block[6] & 0xffffffff;
    W[4] = block[5] >> (bitset_digit_bits / 2);
    W[5] = block[5] & 0xffffffff;
    W[6] = block[4] >> (bitset_digit_bits / 2);
    W[7] = block[4] & 0xffffffff;
    W[8] = block[3] >> (bitset_digit_bits / 2);
    W[9] = block[3] & 0xffffffff;
    W[10] = block[2] >> (bitset_digit_bits / 2);
    W[11] = block[2] & 0xffffffff;
    W[12] = block[1] >> (bitset_digit_bits / 2);
    W[13] = block[1] & 0xffffffff;
    W[14] = block[0] >> (bitset_digit_bits / 2);
    W[15] = block[0] & 0xffffffff;
  }
  else
  {
    W[0] = block[3] >> (3 * bitset_digit_bits / 4);
    W[1] = (block[3] >> (bitset_digit_bits / 2)) & 0xffffffff;
    W[2] = (block[3] >> (bitset_digit_bits / 4)) & 0xffffffff;
    W[3] = block[3] & 0xffffffff;
    W[4] = block[2] >> (3 * bitset_digit_bits / 4);
    W[5] = (block[2] >> (bitset_digit_bits / 2)) & 0xffffffff;
    W[6] = (block[2] >> (bitset_digit_bits / 4)) & 0xffffffff;
    W[7] = block[2] & 0xffffffff;
    W[8] = block[1] >> (3 * bitset_digit_bits / 4);
    W[9] = (block[1] >> (bitset_digit_bits / 2)) & 0xffffffff;
    W[10] = (block[1] >> (bitset_digit_bits / 4)) & 0xffffffff;
    W[11] = block[1] & 0xffffffff;
    W[12] = block[0] >> (3 * bitset_digit_bits / 4);
    W[13] = (block[0] >> (bitset_digit_bits / 2)) & 0xffffffff;
    W[14] = (block[0] >> (bitset_digit_bits / 4)) & 0xffffffff;
    W[15] = block[0] & 0xffffffff;
  }
  for (unsigned int t = 16; t < 80; ++t)
  {
    uint32_t tmp = W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16];
    W[t] = (tmp << 1) | (tmp >> 31);
  }
  A = H0;
  B = H1;
  C = H2;
  D = H3;
  E = H4;
  for (unsigned int t = 0; t < 20; ++t)
  {
    uint32_t tmp = (A << 5) | (A >> 27);
    tmp += D ^ (B & (C ^ D));
    tmp += E;
    tmp += W[t];
    tmp += 0x5A827999;
    E = D;
    D = C;
    C = (B << 30) | (B >> 2);
    B = A;
    A = tmp;
  }
  for (unsigned int t = 20; t < 40; ++t)
  {
    uint32_t tmp = (A << 5) | (A >> 27);
    tmp += B ^ C ^ D;
    tmp += E;
    tmp += W[t];
    tmp += 0x6ED9EBA1;
    E = D;
    D = C;
    C = (B << 30) | (B >> 2);
    B = A;
    A = tmp;
  }
  for (unsigned int t = 40; t < 60; ++t)
  {
    uint32_t tmp = (A << 5) | (A >> 27);
    tmp += (B & C) + (D & (B ^ C));
    tmp += E;
    tmp += W[t];
    tmp += 0x8F1BBCDC;
    E = D;
    D = C;
    C = (B << 30) | (B >> 2);
    B = A;
    A = tmp;
  }
  for (unsigned int t = 60; t < 80; ++t)
  {
    uint32_t tmp = (A << 5) | (A >> 27);
    tmp += B ^ C ^ D;
    tmp += E;
    tmp += W[t];
    tmp += 0xCA62C1D6;
    E = D;
    D = C;
    C = (B << 30) | (B >> 2);
    B = A;
    A = tmp;
  }
  H0 += A;
  H1 += B;
  H2 += C;
  H3 += D;
  H4 += E;
}

void
sha1::process_msg(bitset_digit_t const* message, size_t number_of_bits)
{
  reset();
  bitset<512> tmp;
  unsigned int const digits = number_of_bits == 0 ? 0 : (number_of_bits - 1) / bitset_digit_bits + 1;
  unsigned int const bit_shift = bitset_digit_bits - 1 - ((number_of_bits + bitset_digit_bits - 1) % bitset_digit_bits);
  int d_in = digits;
  int d_out;
  for(int cnt = number_of_bits / 512;; --cnt)
  {
    d_out = bitset<512>::digits;
    while (d_in > 0 && d_out > 0)
    {
      if (bit_shift > 0)
      {
	tmp.rawdigit(--d_out) = (message[--d_in] << bit_shift);
	if (d_in > 0)
	  tmp.rawdigit(d_out) |= message[d_in - 1] >> (bitset_digit_bits - bit_shift);
      }
      else
	tmp.rawdigit(--d_out) = message[--d_in];
    }
    if (cnt == 0)
      break;
    process_block(tmp.digits_ptr());
  }
  while (d_out)
    tmp.rawdigit(--d_out) = 0;

  // Add SHA-1 padding
  tmp.set(511 - (number_of_bits % 512));
  if ((number_of_bits % 512) >= 448)
  {
    process_block(tmp.digits_ptr());
    tmp.reset();
  }
  tmp.rawdigit(0) = static_cast<bitset_digit_t>(number_of_bits);
  // finalize SHA-1 digest calculation
  process_block(tmp.digits_ptr());
}

} // namespace libecc
