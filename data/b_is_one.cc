// This program calculates and prints the value of w(m, 1).

#include <iostream>
#include <iomanip>

// The number of elements with Frobenius order of m, over F_{2^m}.
unsigned long foc(int m)
{
  unsigned long result = 1UL << m;	// All elements by default.
  // Run over proper divisors and get rid of the elements with a Frobenius order less than m.
  for (int d = 2; d <= m; ++d)
  {
    if (m % d != 0)
      continue;
    result -= foc(m / d);		// Subtract all elements that are part of a proper subgroup.
  }
  return result;
}

// The number of elements x that have an inverse 1/x (which is every x except 0) that satisfy Tr(x) = Tr(1/x).
// This number is always odd, because z = 1 is always a solution and if z != 1 is a solution then so is 1/z.
unsigned long s(int m)
{
  static unsigned long cache[64];
  if (cache[m])
    return cache[m];

  if (m == 1)
    return 1;				// x = 1 is the only solution.
  if (m == 2)
    return 3;				// The three solutions are: x = 1, x = t and t + 1.
  					// t (t + 1) = t^2 + t = t + 1 + t = 1 --> t and t + 1 are each others inverse.
  					// Tr(t) = t + t^2 = t + t + 1 = 1.
					// Tr(t + 1) = (t + 1) + (t + 1)^2 = t + 1 + t^2 + 1 = t + t + 1 = 1.
  unsigned long result = (1UL << m) - 1;
  result -= 1 + s(m - 1) + 2 * s(m - 2);

  cache[m] = result;
  return result;
}

// The number of elements x with a Frobenius order of m, over F_{2^m}
// that are a solution to y^2 + xy = x^3 + 1.
unsigned long mwm1(int m)
{
  unsigned long result = s(m) + 1;	// The number of elements x that have a solution to y^2 + xy = x^3 + 1.
  					// After all, if x != 0 then y^2 + xy = x^3 + 1 has a solution
					// if (divide by x^2) (y/x)^2 + y/x = x + 1/x^2 has a solution, which has
					// solutions iff Tr(x + 1/x^2) = 0 --> Tr(x) = Tr(1/x), hence the s(m).
  					// The plus 1 takes x = 0 into account.
  // Run over proper divisors and get rid of the elements with a Frobenius order less than m.
  for (int d = 2; d <= m; ++d)
  {
    if (m % d != 0)
      continue;
    if (d % 2 == 0)
      result -= foc(m / d);		// All, because both, the trace of (abc)(abc) as well as its inverse (which is rotated m/2 bits) are always 0.
    else
      result -= mwm1(m / d);		// The number with trace "(abcd)(abcd)(abcd)" equal to trace "(cdab)(cdab)(cdab)" is equal to the number
      					// with trace "(abcd)" equal to trace "(cdab)". And even if m/d is odd, so that the inverse of (abc) = (efg),
					// we still have that the number with trace "(abc)(abc)(abc)" equal to trace "(efg)(efg)(efg)" is equal to
					// the number with trace "(abc)" equal to trace "(efg)".
  }
  return result;
}

int main()
{
  for (int m = 1; m < 64; ++m)
  {
#if 0	// Print html table.
    std::cout << "<tr><td>" << m << "</td><td>" << (2 * s(m) + 2) << "</td></tr>\n";
#elif 0	// Work in progress.
    std::cout << std::setw(2) << m << ": " << std::setw(14) << s(m);
    if (m - 3 >= 1)
      std::cout << std::setw(14) << ((1UL << (m - 1)) - 0 - 1 * s(m - 2) + 2 * s(m - 3));
    if (m - 4 >= 1)
      std::cout << std::setw(14) << ((1UL << m)       - 4 - 3 * s(m - 2) - 4 * s(m - 4));
    std::cout << '\n';
#else	// Do what the header of this file says.
    std::cout << m << ": " << std::setw(14) << (mwm1(m) / m) << '\n';
#endif
  }
  std::cout << '\n';
}

