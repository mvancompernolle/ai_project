#include "sys.h"
#include <iostream>
#include "debug.h"
#include "libecc/bitset.h"

using libecc::bitset;
using libecc::bitset_digit_t;

int const m = twoDSp2;

int main(void)
{
#ifdef CWDEBUG
  std::ios::sync_with_stdio(false);
  Debug( check_configuration() );
  //Debug( dc::notice.on() );
  Debug( libcw_do.on() );
  Debug( libcw_do.set_ostream(&std::cout) );
  Debug( list_channels_on(libcw_do) );
#endif

  bitset<m> b1;
  bitset<m> b1_orig;
  bitset<m> b2;

#if 0
  bitset_digit_t volatile& w1(*(b1.digits_ptr() - 1));
  bitset_digit_t volatile& w2(*(b1.digits_ptr() + 3));
  bitset_digit_t volatile& w3(*(b2.digits_ptr() - 1));
  bitset_digit_t volatile& w4(*(b2.digits_ptr() + 3));

  std::cout << "awatch *((unsigned int*)" << (void*)&w1 << ")\n";
  std::cout << "awatch *((unsigned int*)" << (void*)&w2 << ")\n";
  std::cout << "awatch *((unsigned int*)" << (void*)&w3 << ")\n";
  std::cout << "awatch *((unsigned int*)" << (void*)&w4 << ")" << std::endl;

  int i;
  i = 1;
#endif

#ifndef CWDEBUG
  for (int samples = 0; samples < 20; ++samples)
  {
#endif

    for (int x1 = 0; x1 < m; ++x1)
    {
      b1.reset();
      for (int x = x1; x < m; x += 1)
	b1.set(x);
      b1_orig = b1;
      for (int x2 = 0; x2 < m; ++x2)
	for (int d2 = 0; d2 < m - x2 && d2 < m - x1; ++d2)
	  for (int offset = 0; offset < 2; ++offset)
	  {
	    b2.reset();
	    for (int x = x2 + offset; x <= x2 + d2; x += 2)
	      b2.set(x);
	    Dout(dc::notice, "x1 = " << x1 << "; x2 = " << x2 << "; d2 = " << d2 << "; offset = " << offset);
	    Dout(dc::notice, "\tbitset1 = " << cwprint_using(b1, &bitset<m>::base2_print_on));
	    Dout(dc::notice|flush_cf, "\tbitset2 = " << cwprint_using(b2, &bitset<m>::base2_print_on));
	    b1.xor_with_zero_padded(b2, x2, x2 + d2, x2 - x1);
	    Dout(dc::notice, "\tHigh digit b1 = " << std::hex << b1.rawdigit(bitset<m>::digits - 1) << std::dec);
	    Dout(dc::notice, "\tbitset1 = " << cwprint_using(b1, &bitset<m>::base2_print_on));

#ifdef CWDEBUG
	    bitset<m> b3;
	    if (x1 < x2)
	    {
	      b3 = b2;
	      b3 >>= (x2 - x1);
	      b3 ^= b1_orig;
	    }
	    else if (x1 == x2)
	    {
	      b3 = b1_orig ^ b2;
	    }
	    else
	    {
	      b3 = b2;
	      b3 <<= (x1 - x2);
	      b3 ^= b1_orig;
	    }
	    Dout(dc::notice|flush_cf, "\tHigh digit b3 = " << std::hex << b3.rawdigit(bitset<m>::digits - 1) << std::dec);
	    b3 ^= b1;
	    Dout(dc::notice|flush_cf, "\tfailures= " << cwprint_using(b3, &bitset<m>::base2_print_on));
	    assert( !b3.any() );
#endif

	    b1 = b1_orig;
	  }
    }

#ifndef CWDEBUG
  }
#endif

  return 0;
}
