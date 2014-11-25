#include "sys.h"
#include "debug.h"
#include <iostream>
#include <cassert>
#include <iomanip>
#include <libecc/point.h>
#include <libecc/fieldmath.h>
#include "utils.h"

using std::cout;
using std::endl;
using std::flush;
using libecc::bitset_base;
using libecc::bitset_digit_t;

typedef libecc::bitset<m> bitset;

poly_t a;
poly_t b;
typedef libecc::point<poly_t, a, b> point;

int main()
{
  // Run over every possible elements except 0.
  for (bitset_digit_t v = 1; v < q; ++v)
  {
    poly_t g(v);
    if (g.trace() == 0)
      continue;
    // Try to create the normal basis.
    poly_t nb[m];
    nb[0] = g;
    unsigned int i;
    for (i = 1; i < m; ++i)
    {
      nb[i] = nb[i - 1] * nb[i - 1];
      if (nb[i] == g)
        break;
    }
    if (i != m)
      continue;
    assert(nb[m - 1] * nb[m - 1] == g);
    libecc::bitset<q> h;
    h.reset();
    bitset_digit_t v2;
    for (v2 = 0; v2 < q; ++v2)
    {
      bitset b2(v2);
      poly_t x(0);
      for (unsigned int i = 0; i < m; ++i)
      {
        if (b2.test(i))
	  x += nb[i];
      }
      if (h.test(x.get_bitset().digit(0)))
        break;
      h.set(x.get_bitset().digit(0));
    }
    if (v2 != q)
      continue;
    cout << "Found normal basis = { " << nb[0];
    for (unsigned int i = 1; i < m; ++i)
      cout << ", " << nb[i];
    cout << " }" << endl;
    assert(g.trace() == 1);
    poly_t x(poly_t::unity());
    unsigned int n = 1;
    for (; n <= q; ++n)
    {
      x *= g;
      // Now x = g^n.
      if (x == poly_t::unity())
        break;
    }
    cout << "n = " << n << endl;
    if (n == q - 1)
      break;
  }

  return 0;
}
