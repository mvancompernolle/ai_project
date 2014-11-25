#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

static char const* const sb_str[]  = {
  "1",								      // bit 0
  "40",								      // bit 6
  "1000",							      // bit 12
  "40000",							      // bit 18
  "1000000",							      // bit 24
  "40000000",							      // bit 30
  "1000000000",							      // bit 36
  "40000000000",						      // bit 42
  "1000000000000",						      // bit 48
  "40000000000000",						      // bit 54
  "1000000000000000",						      // bit 60
  "40000000000000000",						      // bit 66
  "1000000000000000000",					      // bit 72
  "40000000000000000000",					      // bit 78
  "1000000000000000000000",					      // bit 84
  "40000000000000000000000",					      // bit 90
  "1000000000000000000000000",					      // bit 96
  "40000000000000000000000000",					      // bit 102
  "1000000000000000000000000000",				      // bit 108
  "40000000000000000000000000000",				      // bit 114
  "1000000000000000000000000000000",				      // bit 120
  "40000000000000000000000000000000",				      // bit 126
  "1000000000000000000000000000000000",				      // bit 132
  "40000000000000000000000000000000000",			      // bit 138
  "1000000000000000000000000000000000000",			      // bit 144
  "40000000000000000000000000000000000000",			      // bit 150
  "1000000000000000000000000000000000000000",			      // bit 156
  "40000000000000000000000000000000000000000",			      // bit 162
  "1000000000000000000000000000000000000000000",		      // bit 168
  "40000000000000000000000000000000000000000000",		      // bit 174
  "1000000000000000000000000000000000000000000000",		      // bit 180
  "40000000000000000000000000000000000000000000000",		      // bit 186
  "1000000000000000000000000000000000000000000000000",		      // bit 192
  "40000000000000000000000000000000000000000000000000",		      // bit 198
  "1000000000000000000000000000000000000000000000000000",	      // bit 204
  "40000000000000000000000000000000000000000000000000000",	      // bit 210
  "1000000000000000000000000000000000000000000000000000000",	      // bit 216
  "40000000000000000000000000000000000000000000000000000000",	      // bit 222
  "1000000000000000000000000000000000000000000000000000000000",	      // bit 228
  "40000000000000000000000000000000000000000000000000000000000",      // bit 234
  "1000000000000000000000000000000000000000000000000000000000000",    // bit 240
  "40000000000000000000000000000000000000000000000000000000000000",   // bit 246
  "1000000000000000000000000000000000000000000000000000000000000000", // bit 252
};

template<unsigned int n, unsigned int b>
  void bitsetTest::testBitOpsLevel3(void)
  {
    if (b >= n)
      return;
    libecc::bitset<n> const zeros("0");
    libecc::bitset<n> const ones("FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF");
    libecc::bitset<n> const tmp1(sb_str[b / 6]);
    libecc::bitset<n> tmp2 = zeros;
    CPPUNIT_ASSERT( !tmp2.template test<b>() );
    CPPUNIT_ASSERT( !tmp2.any() );
    tmp2.template set<b>();
    CPPUNIT_ASSERT( tmp1 == tmp2 );
    CPPUNIT_ASSERT( tmp2.template test<b>() );
    CPPUNIT_ASSERT( tmp2.any() );
    tmp2.template clear<b>();
    CPPUNIT_ASSERT( tmp2 == zeros );
    tmp2.template flip<b>();
    CPPUNIT_ASSERT( tmp2 == tmp1 );
    tmp2.template flip<b>();
    CPPUNIT_ASSERT( tmp2 == zeros );
    tmp2 = ones;
    CPPUNIT_ASSERT( tmp2.template test<b>() );
    CPPUNIT_ASSERT( tmp2.any() );
    tmp2.template clear<b>();
    libecc::bitset<n> tmp3 = ones ^ tmp1;
    CPPUNIT_ASSERT( tmp2 == tmp3 );
    CPPUNIT_ASSERT( !tmp2.template test<b>() );
    CPPUNIT_ASSERT( tmp2.any() );
    tmp2.template set<b>();
    CPPUNIT_ASSERT( tmp2 == ones );
    tmp2.template flip<b>();
    CPPUNIT_ASSERT( tmp2 == tmp3 );
    tmp2.template flip<b>();
    CPPUNIT_ASSERT( tmp2 == ones );
  }

