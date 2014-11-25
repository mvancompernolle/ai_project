#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

template<unsigned int n, unsigned int m>
  void bitsetTest::testXorAssignLevel3(void)
  {
    libecc::bitset<n> tmp1 = get_x(OverLoadHook<n>());
    tmp1 ^= get_y(OverLoadHook<m>());
    libecc::bitset<n> res = get_x(OverLoadHook<n>());
    res |= get_y(OverLoadHook<m>());
    libecc::bitset<fourDS> tmp2 = get_x(OverLoadHook<n>());
    tmp2 &= get_y(OverLoadHook<m>());
    tmp2 = ~tmp2;
    res &= tmp2;
    CPPUNIT_ASSERT( tmp1 == res );
    CPPUNIT_ASSERT( (tmp1.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
  }

template<unsigned int n>
void bitsetTest::testXorAssignLevel2(void)
{
#ifndef FASTTEST
  testXorAssignLevel3<n, 13>();
  testXorAssignLevel3<n, DS>();
  testXorAssignLevel3<n, DSp13>();
  testXorAssignLevel3<n, twoDS>();
  testXorAssignLevel3<n, twoDSp13>();
  testXorAssignLevel3<n, threeDS>();
#endif
  testXorAssignLevel3<n, threeDSp13>();
#ifndef FASTTEST
  testXorAssignLevel3<n, fourDS>();
#endif
}

void bitsetTest::testXorAssign(void)
{
#ifndef FASTTEST
  testXorAssignLevel2<13>();
  testXorAssignLevel2<DS>();
  testXorAssignLevel2<DSp13>();
  testXorAssignLevel2<twoDS>();
  testXorAssignLevel2<twoDSp13>();
  testXorAssignLevel2<threeDS>();
#endif
  testXorAssignLevel2<threeDSp13>();
#ifndef FASTTEST
  testXorAssignLevel2<fourDS>();
#endif
}
