#include "sys.h"
#include "debug.h"
#include <iostream>
#include <cassert>
#include <libecc/point.h>
#include "utils.h"
#include "graph.h"

using std::cout;
using std::endl;
using std::flush;

poly_t a;
poly_t b;

typedef libecc::point<poly_t, a, b> point;
typedef libecc::bitset<m> bitset;
using libecc::bitset_digit_t;

int main()
{
  cout << "The cardinality of the field (q) is " << q << endl;

  cout << "Calculating the normal to hyperplane A_0..." << endl;
  poly_t n(poly_t::normal());
  cout << "The normal of A0 is " << n << '.' << endl;

  poly_t a0(0);
  poly_t a1(1);
  while(a1.trace() == 0)
    a1.get_bitset() <<= 1;	// Try next bit.

  cout << "The choice for a0 is " << a0 << '.' << endl;
  cout << "The choice for a1 is " << a1 << '.' << endl;

  // Run over all values of b (except 0).
  for (bitset_digit_t bval = 1; bval < q; ++bval)
  {
    b = poly_t(bval);
    cout << "b = " << b << "; Tr(b) = " << b.trace();

    // Calculate the cardinality of the curves E0 and E1.
    unsigned int cardinality0 = 2;		// The points O and (0, sqrt(b)).
    unsigned int cardinality1 = 2;
    // Run over all possible values of x.
    for (bitset_digit_t xval = 1; xval < q; ++xval)
    {
      poly_t const x(xval);
      bitset_digit_t x2buf[poly_t::square_digits];
      poly_t const& x2 = x.square(x2buf);		// x2 = x^2

      bool has_solution0 = true;
      try
      {
	// See if there is a point on the curve with this x coordinate.
	// E0: x^3 + a0*x^2 + b = y^2 + xy
	poly_t left = (x + a0) * x2 + b;		// left = x^3 + a0x^2 + b
	poly_t y(x, left);			// Solve y from y^2 + xy = x^3 + a0x^2 + b
	bitset_digit_t y2buf[poly_t::square_digits];
	poly_t& y2 = y.square(y2buf);		// y2 = y^2
	assert(left == y2 + x * y);
      }
      catch (std::domain_error const& error)
      {
        has_solution0 = false;
      }

      bool has_solution1 = true;
      try
      {
	// See if there is a point on the curve with this x coordinate.
	// E1: x^3 + a1*x^2 + b = y^2 + xy
	poly_t left = (x + a1) * x2 + b;		// left = x^3 + a1x^2 + b
	poly_t y(x, left);			// Solve y from y^2 + xy = x^3 + a1x^2 + b
	bitset_digit_t y2buf[poly_t::square_digits];
	poly_t& y2 = y.square(y2buf);		// y2 = y^2
	assert(left == y2 + x * y);
      }
      catch (std::domain_error const& error)
      {
        has_solution1 = false;
      }

      // Check that indeed there is, or is not, a solution depending on the value of Tr(a).
      assert(has_solution0 != has_solution1);

      if (has_solution0)
        cardinality0 += 2;
      else
        cardinality1 += 2;
    }

    cout << "; #E0 = " << cardinality0 << "; #E1 = " << cardinality1 << endl;
    assert(cardinality0 + cardinality1 == 2 * (q + 1));
  }

  return 0;
}
