#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

// b7v3                                                                                     0000101
// bDSv3                                                           00000000000000000000000000000101
// bDSvDS                                                          11010010001111011010111000000101
// bDSp7vDS                                                   000000011010010001111011010111000000101
// b2DSv2DS                          1111100010100000000100111100011101000010001111011010111000000101
// b2DSv2DSm1                          0100001000111101101011100000010111111000101000000001001111000111
// b2DSp1v2DSp1                         11111100010100000000100111100011101000010001111011010111000000101
// b6DSp3v6DSp3             10111...0001111100010100000000100111100011101000010001111011010111000000101
// b6DSp13v6DSp3   000000000010111...0001111100010100000000100111100011101000010001111011010111000000101
// b6DSp13v6DSp13   110011100010111...0001111100010100000000100111100011101000010001111011010111000000101

void bitsetTest::testEquality(void)
{
#ifndef FASTTEST
  CPPUNIT_ASSERT( b7v3 == b7v3 );
  CPPUNIT_ASSERT( b7v3 == bDSv3 );
  CPPUNIT_ASSERT( bDSv3 == b7v3 );
  CPPUNIT_ASSERT( !(b7v3 == bDSvDS) );
  CPPUNIT_ASSERT( !(bDSvDS == b7v3) );
  CPPUNIT_ASSERT( !(b7v3 == bDSp7vDS) );
  CPPUNIT_ASSERT( !(bDSp7vDS == b7v3) );
  CPPUNIT_ASSERT( !(b7v3 == b2DSv2DS) );
  CPPUNIT_ASSERT( !(b2DSv2DS == b7v3) );
  CPPUNIT_ASSERT( !(b7v3 == b2DSv2DSm1) );
  CPPUNIT_ASSERT( !(b2DSv2DSm1 == b7v3) );
  CPPUNIT_ASSERT( !(b7v3 == b2DSp1v2DSp1) );
  CPPUNIT_ASSERT( !(b2DSp1v2DSp1 == b7v3) );
  CPPUNIT_ASSERT( !(b7v3 == b6DSp3v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp3v6DSp3 == b7v3) );
  CPPUNIT_ASSERT( !(b7v3 == b6DSp13v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp3 == b7v3) );
  CPPUNIT_ASSERT( !(b7v3 == b6DSp13v6DSp13) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp13 == b7v3) );

  CPPUNIT_ASSERT( bDSv3 == bDSv3 );
  CPPUNIT_ASSERT( !(bDSv3 == bDSvDS) );
  CPPUNIT_ASSERT( !(bDSvDS == bDSv3) );
  CPPUNIT_ASSERT( !(bDSv3 == bDSp7vDS) );
  CPPUNIT_ASSERT( !(bDSp7vDS == bDSv3) );
  CPPUNIT_ASSERT( !(bDSv3 == b2DSv2DS) );
  CPPUNIT_ASSERT( !(b2DSv2DS == bDSv3) );
  CPPUNIT_ASSERT( !(bDSv3 == b2DSv2DSm1) );
  CPPUNIT_ASSERT( !(b2DSv2DSm1 == bDSv3) );
  CPPUNIT_ASSERT( !(bDSv3 == b2DSp1v2DSp1) );
  CPPUNIT_ASSERT( !(b2DSp1v2DSp1 == bDSv3) );
  CPPUNIT_ASSERT( !(bDSv3 == b6DSp3v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp3v6DSp3 == bDSv3) );
  CPPUNIT_ASSERT( !(bDSv3 == b6DSp13v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp3 == bDSv3) );
  CPPUNIT_ASSERT( !(bDSv3 == b6DSp13v6DSp13) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp13 == bDSv3) );

  CPPUNIT_ASSERT( bDSvDS == bDSvDS );
  CPPUNIT_ASSERT( bDSvDS == bDSp7vDS );
  CPPUNIT_ASSERT( bDSp7vDS == bDSvDS );
  CPPUNIT_ASSERT( !(bDSvDS == b2DSv2DS) );
  CPPUNIT_ASSERT( !(b2DSv2DS == bDSvDS) );
  CPPUNIT_ASSERT( !(bDSvDS == b2DSv2DSm1) );
  CPPUNIT_ASSERT( !(b2DSv2DSm1 == bDSvDS) );
  CPPUNIT_ASSERT( !(bDSvDS == b2DSp1v2DSp1) );
  CPPUNIT_ASSERT( !(b2DSp1v2DSp1 == bDSvDS) );
  CPPUNIT_ASSERT( !(bDSvDS == b6DSp3v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp3v6DSp3 == bDSvDS) );
  CPPUNIT_ASSERT( !(bDSvDS == b6DSp13v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp3 == bDSvDS) );
  CPPUNIT_ASSERT( !(bDSvDS == b6DSp13v6DSp13) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp13 == bDSvDS) );

  CPPUNIT_ASSERT( bDSp7vDS == bDSp7vDS );
  CPPUNIT_ASSERT( !(bDSp7vDS == b2DSv2DS) );
  CPPUNIT_ASSERT( !(b2DSv2DS == bDSp7vDS) );
  CPPUNIT_ASSERT( !(bDSp7vDS == b2DSv2DSm1) );
  CPPUNIT_ASSERT( !(b2DSv2DSm1 == bDSp7vDS) );
  CPPUNIT_ASSERT( !(bDSp7vDS == b2DSp1v2DSp1) );
  CPPUNIT_ASSERT( !(b2DSp1v2DSp1 == bDSp7vDS) );
  CPPUNIT_ASSERT( !(bDSp7vDS == b6DSp3v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp3v6DSp3 == bDSp7vDS) );
  CPPUNIT_ASSERT( !(bDSp7vDS == b6DSp13v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp3 == bDSp7vDS) );
  CPPUNIT_ASSERT( !(bDSp7vDS == b6DSp13v6DSp13) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp13 == bDSp7vDS) );

  CPPUNIT_ASSERT( b2DSv2DS == b2DSv2DS );
  CPPUNIT_ASSERT( !(b2DSv2DS == b2DSv2DSm1) );
  CPPUNIT_ASSERT( !(b2DSv2DSm1 == b2DSv2DS) );
  CPPUNIT_ASSERT( !(b2DSv2DS == b2DSp1v2DSp1) );
  CPPUNIT_ASSERT( !(b2DSp1v2DSp1 == b2DSv2DS) );
  CPPUNIT_ASSERT( !(b2DSv2DS == b6DSp3v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp3v6DSp3 == b2DSv2DS) );
  CPPUNIT_ASSERT( !(b2DSv2DS == b6DSp13v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp3 == b2DSv2DS) );
  CPPUNIT_ASSERT( !(b2DSv2DS == b6DSp13v6DSp13) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp13 == b2DSv2DS) );

  CPPUNIT_ASSERT( b2DSv2DSm1 == b2DSv2DSm1 );
  CPPUNIT_ASSERT( !(b2DSv2DSm1 == b2DSp1v2DSp1) );
  CPPUNIT_ASSERT( !(b2DSp1v2DSp1 == b2DSv2DSm1) );
  CPPUNIT_ASSERT( !(b2DSv2DSm1 == b6DSp3v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp3v6DSp3 == b2DSv2DSm1) );
  CPPUNIT_ASSERT( !(b2DSv2DSm1 == b6DSp13v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp3 == b2DSv2DSm1) );
  CPPUNIT_ASSERT( !(b2DSv2DSm1 == b6DSp13v6DSp13) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp13 == b2DSv2DSm1) );
