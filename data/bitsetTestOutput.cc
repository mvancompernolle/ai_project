#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <algorithm>
#include <functional>

// gcc 2.96 doesn't have std::ptr_fun
class do_toupper {
  public:
    int operator()(int const left) const { return std::toupper(left); }
};

template<unsigned int n>
  void bitsetTest::testOutputLevel2(void)
  {
    std::ostringstream output;
    output << get_b(OverLoadHook<n>());
    std::string s = output.str();
    std::transform(s.begin(), s.end(), s.begin(), do_toupper() /* ISO C++: std::ptr_fun<int, int>(std::toupper) */);
    CPPUNIT_ASSERT( std::string(b_str[n % 1000]) == s );
  }

void bitsetTest::testOutput(void)
{
  // Tests successful construction of test variables as well as output.
#ifndef FASTTEST
  testOutputLevel2<eb7v3>();
  testOutputLevel2<ebDSv3>();
  testOutputLevel2<ebDSvDS>();
  testOutputLevel2<ebDSp7vDS>();
  testOutputLevel2<eb2DSv2DS>();
  testOutputLevel2<eb2DSv2DSm1>();
  testOutputLevel2<eb2DSp1v2DSp1>();
  testOutputLevel2<eb6DSp3v6DSp3>();
#endif
  testOutputLevel2<eb6DSp13v6DSp3>();
#ifndef FASTTEST
  testOutputLevel2<eb6DSp13v6DSp13>();
#endif
}
