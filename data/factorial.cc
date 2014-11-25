#include <iostream>
#include <gmpxx.h>
#include <sys/time.h>
#include <time.h>
#include <iomanip>
#include <cassert>
#include <limits>
#include <cstdlib>
#include <stdint.h>

#undef DEBUG
#define DEBUG 0
#define STATS 1

#if STATS
int int_int_mult;
int mpz_int_mult;
int mpz_mpz_mult;
#endif

// C/C++ implementation of the binary-split formula of the factorial.
// Copied from http://www.luschny.de/math/factorial/binarysplitfact.html

int nminusnumofbits(int v)
{
  long w = (long)v;
  w -= (0xaaaaaaaa & w) >> 1;
  w = (w & 0x33333333) + ((w >> 2) & 0x33333333);
  w = (w + (w >> 4)) & 0x0f0f0f0f;
  w += w >> 8;
  w += w >> 16;
  return v - (int)(w & 0xff);
}

void partProduct_assign(int n, int m, mpz_class& r);

int tmpc = 0;
mpz_class tmp[32];

void partProduct_multiply(int n, int m, mpz_class& r)
{
  if (m == (n + 2))
  {
    r *= n * m; 
#if STATS
    ++int_int_mult;
    ++mpz_int_mult;
#endif
  }
  else if (m <= (n + 1))
  {
    r *= n;
#if STATS
    ++mpz_int_mult;
#endif
  }
  else
  {
    int k = ((n + m) / 2 - 1) | 1;
    assert(tmpc < 32);
    partProduct_assign(n, k, tmp[tmpc++]);
    partProduct_multiply(k + 2, m, tmp[tmpc - 1]);
    r *= tmp[--tmpc];
#if STATS
    ++mpz_mpz_mult;
#endif
  }
}

void partProduct_assign(int n, int m, mpz_class& r)
{
  if (m == (n + 2))
  {
    r = n * m; 
#if STATS
    ++int_int_mult;
#endif
  }
  else if (m <= (n + 1))
    r = n;
  else
  {
    int k = ((n + m) / 2 - 1) | 1;
    partProduct_assign(n, k, r);
    partProduct_multiply(k + 2, m, r);
  }
}

mpz_class naive_factorial(int n)
{
  if (n == 0)
    return 1;
  mpz_class r(n);
  while (n > 1)
    r *= --n;
  return r;
}

mpz_class factorial(int n)
{
  mpz_class r(1);
  int z = nminusnumofbits(n);
#if 0
  // Speed up for small numbers.
  //                       3  4   5   6    7    8     9     10      11      12       13        14         15         16
  static long data[14] = { 3, 3, 15, 45, 315, 315, 2835, 14175, 155925, 467775, 6081075, 42567525, 638512875, 638512875 };

  if (n <= 16)
  {
    if (n > 2)
      r = data[n - 3];
  }
  else
#endif
  {
    int m[sizeof(int) * 8];
    int j = 0;
    do
    {
      m[j++] = n;
      n >>= 1;
    }
    while (n > 2);
    m[j] = n;
    mpz_class p;
    partProduct_assign((n + 1) | 1, (m[--j] - 1) | 1, p);
    r = p;
    do
    {
      partProduct_multiply((m[j] + 1) | 1, (m[j - 1] - 1) | 1, p);
      r *= p;
#if STATS
      ++mpz_mpz_mult;
#endif
    }
    while (--j > 0);
  }
  return r << z;
}

class BinaryProduct {
  private:
    static mpz_class S_stack[32];
    static uint32_t S_mask;

  public:
    BinaryProduct(void) { S_mask = 0; }

    void multiply(mpz_class const& n, int level)
    {
      int lvl = 1 << level;
      if ((S_mask & lvl) == 0)
      {
	S_stack[level] = n;
	S_mask |= lvl;
      }
      else
      {
#if DEBUG
	std::cout << "Multiplying " << S_stack[level] << " * " << n << " = ";
#endif
	S_stack[level] *= n;
#if STATS
        ++mpz_mpz_mult;
#endif
#if DEBUG
	std::cout << S_stack[level] << " (level " << level << ")" << std::endl;
#endif
	multiply(S_stack[level], level + 1);
	S_mask &= ~lvl;
      }
    }

