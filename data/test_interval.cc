#include <more/math/interval.h>
#include <more/diag/debug.h>
#include <sstream>
#include <limits>
#include <cmath>

using namespace more;

bool opt_verbose = true;

template<typename T, bool is_exact = std::numeric_limits<T>::is_exact>
  struct numeric_equal {
      bool operator()(T const& x, T const& y) { return x == y; }
  };

template<typename T>
  struct numeric_equal<T, false> {
      bool operator()(T const& x, T const& y) {
	  return std::fabs(x - y)
	      < (std::fabs(x) + std::numeric_limits<T>::round_error())
	        *std::numeric_limits<T>::round_error()*16;
      }
  };

template<typename T>
  void check_extractor(char const* str_I, math::interval<T> const& I0) {
      numeric_equal<T> eq;
      math::interval<T> I;
      std::istringstream iss_I(str_I);
      iss_I >> I;
      MORE_CHECK(iss_I.good());
      MORE_CHECK(eq(I.inf(), I0.inf()));
      MORE_CHECK(eq(I.sup(), I0.sup()));
      MORE_CHECK(I.is_left_closed() == I0.is_left_closed());
      MORE_CHECK(I.is_right_closed() == I0.is_right_closed());
      if (opt_verbose)
	  std::cout << '"' << str_I << "\" parsed as " << I
		    << " but should be " << I0
		    << std::endl;
  }

int main() {
    typedef math::interval<double> interval;

    check_extractor("[0, 1)", math::make_interval(0, true, 1, false));
    check_extractor("(2.3, 1.0]", math::make_interval(2.3, false, 1.0, true));
    check_extractor("[ 2.2 ,7.0 ]trash",
		    math::make_interval(2.2, true, 7.0, true));

    MORE_CHECK(math::make_interval(-2, true, 3, true) ==
	       math::closed_interval<int>(-2, 3));
    MORE_CHECK(math::make_interval(-2, true, 9, false) ==
	       math::left_interval<int>(-2, 9));
    MORE_CHECK(math::make_interval(7.0, false, 9.0, true) ==
	       math::right_interval<double>(7.0, 9.0));
    MORE_CHECK(math::make_interval(2, true, 30, true) ==
	       math::closed_interval<int>(2, 30));
    for (int i = 0; i < 4; ++i) {
	math::interval<int> I(0, i&1, 1, i&2);
	MORE_CHECK(inf(I) == 0);
	MORE_CHECK(sup(I) == 1);
	if (opt_verbose)
	    std::cout << I << std::endl;
	MORE_CHECK(I.is_left_closed() == (bool)(i&1));
	MORE_CHECK(I.is_right_closed() == (bool)(i&2));
    }

    return more::diag::check_exit_status();
}
