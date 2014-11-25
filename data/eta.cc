#include <iostream>
#include <gmpxx.h>
#include <cassert>
#include <sys/time.h>
#include <time.h>
#include <iomanip>
#include <cstdlib>

// eta1 -- the first, canonical implementation.
//
// Returns the sum of the multiplication of all elements of
// every combination of the set S = { a, a+1, a+2, ..., n }
// with leaving out i elements (the order not being important).
//
// Note that |S| = n - a + 1.
//
// eta(n, a, i) = 0 if i < 0 (you can't leave out a negative number of elements).
//
// eta(n, a, i) = 0 if i > n - a + 1 (you can't leave out more elements than are in the set).
//
// eta(a, a, 0) = a (S = { a }, not leaving out any element only has a single product: a).
//
// For sane values (n > a, and i not too large), we realize that
//
// eta(n, a, i) = n * eta(n - 1, a, i) + eta(n - 1, a, i - 1)
//
// where the first term takes care of leaving out i elements but not n,
// and the second term takes care of leaving out i elements where n is one of them.
//
// Therefore (with n = a + 1 and i = 1),
//
// eta(a, a, 1) = (eta(a + 1, a, 1) - eta(a, a, 0)) / (a + 1)
//
// and since
//
// eta(a + 1, a, 1) = a + (a + 1)
// eta(a, a, 0) = a
// 
// eta(a, a, 1) = 1, is a logical choice (same reasoning as why 0! = 1 <-- (n-1)! = n!/n).
//
// Moreover (with n = a and i =0),
//
// eta(a - 1, a, 0) = (eta(a, a, 0) - eta(a - 1, a, -1)) / a = a, is also a logical choice.
//
mpz_class eta1(long n, long a, long i)
{
  if (i < 0)			// You cannot leave out a negative number of elements.
    return 0;
  if (i > n - a + 1)		// You can't leave out more elements than are in the set.
    return 0;
  if (n == a)			// This therefore implies that i = 0 or i = 1.
    return (i == 0) ? a : 1;
  if (n < a)			// This can still happen, namely if n = a - 1 and i = 0.
    return a;
  return n * eta1(n - 1, a, i) + eta1(n - 1, a, i - 1);
}

#ifdef TEST
void comment_on(long nma, long a, long i, mpz_class result)
{
  mpz_class correct_result(eta1(nma + a, a, i));
  if (result != correct_result)
  {
    std::cout << "The result for nma = " << nma << ", a = " << a << " and i = " << i <<
        " is incorrect. Result is " << result << " but should be " << correct_result << "." << std::endl;
    exit(1);
  }
}
#endif

mpz_class eta(long n, long a, long i);

