#include "sys.h"
#include "debug.h"
#include <libecc/rng.h>
#include <libecc/polynomial.h>
#include <vector>
#include <fstream>
#include <cerrno>
#include <cstdlib>

using libecc::polynomial;
using libecc::bitset;
using libecc::rng;
using libecc::bitset_digit_t;

static bool cache_open = false;
static std::fstream cache;

template<int m>
  bitset<m>
  randombits(void)
  {
    static char const* const seed = "36551655982734917230981027487475982738710238019357983744";
    static rng rng((bitset<521>(seed)));
    static bitset<512> bitpool;
    static int bitpool_size = 0;
    if (!cache_open)
    {
      std::string filename = "rng.cache";
      cache.open(filename.c_str());
      if (!cache.is_open())
      {
        cache.clear();
	if (errno == ENOENT)
	  cache.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
	if (!cache.is_open())
	  DoutFatal(dc::fatal|error_cf, "Failed to open " << filename);
      }
      cache_open = true;
    }
    if (bitpool_size < m)
    {
      bool cache_exhausted = false;
      if (!cache_exhausted && !cache.read((char*)bitpool.digits_ptr(), 512 / 8))
      {
	  cache_exhausted = true;
	  cache.clear();
      }
      if (cache_exhausted)
      {
	rng.generate_512_bits();
	bitpool = rng.get_512_bits();
        cache.write((char*)bitpool.digits_ptr(), 512 / 8);
      }
      bitpool_size = 512 - m;
    }
    else
    {
      bitpool.template shift_op<m, libecc::right, libecc::assign>(bitpool);
      bitpool_size -= m;
    }
    return bitpool;
  }

int const test_size = 200;

template<unsigned int m, unsigned int k, unsigned int k1, unsigned int k2>
  void
  test(void)
  {
    static bitset_digit_t const wlen = libecc::bitset_digit_bits;
    static bitset_digit_t const q1 = m / wlen;
    static bitset_digit_t const r1 = m % wlen;
    static bitset_digit_t const q2  = (m - k) / wlen;
    static bitset_digit_t const r2  = (m - k) % wlen;
    static bitset_digit_t const z2 = (2 * (m - 1)) / wlen;

    std::cout << "Testing polynomial<" << m << ", " << k;
    if (k1)
      std::cout << ", " << k1 << ", " << k2;
    std::cout << "> (";
    if (q1 == 0)
      std::cout << "case 1";
    else if (q2 == 0)
      std::cout << "case 2";
    else
    {
      if (r1 == 0)
      {
        std::cout << "case 2.5";
      }
      else if (q1 == q2)
      {
	static unsigned int const z3 = (2 * (m - 1) - ((r1 < r2) ? r1 : r2)) / wlen;
	if (z2 == z3)
	  std::cout << "case 3";
	else
	  std::cout << "case 4";
      }
      else if ((q1 - q2) <= 1)
      {
	std::cout << "case 5 (reducea";
	if (r2 == 0)
	  std::cout << " with r2 == 0)";
	else
	  std::cout << " with r2 > 0)";
      }
      else
      {
	if ((((q1 & 1) + ((q1 - q2) & 1)) <= 1))
	  std::cout << "case 6 (reducem)";
	else
	  std::cout << "case 7 (reduce2)";
      }
    }
    std::cout << ')' << std::endl;

    typedef polynomial<m, k, k1, k2> poly_t;

    poly_t const zero("0");

    poly_t p1;
    poly_t p2;
    poly_t p3;
    poly_t p4;

    std::cout << "Generating random numbers      " << std::flush;
    std::vector<bitset<m> > randomnumbers;
    for (int i = 0; i < 50; ++i)
    {
      for (int j = 0; j < 5 * test_size; ++j)
	randomnumbers.push_back(randombits<m>());
      std::cout << '.' << std::flush;
    }
    std::cout << " OK" << std::endl;
    if (cache_open)
      cache.close();
    typename std::vector<bitset<m> >::iterator randombits_iter = randomnumbers.begin();

    std::cout << "    addition and subtraction   " << std::flush;
    for (int i = 0; i < 50; ++i)
    {
      for (int j = 0; j < test_size; ++j)
      {
	// Addition and subtraction.
	p1 = *randombits_iter++;
	p2 = *randombits_iter++;
	p3 = p1 + p2;
	bitset<m> tmp = p1.get_bitset() ^ p2.get_bitset();
	assert( p3.get_bitset() == tmp );
	p4 = p3 - p1;
	assert( p4 == p2 );
      }
      std::cout << '.' << std::flush;
    }
    std::cout << " OK" << std::endl;

    std::cout << "    square and square-root     " << std::flush;
    for (int i = 0; i < 50; ++i)
    {
      for (int j = 0; j < test_size; ++j)
      {
	// Multiplication and square root.
	p1 = *randombits_iter++;
	libecc::bitset_digit_t tmp[poly_t::square_digits];
	p2 = p1.square(tmp);
	//std::cout << p1 << "^2 = " << p2 << std::endl;
	p4 = p1 * p1;
	assert( p2 == p4 );
	p4.sqrt();
	//std::cout << "sqrt(" << p2 << ") = " << p4 << std::endl;
	assert( p4 == p1 );
      }
      std::cout << '.' << std::flush;
    }
    std::cout << " OK" << std::endl;

    std::cout << "    multiplication and division" << std::flush;
    for (int i = 0; i < 50; ++i)
    {
      for (int j = 0; j < test_size; ++j)
      {
	// Multiplication and division.
	p1 = *randombits_iter++;
	p2 = *randombits_iter++;
	p3 = p1 * p2;
	//std::cout << p1 << " * " << p2 << " = " << p3 << std::endl;
	if (p1 != zero)
	{
	  p4 = p3 / p1;
	  //std::cout << p3 << " / " << p1 << " = " << p4 << std::endl;
	  assert( p4 == p2 );
	}
      }
      std::cout << '.' << std::flush;
    }
    std::cout << " OK" << std::endl;

    std::cout << "    solving x*x + b*x == c     " << std::flush;
    int cnt = 0, cnt2 = 0;
    poly_t x1;
    x1.get_bitset().setall();
    poly_t const g("2");
    poly_t b = poly_t::unity();
    do
    {
      poly_t c = x1 * x1 + b * x1;
      //std::cout << x1 << "^2 + " << b << " * " << x1 << " = " << c << std::endl;
      poly_t x(b, c);
      //std::cout << "x^2 + " << b << " * x = " << c << " --> x = " << x << " or x = " << poly_t(x + b) << std::endl;
      assert( x == x1 || poly_t(x + b) == x1 );
      b *= g;
      if (++cnt == test_size)
      {
	std::cout << '.' << std::flush;
	cnt = 0;
	if (++cnt2 == 50)
	  break;
      }
    }
    while(b != poly_t::unity());
    std::cout << " OK" << std::endl;
  }

