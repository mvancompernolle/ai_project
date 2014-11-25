#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"
#include <algorithm>

// gcc 2.96 doesn't have std::ptr_fun
class do_toupper {
  public:
    int operator()(int const left) const { return std::toupper(left); }
};

template<unsigned int n>
  void bitsetTest::testConstructionLevel2(char const* str)
  {
    libecc::bitset<n / 1000> tmp(str);	// n / 1000 is the number of bits of get_b(OverLoadHook<n>()).
    std::ostringstream output;
    output << tmp;
    std::string s = output.str();
    std::transform(s.begin(), s.end(), s.begin(), do_toupper() /* ISO C++: std::ptr_fun<int, int>(std::toupper) */);
    CPPUNIT_ASSERT( std::string(b_str[n % 1000]) == s );
    CPPUNIT_ASSERT( get_b(OverLoadHook<n>()) == tmp );
    CPPUNIT_ASSERT( (tmp.digit(libecc::bitset<n / 1000>::digits - 1) & ~libecc::bitset<n / 1000>::valid_bits) == 0 );
  }

void bitsetTest::testConstruction(void)
{
  // Test equality as well as the constructor.
#ifndef FASTTEST
  testConstructionLevel2<eb7v3>(b7v3_bare);
  testConstructionLevel2<eb7v3>(b7v3_fit);
  testConstructionLevel2<eb7v3>(b7v3_excess);
  testConstructionLevel2<ebDSv3>(bDSv3_bare);
  testConstructionLevel2<ebDSv3>(bDSv3_excess1);
  testConstructionLevel2<ebDSv3>(bDSv3_excess2);
  testConstructionLevel2<ebDSv3>(bDSv3_excess3);
  testConstructionLevel2<ebDSv3>(bDSv3_excess4);
  testConstructionLevel2<ebDSvDS>(bDSvDS_excess1);
  testConstructionLevel2<ebDSvDS>(bDSvDS_excess2);
  testConstructionLevel2<ebDSvDS>(bDSvDS_excess3);
  testConstructionLevel2<ebDSp7vDS>(bDSp7vDS_bare);
  testConstructionLevel2<ebDSp7vDS>(bDSp7vDS_fit);
  testConstructionLevel2<ebDSp7vDS>(bDSp7vDS_excess1);
  testConstructionLevel2<ebDSp7vDS>(bDSp7vDS_excess2);
  testConstructionLevel2<eb2DSv2DS>(b2DSv2DS_excess);
  testConstructionLevel2<eb2DSv2DSm1>(b2DSv2DSm1_excess1);
  testConstructionLevel2<eb2DSv2DSm1>(b2DSv2DSm1_excess2);
  testConstructionLevel2<eb2DSp1v2DSp1>(b2DSp1v2DSp1_fit);
  testConstructionLevel2<eb2DSp1v2DSp1>(b2DSp1v2DSp1_excess);
#endif
  testConstructionLevel2<eb6DSp3v6DSp3>(b6DSp3v6DSp3_fit);
  testConstructionLevel2<eb6DSp13v6DSp3>(b6DSp13v6DSp3_bare);
  testConstructionLevel2<eb6DSp13v6DSp3>(b6DSp13v6DSp3_fit);
  testConstructionLevel2<eb6DSp13v6DSp13>(b6DSp13v6DSp13_fit);
}