    void multiply(unsigned int n, int level)
    {
      int lvl = 1 << level;
      if ((S_mask & lvl) == 0)
      {
	S_stack[level] = n;
	S_mask |= lvl;
      }
      else
      {
#if DEBUG
	std::cout << "Multiplying " << S_stack[level] << " * " << n << " = ";
#endif
	S_stack[level] *= n;
#if STATS
        ++mpz_int_mult;
#endif
#if DEBUG
	std::cout << S_stack[level] << " (level " << level << ")" << std::endl;
#endif
	multiply(S_stack[level], level + 1);
	S_mask &= ~lvl;
      }
    }

    mpz_class& collect(void)
    {
      int level = 0;
      int last_level = -1;
      uint32_t lvl = 1;
      do
      {
	if ((S_mask & lvl))
	{
	  if (last_level != -1)
	  {
#if DEBUG
	    std::cout << "Collecting " << S_stack[level] << " * " << S_stack[last_level] << " = ";
#endif
	    S_stack[level] *= S_stack[last_level];
#if STATS
	    ++mpz_mpz_mult;
#endif
#if DEBUG
	    std::cout << S_stack[level] << std::endl;
#endif
	  }
	  last_level = level;
	}
	++level;
	lvl <<= 1;
      }
      while (lvl);
      return S_stack[last_level];
    }
};

mpz_class BinaryProduct::S_stack[32];
uint32_t BinaryProduct::S_mask;

int multiply4(int a, int n4, BinaryProduct& bp)
{
  // Calculate the product a * (a + 1) * ... * (a + n4 - 1)

  // We have to calculate the product of a * (a + 1) * ... * (a + 4n - 1)
  unsigned int const n = n4 >> 2;

  // We will calculate the product of four terms at once, starting and then proceeding as follows.
  // a * ...   ... * (a + 2n - 1) * (a + 2n) * ...   ... * (a + 4n - 1)
  // \--->             <---/           \--->                <---/

  // We'll frequently need twice the square of n.
#if DEBUG
  assert(n < 46341);
#endif
  unsigned long const dn2 = (unsigned long)n * n * 2;

  unsigned int k = 0;	// The k-th quadruplex.

  // Calculate a * (a + 4n - 1) / 2.
  unsigned long c1 = a * (n4 - 1 + a) >> 1;
#if STATS
  ++int_int_mult;
#endif
#if DEBUG
  assert(c1 * 2 / a == n4 - 1 + a);
#endif

  // Calculate (a + 2n - 1) * (a + 2n) / 2.
  unsigned long c2 = c1 + dn2 - n;
#if DEBUG
  assert(c2 > c1 && c2 > dn2 - n);
#endif

  // Calculate the 0-th quadruplex.
  mpz_class p4(c1); p4 *= c2;
#if STATS
  ++mpz_int_mult;
#endif

  bp.multiply(p4, 2);

#if DEBUG
  std::cout << "c2 = " << c2 << std::endl;
  std::cout << "(" << (a + k) << " * " << (a + n4 - k - 1) << ") * (" << (a + 2 * n - k - 1) << " * " << (a + 2 * n + k) << ") = " << 4 * p4 << std::endl;
#endif

  // Knowing the k-th quadruplex, (a + k) * (a + 4n - 1 - k) * (a + 2n - 1 - k) * (a + 2n + k),
  // we can calculate the (k+1)th quadruplex, (a + (k + 1)) * (a + 4n - 1 - (k + 1)) * (a + 2n - 1 - (k + 1)) * (a + 2n + (k + 1)),
  // by adding the difference, which is, 4(1 + k - n)(1 + a - a^2 + k^2 - 2k(-1 + n) - 4an - 4n^2)
  // The initial value, for k=0, is thus, 4(1 - n)(1 + a - a^2 - 4an - 4n^2) = (4n - 4)(c2 + 2n - 1).
  // Also in this case, actually use 1/4 of that.
  mpz_class d = (n - 1); d *= 2 * (c2 + n) - 1;
#if STATS
  ++mpz_int_mult;
#endif

  // As this d is also a function of k, we can write d(k+1) = d(k) - dd(k),
  // where dd(k) = 4 (a^2 + a (4n - 1) + (2n^2 + 7n - 7 - 3k^2 - (9 - 6n)k))
  // so that the initial value is, 4 (a^2 + a (4n - 1) + (2n^2 + 7n - 7)).
  // Also in this case, actually use 1/4 of that.
  //
  // The final value of dd, for k=n-1, will be (-1 + a^2 - 2n + 5n^2 + a(-1 + 4n)) which
  // is less than (a^2 + 5n^2 + 4.472 an) = (a + sqrt(5) * n)^2, so if a + 3n < 2^16
  // then it should fit in an unsigned long.

  // Finally, dd(k+1) = dd(k) + ddd(k), where ddd(k) = 24 (n - k - 2).
  // Also in this case, actually use 1/4 of that.
  unsigned long ddd = 6 * (n - 2);
#if STATS
  ++int_int_mult;
#endif

  if ((unsigned long)a + 3 * n < (1UL << (sizeof(unsigned long) * 4)))
  {
    unsigned long dd = 2 * c1 + dn2 + 7 * (n - 1);
#if STATS
    int_int_mult += 2;
#endif

#if DEBUG
    std::cout << "d = " << d << std::endl;
    std::cout << "dd = " << dd << std::endl;
    std::cout << "ddd = " << ddd << std::endl;
#endif

    // There are in total n quadruplexes.
    while (++k < n)
    {
      // Calculate the next quadruplex.
      p4 += d;

#if DEBUG
      mpz_class quadruple = (a + k) * (a + n4 - k - 1); quadruple *= (a + 2 * n - k - 1) * (a + 2 * n + k);
      std::cout << "(" << (a + k) << " * " << (a + n4 - k - 1) << ") * (" <<
			  (a + 2 * n - k - 1) << " * " << (a + 2 * n + k) << ") = " << 4 * p4 << std::endl;
      assert(4 * p4 == quadruple);
#endif

      bp.multiply(p4, 2);

      // Update the differences.
      d -= dd;
      dd += ddd;
      ddd -= 6;
    }
  }
  else
  {
    mpz_class dd(dn2); dd += (7 * (n - 1)); dd += 2 * c1;
#if STATS
    ++int_int_mult;
#endif
    while (++k < n)
    {
      p4 += d;

      bp.multiply(p4, 2);

#if DEBUG
      mpz_class quadruple = (a + k); quadruple *= (a + n4 - k - 1); quadruple *= (a + 2 * n - k - 1); quadruple *= (a + 2 * n + k);
      assert(p4 == quadruple);
#endif

      d -= dd;
      dd += ddd;
      ddd -= 6;
    }
  }
  return 2 * n;
}

