#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

template<unsigned int n, unsigned int m>
  void bitsetTest::testXorAssignInvertLevel3(void)
  {
    libecc::bitset<n> tmp1 = get_x(OverLoadHook<n>());
    tmp1 ^= ~get_y(OverLoadHook<m>());
    libecc::bitset<n> res = get_x(OverLoadHook<n>());
    libecc::bitset<fourDS> tmp2 = ~get_y(OverLoadHook<m>());
    res ^= tmp2;
    CPPUNIT_ASSERT( tmp1 == res );
    CPPUNIT_ASSERT( (tmp1.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
  }

template<unsigned int n>
  void bitsetTest::testXorAssignInvertLevel2(void)
  {
#ifndef FASTTEST
    testXorAssignInvertLevel3<n, 13>();
    testXorAssignInvertLevel3<n, DS>();
    testXorAssignInvertLevel3<n, DSp13>();
    testXorAssignInvertLevel3<n, twoDS>();
    testXorAssignInvertLevel3<n, twoDSp13>();
    testXorAssignInvertLevel3<n, threeDS>();
#endif
    testXorAssignInvertLevel3<n, threeDSp13>();
#ifndef FASTTEST
    testXorAssignInvertLevel3<n, fourDS>();
#endif
  }

void bitsetTest::testXorAssignInvert(void)
{
#ifndef FASTTEST
  testXorAssignInvertLevel2<13>();
  testXorAssignInvertLevel2<DS>();
  testXorAssignInvertLevel2<DSp13>();
  testXorAssignInvertLevel2<twoDS>();
  testXorAssignInvertLevel2<twoDSp13>();
  testXorAssignInvertLevel2<threeDS>();
#endif
  testXorAssignInvertLevel2<threeDSp13>();
#ifndef FASTTEST
  testXorAssignInvertLevel2<fourDS>();
#endif
}