mpz_class eta2(long nma, long a, long i)
{
  assert(i >= 0);
  assert((nma & 1) == 0);
  if (i < 2)
  {
    // i = 0 or i = 1
    if (i == 0)
    {
      // No elements are removed; the result is the product of all of S.
      // Use the fact that nma is even.
      long n = nma + a;
      mpz_class fac(n);
      while (n > a) { n -= 2; fac *= (n + 1) * n; }
      return fac;
    }
    // i = 1
    if (nma == 0)
      return 1;		// Convention for when i == |S|.
    if (nma == 2)
      return (3 * a) * (a + 2) + 2;	// a * (a+1) + a * (a+2) + (a+1) * (a+2)
    // nma = 4 + 2k
    // The result can be recursively calculated involving a long product.
    // By doing it in a loop, we can keep track of this product instead
    // of calculating it again every time.
    // If nma == 4, then we have S = { a, a+1, a+2, a+3, a+4 }
    // Leaving out one element and summing the product of that gives,
    // a * (a+1) * (a+2) * (a+3) + a * (a+1) * (a+2) * (a+4) +
    // a * (a+1) * (a+3) * (a+4) + a * (a+2) * (a+3) * (a+4) +
    // (a+1) * (a+2) * (a+3) * (a+4) = 5 a (4 + a) (5 + a (4 + a)) + 24
    // The long product that we need for the next value of nma is
    // a * (a + 1) * (a + 2) * (a + 3) * (a + 4).
    long ni = a + 4;
    long aap4 = a * ni;
    mpz_class prod(aap4);
    mpz_class tmp(5 * aap4);
    prod *= (a + 1) * (a + 2);
    tmp *= 5 + aap4;
    prod *= (a + 3);
    tmp += 24;
    long n = nma + a;
    while(ni < n)
    {
      tmp *= ++ni;
      tmp += prod;
      prod *= ni;
    }
    return tmp;
  }
  if (nma <= i)
  {
    if (nma == i - 1)
      return 1;		// Convention for when i == |S|.
    if (nma < i - 1)
      return 0;		// Can't remove more than |S| elements.
    // nma == i; the result is therefore the sum of all of S = { a, a+1, ..., a + nma }
    return ((nma * (nma + 1) / 2 + a * (nma + 1)));
  }
  long n = nma + a;
  // We would like to subtract half of nma, but it has to stay even and we can't subtract more than i.
  long d = std::min(i, nma / 2) & ~1L;
  nma -= d;
  mpz_class result(eta2(nma, a, i - d));
  for (int j = d - 1; j > 0; --j)
  {
    // eta(n, n - d + 1, j)
    result += (n * eta2(d - 2, n - d + 1, j) + eta2(d - 2, n - d + 1, j - 1)) * eta2(nma, a, i - j);
  }
  result += eta(n, n - d + 1, 0) * eta2(nma, a, i);
  return result;
}

// Optimized version.
mpz_class eta(long n, long a, long i)
{
  long nma = n - a;
  if (i < 0 || nma < i - 1)
    return 0;
  if (nma == 0)
    return (i == 0) ? a : 1;
  if (nma < 0)
    return a;
  if (i == 0)
  {
    mpz_class fac(a);
    while (--nma >= 0) fac *= ++a;
    return fac;
  }
  if ((nma & 1) == 0)
    return n * eta(n - 1, a, i) + eta(n - 1, a, i - 1);
  // Calling eta2 with even nma.
  --nma;
  return n * eta2(nma, a, i) + eta2(nma, a, i - 1);
}

int main(int argc, char* argv[])
{
  long n;
  long a;
  long i;
#ifdef TEST
  for (a = 0; a < 5; ++a)
    for (n = a - 2; n < a + 16; ++n)
      for (i = -1; i < n - a + 2; ++i)
      {
        mpz_class res1 = eta1(n, a, i);
	mpz_class res2 = eta(n, a, i);
	if (res1 != res2)
	{
	  std::cout << "eta1(" << n << ", " << a << ", " << i << ") = " << res1 << std::endl;
	  std::cout << " eta(" << n << ", " << a << ", " << i << ") = " << res2 << std::endl;
        }
      }
#endif
  if (argc != 4)
  {
    std::cerr << "Usage: " << argv[0] << " n a i\n";
    std::cerr << "Calculates the sum of { the product of the elements of S' }, where S' runs\n"
                 "over all sets constructed from the set S = { a, a+1, ..., n } by leaving\n"
		 "out precisely 'i' elements.\n";
    return 0;
  }
  n = atoi(argv[1]);
  a = atoi(argv[2]);
  i = atoi(argv[3]);
  struct timeval before, after;
  gettimeofday(&before, NULL);
  mpz_class res(eta(n, a, i));
  gettimeofday(&after, NULL);
  std::cout << "eta(" << n << ", " << a << ", " << i << ") = " << res << std::endl;
  timersub(&after, &before, &after);
  std::cout << "Computing time: " << after.tv_sec << "." << std::setw(6) << std::setfill('0') << after.tv_usec << " seconds." << std::endl;

  return 0;
}

