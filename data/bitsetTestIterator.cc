#include "sys.h"
#include "debug.h"
#include "bitsetTest.h"

template<unsigned int n>
  void bitsetTest::testIteratorLevel2(void)
  {
    libecc::bitset<n> x(0);
    unsigned int cnt = 0;
    for (typename libecc::bitset<n>::const_iterator iter = x.begin(); iter != x.end(); ++iter)
    {
      CPPUNIT_ASSERT( *iter == 0 );
      ++cnt;
    }
    CPPUNIT_ASSERT( cnt == n );
    x.setall();
    cnt = 0;
    for (typename libecc::bitset<n>::const_iterator iter = x.begin(); iter != x.end(); ++iter)
    {
      CPPUNIT_ASSERT( *iter == 1UL << (cnt % libecc::bitset_digit_bits) );
      CPPUNIT_ASSERT( *iter == iter[0] );
      CPPUNIT_ASSERT( *iter == x.begin()[cnt] );
      CPPUNIT_ASSERT( *iter == x.end()[cnt - n] );
      ++cnt;
    }
    for (unsigned int bit = 0; bit < n; ++bit)
    {
      x.reset();
      x.set(bit);
      cnt = 0;
      for (typename libecc::bitset<n>::const_iterator iter = x.begin(); iter != x.end(); ++iter)
      {
	CPPUNIT_ASSERT( *iter == ((cnt == bit) ? (1UL << (cnt % libecc::bitset_digit_bits)) : 0UL) );
	CPPUNIT_ASSERT( *iter == iter[0] );
	CPPUNIT_ASSERT( *iter == x.begin()[cnt] );
	CPPUNIT_ASSERT( *iter == x.end()[cnt - n] );
	typename libecc::bitset<n>::const_iterator iter1, iter2(iter);
	iter1 = iter;
	CPPUNIT_ASSERT( iter1 == iter2 && iter == iter2 );
	CPPUNIT_ASSERT( *iter == *iter1 && *iter == *iter2 );
	for (unsigned int bitoffset = 0; bitoffset < n - cnt; ++bitoffset)
	{
	  iter1 = iter;
	  iter1 += bitoffset;
	  CPPUNIT_ASSERT( *iter1 == x.begin()[cnt + bitoffset] );
	  iter2 = x.begin();
	  for (unsigned int c = 0; c < cnt + bitoffset; ++c)
	    ++iter2;
	  CPPUNIT_ASSERT( iter1 == iter2 && *iter1 == *iter2 );
	}
	CPPUNIT_ASSERT( ++iter1 == x.end() );
	for (unsigned int bitoffset = 0; bitoffset <= cnt; ++bitoffset)
	{
	  iter1 = iter;
	  iter1 -= bitoffset;
	  CPPUNIT_ASSERT( *iter1 == x.begin()[cnt - bitoffset] );
	  iter2 = x.begin();
	  for (unsigned int c = 0; c < cnt - bitoffset; ++c)
	    ++iter2;
	  CPPUNIT_ASSERT( iter1 == iter2 && *iter1 == *iter2 );
	}
	CPPUNIT_ASSERT( iter1 == x.begin() );
	++cnt;
      }
    }
    int c1 = 0;
    for (typename libecc::bitset<n>::const_iterator iter1 = x.begin(); iter1 != x.end(); ++iter1)
    {
      int c2 = 0;
      for (typename libecc::bitset<n>::const_iterator iter2 = x.begin(); iter2 != x.end(); ++iter2)
      {
	CPPUNIT_ASSERT( (iter1 - iter2) == c1 - c2 );
	++c2;
      }
      ++c1;
    }

    x.reset();
    cnt = 0;
    for (typename libecc::bitset<n>::const_reverse_iterator iter = x.rbegin(); iter != x.rend(); ++iter)
    {
      CPPUNIT_ASSERT( *iter == 0 );
      ++cnt;
    }
    CPPUNIT_ASSERT( cnt == n );
    x.setall();
    cnt = 0;
    for (typename libecc::bitset<n>::const_reverse_iterator iter = x.rbegin(); iter != x.rend(); ++iter)
    {
      CPPUNIT_ASSERT( *iter == (1UL << ((n - 1 - cnt) % libecc::bitset_digit_bits)) );
      CPPUNIT_ASSERT( *iter == iter[0] );
      CPPUNIT_ASSERT( *iter == x.rbegin()[cnt] );
      CPPUNIT_ASSERT( *iter == x.rend()[cnt - n] );
      ++cnt;
    }
    for (unsigned int bit = 0; bit < n; ++bit)
    {
      x.reset();
      x.set(n - 1 - bit);
      cnt = 0;
      for (typename libecc::bitset<n>::const_reverse_iterator iter = x.rbegin(); iter != x.rend(); ++iter)
      {
	CPPUNIT_ASSERT( *iter == ((cnt == bit) ? (1UL << ((n - 1 - cnt) % libecc::bitset_digit_bits)) : 0UL) );
	CPPUNIT_ASSERT( *iter == iter[0] );
	CPPUNIT_ASSERT( *iter == x.rbegin()[cnt] );
	CPPUNIT_ASSERT( *iter == x.rend()[cnt - n] );
	typename libecc::bitset<n>::const_reverse_iterator iter1, iter2(iter);
	iter1 = iter;
	CPPUNIT_ASSERT( iter1 == iter2 && iter == iter2 );
	CPPUNIT_ASSERT( *iter == *iter1 && *iter == *iter2 );
	for (unsigned int bitoffset = 0; bitoffset < n - cnt; ++bitoffset)
	{
	  iter1 = iter;
	  iter1 += bitoffset;
	  CPPUNIT_ASSERT( *iter1 == x.rbegin()[cnt + bitoffset] );
	  iter2 = x.rbegin();
	  for (unsigned int c = 0; c < cnt + bitoffset; ++c)
	    ++iter2;
	  CPPUNIT_ASSERT( iter1 == iter2 && *iter1 == *iter2 );
	}
	CPPUNIT_ASSERT( ++iter1 == x.rend() );
	for (unsigned int bitoffset = 0; bitoffset <= cnt; ++bitoffset)
	{
	  iter1 = iter;
	  iter1 -= bitoffset;
	  CPPUNIT_ASSERT( *iter1 == x.rbegin()[cnt - bitoffset] );
	  iter2 = x.rbegin();
	  for (unsigned int c = 0; c < cnt - bitoffset; ++c)
	    ++iter2;
	  CPPUNIT_ASSERT( iter1 == iter2 && *iter1 == *iter2 );
	}
	CPPUNIT_ASSERT( iter1 == x.rbegin() );
	++cnt;
      }
    }
    c1 = 0;
    for (typename libecc::bitset<n>::const_reverse_iterator iter1 = x.rbegin(); iter1 != x.rend(); ++iter1)
    {
      int c2 = 0;
      for (typename libecc::bitset<n>::const_reverse_iterator iter2 = x.rbegin(); iter2 != x.rend(); ++iter2)
      {
	CPPUNIT_ASSERT( (iter1 - iter2) == c1 - c2 );
	++ c2;
      }
      ++c1;
    }
  }

void bitsetTest::testIterator(void)
{
#ifndef FASTTEST
  testIteratorLevel2<twoDSm1>();
  testIteratorLevel2<twoDS>();
#endif
  testIteratorLevel2<twoDSp1>();
#ifndef FASTTEST
  testIteratorLevel2<threeDSp13>();
#endif
}
