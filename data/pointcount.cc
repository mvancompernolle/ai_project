#include "sys.h"
#include "debug.h"
#include <iostream>
#include <cassert>
#include <libecc/point.h>
#include "utils.h"

#define VERBOSE 1

using std::cout;
using std::endl;
using std::flush;
using libecc::bitset_base;
using libecc::bitset_digit_t;

typedef libecc::bitset<m> bitset;

bitset_digit_t aval = 0;

poly_t a(aval);
poly_t b;

typedef libecc::point<poly_t, a, b> point;
using libecc::bitset_digit_t;

int main()
{
  initialize_utils();

#ifdef VERBOSE
  cout << "The cardinality of the field (q) is " << q << " (m = " << m << ")." << endl;
  cout << "n = " << poly_t(poly_t::normal()) << endl;
#endif

  // Run over all values of b (except 0)
  for (bitset_digit_t bval = 1; bval < q; ++bval)
  {
    b = poly_t(bval);

    // Run over all values of x (except 0).
    int count = 0;
    for (bitset_digit_t xval = 1; xval < q; ++xval)
    {
      poly_t x(xval);
      poly_t bxinv = b / x;		// bxinv = b/x

      int tracex = x.trace();
      int tracebxinv = bxinv.trace();

      if (tracex == tracebxinv)
	count++;
    }

    cout << "b = " << b << "; #E = " << (2 + 2 * count) << endl;

    break;
  }

  return 0;
}

