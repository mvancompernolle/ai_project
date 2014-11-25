#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

template<unsigned int n, unsigned int n13, unsigned int nDS, unsigned int nDSp13, unsigned int n2DS, unsigned int n2DSp13, unsigned int n3DS, unsigned int n3DSp13, unsigned int n4DS>
  void bitsetTest::testAndAssignLevel2(void)
  {
    libecc::bitset<n> res;
#ifndef FASTTEST
    res = get_x(OverLoadHook<n>());
    res &= y13;
    CPPUNIT_ASSERT( res == get_AND(OverLoadHook<n13>()) );
    CPPUNIT_ASSERT( (res.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    res = get_x(OverLoadHook<n>());
    res &= yDS;
    CPPUNIT_ASSERT( res == get_AND(OverLoadHook<nDS>()) );
    CPPUNIT_ASSERT( (res.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    res = get_x(OverLoadHook<n>());
    res &= yDSp13;
    CPPUNIT_ASSERT( res == get_AND(OverLoadHook<nDSp13>()) );
    CPPUNIT_ASSERT( (res.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    res = get_x(OverLoadHook<n>());
    res &= y2DS;
    CPPUNIT_ASSERT( res == get_AND(OverLoadHook<n2DS>()) );
    CPPUNIT_ASSERT( (res.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    res = get_x(OverLoadHook<n>());
    res &= y2DSp13;
    CPPUNIT_ASSERT( res == get_AND(OverLoadHook<n2DSp13>()) );
    CPPUNIT_ASSERT( (res.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    res = get_x(OverLoadHook<n>());
    res &= y3DS;
    CPPUNIT_ASSERT( res == get_AND(OverLoadHook<n3DS>()) );
    CPPUNIT_ASSERT( (res.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
#endif
    res = get_x(OverLoadHook<n>());
    res &= y3DSp13;
    CPPUNIT_ASSERT( res == get_AND(OverLoadHook<n3DSp13>()) );
    CPPUNIT_ASSERT( (res.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
#ifndef FASTTEST
    res = get_x(OverLoadHook<n>());
    res &= y4DS;
    CPPUNIT_ASSERT( res == get_AND(OverLoadHook<n4DS>()) );
    CPPUNIT_ASSERT( (res.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
#endif
  }

void bitsetTest::testAndAssign(void)
{
#ifndef FASTTEST
  testAndAssignLevel2<13, 13, 13, 13, 13, 13, 13, 13, 13>();
  testAndAssignLevel2<DS, 13, DS, DS, DS, DS, DS, DS, DS>();
  testAndAssignLevel2<DSp13, 13, DS, DSp13, DSp13, DSp13, DSp13, DSp13, DSp13>();
  testAndAssignLevel2<twoDS, 13, DS, DSp13, twoDS, twoDS, twoDS, twoDS, twoDS>();
  testAndAssignLevel2<twoDSp13, 13, DS, DSp13, twoDS, twoDSp13, twoDSp13, twoDSp13, twoDSp13>();
  testAndAssignLevel2<threeDS, 13, DS, DSp13, twoDS, twoDSp13, threeDS, threeDS, threeDS>();
#endif
  testAndAssignLevel2<threeDSp13, 13, DS, DSp13, twoDS, twoDSp13, threeDS, threeDSp13, threeDSp13>();
#ifndef FASTTEST
  testAndAssignLevel2<fourDS, 13, DS, DSp13, twoDS, twoDSp13, threeDS, threeDSp13, fourDS>();
#endif
}