int multiply8(int a, int n8, BinaryProduct& bp)
{
  // Calculate the product a * (a + 1) * ... * (a + n8 - 1)

  // We have to calculate the product of a * (a + 1) * ... * (a + 8n - 1)
  int const n = n8 >> 3;

  // We will calculate the product of eight terms at once, starting and then proceeding as follows.
  // a * ...   ... * (a + 2n - 1) * (a + 2n) * ...   ... * (a + 4n - 1) * (a + 4n) * ...   ... * (a + 6n - 1) * (a + 6n) * ...   ... * (a + 8n - 1)
  // \--->             <---/           \--->                <---/            \--->                <---/            \--->                <---/

  // The k-th product is:
  //
  // (a + k) * (a + 2n - 1 - k) * (a + 2n + k) * (a + 4n - 1 - k) * (a + 4n +k) * (a + 6n - 1 - k) * (a + 6n + k) * (a + 8n - 1 - k).
  //
  // Thus
  //
  // d1(k) = (1 + a + k) (-2 + a - k + 2 n) (1 + a + k + 2 n) (-2 + a - k + 4 n)
  //         (1 + a + k + 4 n) (-2 + a - k + 6 n) (1 + a + k + 6 n) (-2 + a - k + 8 n) -
  //         (a + k) (-1 + a - k + 2 n) (a + k + 2 n) (-1 + a - k + 4 n) (a + k + 4 n)
  //         (-1 + a - k + 6 n) (a + k + 6 n) (-1 + a - k + 8 n)
  // d2(k) = (2 + a + k) (-3 + a - k + 2 n) (2 + a + k + 2 n) (-3 + a - k + 4 n)
  //         (2 + a + k + 4 n) (-3 + a - k + 6 n) (2 + a + k + 6 n) (-3 + a - k + 8 n) - 
  //         2 (1 + a + k) (-2 + a - k + 2 n) (1 + a + k + 2 n) (-2 + a - k + 4 n)
  //         (1 + a + k + 4 n) (-2 + a - k + 6 n) (1 + a + k + 6 n) (-2 + a - k + 8 n) +
  //         (a + k) (-1 + a - k + 2 n) (a + k + 2 n) (-1 + a - k + 4 n) (a + k + 4 n)
  //         (-1 + a - k + 6 n) (a + k + 6 n) (-1 + a - k + 8 n)
  // d3(k) = 48 (2 + k - n) (3 a^4 + 7 k^4 - 28 k^3 (-2 + n) + 6 a^3 (-1 + 8 n) -
  //         a^2 (43 + 10 k^2 - 20 k (-2 + n) + 32 n - 288 n^2) +
  //         k^2 (183 - 128 n - 168 n^2) +
  //         2 a (-1 + 8 n) (-23 - 5 k^2 + 10 k (-2 + n) + 8 n + 48 n^2) +
  //         k (284 - 206 n - 752 n^2 + 392 n^3) +
  //         4 (44 - 25 n - 225 n^2 + 100 n^3 + 196 n^4))
  // d4(k) = 48 (3 a^4 + 35 k^4 - 70 k^3 (-5 + 2 n) + 6 a^3 (-1 + 8 n) -
  //         5 k^2 (-275 + 186 n + 84 n^2) + 10 k (250 - 215 n - 234 n^2 + 112 n^3) + 
  //         2 (883 - 858 n - 1649 n^2 + 1248 n^3 + 196 n^4) + 
  //         2 a (-1 + 8 n) (-98 - 15 k^2 + 63 n + 38 n^2 + 15 k (-5 + 2 n)) + 
  //         a^2 (-193 - 30 k^2 + 78 n + 268 n^2 + 30 k (-5 + 2 n)))
  // d5(k) = -960 (3 + k - n) (-71 + 3 a^2 - 7 k^2 + 14 k (-3 + n) + 30 n + 56 n^2 + 3 a (-1 + 8 n))
  // d6(k) = -2880 (-89 + a^2 - 7 k^2 + 45 n + 14 n^2 + 7 k (-7 + 2 n) + a (-1 + 8 n))
  // d7(k) = 40320 (4 + k - n)
  //
  // Thus, for the initialization values we have
  //
  // d7 = 40320 (4 - n)
  // d6 = -2880 (-89 + a^2 + 45 n + 14 n^2 + a (-1 + 8 n))
  // d5 = 960 (-3 + n) (-71 + 3 a^2 + 30 n + 56 n^2 + 3 a (-1 + 8 n))
  // d4 = 48 (3 a^4 + 6 a^3 (-1 + 8 n) + a^2 (-193 + 78 n + 268 n^2) +
  //      2 a (98 - 847 n + 466 n^2 + 304 n^3) + 2 (883 - 858 n - 1649 n^2 + 1248 n^3 + 196 n^4))
  // d3 = -48 (-2 + n) (3 a^4 + 6 a^3 (-1 + 8 n) + a^2 (-43 - 32 n + 288 n^2) +
  //      a (46 - 384 n + 32 n^2 + 768 n^3) + 4 (44 - 25 n - 225 n^2 + 100 n^3 + 196 n^4))
  // d2 = 1264 + 800 a - 632 a^2 - 328 a^3 + 144 a^4 + 24 a^5 - 8 a^6 - 928 n - 6928 a n -
  //      2280 a^2 n + 2736 a^3 n + 264 a^4 n - 192 a^5 n - 16112 n^2 - 1392 a n^2 +
  //      18016 a^2 n^2 + 160 a^3 n^2 - 1808 a^4 n^2 + 15200 n^3 + 49664 a n^3 -
  //      8064 a^2 n^3 - 8448 a^3 n^3 + 45376 n^4 - 35264 a n^4 - 20032 a^2 n^4 -
  //      45952 n^5 - 20992 a n^5 - 4608 n^6
  // d1 = 16 + 32 a - 8 a^2 - 40 a^3 + 24 a^5 - 8 a^6 + 32 n - 208 a n - 408 a^2 n +
  //      136 a^3 n + 432 a^4 n - 216 a^5 n + 8 a^6 n - 656 n^2 - 1200 a n^2 +
  //      1600 a^2 n^2 + 2848 a^3 n^2 - 2288 a^4 n^2 + 192 a^5 n^2 - 352 n^3 +
  //      6304 a n^3 + 7968 a^2 n^3 - 12160 a^3 n^3 + 1856 a^4 n^3 + 7744 n^4 +
  //      6976 a n^4 - 33856 a^2 n^4 + 9216 a^3 n^4 - 3712 n^5 - 45696 a n^5 +
  //      24704 a^2 n^5 - 21504 n^6 + 33792 a n^6 + 18432 n^7
  //
  // The problem is, how to calculate that in an efficient way.
  // Lets see...
  //

  static mpz_class r[9], p[9], d1, d2, d3, d4, d5, d6;

  int const mi = std::min(9, n);
  unsigned long di1 = n - 1;
  unsigned long i1 = a + 2 * n - 1;
  p[0] = r[0] = (i1 * a) >> 1;		// Can always be devided by 2, either a is even, or i1 will be even.
#if STATS
  ++int_int_mult;
#endif
#if DEBUG
  std::cout << "Factor of p[0]: " << a  << " * " << (a + 2 * n - 1) << " = " << 2 * r[0] << std::endl;
  assert(a * (a + 2 * n - 1) == 2 * r[0]);
#endif
  for (int i = 1; i < mi; ++i)
  {
    r[i] = r[i - 1];
    r[i] += di1;
#if DEBUG
    std::cout << "Factor of p[" << i << "]: " << (a + i) << " * " << (a + 2 * n - 1 - i) << " = " << 2 * r[i] << std::endl;
    assert((a + i) * (a + 2 * n - 1 - i) == 2 * r[i]);
#endif
    --di1;
    p[i] = r[i];
  }
  unsigned long n2 = n * n;
#if STATS
  ++int_int_mult;
#endif
  d1 = i1;
  d1 *= 2 * n;
#if STATS
  ++mpz_int_mult;
#endif
  d1 += n;
  d2 = n2;
  d2 <<= 2;
  for (int k = 1; k < 4; ++k)
  {
    for (int i = 0; i < mi; ++i)
    {
      r[i] += d1;
#if DEBUG
      std::cout << "Factor of p[" << i << "]: " << (a + i + 2 * n * k) << " * " << (a + 2 * n - 1 - i + 2 * n * k) << " = " << 2 * r[i] << std::endl;
      assert((a + i + 2 * n * k) * (a + 2 * n - 1 - i + 2 * n * k) == 2 * r[i]);
#endif
      p[i] *= r[i];
#if STATS
      ++mpz_mpz_mult;
#endif
    }
    d1 += d2;
  }

  for (int i = 0; i < mi; ++i)
    bp.multiply(p[i], 3);

  if (n <= mi)
    return 4 * mi;	// Each p[i] is a factor 2^4 too small.

  // Let the magic begin...

  d1 = p[8] - p[7];	// d1[7]
  d2 = p[6] - p[7];
  d3 = p[6] - p[5];
  d4 = p[4] - p[5];
  d5 = p[4] - p[3];
  d6 = p[2] - p[3];

  d6 += d5;

  d5 += d4;
  d6 += d5;

  d4 += d3;
  d5 += d4;
  d6 += d5;

  d3 += d2;
  d4 += d3;
  d5 += d4;
  d6 += d5;

  d2 += d1;		// d2[6]
  d3 += d2;		// d3[5]
  d4 += d3;		// d4[4]
  d5 += d4;		// d5[3]
  d6 += d5;		// d6[2]

  long d7 = (n - 6) * -2520;		// d7[2]
#if STATS
  ++int_int_mult;
#endif
  unsigned long const d8 = 2520;

  d6 += d7;		// d6[3]
  d5 += d6;		// d5[4]
  d4 += d5;		// d4[5]
  d3 += d4;		// d3[6]
  d2 += d3;		// d2[7]

  d7 += d8;		// d7[3]
  d6 += d7;		// d6[4]
  d5 += d6;		// d5[5]
  d4 += d5;		// d4[6]
  d3 += d4;		// d3[7]

  d7 += d8;		// d7[4]
  d6 += d7;		// d6[5]
  d5 += d6;		// d5[6]
  d4 += d5;		// d4[7]

  d7 += d8;		// d7[5]
  d6 += d7;		// d6[6]
  d5 += d6;		// d5[7]

  d7 += d8;		// d7[6]
  d6 += d7;		// d6[7]

  d7 += d8;		// d7[7]

  for (int k = 9; k < n; ++k)
  {
    d1 += d2;		// d1[8]
    d2 += d3;		// d2[8]
    d3 += d4;		// d3[8]
    d4 += d5;		// d4[8]
    d5 += d6;		// d5[8]
    d6 += d7;		// d6[8]
    d7 += d8;		// d7[8]
    p[8] += d1;
    bp.multiply(p[8], 3);
  }

  return 4 * n;
}

