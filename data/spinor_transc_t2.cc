#include <more/math/math.h>
#include <more/diag/stopwatch.h>
#include <limits>
#include <iostream>
#include <iomanip>
#include <more/diag/debug.h>
#include <more/math/spinor.h>

#define PRECISION 1e-5
#define TEST(x, y) (test((x), (y)) || test_error(#x, #y, (x), (y)))

using namespace more;

bool test_failed_flag = false;

template<typename T, typename U>
  bool test_error(const char* strx, const char* stry,
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
  bool test(const T& x, const T& y)
  {
      return abs(x - y) < PRECISION || (abs(x - y) < PRECISION*abs(x));
  }
template<typename T>
  bool test(const math::spinor<T>& x, const math::spinopr<T>& y)
  {
      using math::half;
      return test(x(half), y(half)) && test(x(-half), y(-half));
  }
template<typename T>
  bool test(const math::spinopr<T>& x, const math::spinopr<T>& y)
  {
      using math::half;
      return test(x(half, half), y(half, half))
	  && test(x(half, -half), y(half, -half))
	  && test(x(-half, half), y(-half, half))
	  && test(x(-half, -half), y(-half, -half));
  }


using math::half;

typedef double real_type;
typedef std::complex<real_type> complex_type;
typedef math::spinor<complex_type> spinor_type;
typedef math::spinopr<complex_type> spinopr_type;



void testit1(spinopr_type const& A)
{
    spinopr_type one(math::one);
    spinopr_type zero(math::zero);

    TEST(exp(A)*exp(-A), one);
    TEST(exp(2*A), pow2(exp(A)));
    if (det(A) != 0.0) {
	TEST(exp(log(A)), A);
	// TEST(log(exp(A)), A); // fails, presumably due to branch cut.
	TEST(log(2*A), log(2.0) + log(A));
    }
    TEST(pow2(cos(A)) + pow2(sin(A)), one);
    TEST(pow2(cosh(A)) - pow2(sinh(A)), one);
    TEST(exp(zero), one);
    TEST(pow2(sqrt(A)), A);
    TEST(cos(2*A), 2*pow2(cos(A)) - 1);
    TEST(sin(2*A), 2*sin(A)*cos(A));
    TEST(tan(A)*cos(A), sin(A));
    TEST(tanh(A)*cosh(A), sinh(A));
}

void randomize(spinopr_type& A)
{
    for(int i = 0; i < 4; ++i) {
	math::pm_half m1 = half, m2 = half;
	if(i >= 2) m1 = -m1;
	if(i % 2) m2 = -m2;
	A(m1, m2) = exp(1.5*drand48() +
			2*math::numbers::pi*math::onei*drand48());
    }
}

int main()
{
    more::diag::stopwatch sw;
    sw.start();
    for(int count = 0; count < 400; ++count) {
	spinopr_type A;
	randomize(A);
	testit1(A);
	randomize(A);
	testit1(A + adj(A)); // hermitian
	randomize(A);
	A(half, half) = A(half, -half) = 0;
	testit1(A);
	testit1(adj(A));
	if(test_failed_flag) {
	    std::cerr << "Failed on # " << count << ".\n";
	    return 1;
	}
    }
    testit1(math::one);
    testit1(math::zero);
    spinopr_type A;
    A(half, half) = A(half, -half) = A(-half, half) = A(-half, -half) = 1;
    testit1(A);
    sw.stop();
    std::clog << "Time used: " << sw << ".\n";
    return 0;
}
