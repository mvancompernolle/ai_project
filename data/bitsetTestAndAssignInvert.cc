#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

template<unsigned int n, unsigned int m>
  void bitsetTest::testAndAssignInvertLevel3(void)
  {
    libecc::bitset<n> tmp1;
    tmp1 = get_x(OverLoadHook<n>());
    tmp1 &= ~get_y(OverLoadHook<m>());
    libecc::bitset<n> res = get_x(OverLoadHook<n>());
    libecc::bitset<fourDS> tmp2 = ~get_y(OverLoadHook<m>());
    res &= tmp2;
    CPPUNIT_ASSERT( tmp1 == res );
    CPPUNIT_ASSERT( (tmp1.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
  }

template<unsigned int n>
  void bitsetTest::testAndAssignInvertLevel2(void)
  {
#ifndef FASTTEST
    testAndAssignInvertLevel3<n, 13>();
    testAndAssignInvertLevel3<n, DS>();
    testAndAssignInvertLevel3<n, DSp13>();
    testAndAssignInvertLevel3<n, twoDS>();
    testAndAssignInvertLevel3<n, twoDSp13>();
    testAndAssignInvertLevel3<n, threeDS>();
#endif
    testAndAssignInvertLevel3<n, threeDSp13>();
#ifndef FASTTEST
    testAndAssignInvertLevel3<n, fourDS>();
#endif
  }

void bitsetTest::testAndAssignInvert(void)
{
#ifndef FASTTEST
  testAndAssignInvertLevel2<13>();
  testAndAssignInvertLevel2<DS>();
  testAndAssignInvertLevel2<DSp13>();
  testAndAssignInvertLevel2<twoDS>();
  testAndAssignInvertLevel2<twoDSp13>();
  testAndAssignInvertLevel2<threeDS>();
#endif
  testAndAssignInvertLevel2<threeDSp13>();
#ifndef FASTTEST
  testAndAssignInvertLevel2<fourDS>();
#endif
}