mpz_class product8(int a, unsigned int n)
{
  unsigned int n8 = n & ~7U;
  BinaryProduct bp;
  int pwr2 = 0;
  for (unsigned int f = a; f < a + ((n - n8) & ~1); f += 2)
  {
    bp.multiply(f * (f + 1), 1);
#if STATS
    ++int_int_mult;
#endif
  }
  if ((n & 1) == 1)
    bp.multiply(a + n - n8 - 1, 0);
  if (n8)
    pwr2 = multiply8(a + n - n8, n8, bp);
  mpz_class res(bp.collect());
  if (pwr2)
    res <<= pwr2;
  return res;
}

mpz_class product4(int a, unsigned int n)
{
  unsigned int n4 = n & ~3U;
  BinaryProduct bp;
  int pwr2 = 0;
  for (unsigned int f = a; f < a + ((n - n4) & ~1); f += 2)
  {
    bp.multiply(f * (f + 1), 1);
#if STATS
    ++int_int_mult;
#endif
  }
  if ((n & 1) == 1)
    bp.multiply(a + n - n4 - 1, 0);
  if (n4)
    pwr2 = multiply4(a + n - n4, n4, bp);
  mpz_class res(bp.collect());
  if (pwr2)
    res <<= pwr2;
  return res;
}

