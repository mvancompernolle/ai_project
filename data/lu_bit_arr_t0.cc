// Result of this benchmark:
//
//   Platform:	"Athlon 500 MHz 256 Mb mem (32 bit ints)", "gcc -O2 ..."
//   Result 0:	"algo: 0.73 s; arr: 0.42 s" with prime = 1 below)
//   Pesult 1:	"algo: 0.72 s; arr: 1.49 s" with prime = 424429 below
//
// So, the memory access time of the array-based version outweights
// the extra operations needed by the algorithmic version, as result 0
// is a very artificial case.


#include <more/math/math.h>
#include <more/diag/debug.h>
#include <cstdlib>

namespace more {
namespace math {
  extern int lower_bit_arr[];
  extern int upper_bit_arr[];

  inline int
  lower_bit_32(unsigned int n)
  {
      if (n == 0)
	  throw std::domain_error("Don't give me that.");
      if (n & 0xffff)
	  return lower_bit_arr[n & 0xffff];
      else
	  return lower_bit_arr[n >> 16] + 16;
  }

  inline int
  upper_bit_32(unsigned int n)
  {
      if (n == 0)
	  throw std::domain_error("Don't give me that.");
      if (n & 0xffff0000)
	  return upper_bit_arr[n >> 16] + 16;
      else
	  return upper_bit_arr[n];
  }
}}

int
main()
{
    using more::math::lower_bit;
    using more::math::upper_bit;

    int const n_bm = 10000000;
    int const prime = 424429;
    int sum = 0;
    clock_t c = -clock();
    for (int i = 1; i < n_bm; ++i) {
	unsigned int n = i*prime;
	int l = lower_bit(n);
	int u = upper_bit(n);
	sum += l + u;
    }
    c += clock();

    sum = 0;
    clock_t c0 = -clock();
    for (int i = 1; i < n_bm; ++i) {
	unsigned int n = i*prime;
	int l = more::math::lower_bit_32(n & 0xffffffff);
	int u = more::math::upper_bit_32(n & 0xffffffff);
	sum += l + u;
    }
    c0 += clock();

    std::cout << "Benchmark: algo: "
	      << c/(double)CLOCKS_PER_SEC << " s; arr: "
	      << c0/(double)CLOCKS_PER_SEC << " s\n";

    return 0;
}
