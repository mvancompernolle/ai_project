#include "sys.h"
#include "debug.h"
#include <iostream>
#include <cassert>
#include <libecc/point.h>
#include "utils.h"

#define VERBOSE

using std::cout;
using std::endl;
using std::flush;
using libecc::bitset_base;
using libecc::bitset_digit_t;

typedef libecc::bitset<m> bitset;

poly_t a;
poly_t b;
typedef libecc::point<poly_t, a, b> point;

libecc::bitset<q> frobenius_equivalence_class;		// This bitset represents each element of F_q.
							// A bit is set when we already processed the
							// corresponding element, or when the element
							// is in an Frobenius equivalence class of an
							// element that we already processed.

// Returns the number of elements with foc n and trace 1.
mpz_class tr1c(unsigned int n)
{
  mpz_class result = foc(n);
  while (n % 2 == 0)
  {
    n /= 2;
    result += foc(n);
  }
  result /= 2;
  return result;
}

int main()
{
  initialize_utils();

#ifdef VERBOSE
  cout << "The cardinality of the field (q) is " << q << " (m = " << m << ")." << endl;
  cout << "n = " << poly_t(poly_t::normal()) << endl;
#endif

  int count[m + 1];
  int tracex_count[2];
  int tracexinv_count[2];
  int equal_tracex_count[2];
  int equal_tracehx_count[2];
  std::memset(count, 0, sizeof(count));
  std::memset(tracex_count, 0, 2 * sizeof(int));
  std::memset(tracexinv_count, 0, 2 * sizeof(int));
  std::memset(equal_tracex_count, 0, 2 * sizeof(int));
  std::memset(equal_tracehx_count, 0, 2 * sizeof(int));

  // Run over all values of x (except 0).
  for (bitset_digit_t xval = 1; xval < q; ++xval)
  {
    if (frobenius_equivalence_class.test(xval))
      continue;
    frobenius_equivalence_class.set(xval);

    libecc::bitset_digit_t x2buf1[poly_t::square_digits];
    libecc::bitset_digit_t x2buf2[poly_t::square_digits];

    poly_t x(xval);

    // Find the multiplicity (the smallest positive integer n such that x^(2^n) = x).
    unsigned int frobenius_multiplicity = 1;
    poly_t* x2n1 = &x.square(x2buf1);
    while (*x2n1 != x)
    {
      frobenius_equivalence_class.set(x2n1->get_bitset().digit(0));
      ++frobenius_multiplicity;
      poly_t* x2n2 = &x2n1->square(x2buf2);
      if (*x2n2 == x)
        break;
      frobenius_equivalence_class.set(x2n2->get_bitset().digit(0));
      ++frobenius_multiplicity;
      x2n1 = &x2n2->square(x2buf1);
    }
    
    poly_t xinv = poly_t(1) / x;	// xinv = 1/x
    int tracex = x.trace();
    int tracexinv = xinv.trace();

#if 0 //def VERBOSE
    int static cnt;
    ++cnt;
    cout << cnt << ": x = " << x << " = " << x.get_bitset().digit(0) << "; multiplicity: " << frobenius_multiplicity <<
            "; Tr(x) = " << tracex << "; Tr(1/x) = " << tracexinv << endl;
#endif

    if (tracex == tracexinv)
      count[frobenius_multiplicity]++;
    if (frobenius_multiplicity == m)
    {
      tracex_count[tracex]++;
      tracexinv_count[tracexinv]++;
      if (tracex == tracexinv)
        equal_tracex_count[tracex]++;
    }
    if ((m & 1) == 0 && frobenius_multiplicity == m / 2)
    {
      if (tracex == tracexinv)
	equal_tracehx_count[tracex]++;
    }
  }

  int total = 1;
  for (unsigned int i = 2; i <= m; ++i)
    total += i * count[i];

  // Kill tracexinv_count out of the equation.
  assert(tracex_count[0] == tracexinv_count[0]);
  assert(tracex_count[1] == tracexinv_count[1]);

  // Express tracex_count in foc.
  assert(foc(m) == m * (tracex_count[0] + tracex_count[1]));	// Definition of foc().
  assert(tr1c(m) == m * tracex_count[1]);			// Definition of tr1c().
  assert(m % 2 == 0 || tracex_count[1] == tracex_count[0]);
  assert(m % 2 == 1 || m * (tracex_count[1] - tracex_count[0]) == 2 * tr1c(m / 2));

  // Observations that relate this to the elliptic curve point count.
  assert(equal_tracex_count[1] - equal_tracex_count[0] == tracex_count[1] - tracex_count[0]);
  assert(m % 2 == 1 || m * (equal_tracehx_count[0] + equal_tracehx_count[1]) ==  2 * foc(m / 2));

#ifdef VERBOSE
  cout << "There are " << total << " = 1";
  for (unsigned int i = m; i >= 2; --i)
  {
    if (count[i] > 0)
    {
      if (i == m)
      {
	cout << " + (" << equal_tracex_count[0] << " + " << equal_tracex_count[1] << ") * " << i;
      }
      else if ((m & 1) == 0 && i == m / 2)
	cout << " + (" << equal_tracehx_count[0] << " + " << equal_tracehx_count[1] << ") * " << i;
      else
	cout << " + " << count[i] << " * " << i;
    }
  }
  cout << " non-zero values for which Tr(x) = Tr(1/x)" << endl;
#endif

  cout << m << ": " <<
      tracex_count[0] << ", " <<
      tracex_count[0] << " - 2 * " << equal_tracex_count[0] << " = " <<
      (tracex_count[0] - 2 * equal_tracex_count[0]) << ", " <<
      tracex_count[0] << " - (" << equal_tracex_count[0] << " + " << equal_tracex_count[1] << ") = " <<
      (tracex_count[0] - (equal_tracex_count[0] + equal_tracex_count[1])) << endl;

  return 0;
}