mpz_class little(int a, unsigned n)
{
  BinaryProduct bp;

  // If a divisible by 2?
  int t = a % 30;
  
  static int const s30[30] = { 0, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
  // First multiply all numbers that are divisible by 2*3*5.
  for (unsigned int i = a + s30[t], j = (a - s30[t]) / 30; i < a + n; i += 30, ++j)
    bp.multiply(i, 0); 
  static int const s15[30] = { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16 };
  // Then multiply all numbers that are divisible by 3*5.
  for (unsigned int i = a + s15[t], j = (a - s30[t]) / 30; i < a + n; i += 30, ++j)
    bp.multiply(i, 0); 
  static int const s10[30] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 19, 18, 17, 16, 15, 14, 13, 12, 11 };
  // Then multiply all numbers that are divisible by 2*5.
  for (unsigned int i = a + s10[t], j = (a - s30[t]) / 30; i < a + n; i += 30, ++j)
    bp.multiply(i, 0); 
  return bp.collect();
}

int main(int argc, char* argv[])
{
  if (argc != 4)
  {
    std::cerr << "Usage: " << argv[0] << " a n b" << std::endl;
    return 1;
  }
  int a = atoi(argv[1]);
  int n = atoi(argv[2]);
  int b = atoi(argv[3]);
  assert(n >= a);
  mpz_class res;
  struct timeval before, after;
  int const s = 1; //20000;
  int c = 0;//for (int c = 0; c < (!b ? 2 : 1); ++c)
  {
#if STATS
    int_int_mult = 0;
    mpz_int_mult = 0;
    mpz_mpz_mult = 0;
#endif
    gettimeofday(&before, NULL);
    if (!b)
    {
      if (c == 0)
	for (int j = 0; j < s; ++j)
	  res = little(a, n - a + 1);
      else if (c == 1)
	for (int j = 0; j < s; ++j)
	  res = product4(a, n - a + 1);
      else if (c == 2)
	for (int j = 0; j < s; ++j)
	  res = product8(a, n - a + 1);
    }
    else
      for (int j = 0; j < s; ++j)
	res = factorial(n);
    gettimeofday(&after, NULL);
    timersub(&after, &before, &after);
    if (!b)
      std::cout << n << "!/" << (a - 1) << "! = " << res << std::endl;
    else
      std::cout << n << "! = " << res << std::endl;
    mpz_class t = after.tv_sec;
    t *= 1000000;
    t += after.tv_usec;
    t /= s;
    std::cout << "Computing time: " << t << " microseconds." << std::endl;
#if STATS
    std::cout << "int * int: " << int_int_mult / s << "; mpz * int: " << mpz_int_mult / s << "; mpz * mpz: " << mpz_mpz_mult / s << std::endl;
#endif
  }
}
