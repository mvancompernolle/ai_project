#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

template<unsigned int n>
  void bitsetTest::testCopyLevel2(void)
  {
#ifndef FASTTEST
    libecc::bitset<n> tmp7v3(b7v3);
    CPPUNIT_ASSERT( tmp7v3 == libecc::bitset<n>(b7v3_str) );
    CPPUNIT_ASSERT( (tmp7v3.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    libecc::bitset<n> tmpDSv3(bDSv3);
    CPPUNIT_ASSERT( tmpDSv3 == libecc::bitset<n>(bDSv3_str) );
    CPPUNIT_ASSERT( (tmpDSv3.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    libecc::bitset<n> tmpDSvDS(bDSvDS);
    CPPUNIT_ASSERT( tmpDSvDS == libecc::bitset<n>(bDSvDS_str) );
    CPPUNIT_ASSERT( (tmpDSvDS.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    libecc::bitset<n> tmpDSp7vDS(bDSp7vDS);
    CPPUNIT_ASSERT( tmpDSp7vDS == libecc::bitset<n>(bDSp7vDS_str) );
    CPPUNIT_ASSERT( (tmpDSp7vDS.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    libecc::bitset<n> tmp2DSv2DS(b2DSv2DS);
    CPPUNIT_ASSERT( tmp2DSv2DS == libecc::bitset<n>(b2DSv2DS_str) );
    CPPUNIT_ASSERT( (tmp2DSv2DS.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    libecc::bitset<n> tmp2DSv2DSm1(b2DSv2DSm1);
    CPPUNIT_ASSERT( tmp2DSv2DSm1 == libecc::bitset<n>(b2DSv2DSm1_str) );
    CPPUNIT_ASSERT( (tmp2DSv2DSm1.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    libecc::bitset<n> tmp2DSp1v2DSp1(b2DSp1v2DSp1);
    CPPUNIT_ASSERT( tmp2DSp1v2DSp1 == libecc::bitset<n>(b2DSp1v2DSp1_str) );
    CPPUNIT_ASSERT( (tmp2DSp1v2DSp1.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
#endif
    libecc::bitset<n> tmp6DSp3v6DSp3(b6DSp3v6DSp3);
    CPPUNIT_ASSERT( tmp6DSp3v6DSp3 == libecc::bitset<n>(b6DSp3v6DSp3_str) );
    CPPUNIT_ASSERT( (tmp6DSp3v6DSp3.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
#ifndef FASTTEST
    libecc::bitset<n> tmp6DSp13v6DSp3(b6DSp13v6DSp3);
    CPPUNIT_ASSERT( tmp6DSp13v6DSp3 == libecc::bitset<n>(b6DSp13v6DSp3_str) );
    CPPUNIT_ASSERT( (tmp6DSp13v6DSp3.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    libecc::bitset<n> tmp6DSp13v6DSp13(b6DSp13v6DSp13);
    CPPUNIT_ASSERT( tmp6DSp13v6DSp13 == libecc::bitset<n>(b6DSp13v6DSp13_str) );
    CPPUNIT_ASSERT( (tmp6DSp13v6DSp13.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
#endif
  }

void bitsetTest::testCopy(void)
{
#ifndef FASTTEST
  testCopyLevel2<7>();
  testCopyLevel2<DS>();
  testCopyLevel2<DSp7>();
  testCopyLevel2<twoDS>();
  testCopyLevel2<twoDSp1>();
#endif
  testCopyLevel2<sixDSp3>();
#ifndef FASTTEST
  testCopyLevel2<sixDSp13>();
#endif
}
