#include "sys.h"
#include "debug.h"
#include <iostream>
#include <inttypes.h>
#include <sstream>
#include <libecc/bitset.h>
#include <libecc/rng.h>

// This program has been used to verify that the code
// of class rng indeed generates the correct bitstream
// and therefore can be assumed to be implemented correctly
// and thus garantees us that the period will be 2^521 - 1.
//
// The following has been found to have a period of 2^521 - 1 (using rngTestMatrix.cc)
// 521/2/3/7/13/31/61/131/151/251
//

// Implementation 0.
// This is the Canonical Implementation.  It is correct because it is a direct implementation of the
// mathematics involved.

void generator0(std::ostream& out)
{
  libecc::bitset<521> state;

  state.reset();
  state.set(0);

  libecc::bitset<521> bottom_row;

  bottom_row.reset();
  bottom_row.set(0);
  bottom_row.set(521 - 2);
  bottom_row.set(521 - 3);
  bottom_row.set(521 - 7);
  bottom_row.set(521 - 13);
  bottom_row.set(521 - 31);
  bottom_row.set(521 - 61);
  bottom_row.set(521 - 131);
  bottom_row.set(521 - 151);
  bottom_row.set(521 - 251);

  for(int cnt = 0; cnt < 1024; ++cnt)
  {
    libecc::bitset<521> inproduct;

    inproduct = bottom_row & state;
    bool resulting_bit = inproduct.odd();

    out << (resulting_bit ? '1' : '0');

    state.shift_op<1, libecc::right, libecc::assign>(state);
    if (resulting_bit)
      state.set(520);
    else
      state.clear(520);
  }
}

// Implementation 1.
// This is the Reference Implementation.  It is believed to be correct because of its simplicity.

unsigned int const number_of_bits = 521;
unsigned int const number_of_feedbackpoints = 9;
unsigned int const feedbackpoints[number_of_feedbackpoints] = { 2, 3, 7, 13, 31, 61, 131, 151, 251 };

void generator1(std::ostream& out)
{
  static uint32_t sr[number_of_bits * 2];
  sr[0] = sr[number_of_bits] = 1;
  uint32_t* head = sr;

  for(int cnt = 0; cnt < 1024; ++cnt)
  {
    uint32_t a = head[0];
    for(unsigned int fp = 0; fp < number_of_feedbackpoints; ++fp)
      a ^= head[number_of_bits - feedbackpoints[fp]];
    head[0] = head[number_of_bits] = a;
    out << a;
    if (++head == &sr[number_of_bits])
      head = sr;
  }
}

// Implementation 2.

unsigned int const number_of_ints = (number_of_bits / 32) + 1;

class BitIterator {
  private:
    uint32_t* M_ptr;
    uint32_t M_head_mask;
  public:
    BitIterator(uint32_t* pool, int bit) : M_ptr(pool + bit / 32), M_head_mask(1 << bit % 32) { }
    uint32_t increment_and_test(uint32_t* pool);
    bool is_set(void) const { return (*M_ptr & M_head_mask); }
    void toggle(void) const { *M_ptr ^= M_head_mask; }
    friend std::ostream& operator<<(std::ostream& os, BitIterator const& iter);
};

inline uint32_t BitIterator::increment_and_test(uint32_t* pool)
{
  static uint32_t const head_mask_end = 1 << (number_of_bits % 32);
  M_head_mask <<= 1;
  if (M_head_mask == 0)
  {
    M_head_mask = 1;
    ++M_ptr;
  }
  else if (M_head_mask == head_mask_end && M_ptr - pool == number_of_ints - 1)
  {
    M_ptr = pool;
    M_head_mask = 1;
  }
  return (*M_ptr & M_head_mask);
}

void generator2(std::ostream& out)
{
  static uint32_t sr[number_of_ints];
  sr[0] = 2;
  BitIterator head(sr, 0);
  BitIterator fbp1(sr, number_of_bits - 2);
  BitIterator fbp2(sr, number_of_bits - 3);
  BitIterator fbp3(sr, number_of_bits - 7);
  BitIterator fbp4(sr, number_of_bits - 13);
  BitIterator fbp5(sr, number_of_bits - 31);
  BitIterator fbp6(sr, number_of_bits - 61);
  BitIterator fbp7(sr, number_of_bits - 131);
  BitIterator fbp8(sr, number_of_bits - 151);
  BitIterator fbp9(sr, number_of_bits - 251);

  for(int cnt = 0; cnt < 1024; ++cnt)
  {
    head.increment_and_test(sr);
    uint32_t a = fbp1.increment_and_test(sr);
    a ^= fbp2.increment_and_test(sr);
    a ^= fbp3.increment_and_test(sr);
    a ^= fbp4.increment_and_test(sr);
    a ^= fbp5.increment_and_test(sr);
    a ^= fbp6.increment_and_test(sr);
    a ^= fbp7.increment_and_test(sr);
    a ^= fbp8.increment_and_test(sr);
    a ^= fbp9.increment_and_test(sr);
    uint32_t b = a >> 16;
    a ^= b;
    b = a >> 8;
    a ^= b;
    if (libecc::oddnumberofbits[a & 0xff])
      head.toggle();
    out << (head.is_set() ? '1' : '0');
  }
}

// Final implementation.

void generator3(std::ostream& out)
{
  libecc::rng::pool_type pool("2");
  libecc::rng rng(pool);
  for (int cnt = 0; cnt < 2; ++cnt)
  {
    rng.generate_512_bits();
    libecc::bitset<512> b512 = rng.get_512_bits();
    for (int b = 0; b < 512; ++b)
      out << (b512.test(b) ? '1' : '0');
  }
}

int main(void)
{
  std::ostringstream out0, out1, out2, out3;
  generator0(out0);
  std::cout << out0.str() << '\n';
  generator1(out1);
  std::cout << out1.str() << '\n';
  generator2(out2);
  std::cout << out2.str() << '\n';
  generator3(out3);
  std::cout << out3.str() << '\n';
  assert(out0.str() == out1.str());
  assert(out0.str() == out2.str());
  assert(out0.str() == out3.str());
  std::cout << "\nSuccess: All four implementations give the same output.\n";
  return 0;
}

