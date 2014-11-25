#include "sys.h"
#include "debug.h"
#include <cstring>	// memset
#include <iostream>
#include <inttypes.h>
#include <vector>
#include <libecc/bitset.h>
#include <math.h>

// This program has been used to determine feedback points.
// It is multipurpose: you have to edit the source code a
// bit in order to make it do whatever you want.
// Compile as g++ -O3 -o rngTestMatrix -I../include rngTestMatrix.cc -L../.libs -Wl,-rpath -Wl,/home/carlo/c++/libecc/.libs -lecc

// Define this to use a faster algorithm that is MUCH harder to understand :p
#define COMPRESSED 1

// This program calculates the period of a
// shift register random number generator
// with arbitrary feedback points.
//
// Let the internal state of the RNG consist of
// 'b' buckets:
//
// bucket:  0   1   2   3   4  ...   b-1
// value : n0  n1  n2  n3  n4  ... n(b-1)
//
// where b equals the number of buckets and each n represents
// an integer in the range 0 <= n < m, with m = 2^number_of_bits_per_bucket.
// Because of the memory constrains and because the period of
// the RNG doesn't increase much anyway, the rest of the program
// assumes that each bucket is only 1 bit.

// 2^number_of_bits - 1 must be a prime, which
// are called Mersenne primes.

// See http://www.utm.edu/research/primes/mersenne/
unsigned int const mersenne_prime_powers[] =
  { 2, 3, 5, 7, 13, 17, 19, 31, 61, 89, 107, 127, 521, 607, 1279, 2203, 2281,
    3217, 4253, 4423, 9689, 9941, 11213, 19937, 21701, 23209, 44497, 86243, 110503, 132049, 216091,
    756839, 859433, 1257787, 1398269, 2976221, 3021377, /* ...???... */ 6972593, /* ...???...*/ 13466917 };

// which limits the possible values of number_of_bits.

#ifdef M
unsigned int const number_of_bits = M;
#else
unsigned int const number_of_bits = 63;
#endif

// The state of the RNG is changed so that we get
// the following state:
//
// bucket:  0   1   2   3  ...   b-2  b-1
// value : n1  n2  n3  n4  ... n(b-1)  x
//
// where x = n0 + n(b-f0) + n(b-f1) + n(b-f2) + ... + n(b-f(fp-1)) (MOD m)
//       fp are the number of feedback points, and f0, f1, ... f(fp-1)
//       are the feedback points.
// 
// x is also passed as the output of the RNG.

unsigned int const number_of_feedback_points = 1;

// The feedback points must be small prime numbers.
std::vector<unsigned int> primes;
unsigned int const max_prime = number_of_bits;
void init_primes(void);

unsigned int feedback_points[number_of_feedback_points];
int feedback_point_indexes[number_of_feedback_points];
void init_feedback_points(void);

// If we consider the state of a RNG to be a vector.
// then we can calculate the next state of the RNG
// by multiplying the state with a matrix.

struct Matrix {
  //                    columns            rows
  libecc::bitset<number_of_bits> M_matrix[number_of_bits];
  void clear(void) { for (int row = 0; row < number_of_bits; ++row) M_matrix[row].reset(); }
  Matrix(void) { clear(); }
  void square(Matrix const& m);
  void mult(Matrix const& m1, Matrix const& m2);
  friend bool operator==(Matrix const& m1, Matrix const& m2);
  friend std::ostream& operator<<(std::ostream& os, Matrix const& m);
};

void init_m0(Matrix* m0)
{
  // m0 is the matrix describing a shift of one.
  // This means that:
  // 
  // 0 1 0 0 0 0 0 ... 0   n0        n1
  // 0 0 1 0 0 0 0 ... 0   n1        n2
  // 0 0 0 1 0 0 0 ... 0   n2        n3
  // 0 0 0 0 1 0 0 ... 0   n3    =   .
  //       .	       .   .         .
  //       .	       .   .         .
  //	   .	       .   .         n(b-1)
  // 1 ... 1 . 1  ... 	   n(b-1)    n0 + n(b-f0) + n(b-f1) + n(b-f2) + ... + n(b-f(fp-1))
  //  
  // Where the 1's on the bottom row are in column 0, b-f0, b-f1, etc.
  m0->clear();
  for (int row = 0; row < number_of_bits - 1; ++row)
    m0->M_matrix[row].set(row + 1);
  m0->M_matrix[number_of_bits - 1].set<0>();
  for (int f = 0; f < number_of_feedback_points; ++f)
    m0->M_matrix[number_of_bits - 1].set(number_of_bits - feedback_points[f]);
}

