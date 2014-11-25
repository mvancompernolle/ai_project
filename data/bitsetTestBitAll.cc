#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

template<unsigned int n>
  void bitsetTest::testBitAllLevel2(void)
  {
    libecc::bitset<n> const zeros("0");
    libecc::bitset<n> const ones("FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF");
    libecc::bitset<n> tmp1 = zeros;
    tmp1.setall();
    CPPUNIT_ASSERT( tmp1 == ones );
    tmp1.reset();
    CPPUNIT_ASSERT( tmp1 == zeros );
  }

void bitsetTest::testBitAll(void)
{
#ifndef FASTTEST
  testBitAllLevel2<13>();
  testBitAllLevel2<DS>();
  testBitAllLevel2<DSp13>();
  testBitAllLevel2<twoDS>();
  testBitAllLevel2<twoDSp13>();
  testBitAllLevel2<threeDS>();
#endif
  testBitAllLevel2<threeDSp13>();
#ifndef FASTTEST
  testBitAllLevel2<fourDS>();
#endif
}

