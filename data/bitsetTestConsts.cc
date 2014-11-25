#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

void bitsetTest::testConsts(void)
{
  CPPUNIT_ASSERT( libecc::bitset<sixDSp13>::number_of_bits == sixDSp13 );
  CPPUNIT_ASSERT( libecc::bitset_digit_bits == digit_bits );

  CPPUNIT_ASSERT( libecc::bitset<fourDSm1>::digits == libecc::bitset<fourDS>::digits );
  CPPUNIT_ASSERT( libecc::bitset<fourDS>::digits == libecc::bitset<fourDSp1>::digits - 1 );
  CPPUNIT_ASSERT( libecc::bitset<fourDSp1>::digits == libecc::bitset<fourDSp2>::digits );
  CPPUNIT_ASSERT( libecc::bitset<digit_bits + fourDSm1>::digits == libecc::bitset<digit_bits + fourDS>::digits );
  CPPUNIT_ASSERT( libecc::bitset<digit_bits + fourDS>::digits == libecc::bitset<digit_bits + fourDSp1>::digits - 1 );
  CPPUNIT_ASSERT( libecc::bitset<digit_bits + fourDSp1>::digits == libecc::bitset<digit_bits + fourDSp2>::digits );
  CPPUNIT_ASSERT( libecc::bitset<fourDSp2>::digits == libecc::bitset<digit_bits + fourDSp2>::digits - 1 );
  CPPUNIT_ASSERT( libecc::bitset<fourDS>::digits * digit_bits == fourDS );
  CPPUNIT_ASSERT( libecc::bitset<digit_bits + fourDS>::digits * digit_bits == digit_bits + fourDS );

  CPPUNIT_ASSERT( libecc::bitset<fourDSm1>::valid_bits == (~static_cast<libecc::bitset_digit_t>(0) >> 1) );
  CPPUNIT_ASSERT( libecc::bitset<fourDS>::valid_bits == ~static_cast<libecc::bitset_digit_t>(0) );
  CPPUNIT_ASSERT( libecc::bitset<fourDSp1>::valid_bits == 1);
  CPPUNIT_ASSERT( libecc::bitset<fourDSp2>::valid_bits == 3);
  CPPUNIT_ASSERT( libecc::bitset<fourDSp3>::valid_bits == 7);
  CPPUNIT_ASSERT( libecc::bitset<digit_bits + fourDSm1>::valid_bits == (~static_cast<libecc::bitset_digit_t>(0) >> 1) );
  CPPUNIT_ASSERT( libecc::bitset<digit_bits + fourDS>::valid_bits == ~static_cast<libecc::bitset_digit_t>(0) );
  CPPUNIT_ASSERT( libecc::bitset<digit_bits + fourDSp1>::valid_bits == 1 );
  CPPUNIT_ASSERT( libecc::bitset<digit_bits + fourDSp2>::valid_bits == 3 );
  CPPUNIT_ASSERT( libecc::bitset<digit_bits + fourDSp3>::valid_bits == 7 );

  CPPUNIT_ASSERT( libecc::bitset<fourDSm1>::has_excess_bits );
  CPPUNIT_ASSERT( libecc::bitset<fourDS>::has_excess_bits == false );
  CPPUNIT_ASSERT( libecc::bitset<fourDSp1>::has_excess_bits );
  CPPUNIT_ASSERT( libecc::bitset<digit_bits + fourDSm1>::has_excess_bits );
  CPPUNIT_ASSERT( libecc::bitset<digit_bits + fourDS>::has_excess_bits == false );
  CPPUNIT_ASSERT( libecc::bitset<digit_bits + fourDSp1>::has_excess_bits );
}
