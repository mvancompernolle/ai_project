#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>
#include <stdlib.h>

#define PRECISION 1e-5
#define TEST(x, y) (test((x), (y)) || test_error(#x, #y, (x), (y)))

bool test_failed_flag = false;

template<typename T, typename U>
  bool
  test_error(const char* strx, const char* stry,
	     const T& x, const U& y)
  {
      std::cerr << "Test failed!  " << strx << " == " << stry << '\n'
	  //<< std::setw(30) << typeid(T).name()
		<< "  lhs = " << x << '\n'
	  //<< std::setw(30) << typeid(U).name()
		<< "  rhs = " << y << std::endl;
      test_failed_flag = true;
      return true;
  }

template<typename T>
  bool
  test(const T& x, const T& y)
  {
      return std::abs(x - y) < PRECISION ||
	(std::abs(x - y) < PRECISION*std::abs(x));
  }


template<typename T> inline T pow2(T x) { return x*x; }

template<typename T>
  void
  testit1(T const& x, T const& y, T const& z)
  {
      using std::exp;
      using std::log;
      using std::sqrt;
      using std::sin;
      using std::cos;
      using std::tan;
      using std::sinh;
      using std::cosh;
      using std::tanh;
      using std::atan;
      using std::asin;
      using std::acos;

      // algebraic
      TEST(x, +x);
      TEST(T(0) - x, -x);
      TEST((x + y) + z, x + (y + z));
      TEST(x + y, y + x);
      TEST(x + (-y), x - y);
      TEST(x*(y + z), x*y + x*z);
      TEST(x*y, y*x);
      TEST(pow2(sqrt(x)), x);

      T u;
      TEST((u = x, u *= y, u), x*y);
      TEST((u = x, u /= y, u), x/y);
      TEST((u = x, u += y, u), x + y);
      TEST((u = x, u -= y, u), x - y);

      // exp and log
      TEST(exp(x)*exp(-x), T(1));
      TEST(exp(T(2)*x), pow2(exp(x)));
      if (x != T(0)) {
	  TEST(exp(log(x)), x);
	  TEST(log(T(2)*x), log(T(2)) + log(x));
      }
      TEST(exp(T(0)), T(1));

      // trigonometric
      TEST(pow2(cos(x)) + pow2(sin(x)), T(1));
      TEST(tan(x)*cos(x), sin(x));
      TEST(cos(T(2)*x), T(2)*pow2(cos(x)) - T(1));
      TEST(sin(T(2)*x), T(2)*sin(x)*cos(x));

      // hyperbolic
      TEST(pow2(cosh(x)) - pow2(sinh(x)), T(1));
      TEST(tanh(x)*cosh(x), sinh(x));
  }

int
main()
{
    std::clog << "\nTesting double functions." << std::endl;
    for (int i = 0; i < 1000; ++i) {
	double x = drand48();
	double y = drand48();
	double z = drand48();
	testit1(x, y, z);
    }
    std::clog << "\nTesting complex<double> functions." << std::endl;
    for (int i = 0; i < 1000; ++i) {
	std::complex<double> x(drand48(), drand48());
	std::complex<double> y(drand48(), drand48());
	std::complex<double> z(drand48(), drand48());
	testit1(x, y, z);
    }
}