template<unsigned int n>
  void bitsetTest::testBitOpsLevel2(void)
  {
    libecc::bitset<n> const zeros("0");
    libecc::bitset<n> const ones("FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF");
    for (unsigned int b = 0; b < n; b += 6)
    {
      libecc::bitset<n> const tmp1(sb_str[b / 6]);
      libecc::bitset<n> tmp2 = zeros;
      CPPUNIT_ASSERT( !tmp2.test(b) );
      CPPUNIT_ASSERT( !tmp2.any() );
      tmp2.set(b);
      CPPUNIT_ASSERT( tmp1 == tmp2 );
      CPPUNIT_ASSERT( tmp2.test(b) );
      CPPUNIT_ASSERT( tmp2.any() );
      tmp2.clear(b);
      CPPUNIT_ASSERT( tmp2 == zeros );
      tmp2.flip(b);
      CPPUNIT_ASSERT( tmp2 == tmp1 );
      tmp2.flip(b);
      CPPUNIT_ASSERT( tmp2 == zeros );
      tmp2 = ones;
      CPPUNIT_ASSERT( tmp2.test(b) );
      CPPUNIT_ASSERT( tmp2.any() );
      tmp2.clear(b);
      libecc::bitset<n> tmp3 = ones ^ tmp1;
      CPPUNIT_ASSERT( tmp2 == tmp3 );
      CPPUNIT_ASSERT( !tmp2.test(b) );
      CPPUNIT_ASSERT( tmp2.any() );
      tmp2.set(b);
      CPPUNIT_ASSERT( tmp2 == ones );
      tmp2.flip(b);
      CPPUNIT_ASSERT( tmp2 == tmp3 );
      tmp2.flip(b);
      CPPUNIT_ASSERT( tmp2 == ones );
    }
#ifndef FASTTEST
    testBitOpsLevel3<n, 0>();
    testBitOpsLevel3<n, 6>();
    testBitOpsLevel3<n, 12>();
    testBitOpsLevel3<n, 18>();
    testBitOpsLevel3<n, 24>();
    testBitOpsLevel3<n, 30>();
    testBitOpsLevel3<n, 36>();
    testBitOpsLevel3<n, 42>();
    testBitOpsLevel3<n, 48>();
    testBitOpsLevel3<n, 54>();
    testBitOpsLevel3<n, 60>();
    testBitOpsLevel3<n, 66>();
    testBitOpsLevel3<n, 72>();
    testBitOpsLevel3<n, 78>();
    testBitOpsLevel3<n, 84>();
    testBitOpsLevel3<n, 90>();
    testBitOpsLevel3<n, 96>();
    testBitOpsLevel3<n, 102>();
#endif
    testBitOpsLevel3<n, 108>();
#ifndef FASTTEST
    testBitOpsLevel3<n, 114>();
    testBitOpsLevel3<n, 120>();
    testBitOpsLevel3<n, 126>();
#if ECC_BITS >= 64
    testBitOpsLevel3<n, 132>();
    testBitOpsLevel3<n, 138>();
    testBitOpsLevel3<n, 144>();
    testBitOpsLevel3<n, 150>();
    testBitOpsLevel3<n, 156>();
    testBitOpsLevel3<n, 162>();
    testBitOpsLevel3<n, 168>();
    testBitOpsLevel3<n, 174>();
    testBitOpsLevel3<n, 180>();
    testBitOpsLevel3<n, 186>();
    testBitOpsLevel3<n, 192>();
    testBitOpsLevel3<n, 198>();
    testBitOpsLevel3<n, 204>();
    testBitOpsLevel3<n, 210>();
    testBitOpsLevel3<n, 216>();
    testBitOpsLevel3<n, 222>();
    testBitOpsLevel3<n, 228>();
    testBitOpsLevel3<n, 234>();
    testBitOpsLevel3<n, 240>();
    testBitOpsLevel3<n, 246>();
    testBitOpsLevel3<n, 252>();
#endif
#endif
  }

void bitsetTest::testBitOps(void)
{
#ifndef FASTTEST
  testBitOpsLevel2<13>();
  testBitOpsLevel2<DS>();
  testBitOpsLevel2<DSp13>();
  testBitOpsLevel2<twoDS>();
  testBitOpsLevel2<twoDSp13>();
  testBitOpsLevel2<threeDS>();
#endif
  testBitOpsLevel2<threeDSp13>();
#ifndef FASTTEST
  testBitOpsLevel2<fourDS>();
#endif
}

