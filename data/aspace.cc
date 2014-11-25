#include "sys.h"
#include "debug.h"
#include <libecc/point.h>
#include "mk.h"

using std::cout;
using std::endl;

unsigned int const m = libecc::field_extension_degree;
unsigned int const k = libecc::reduction_polynomial_exponents<m>::k;
unsigned int const k1 = libecc::reduction_polynomial_exponents<m>::k1;
unsigned int const k2 = libecc::reduction_polynomial_exponents<m>::k2;

libecc::polynomial<m, k, k1, k2> a;
libecc::polynomial<m, k, k1, k2> b;

typedef libecc::polynomial<m, k, k1, k2> poly;
typedef libecc::point<poly, a, b> point;
using libecc::bitset_digit_t;

int main()
{
  // Chose a fixed b.
  b = poly(1);

  for (unsigned int ag = 0; ag < (1 << m); ++ag)
  {
    a = poly(ag);

    for (unsigned int xg = 1; xg < (1 << m); ++xg)
    {
      poly x(xg);
      // x^3 + a*x^2 + b = y^2 + xy
      libecc::bitset_digit_t x2buf[poly::square_digits];
      poly& x2 = x.square(x2buf);	// x2 = x^2
      poly left = (x + a) * x2 + b;	// left = x^3 + ax^2 + b
      try
      {
	poly y(x, left);		// Solve y from y^2 + xy = x^3 + ax^2 + b
	point P(x, y);
	mpz_class order = P.order();
	cout << "The order of " << P << " is " << order << "; the trace of a (= " << a << ") is " << a.trace() << endl;
	assert((order % 4 == 0) == (a.trace() == 0));
	break;
      }
      catch (std::domain_error const&)
      {
        // No solution.
      }
    }
  }

  return 0;
}

