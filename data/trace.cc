#include "sys.h"
#include "debug.h"
#include <libecc/point.h>
#include <set>
#include "mk.h"

using std::cout;
using std::endl;

unsigned int const m = libecc::field_extension_degree;
unsigned int const k = libecc::reduction_polynomial_exponents<m>::k;
unsigned int const k1 = libecc::reduction_polynomial_exponents<m>::k1;
unsigned int const k2 = libecc::reduction_polynomial_exponents<m>::k2;

typedef libecc::polynomial<m, k, k1, k2> poly;
using libecc::bitset_digit_t;
using libecc::bitset;

struct Compare {
  bool operator()(poly const& b1, poly const& b2)
  {
    for (int d = libecc::bitset<m>::digits - 1; d >= 0; --d)
      if (b1.get_bitset().digit(d) != b2.get_bitset().digit(d))
	return b1.get_bitset().digit(d) < b2.get_bitset().digit(d);
    return false;
  }
};

std::set<poly, Compare> done;

int main()
{
  done.insert(poly(0));
  bitset<m> mask;
  mask.reset();
  for (unsigned int i = (m + 1) / 2; i < m; ++i)
    if (i % 2 == 1)
      mask.set(i);
  cout << poly(poly::normal()) << " = n" << endl;
  cout << poly(mask) << " = mask" << endl;
  for (bitset<m> yb(1); yb.any(); yb <<= 1)
  {
    poly y(yb);
    y.get_bitset() &= mask;
    if (done.find(y) != done.end())
      continue;
    poly x(y);
    cout << "-----------\n";
    cout << x << endl;
    do
    {
      done.insert(x);
      x *= x;
      //cout << x << " = squared" << endl;
      x.get_bitset() &= mask;
      cout << x << " = masked" << endl;
    }
    while (done.find(x) == done.end());
    if (!x.get_bitset().any())
      mask &= ~yb;
  }
  return 0;
}