// For debugging purposes.
std::ostream& operator<<(std::ostream& os, libecc::bitset<number_of_bits> const* vector)
{
  for (int c = 0; c < number_of_bits; ++c)
  {
    if (vector->test(c))
      os << "1 ";
    else
      os << "0 ";
  }
  return os;
}

int main(void)
{
  Debug( check_configuration() );
//  Debug( libcw_do.on() );
  Debug( dc::notice.on() );

#if 0
  // Make sure that 2^number_of_bits - 1 is a prime.
  int i;
  for (i = 0; i < sizeof(mersenne_prime_powers) / sizeof(unsigned int); ++i)
    if (mersenne_prime_powers[i] == number_of_bits)
      break;
  assert( i != sizeof(mersenne_prime_powers) / sizeof(unsigned int) );
#endif

  init_primes();

//  for(std::vector<unsigned int>::iterator iter = primes.begin(); iter != primes.end(); ++iter)
//    std::cout << (*iter) << std::endl;
//  exit(0);

  init_feedback_points();

#if !COMPRESSED
  Matrix* m0 = new Matrix;
  Matrix* m1 = new Matrix;
#endif

  for(;;)
  {
    std::cout << "Trying " << number_of_bits;
    for (int f = 0; f < number_of_feedback_points; ++f)
      std::cout << '/' << feedback_points[f];
    std::cout << "... " << std::flush;

#if !COMPRESSED
    init_m0(m0);

    // std::cout << "\nShift of 1:\n" << *m0 << std::endl;

#if 0
    // Print out matrixes for arbitrary powers of M0.
    Matrix* m2 = new Matrix;

    int shift = 1;
    int nextshift = 1;
    *m1 = *m0;
    for(;;)
    {
      if (shift == nextshift)
      {
	std::cout << "Shift = " << shift << '\n';
	std::cout << *m1 << '\n';
	nextshift *= 2;				// Edit this to get the powers you want to see.
      }
      m2->mult(*m1, *m0);
      ++shift;
      *m1 = *m2;
      if (shift == nextshift)
      {
	std::cout << "(hit return)\n";
	char tmp[8];
	std::cin.getline(tmp, 8);
      }
    }
#endif

    // A shift of 2 is m[1] = m[0] * m[0]
    // A shift of 4 is m[2] = m[1] * m[1]
    // A shift of 8 is m[3] = m[2] * m[2]
    // etc.
    if (number_of_bits > 521)
      std::cout << "Calculating... ";
    for (int n = 1;;)
    {
      if (number_of_bits > 521)
	std::cout << 'M' << n << ' ' << std::flush;
      m1->square(*m0);

      if (n++ == number_of_bits)
      {
	init_m0(m0);
	if (*m0 == *m1)
	  std::cout << "successful, the period is 2^" << number_of_bits << " - 1.";
	std::cout << std::endl;
	break;
      }

      if (number_of_bits > 521)
	std::cout << 'M' << n << ' ' << std::flush;
      m0->square(*m1);

      if (n++ == number_of_bits)
      {
	init_m0(m1);
	if (*m0 == *m1)
	  std::cout << "successful, the period is 2^" << number_of_bits << " - 1.";
	std::cout << std::endl;
	break;
      }
    }
    if (number_of_bits > 521)
      std::cout << '\n';
#else	// Using compressed matrices.

    // Meaning of M0: See above.
    //
    // The period of n/f1/f2/f3.../fx is the same as the period of n/(n-f1)/(n-f2)/(n-f3).../(n-fx).
    // All our feedback points are < n/2 but we calculate the period of a RNG with the mirrored
    // values, thus a RNG for which all feedback points are > n/2.
    //
    // Example M0
    //
    // Consider the period of a 7/3 RNG (n == 7, feedback point at 3).
    // Its period is the same as that of a 7/4 RNG.
    //
    // M0 is then:
    //
    // 0 1 0 0 0 0 0
    // 0 0 1 0 0 0 0
    // 0 0 0 1 0 0 0
    // 0 0 0 0 1 0 0
    // 0 0 0 0 0 1 0
    // 0 0 0 0 0 0 1
    // 1 0 0 \ 0 0 0
    //       |______ feedback point at '4' , '\' == '1' but represents a feedback point.
    //
    // M0^6 is
    //
    // 0 0 0 0 0 0 1
    // 1 0 0 \ 0 0 0 <-- same feedback point, this row is equal to the last row of M0.
    // 0 1 0 0 \ 0 0 <-- subsequential rows are shifted to the left.
    // 0 0 1 0 0 \ 0
    // 0 0 0 1 0 0 \
    // \ 0 0 \ 1 0 0 \__ when a diagonal "feedback line" runs out of the matrix, it causes all feedback points to be toggled.
    // 0 \ 0 0 \ 1 0

    // The top row of M0^(number_of_bits-1) is (0 0 0 0 0 ... 0 0 1)
    libecc::bitset<number_of_bits>* row0 = new libecc::bitset<number_of_bits>;
    row0->reset();
    row0->set<number_of_bits - 1>();

    // The second row of M0^(number_of_bits-1) is equal to the bottom row of M0.
    libecc::bitset<number_of_bits>* row1 = new libecc::bitset<number_of_bits>("1");
    for (int f = 0; f < number_of_feedback_points; ++f)
      row1->set(feedback_points[f]);

    // Because all feedbacks are > n/2, each feedback point 'line' will reach the left
    // edge at most once and the maximum number of 1's in any column will be
    // number_of_feedback_points * (number_of_feedback_points + 1) + 1.
    //
    // Also, because every column is equal the previous column shifted one bit down
    // except for the columns of feedback points because there the bit can be toggled
    // when a "feedback line" (see graph above) runs left outside the matrix, we only
    // need to store the first column and the columns that correspond to feedback points.
    static unsigned short column[number_of_feedback_points + 1][number_of_feedback_points * (number_of_feedback_points + 1) + 2];
    // Empty everything.
    for (int i = 0; i <= number_of_feedback_points; ++i)
      column[i][0] = 0;		// No 1's in this column.

    // Generation of the first column, and all columns corresponding to feedback points, of M0^(number_of_bits-1).
    // Index of 0 corresponds to the first column, larger indices correspond to column 'feedback_points[index + 1]'.
    for (unsigned short r = 0; r < number_of_bits; ++r)			// r is the row of m0^(number_of_bits-1) that row0 refers to.
    {
      Dout(dc::notice, "row0 = " << row0);
      // First column.
      if (row0->test<0>())
	column[0][++column[0][0]] = r;
      // Columns corresponding to feedback points.
      for (int f = 0; f < number_of_feedback_points; ++f)
      {
	if (row0->test(feedback_points[f]))
	  column[f + 1][++column[f + 1][0]] = r;
      }
      // Generate next `row'.
      bool last_bit_set = row0->test<number_of_bits - 1>();
      row0->shift_op<1, libecc::left, libecc::assign>(*row0);
      if (last_bit_set)
	*row0 ^= *row1;
    }

    // Abuse row0 and row1 alternating as input and output in the loop below.
    libecc::bitset<number_of_bits>* out = row0;
    libecc::bitset<number_of_bits>* in = row1;

    // Calculate M0^(2^number_of_bits).
    // Maxtrices are stored 'compressed' (just the top row).
    // The top row of M0 is (0 1 0 0 0 ... 0).
    out->reset();
    out->set<1>();

    if (number_of_bits > 2000)
      std::cout << "Calculating... ";
    // p == log2(current_power).
    for (int p = 1;;)
    {
      Dout(dc::notice, "Calculating M0^(2^" << p << "))");

      // Swap in and out.
      libecc::bitset<number_of_bits>* tmp = in;
      in = out;
      out = tmp;

      if (number_of_bits > 2000)
	std::cout << 'M' << p << ' ' << std::flush;

      // The square of a compressed matrix C (a vector) is given by
      // the formula: bit n of C^2 is: CT Qn C, where CT is C transposed
      // and Qn is a symmetric matrix where column x is the n-th column
      // M0^x.  This implies that the last column of every Qn is n-th
      // column of M0^(number_of_bits-1) and because of the symmetry
      // this is equal to the transpose of the bottom row.
      // It turns out that Qn exists of a mirror-and-rotation part
      // half of which exists in the left-top part of the matrix and
      // half of which exists in the right-bottom part.  The right
      // bottom part also contains all bits that represent feedback
      // points.
      //
      // A typical Qn looks as follows
      // (this is Q1 of the previous example):
      //
      // Starts in
      //  col 1    Column 1 of M0^6   (both because this is Q1)
      //   | _______ |__
      //   V/        V  \
      // 0 1 0 0 0 0 0  _\ mirror/rotation part.
      // 1 0 0 0 0 0 0 /
      // 0 0 0 0 0 0 1
      // 0 0 0 0 0 1 0
      // 0 0 0 0 1 0 0
      // 0 0 0 1 0 0 0
      // 0 0 1 0 0 0 / <-- bottom row is transpose of last column.
      //
      // (this is Q4):
      //
      // Starts in
      //   col 4   Column 4 of M0^6
      //        \    |
      //         V   V
      // 0 0 0 0 1 0 0 
      // 0 0 0 1 0 0 0
      // 0 0 1 0 0 0 /
      // 0 1 0 0 0 / 0
      // 1 0 0 0 / 0 0
      // 0 0 0 / 0 0 1
      // 0 0 / 0 0 1 /
      //

      // Instead of calculating Q_output_bit, we only determine its bottom row.
      // The bottom row is, as said before, equal to the transpose of the last column.
      // The last column is equal to column `output_bit' of M0^(number_of_bits-1).
      // When output_bit == 0, this is equal to the column[0], when `output_bit'
      // is equal to one of the feedback points, this last column is equal to
      // column[feedback point + 1].  In all other cases, it is equal to the
      // column[] that is on its left side, shifted down that distance.
      // For example:
      //
      // Q6 is
      //           Column 6 of M0^6
      //             |
      //             V
      // 0 0 0 0 0 0 1
      // 0 0 0 0 0 1 0
      // 0 0 0 0 1 0 0
      // 0 0 0 1 0 0 0
      // 0 0 1 0 0 0 /
      // 0 1 0 0 0 / 0
      // 1 0 0 0 / 0 0  <-- bottom row is transpose of column 6 of M0^6.
      //             |
      // M0^6 is (see|above)
      //             V
      // 0 0 0 0 0 0 1
      // 1 0 0 \ 0 0 0
      // 0 1 0 0 \ 0 0
      // 0 0 1 0 0 \ 0
      // 0 0 0 1 0 0 \
      // \ 0 0 \ 1 0 0
      // 0 \ 0 0 \ 1 0
      // ^     ^
      // |     |
      // |   column[1] (first feedback point)
      // column[0] (first column)          Q6   feedback
      //                                    |   |
      //                                    V   V
      // column[1], transposed and shifted (6 - 3) is: (0 0 0 0 / 0 0) which has
      // all 1's that correspond to feedback points set as in the bottom row of Q6.
      //
      int next_feedback_index = 0;
      int feedback_column_offset = 0;
      unsigned short* feedback_column = column[0];

      Dout(dc::notice, "Input is " << in);

      // Calculate the square of 'in' and write the result to 'out'.
      // We do this bit by bit.
      for (int output_bit = 0; output_bit < number_of_bits;)
      {
	bool result = false;
	// This bit is the inproduct of `*in' with 'Q_output_bit *in'.
	// Loop over the bits of the left `*in'.
	int left_in_bit_start = 0;
        for (int left_in_bit_digit = 0;
	     left_in_bit_digit < libecc::bitset<number_of_bits>::digits;
	     ++left_in_bit_digit, left_in_bit_start += libecc::bitset_digit_bits)
	{
	  libecc::bitset_digit_t in_digit = in->digit(left_in_bit_digit);
	  if (in_digit == 0)
	    continue;
	  libecc::bitset_digit_t mask = 1;
	  int left_in_bit = left_in_bit_start;
	  do
	  {
	    // If this bit is not set then it makes no sense to calculate the corresponding bit of Q_output_bit *in.
	    if (in_digit & mask)
	    {
	      // Now we need to toggle `result' if the inproduct
	      // of the right `*in' with the `left_in_bit's row of Q_output_bit is set.
	      // We achieve this by toggling result when the corresponding bit
	      // in `*in' is set for every set bit in `left_in_bit's row of Q_output_bit.

	      // First of all, the bits resulting from M0^0 in P (see Theory), the
	      // mirror/rotate part in the left-top part of the Qn matrix.
	      if (left_in_bit <= output_bit && in->test(output_bit - left_in_bit))
		result = !result;		// Toggle output bit.

	      // Next, all other bits.
	      for (int i = 1; i <= feedback_column[0]; ++i)
	      {
		int b = feedback_column[i] + feedback_column_offset + number_of_bits - 1 - left_in_bit;
		if (b >= number_of_bits)
		  break;
		if (in->test(b))
		  result = !result;	// Toggle output bit.
	      }
	    }
	    mask <<= 1;
	    ++left_in_bit;
	  }
	  while((left_in_bit & (libecc::bitset_digit_bits - 1)));
	}
	Dout(dc::notice, "Output bit " << output_bit << " is " << result);
	if (result)
	  out->set(output_bit);
	else
	  out->clear(output_bit);

        if (++output_bit == feedback_points[next_feedback_index])
	{
	  feedback_column = column[next_feedback_index + 1];
	  if (next_feedback_index < number_of_feedback_points - 1)
	    ++next_feedback_index;
	  feedback_column_offset = 0;
	}
	else
	  ++feedback_column_offset;
      }

      if (p++ == number_of_bits)
      {
	in->reset();
	in->set<1>();
	if (*in == *out)
	  std::cout << "successful, the period is 2^" << number_of_bits << " - 1.";
	std::cout << std::endl;
	break;
      }
    }
    if (number_of_bits > 2000)
      std::cout << '\n';
#endif

    //feedback_point_indexes[8] = 54;
    // Next feedback point
    for (int f = number_of_feedback_points - 1;; --f)
    {
      feedback_points[f] = primes[++feedback_point_indexes[f]];
      if (feedback_points[f] <= number_of_bits / 2)
      {
	while(++f < number_of_feedback_points)
	{
	  feedback_point_indexes[f] = feedback_point_indexes[f - 1] + 1;
	  feedback_points[f] = primes[feedback_point_indexes[f]];
	  if (feedback_points[f] > number_of_bits / 2)
	    break;
	}
	if (feedback_points[f] <= number_of_bits / 2)
	  break;
      }
      if (f == 0)
	return 0;
    }
    //feedback_point_indexes[number_of_feedback_points - 1] = 53;
    //feedback_points[number_of_feedback_points - 1] = primes[53];
  }

  return 0;
}

