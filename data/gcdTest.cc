#include "sys.h"
#include "debug.h"
#include <libecc/fieldmath.h>
#include <libecc/rng.h>
#include <libecc/polynomial.h>

using libecc::polynomial;
using libecc::bitset;
using libecc::rng;
using libecc::bitset_digit_t;

template<int m>
  bitset<m>
  randombits(void)
  {
    static rng rng(bitset<521>("BF9001A5C299283918758781041873587129837198"));
    static bitset<512> bitpool;
    static int bitpool_size = 0;
    if (bitpool_size < m)
    {
      rng.generate_512_bits();
      bitpool = rng.get_512_bits();
      bitpool_size = 512 - m;
    }
    else
    {
      bitpool.template shift_op<m, libecc::right, libecc::assign>(bitpool);
      bitpool_size -= m;
    }
    return bitpool;
  }

unsigned int const m = 223;
unsigned int const k = 33;

int main(void)
{
  bitset<m> t0, t1;
  for (int c = 0; c < 1000000; ++c)
  {
    if (c % 1000 == 0)
      std::cout << c << ' ' << std::endl;
    bitset<m> p0 = randombits<m - 64>();
    bitset<m> p1 = randombits<m - 64>();
    p0.set<0>();
    p1.set<0>();
    t0 = p0;
    t1 = p1;
    //std::cout << "GCD("; t0.base2_print_on(std::cout); std::cout << ", "; t1.base2_print_on(std::cout); std::cout << ") = ";
    bitset<m> g1(libecc::gcd(t0, t1));
    //g1.base2_print_on(std::cout); std::cout << std::endl;
    t0 = p1;
    t1 = p0;
    //std::cout << "GCD("; t0.base2_print_on(std::cout); std::cout << ", "; t1.base2_print_on(std::cout); std::cout << ") = ";
    bitset<m> g2(libecc::gcd(t0, t1));
    //g2.base2_print_on(std::cout); std::cout << std::endl;
    assert( g1 == g2 );
    polynomial<m, k> pp0(p0);
    polynomial<m, k> pp1(p1);
    polynomial<m, k> pg(g1);
    polynomial<m, k> pd0(pp0 / pg);
    polynomial<m, k> pd1(pp1 / pg);
    polynomial<1103, 65> b0(pd0.get_bitset());
    polynomial<1103, 65> b1(pd1.get_bitset());
    polynomial<1103, 65> bg(pg.get_bitset());
    polynomial<1103, 65> bp0(b0 * bg);
    polynomial<1103, 65> bp1(b1 * bg);
    bitset<m> rp0(bp0.get_bitset());
    bitset<m> rp1(bp1.get_bitset());
    assert( rp0 == p0 );
    assert( rp1 == p1 );
    //std::cout << "GCD(" << pp0 << ", " << pp1 << ") = GCD(" << pd0 << " * " << pg << ", " << pd1 << " * " << pg << ") = " << pg << std::endl;
    bitset<m> rf = randombits<64>();
    polynomial<1103, 65> brf(rf);
    bp0 *= brf;
    bp1 *= brf;
    bg *= brf;
    rp0 = bp0.get_bitset();
    rp1 = bp1.get_bitset();
    bitset<m> grf(bg.get_bitset());
    //std::cout << "GCD("; rp0.base2_print_on(std::cout); std::cout << ", "; rp1.base2_print_on(std::cout); std::cout << ") = ";
    bitset<m> g3(libecc::gcd(rp0, rp1));
    //g3.base2_print_on(std::cout); std::cout << std::endl;
    assert( g3 == grf );
  }

  return 0;
}
