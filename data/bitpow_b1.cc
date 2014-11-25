#include <iostream>
#include <iomanip>
#include <more/math/math.h>
#include <stdlib.h>
#include <more/diag/stopwatch.h>


#define REPEAT for (int _rpt = 0; _rpt < 800000; ++_rpt)

template<typename T>
  int testit()
  {
      int stat = 0;
      for (int i = 0; i < 20; ++i) {
	  more::diag::stopwatch sw1, sw2;
	  int n = lrand48()%3 + 2;
	  unsigned long x = lrand48() >>
	      (8*sizeof(unsigned long)-1)
	      - (8*sizeof(unsigned long)-1)/std::max(n, 1);

	  unsigned long z1;
	  sw1.start();
	  REPEAT z1 = more::math::bitpow(x, n);
	  sw1.stop();
	  T z2;
	  sw2.start();
	  REPEAT z2 = more::math::bitpow((T)x, n);
	  sw2.stop();

// 	  std::cout
// 	      << "bitpow(" << x << ", " << n << "):\n"
// 	      << "  " << std::setw(8) << sw1 << ", " << z1 << '\n'
// 	      << "  " << std::setw(8) << sw2 << ", " << z2 << std::endl;
	  std::cout
	      << "bitpow(" << std::setw(8) << x << ", " << n << "): "
	      << std::setw(6) << sw1
	      << std::setw(6) << sw2 << std::endl;
      }
      return stat;
  }

int main()
{
    int stat = 0;
    std::cout << "Benchmarking float.\n";
    stat += testit<float>();
    std::cout << "Benchmarking double.\n";
    stat += testit<double>();
//     std::cout << "Benchmarking long double.\n";
//     stat += testit<long double>();
    return stat;
}
