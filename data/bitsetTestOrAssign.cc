#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

template<unsigned int n, unsigned int m>
  void bitsetTest::testOrAssignLevel3(void)
  {
    libecc::bitset<n> tmp1;
    libecc::bitset<fourDS> tmp2;
    libecc::bitset<n> tmp3;
    tmp1 = get_x(OverLoadHook<n>());
    tmp1 |= get_y(OverLoadHook<m>());
    tmp3 = ~get_x(OverLoadHook<n>());
    tmp2 = get_y(OverLoadHook<m>());
    tmp2 = ~tmp2;
    tmp3 &= tmp2;
    libecc::bitset<n> res;
    res = ~tmp3;
    CPPUNIT_ASSERT( tmp1 == res );
    CPPUNIT_ASSERT( (tmp1.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
  }

template<unsigned int n>
  void bitsetTest::testOrAssignLevel2(void)
  {
#ifndef FASTTEST
    testOrAssignLevel3<n, 13>();
    testOrAssignLevel3<n, DS>();
    testOrAssignLevel3<n, DSp13>();
    testOrAssignLevel3<n, twoDS>();
    testOrAssignLevel3<n, twoDSp13>();
    testOrAssignLevel3<n, threeDS>();
#endif
    testOrAssignLevel3<n, threeDSp13>();
#ifndef FASTTEST
    testOrAssignLevel3<n, fourDS>();
#endif
  }

void bitsetTest::testOrAssign(void)
{
#ifndef FASTTEST
  testOrAssignLevel2<13>();
  testOrAssignLevel2<DS>();
  testOrAssignLevel2<DSp13>();
  testOrAssignLevel2<twoDS>();
  testOrAssignLevel2<twoDSp13>();
  testOrAssignLevel2<threeDS>();
#endif
  testOrAssignLevel2<threeDSp13>();
#ifndef FASTTEST
  testOrAssignLevel2<fourDS>();
#endif
}