void init_primes(void)
{
#if 1
  // Use any feedback point, not just primes.
  for (int p = 1; p < number_of_bits; ++p)
    primes.push_back(p);
  return;
#endif
  primes.push_back(2);
  int const zs = max_prime / 64 + 1;
  double mp = (32 * zs - 1) * 2 + 3;
  unsigned int sr = (unsigned int)(sqrt(mp) + 0.5);
  uint32_t* z = new uint32_t [zs];
  std::memset(z, 0, zs * sizeof(uint32_t));
  for (int i = 0; i < zs; ++i)
    for (int s = 0; s < 32; ++s)
      if ((z[i] & (1 << s)) == 0)
      {
	unsigned int p = (32 * i + s) * 2 + 3;
	if (p > max_prime)
	{
	  delete [] z;
	  return;
	}
	primes.push_back(p);
	if (p >= sr)
	  continue;
	for (int q = p; q < p * 33; q += p)
	{
	  uint32_t m = 1 << ((s + q) % 32);
	  for (int j = i + (s + q) / 32; j < zs; j += p)
	    z[j] |= m;
	}
      }
  delete [] z;
}

void init_feedback_points(void)
{
#if 1
  // Use small feedback numbers.
  for (int f = 0; f < number_of_feedback_points; ++f)
  {
    feedback_point_indexes[f] = f /* + 1 */;	// +1 = skip feedback of 2.
  }
#else
  // Manual override.
  feedback_point_indexes[0] = 0;
  feedback_point_indexes[1] = 1;
  feedback_point_indexes[2] = 3;
  feedback_point_indexes[3] = 5;
  feedback_point_indexes[4] = 10;
  feedback_point_indexes[5] = 17;
  feedback_point_indexes[6] = 31;
  feedback_point_indexes[7] = 35;
  feedback_point_indexes[8] = 53;
#endif

  for (int f = 0; f < number_of_feedback_points; ++f)
  {
    feedback_points[f] = primes[feedback_point_indexes[f]];
  }
}

