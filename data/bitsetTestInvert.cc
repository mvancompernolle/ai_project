#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

template<unsigned int n, unsigned int m>
  void bitsetTest::testInvertLevel3(void)
  {
    libecc::bitset<n> res;
    res = ~get_x(OverLoadHook<m>());
    libecc::bitset<n> tmp(get_x(OverLoadHook<m>()));
    for (unsigned int d = 0; d < libecc::bitset<n>::digits; ++d)
      tmp.rawdigit(d) ^= ~static_cast<libecc::bitset_digit_t>(0);
    tmp.rawdigit(libecc::bitset<n>::digits - 1) &= libecc::bitset<n>::valid_bits;
    CPPUNIT_ASSERT( res == tmp );
    CPPUNIT_ASSERT( (res.rawdigit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
  }

template<unsigned int n>
  void bitsetTest::testInvertLevel2(void)
  {
#ifndef FASTTEST
    testInvertLevel3<n, 13>();
    testInvertLevel3<n, DS>();
    testInvertLevel3<n, DSp13>();
    testInvertLevel3<n, twoDS>();
    testInvertLevel3<n, twoDSp13>();
    testInvertLevel3<n, threeDS>();
#endif
    testInvertLevel3<n, threeDSp13>();
#ifndef FASTTEST
    testInvertLevel3<n, fourDS>();
#endif
  }

void bitsetTest::testInvert(void)
{
#ifndef FASTTEST
  testInvertLevel2<13>();
  testInvertLevel2<DS>();
  testInvertLevel2<DSp13>();
  testInvertLevel2<twoDS>();
  testInvertLevel2<twoDSp13>();
  testInvertLevel2<threeDS>();
#endif
  testInvertLevel2<threeDSp13>();
#ifndef FASTTEST
  testInvertLevel2<fourDS>();
#endif
}