#endif

  CPPUNIT_ASSERT( b2DSp1v2DSp1 == b2DSp1v2DSp1 );
  CPPUNIT_ASSERT( !(b2DSp1v2DSp1 == b6DSp3v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp3v6DSp3 == b2DSp1v2DSp1) );
  CPPUNIT_ASSERT( !(b2DSp1v2DSp1 == b6DSp13v6DSp3) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp3 == b2DSp1v2DSp1) );
  CPPUNIT_ASSERT( !(b2DSp1v2DSp1 == b6DSp13v6DSp13) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp13 == b2DSp1v2DSp1) );

#ifndef FASTTEST
  CPPUNIT_ASSERT( b6DSp3v6DSp3 == b6DSp3v6DSp3 );
  CPPUNIT_ASSERT( b6DSp3v6DSp3 == b6DSp13v6DSp3 );
  CPPUNIT_ASSERT( b6DSp13v6DSp3 == b6DSp3v6DSp3 );
  CPPUNIT_ASSERT( !(b6DSp3v6DSp3 == b6DSp13v6DSp13) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp13 == b6DSp3v6DSp3) );

  CPPUNIT_ASSERT( b6DSp13v6DSp3 == b6DSp13v6DSp3 );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp3 == b6DSp13v6DSp13) );
  CPPUNIT_ASSERT( !(b6DSp13v6DSp13 == b6DSp13v6DSp3) );

  CPPUNIT_ASSERT( b6DSp13v6DSp13 == b6DSp13v6DSp13 );
#endif
}