struct BitIterator {
  int cnt;
  libecc::bitset<number_of_bits> const* bp;
  libecc::bitset_digit_t bit_mask;
  int digit;
  BitIterator(Matrix const& m) : cnt(number_of_bits), bp(&m.M_matrix[0]), bit_mask(1), digit(0) { }
  bool next_bit(void)
  {
    if (--cnt == 0)
      return false;
    ++bp;
    bit_mask <<= 1;
    if (bit_mask == 0)
    {
      bit_mask = 1;
      ++digit;
    }
    return true;
  }
};

void Matrix::square(Matrix const& m)
{
  for (int row = 0; row < number_of_bits; ++row)
    M_matrix[row].reset();
  BitIterator col(m);
  do
  {
    // Make a copy of column col. 
    libecc::bitset<number_of_bits> column;
    libecc::bitset_digit_t res = 0;
    BitIterator r(m);
    do
    {
      if (r.bp->digit(col.digit) & col.bit_mask)
	res |= r.bit_mask;
      if (r.bit_mask == 0x80000000)
      {
	column.rawdigit(r.digit) = res;
	res = 0;
      }
    }
    while(r.next_bit());
    column.rawdigit(libecc::bitset<number_of_bits>::digits - 1) = res;
    // Calculate inproduct of this column with any of the rows
    for (int row = 0; row < number_of_bits; ++row)
    {
      libecc::bitset<number_of_bits> tmp = column & m.M_matrix[row];
      if (tmp.odd())
	M_matrix[row].rawdigit(col.digit) |= col.bit_mask;
    }
  }
  while(col.next_bit());
}

