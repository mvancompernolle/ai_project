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

  // Run over all values of b (except 0).
  int count = 0;
  for (bitset_digit_t bval = 1; bval < q; ++bval)
  {
    poly_t b(bval);

    // Try to find the cube root of b.
    bool found = false;
    for (bitset_digit_t crbval = 1; crbval < q; ++crbval)
    {
      poly_t crb(crbval);
      if (crb * crb * crb == b)
      {
        found = true;
        break;
      }
    }

    if (found)
      ++count;
  }

  cout << "There are " << count << " non-zero values for which the cube root exists." << endl;
  assert(((m & 1) && count == (q - 1)) || (!(m & 1) && 3 * count == (q - 1)));
  if ((m & 1))
    cout << "q - 1 == count" << endl;
  else
    cout << "q - 1 == 3 * count" << endl;

  return 0;
}

