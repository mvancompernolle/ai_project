#include "sys.h"
#include "debug.h"
#include <iostream>
#include <libecc/point.h>
#include "utils.h"

using std::cout;
using std::endl;
using std::flush;
using libecc::bitset_base;
using libecc::bitset_digit_t;

typedef libecc::bitset<m> bitset;

poly_t a;
poly_t b;
poly_t const t(2);

typedef libecc::point<poly_t, a, b> point;

bool is_prime(int p)
{
  for (int d = 2; d < p; ++d)
    if (p % d == 0)
      return false;
  return true;
}

int feca[q];
int fma[q];
int nfeca = 1;

int cfm(int p)
{
  if (fma[p] > 0)
    return fma[p];
  int i = p;
  int fm = 0;
  do
  {
    ++fm;
    i = (2 * i) % (q -1);
  }
  while (i != p);
  i = p;
  do
  {
    fma[i] = fm;
    i = (2 * i) % (q -1);
  }
  while (i != p);
  return fm;
}

int fec(int p)
{
  if (feca[p] > 0)
    return feca[p];
  int i = p;
  do
  {
    feca[i] = nfeca;
    i = (2 * i) % (q -1);
  }
  while (i != p);
  if (nfeca > 26)
    std::cerr << "nfeca = " << nfeca << std::endl;
  return nfeca++; 
}

int main()
{
  cout << "<table cellspacing=\"0\" class=\"F16F4\" border=\"1\" cellpadding=\"3\">\n";
  cout << "<tr><th colspan=10>F<sub>2<sup>" << m << "</sup></sub>, t<sup>" << m << "</sup>+t";
  if (k > 1)
    cout << "<sup>" << k << "</sup>";
  if (k1 > 0)
  {
    cout << "+t";
    if (k1 > 1)
      cout << "<sup>" << k1 << "</sup>";
  }
  if (k2 > 0)
  {
    cout << "+t";
    if (k2 > 1)
      cout << "<sup>" << k2 << "</sup>";
  }
  cout << "+1=0</th></tr>\n";

  cout << "<tr>";
  cout << "<th colspan=3>x = g<sup>n</sup></th>";			// Column x
  cout << "<th>" << (q - 1) << "/GCD(n, " << (q - 1) << ")</th>";	// Column (q-1)/GCD(n, q-1)
  cout << "<th colspan=3>1/x</th>";					// Column 1/x
  cout << "<th>Tr(x)</th>";						// Column Tr(x)
  cout << "<th>Tr(1/x)</th>";						// Column Tr(1/x)
  cout << "<th>m/d</th>";						// Column m/d
#if 0
  cout << "<th>x<sup>2</sup>+x+1</th>";					// Column x^2+x+1
  cout << "<th>x<sup>3</sup>+x+1</th>";					// Column x^3+x+1
#endif
  cout << "</tr>\n";

  // Run over all values of x in F_q \ { 0, 1 }.
#if LIBECC_M == 8
  poly_t g(3);
#elif LIBECC_M == 9
  poly_t g(7);
#elif LIBECC_M == 12
  poly_t g(3);
#elif LIBECC_M == 14
  poly_t g(7);
#elif LIBECC_M == 16
  poly_t g(3);
#elif LIBECC_M == 18
  poly_t g(10);
#else
  poly_t g(2);
#endif
  int n = 0;
  for (poly_t x(g); x != one; x *= g)
  {
    ++n;

    poly_t xinv = one / x;
    bool solution = x.trace() == xinv.trace();
    int fm = cfm(n);
    char x_fec = 0, xinv_fec = 0;

    if ((unsigned int)fm != m)
      continue;

    if (solution)
    {
      x_fec = 'A' + fec(n);
      xinv_fec = 'A' + fec(q - 1 - n);
    }

    cout << "<tr>";
    // Column x
    cout << "<td class=\"generator";
    if (solution)
      cout << ' ' << x_fec;
    cout << "\">g<sup>" << n << "</sup></td>";
    cout << "<td class=\"equal";
    if (solution)
      cout << ' ' << x_fec;
    cout << "\">=</td>";
    cout << "<td class=\"element";
    if (solution)
      cout << ' ' << x_fec;
    cout << "\">";
    print_poly_on(cout, x);
    cout << "</td>";
    // Column (q-1)/GCD(n, q-1)
    cout << "<td class=\"star\">" << ((q - 1)/gcd(q - 1, n)) << "</td>";
    // Column 1/x
    cout << "<td class=\"generator";
    if (solution)
      cout << ' ' << xinv_fec;
    cout << "\">g<sup>" << (q - 1 - n) << "</sup></td>";
    cout << "<td class=\"equal";
    if (solution)
      cout << ' ' << xinv_fec;
    cout << "\">=</td><td class=\"element";
    if (solution)
      cout << ' ' << xinv_fec;
    cout << "\">";
    print_poly_on(cout, xinv);
    cout << "</td>";
    // Column Tr(x)
    cout << "<td class=\"star\">" << x.trace() << "</td>";
    // Column Tr(1/x)
    cout << "<td class=\"star\">" << xinv.trace() << "</td>";
    // Column m/d
    cout << "<td class=\"star\">";
    cout << fm;
    cout << "</td>";
#if 0
    // Column x^2+x+1
    cout << "<td>";
    poly_t col3 = x * x;
    col3 += x + one;
    print_poly_on(cout, col3);
    cout << "</td>";
    // Column x^3+x+1
    cout << "<td>";
    poly_t col4 = x * x * x; 
    col4 += x + one;
    print_poly_on(cout, col4);
    cout << "</td>";
#endif
    // End row
    cout << "</tr>\n";
  }
  assert(n == q - 2);
  cout << "</table>\n" << endl;
}