void Matrix::mult(Matrix const& m1, Matrix const& m2)
{
  for (int row = 0; row < number_of_bits; ++row)
    M_matrix[row].reset();
  BitIterator col(m2);
  do
  {
    // Make a copy of column col. 
    libecc::bitset<number_of_bits> column;
    libecc::bitset_digit_t res = 0;
    BitIterator r(m1);
    do
    {
      if (r.bp->digit(col.digit) & col.bit_mask)
	res |= r.bit_mask;
      if (r.bit_mask == 0x80000000)
      {
	column.rawdigit(r.digit) = res;
	res = 0;
      }
    }
    while(r.next_bit());
    column.rawdigit(libecc::bitset<number_of_bits>::digits - 1) = res;
    // Calculate inproduct of this column with any of the rows
    for (int row = 0; row < number_of_bits; ++row)
    {
      libecc::bitset<number_of_bits> tmp = column & m2.M_matrix[row];
      if (tmp.odd())
	M_matrix[row].rawdigit(col.digit) |= col.bit_mask;
    }
  }
  while(col.next_bit());
}

std::ostream& operator<<(std::ostream& os, Matrix const& m)
{
  for (int r = 0; r < number_of_bits; ++r)
  {
    for (int c = 0; c < number_of_bits; ++c)
    {
      if (m.M_matrix[r].test(c))
	os << "1 ";
      else
	os << "0 ";
    }
    os << '\n';
  }
  return os;
}

bool operator==(Matrix const& m1, Matrix const& m2)
{
  for (int r = 0; r < number_of_bits; ++r)
    if (m1.M_matrix[r] != m2.M_matrix[r])
	return false;
  return true;
}
