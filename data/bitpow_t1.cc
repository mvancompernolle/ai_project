#include <iostream>
#include <iomanip>
#include <more/math/math.h>
#include <stdlib.h>
#include <limits>


template<typename T>
  inline bool
  flteq(T x, T y, T eps = std::numeric_limits<T>::epsilon()) {
      return std::fabs(x) <= eps && std::fabs(y) <= eps
	  || std::fabs(x - y) <= 2.0*eps*std::fabs(x);
  }


template<typename T>
  int testit(const char* name) {
      int stat = 0;
      int cnt = 40;
      std::cerr.precision(std::numeric_limits<T>::digits10);
      for (int i = 0; i < 10000; ++i) {
	  int n = lrand48()%4;
	  unsigned long x = lrand48() >>
	      (8*sizeof(unsigned long)-1)
	      - (8*sizeof(unsigned long)-1)/std::max(n, 1);
	  unsigned long z1 = more::math::bitpow(x, n);
	  T z2 = more::math::bitpow(static_cast<T>(x), n);
	  if (!flteq((T)z1, z2) && cnt) {
	      stat = 1;
	      if (!--cnt)
		  std::cerr << "*** More errors." << std::endl;
	      else
		  std::cerr
		      << "*** " << name << ", #" << i
		      << ": bitpow(" << x << ", " << n << "): "
		      << z1 << " != " << z2 << std::endl;
	  }
      }
      return stat;
  }

int main() {
    int stat = 0;
    stat += testit<float>("float");
    stat += testit<double>("double");
//     stat += testit<long double>("long double");
    return stat;
}