int main(void)
{
  Debug(libcw_do.on());
  Debug(debug::init());

  test<3, 1, 0, 0>();
  test<4, 1, 0, 0>();
  test<5, 2, 0, 0>();
  test<6, 1, 0, 0>();
  test<6, 3, 0, 0>();
  test<7, 1, 0, 0>();
  test<8, 4, 3, 1>();
  test<7, 3, 0, 0>();
  test<9, 1, 0, 0>();
  test<9, 4, 0, 0>();
  test<10, 3, 0, 0>();
  test<11, 2, 0, 0>();
  test<12, 3, 0, 0>();
  test<12, 5, 0, 0>();
  test<13, 4, 3, 1>();
  test<14, 5, 0, 0>();
  test<15, 1, 0, 0>();
  test<15, 4, 0, 0>();
  test<15, 7, 0, 0>();
  test<16, 5, 3, 1>();
  test<17, 3, 0, 0>();
  test<17, 5, 0, 0>();
  test<17, 6, 0, 0>();
  test<18, 3, 0, 0>();
  test<18, 7, 0, 0>();
  test<18, 9, 0, 0>();
  test<19, 5, 2, 1>();
  test<20, 3, 0, 0>();
  test<20, 5, 0, 0>();
  test<21, 2, 0, 0>();
  test<21, 7, 0, 0>();
  test<22, 1, 0, 0>();
  test<23, 5, 0, 0>();
  test<23, 9, 0, 0>();
  test<24, 4, 3, 1>();
  test<25, 3, 0, 0>();
  test<25, 7, 0, 0>();
  test<25, 3, 0, 0>();
  test<26, 4, 3, 1>();
  test<27, 5, 2, 1>();
  test<28, 1, 0, 0>();
  test<28, 3, 0, 0>();
  test<28, 9, 0, 0>();
  test<28, 13, 0, 0>();
  test<29, 2, 0, 0>();
  test<30, 1, 0, 0>();
  test<30, 9, 0, 0>();
  test<31, 3, 0, 0>();
  test<31, 6, 0, 0>();
  test<31, 7, 0, 0>();
  test<31, 13, 0, 0>();
  test<32, 7, 3, 2>();
  test<37, 6, 4, 1>();
  test<33, 10, 0, 0>();		// Not primitive.
  test<35, 2, 0, 0>();
  test<36, 9, 0, 0>();		// Not primitive.
  test<40, 5, 4, 3>();
  test<34, 7, 0, 0>();
  test<36, 15, 0, 0>();
  test<39, 14, 0, 0>();
  test<41, 20, 0, 0>();
  test<47, 21, 0, 0>();
  test<49, 22, 0, 0>();
  test<52, 21, 0, 0>();
  test<55, 24, 0, 0>();
  test<49, 9, 0, 0>();
  test<58, 19, 0, 0>();
  test<60, 1, 0, 0>();
  test<62, 29, 0, 0>();		// Not primitive.
  test<63, 1, 0, 0>();
  test<64, 4, 3, 1>();
  test<65, 18, 0, 0>();
  test<65, 33, 0, 0>();		// Not irreducible!!!
  test<71, 6, 0, 0>();
  test<81, 4, 0, 0>();
  test<90, 27, 0, 0>();		// Not primitive.
  test<97, 6, 0, 0>();
  test<97, 33, 0, 0>();
  test<113, 9, 0, 0>();
  test<118, 33, 0, 0>();
  test<129, 5, 0, 0>();
  test<134, 57, 0, 0>();
  test<145, 52, 0, 0>();
  test<147, 14, 0, 0>();	// Not irreducible!!!
  test<154, 15, 0, 0>();	// Not primitive.
  test<155, 62, 0, 0>();	// Not irreducible!!!
  test<164, 10, 8, 7>();
  test<233, 74, 0, 0>();
  test<242, 95, 0, 0>();	// Not primitive.
  test<161, 18, 0, 0>();
  test<161, 39, 0, 0>();
  test<1196, 519, 0, 0>();
  test<1779, 30, 27, 17>();
  test<3520, 32, 29, 3>();
  test<6016, 35, 34, 2>();
  test<7776, 41, 39, 36>();
  return 0;
}
