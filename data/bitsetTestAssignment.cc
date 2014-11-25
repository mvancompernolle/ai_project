#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

template<unsigned int n>
  void bitsetTest::testAssignmentLevel2(void)
  {
    libecc::bitset<n> tmp;
#ifndef FASTTEST
    tmp = b7v3;
    CPPUNIT_ASSERT( tmp == libecc::bitset<n>(b7v3_str) );
    CPPUNIT_ASSERT( (tmp.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    tmp = bDSv3;
    CPPUNIT_ASSERT( tmp == libecc::bitset<n>(bDSv3_str) );
    CPPUNIT_ASSERT( (tmp.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    tmp = bDSvDS;
    CPPUNIT_ASSERT( tmp == libecc::bitset<n>(bDSvDS_str) );
    CPPUNIT_ASSERT( (tmp.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    tmp = bDSp7vDS;
    CPPUNIT_ASSERT( tmp == libecc::bitset<n>(bDSp7vDS_str) );
    CPPUNIT_ASSERT( (tmp.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    tmp = b2DSv2DS;
    CPPUNIT_ASSERT( tmp == libecc::bitset<n>(b2DSv2DS_str) );
    CPPUNIT_ASSERT( (tmp.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    tmp = b2DSv2DSm1;
    CPPUNIT_ASSERT( tmp == libecc::bitset<n>(b2DSv2DSm1_str) );
    CPPUNIT_ASSERT( (tmp.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    tmp = b2DSp1v2DSp1;
    CPPUNIT_ASSERT( tmp == libecc::bitset<n>(b2DSp1v2DSp1_str) );
    CPPUNIT_ASSERT( (tmp.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
#endif
    tmp = b6DSp3v6DSp3;
    CPPUNIT_ASSERT( tmp == libecc::bitset<n>(b6DSp3v6DSp3_str) );
    CPPUNIT_ASSERT( (tmp.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
#ifndef FASTTEST
    tmp = b6DSp13v6DSp3;
    CPPUNIT_ASSERT( tmp == libecc::bitset<n>(b6DSp13v6DSp3_str) );
    CPPUNIT_ASSERT( (tmp.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
    tmp = b6DSp13v6DSp13;
    CPPUNIT_ASSERT( tmp == libecc::bitset<n>(b6DSp13v6DSp13_str) );
    CPPUNIT_ASSERT( (tmp.digit(libecc::bitset<n>::digits - 1) & ~libecc::bitset<n>::valid_bits) == 0 );
#endif
  }

void bitsetTest::testAssignment(void)
{
#ifndef FASTTEST
  testAssignmentLevel2<7>();
  testAssignmentLevel2<DS>();
  testAssignmentLevel2<DSp7>();
  testAssignmentLevel2<twoDS>();
  testAssignmentLevel2<twoDSp1>();
#endif
  testAssignmentLevel2<sixDSp3>();
#ifndef FASTTEST
  testAssignmentLevel2<sixDSp13>();
#endif
}
