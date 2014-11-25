#include <iostream>
#include <more/math/qtypes.h>

#define CK(a, b) ckeq(a, b, #a, #b)

using namespace more;

int ret = 0;

void ckeq(math::halfint j1, math::halfint j2,
	  char const* sj1, char const* sj2)
  {
      if (j1.twice != j2.twice) {
	  std::cerr << "Failed: " << sj1 << " != " << sj2 << '\n';
	  ret = 1;
      }
  }
template<typename T>
  void ckeq(T x1, T x2, char const* sx1, char const* sx2)
  {
      if (x1 != x2) {
	  std::cerr << "Failed: " << sx1 << " != " << sx2 << '\n';
	  ret = 1;
      }
  }

void
testit(math::halfint j1, math::halfint j2, math::halfint j3, int a)
{
    using math::abs;

    CK(j1 + j2, j2 + j1);
    CK((j1 + j2) + j3, j1 + (j2 + j3));
    CK(j1 + j1, 2*j1);
    CK(1*j1, j1);
    CK(0*j1, 0*j2);
    CK(a*(j1 + j2), a*j1 + a*j2);
    CK(a*j1, j1*a);

    CK(j1 - j1, 0);
    CK(j1 - j2, j1 + (-j2));
    CK((-1)*j1, -j1);

    CK(j1 < j2, j1.twice < j2.twice);
    CK(j1 < j2, j2 > j1);
    CK(j1 < j2, !(j1 >= j2));
    CK(j1 > j2, !(j1 <= j2));
    CK(j1 == j2, !(j1 != j2));

    CK(abs(j1), abs(-j1));
    CK(abs(a*j1), abs(a)*abs(j1));
    CK(abs(j1) >= 0, true);
    CK(abs(j1) == j1 || abs(j1) == -j1, true);
}

int
main()
{
    using math::half;
    math::halfint omega = -3*half;
    std::cout << omega << ' ' << -omega << ' ' << abs(omega) << std::endl;
    int imax = 6;
    for (int i1 = -imax; i1 <= imax; ++i1)
	for (int i2 = -imax; i2 <= imax; ++i2)
	    for (int i3 = -imax; i3 <= imax; ++i3)
		testit(i1*half, i2*half, i3*half, -7);
    return ret;
}
