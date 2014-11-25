#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

template<unsigned int n, unsigned int m>
  void bitsetTest::testOrAssignInvertLevel3(void)
  {
    libecc::bitset<n> tmp1;
    tmp1 = get_x(OverLoadHook<n>());
    tmp1 |= ~get_y(OverLoadHook<m>());
    libecc::bitset<n> res = get_x(OverLoadHook<n>());
    libecc::bitset<fourDS> tmp2 = ~get_y(OverLoadHook<m>());
    res |= tmp2;
    CPPUNIT_ASSERT( tmp1 == res );
    CPPUNIT_ASSERT( (tmp1.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
  }

template<unsigned int n>
  void bitsetTest::testOrAssignInvertLevel2(void)
  {
#ifndef FASTTEST
    testOrAssignInvertLevel3<n, 13>();
    testOrAssignInvertLevel3<n, DS>();
    testOrAssignInvertLevel3<n, DSp13>();
    testOrAssignInvertLevel3<n, twoDS>();
    testOrAssignInvertLevel3<n, twoDSp13>();
    testOrAssignInvertLevel3<n, threeDS>();
#endif
    testOrAssignInvertLevel3<n, threeDSp13>();
#ifndef FASTTEST
    testOrAssignInvertLevel3<n, fourDS>();
#endif
  }

void bitsetTest::testOrAssignInvert(void)
{
#ifndef FASTTEST
  testOrAssignInvertLevel2<13>();
  testOrAssignInvertLevel2<DS>();
  testOrAssignInvertLevel2<DSp13>();
  testOrAssignInvertLevel2<twoDS>();
  testOrAssignInvertLevel2<twoDSp13>();
  testOrAssignInvertLevel2<threeDS>();
#endif
  testOrAssignInvertLevel2<threeDSp13>();
#ifndef FASTTEST
  testOrAssignInvertLevel2<fourDS>();
#endif
}
