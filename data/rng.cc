//
//* \file  rng.cc
//* \brief Random number generator.
//
// This file is part of the libecc package.
// Copyright (C) 2002, by
//
// Carlo Wood, Run on IRC <carlo@alinoe.com>
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "sys.h"
#include "debug.h"
#include <libecc/rng.h>

namespace libecc {

/**\class rng rng.h libecc/rng.h
 *
 * \brief Pseudo Random Number Generator.
 *
 * This random number generator was designed from scratch.
 * It is fast (2.5 million bit/second on a 900 Mhz PC),
 * has a long period (2^521 - 1) and passes all known statistical
 * tests for randomness.&nbsp;
 *
 * \note
 * \htmlonly<FONT size=-1>\endhtmlonly
 * The chosen period of 2^521 - 1 is by far large enough for any purpose;
 * when 15 billion people each using 10,000 PCs with a clock frequency of 100 Ghz
 * generate random numbers for the next 10 million years - then the chance that \e any two of
 * them ever generated a (partly) overlapping sequence of bits is less than
 * 0.000...(108&nbsp;zeroes)...0001.&nbsp;
 * In order words, if you chose a good random seed, you \e will be
 * the only one who ever uses the resulting sequence of output bits.
 * \htmlonly</FONT>\endhtmlonly
 *
 * <H4>Theory</H4>
 *
 * The basis of this Random Number Generator (RNG) is a shift register of 521 bit using nine feedback points.&nbsp;
 * Because such a feedback is linear, it is possible to
 * write any amount of shift as <TT>&nbsp;NEWSTATE = MATRIX * OLDSTATE</TT>.&nbsp;
 * The matrix that corresponds with a single shift can easily be determined
 * as function of the feedback points.&nbsp;
 * Let's call this matrix <TT>&nbsp;M0</TT>.&nbsp;
 * The matrix corresponding to a shift of 2 then corresponds to <TT>&nbsp;M1 = M0 * M0</TT>.&nbsp;
 * A shift of 4 corresponds to <TT>&nbsp;M2 = M1 * M1</TT>, etcetera.
 *
 * The period of the RNG will be equal to the number of different internal states that will
 * be used.&nbsp; In the optimal case all possible internal states, except all zeroes which would
 * lead to an output of only zeroes, will be used and the period of the RNG
 * will be equal to 2<SUP>521</SUP> - 1.&nbsp;
 * This hypothesis can be checked by confirming that <TT>M521 = M0<SUP>2<SUP>521</SUP></SUP> == M0&nbsp;</TT>
 * and because 2<SUP>521</SUP> - 1 is a prime this value then must be the real period of the RNG.&nbsp;
 * Of course, one of the feedback points is fixed at bit 520 (feeding to bit 0).&nbsp;
 * By choosing the other feedback points close to bit 0, a very fast mangling of the bits is achieved.
 *
 * \note
 * \htmlonly<FONT size=-1>\endhtmlonly
 * Even with a seed of '1' (a single bit) the first resulting 512 bits of output appear to be totally random already.&nbsp;
 * After 9 calls to generate_512_bits(), each bit in the pool has been set at least once.&nbsp;
 * \htmlonly</FONT>\endhtmlonly
 *
 * <H5>Verifying the period, an example</H5>
 *
 * Suppose we have a shift register of 3 bits (2<SUP>3</SUP>-1 = 7 is a prime).&nbsp;
 * Let's use one feedback point from bit 1 (and from bit 2 of course), then we have:
 *
 * <PRE>
 * bit        0    1    2(output)
 * state0:    a    b    c
 * state1:    b    c   a^b
 * state2:    c   a^b  b^c
 *   ...
 * </PRE>
 *
 * Which gives, with a starting seed of 001:
 *
 * <PRE>
 * 001
 * 010
 * 101
 * 011
 * 111
 * 110
 * 100
 * and back to the top
 * </PRE>
 *
 * Or rotated 90 degrees:
 *
 * <PRE>
 * bit
 *  0  0010111
 *  1  0101110
 *  2  1011100
 * </PRE>
 *
 * Each column can be considered as a vector:
 *
 * <PRE>
 * a
 * b
 * c
 * </PRE>
 *
 * The matrix representing one shift now is:
 *
 * <PRE>
 * m11 m12 m13  a   a m11 + b m12 + c m13    b
 * m21 m22 m23  b = a m21 + b m22 + c m23 =  c
 * m31 m32 m33  c   a m31 + b m32 + c m33    a+b mod 2 = a^b
 * </PRE>
 *
 * From which we can deduce the matrix
 *
 * <PRE>
 *      0 1 0
 * M0 = 0 0 1
 *      1 1 0
 * </PRE>
 *
 * and that we have to do our calculation modulo 2.
 *
 * Note that
 *
 * <PRE>
 *                0 1 0   0 1 0   0 0 1
 * M1 = M0 * M0 = 0 0 1   0 0 1 = 1 1 0
 *                1 1 0   1 1 0   0 1 1
 *
 *                0 0 1   0 0 1   0 1 1
 * M2 = M1 * M1 = 1 1 0   1 1 0 = 1 1 1
 *                0 1 1   0 1 1   1 0 1
 * </PRE>
 *
 * and
 *
 * <PRE>
 *                0 1 1   0 1 1   0 1 0
 * M3 = M2 * M2 = 1 1 1   1 1 1 = 0 0 1 = M0
 *                1 0 1   1 0 1   1 1 0
 *
 * </PRE>
 *
 * Which proves that this repeats after 2<SUP>3</SUP>-1 = 7 times.
 *
 * <H5>Predictability</H5>
 *
 * Because the state of the RNG changes in a linear way (we can use matrices),
 * the output is predictable: the internal state can be calculated from a
 * small amount of output.
 *
 * In the above example assume that the RNG produces the output sequence xyz.&nbsp;
 * If the orginal state was abc then after three bits of output the state would
 * be:
 *
 * <PRE>
 *       a   0 1 0   0 0 1   a   1 1 0   a   a+b
 * M0 M1 b = 0 0 1   1 1 0   b = 0 1 1   b = b+c      (mod 2)
 *       c   1 1 0   0 1 1   c   1 1 1   c   a+b+c
 * </PRE>
 *
 * while the output would be
 *
 * <PRE>
 * x   0 0 1   <SUP> </SUP> a     0 0 0   <SUP> </SUP> a     0 0 0   <SUP> </SUP> a   1 1 0  a
 * y = 0 0 0 M0<SUP>1</SUP> b  +  0 0 1 M0<SUP>2</SUP> b  +  0 0 0 M0<SUP>3</SUP> b = 0 1 1  b
 * z   0 0 0   <SUP> </SUP> c     0 0 0   <SUP> </SUP> c     0 0 1   <SUP> </SUP> c   1 1 1  c
 * </PRE>
 *
 * The inverse of the latter allows one to crack the RNG (calculate the current state from x,y,z).
 *
 * <H5>Matrix compression</H5>
 *
 * Although it might sound easy to check if the period of a given
 * RNG is maximal, in practise it still isn't when we want to use
 * a huge number of bits.&nbsp;
 * I wrote a program that does 4 Gigabit operations per second,
 * it was therefore able to do 3 times 127 matrix multiplications
 * of 127x127 matrices per second.&nbsp;
 * That means that it is reasonably fast to look for working
 * feedback points.&nbsp; Let's say, one minute was enough.&nbsp;
 * But if then we try to use this program to find feedback points
 * for a shift register of 1279 bits, it suddenly takes (1279/127)<SUP>4</SUP> = 10286 minutes = 1 week!
 * And 1279 bits is <EM>still</EM> not really big...&nbsp; Moreover, storing large matrices
 * uses a lot of memory.&nbsp; A matrix of 19937 bits (2<SUP>19937</SUP> - 1 is the period
 * of the <A HREF="http://www.math.keio.ac.jp/~matumoto/emt.html">Mersenne Twister</A>) would
 * use 47.4 Mb per matrix already.
 *
 * Fortunately, the matrices that we need to prove the period of are not arbitrary.&nbsp;
 * As can be deduced from the paragraphs above, M0 can be written as
 *
 * <PRE>
 *      0 1 0 0 0 0 0 ... 0 0
 *      0 0 1 0 0 0 0 ... 0 0
 *      0 0 0 1 0 0 0 ... 0 0
 *            .&nbsp;
 * M0 =       .&nbsp;
 *            .&nbsp;
 *      0 0 0 0 0 0 0 ... 1 0
 *      0 0 0 0 0 0 0 ... 0 1
 *      1 0 0 ... 1 ... 1 ...
 * </PRE>
 *
 * Where the 1's on the bottom row are at the feedback point places.
 *
 * As a result, we can write M0 (for a pxp matrix) as
 * a vector of rows as follows
 *
 * <PRE>
 *       (0 1 0 0 0 0 0 ... 0)
 *       (0 1 0 0 0 0 0 ... 0) M0<SUP>1</SUP>
 *       (0 1 0 0 0 0 0 ... 0) M0<SUP>2</SUP>
 *       (0 1 0 0 0 0 0 ... 0) M0<SUP>3</SUP>
 * M0 =       .&nbsp;
 *            .&nbsp;
 *            .&nbsp;
 *       (0 1 0 0 0 0 0 ... 0) M0<SUP>p-1</SUP>
 * </PRE>
 *
 * If we multiply this an arbitrary number of times with M0 (say k times),
 * then we get:
 *
 * <PRE>
 *      <SUP> </SUP>   (0 1 0 0 0 0 0 ... 0) M0<SUP>k&nbsp;</SUP>           (0 1 0 0 0 0 0 ... 0) M0<SUP>k</SUP>
 *      <SUP> </SUP>   (0 1 0 0 0 0 0 ... 0) M0<SUP>1</SUP> M0<SUP>k</SUP>        (0 1 0 0 0 0 0 ... 0) M0<SUP>k</SUP> M0<SUP>1</SUP>
 *      <SUP> </SUP>   (0 1 0 0 0 0 0 ... 0) M0<SUP>2</SUP> M0<SUP>k</SUP>        (0 1 0 0 0 0 0 ... 0) M0<SUP>k</SUP> M0<SUP>2</SUP>
 *      <SUP> </SUP>   (0 1 0 0 0 0 0 ... 0) M0<SUP>3</SUP> M0<SUP>k</SUP>        (0 1 0 0 0 0 0 ... 0) M0<SUP>k</SUP> M0<SUP>3</SUP>
 * M0 M0<SUP>k</SUP> =       .&nbsp;                        =          .&nbsp;
 *      <SUP> </SUP>         .&nbsp;                                   .&nbsp;
 *      <SUP> </SUP>         .&nbsp;                                   .&nbsp;
 *      <SUP> </SUP>   (0 1 0 0 0 0 0 ... 0) M0<SUP>p-1</SUP> M0<SUP>k</SUP>       (0 1 0 0 0 0 0 ... 0) M0<SUP>k</SUP> M0<SUP>p-1</SUP>
 * </PRE>
 *
 * which proves that every matrix that is a power of M0 can be expressed in terms of the top row
 * of the matrix and M0 as follows:
 *
 * <PRE>
 * Row(N+1) = Row(N) M0
 * </PRE>
 *
 * <b>Example</b>
 *
 * Consider a RNG of 7 bits with a single feedback points at bit 7 - 3 = 4.&nbsp;
 * The first 7 powers of M0 for that RNG are
 *
 * <PRE>
 *   <SUP> </SUP>   1 0 0 0 0 0 0
 *   <SUP> </SUP>   <B>0 1 0 0 0 0 0</B>
 *   <SUP> </SUP>   0 0 1 0 0 0 0
 * M0<SUP>0</SUP> = 0 0 0 1 0 0 0
 *   <SUP> </SUP>   0 0 0 0 1 0 0
 *   <SUP> </SUP>   0 0 0 0 0 1 0
 *   <SUP> </SUP>   0 0 0 0 0 0 1
 *
 *   <SUP> </SUP>   0 1 0 0 0 0 0       <SUP> </SUP>   0 0 1 0 0 0 0       <SUP> </SUP>   0 0 0 1 0 0 0
 *   <SUP> </SUP>   <B>0 0 1 0 0 0 0</B>       <SUP> </SUP>   <B>0 0 0 1 0 0 0</B>       <SUP> </SUP>   <B>0 0 0 0 1 0 0</B>
 *   <SUP> </SUP>   0 0 0 1 0 0 0       <SUP> </SUP>   0 0 0 0 1 0 0       <SUP> </SUP>   0 0 0 0 0 1 0
 * M0<SUP>1</SUP> = 0 0 0 0 1 0 0  ,  M0<SUP>2</SUP> = 0 0 0 0 0 1 0  ,  M0<SUP>3</SUP> = 0 0 0 0 0 0 1
 *   <SUP> </SUP>   0 0 0 0 0 1 0       <SUP> </SUP>   0 0 0 0 0 0 1       <SUP> </SUP>   1 0 0 0 1 0 0
 *   <SUP> </SUP>   0 0 0 0 0 0 1       <SUP> </SUP>   1 0 0 0 1 0 0       <SUP> </SUP>   0 1 0 0 0 1 0
 *   <SUP> </SUP>   1 0 0 0 1 0 0       <SUP> </SUP>   0 1 0 0 0 1 0       <SUP> </SUP>   0 0 1 0 0 0 1
 *
 *   <SUP> </SUP>   0 0 0 0 1 0 0      <SUP> </SUP>   0 0 0 0 0 1 0       <SUP> </SUP>   0 0 0 0 0 0 1
 *   <SUP> </SUP>   <B>0 0 0 0 0 1 0</B>      <SUP> </SUP>   <B>0 0 0 0 0 0 1</B>       <SUP> </SUP>   <B>1 0 0 0 1 0 0</B>
 *   <SUP> </SUP>   0 0 0 0 0 0 1      <SUP> </SUP>   1 0 0 0 1 0 0       <SUP> </SUP>   0 1 0 0 0 1 0
 * M0<SUP>4</SUP> = 1 0 0 0 1 0 0 ,  M0<SUP>5</SUP> = 0 1 0 0 0 1 0  ,  M0<SUP>6</SUP> = 0 0 1 0 0 0 1
 *   <SUP> </SUP>   0 1 0 0 0 1 0      <SUP> </SUP>   0 0 1 0 0 0 1       <SUP> </SUP>   1 0 0 1 1 0 0
 *   <SUP> </SUP>   0 0 1 0 0 0 1      <SUP> </SUP>   1 0 0 1 1 0 0       <SUP> </SUP>   0 1 0 0 1 1 0
 *   <SUP> </SUP>   1 0 0 1 1 0 0      <SUP> </SUP>   0 1 0 0 1 1 0       <SUP> </SUP>   0 0 1 0 0 1 1
 * </PRE>
 *
 * And indeed, each second row of a sequential power of M0 is equivalent to a next row of M0 itself.
 *
 * Now let's consider an arbitrary power k of M0 (in the example below k = 48).
 *
 * <PRE>
 *   <SUP> </SUP>   <B>0 0 1 1 0 1 0</B>
 *   <SUP> </SUP>   0 0 0 1 1 0 1
 *   <SUP> </SUP>   1 0 0 0 0 1 0
 * M0<SUP>k</SUP> = 0 1 0 0 0 0 1
 *   <SUP> </SUP>   1 0 1 0 1 0 0
 *   <SUP> </SUP>   0 1 0 1 0 1 0
 *   <SUP> </SUP>   0 0 1 0 1 0 1
 * </PRE>
 *
 * and let's call the first row C<SUB>k</SUB><SUP>T</SUP>
 *
 * <PRE>
 *  <SUB> </SUB>   0
 *  <SUB> </SUB>   0
 *  <SUB> </SUB>   1
 * C<SUB>k</SUB> = 1
 *  <SUB> </SUB>   0
 *  <SUB> </SUB>   1
 *  <SUB> </SUB>   0
 * </PRE>
 *
 * then
 *
 * <PRE>
 * M0<SUP>k</SUP> = C<SUB>k</SUB><SUP>T</SUP> P
 * </PRE>
 *
 * where <TT>P</TT> is the vector of matrices
 *
 * <PRE>
 *     M0<SUP>0</SUP>
 *     M0<SUP>1</SUP>
 *     M0<SUP>2</SUP>
 * P = M0<SUP>3</SUP>
 *     M0<SUP>4</SUP>
 *     M0<SUP>5</SUP>
 *     M0<SUP>6</SUP>
 * </PRE>
 *
 * <H5>Multiplying compressed matrices</H5>
 *
 * Let <TT>I<SUB>n</SUB></TT> be the n-th column of <TT>I</TT>
 * and <TT>P<SUB>n</SUB> = P I<SUB>n</SUB></TT>, a vector of the n-th column of each of the matrices of <TT>P</TT>.
 * Then the n-th column of M0<SUP>k</SUP> is
 *
 * <PRE>
 * M0<SUP>k</SUP> I<SUB>n</SUB> = C<SUB>k</SUB><SUP>T</SUP> P I<SUB>n</SUB> = C<SUB>k</SUB><SUP>T</SUP> P<SUB>n</SUB>
 * </PRE>
 *
 * or
 *
 * <PRE>
 * I<SUB>n</SUB><SUP>T</SUP> (M0<SUP>k</SUP>)<SUP>T</SUP> = P<SUB>n</SUB><SUP>T</SUP> C<SUB>k</SUB>
 * </PRE>
 *
 * Recall that <TT>C<SUB>k</SUB><SUP>T</SUP></TT> is the top row of <TT>M0<SUP>k</SUP></TT>.&nbsp;
 * That means we can write
 *
 * <PRE>
 * C<SUB>k</SUB> = (I<SUB>0</SUB><SUP>T</SUP> M0<SUP>k</SUP>)<SUP>T</SUP> = (M0<SUP>k</SUP>)<SUP>T</SUP> I<SUB>0</SUB>
 * </PRE>
 *
 * and can deduce that
 *
 * <PRE>
 * C<SUB>m+k</SUB> = (M0<SUP>m+k</SUP>)<SUP>T</SUP> I<SUB>0</SUB> = (M0<SUP>m</SUP> M0<SUP>k</SUP>)<SUP>T</SUP> I<SUB>0</SUB> = (M0<SUP>k</SUP>)<SUP>T</SUP> (M0<SUP>m</SUP>)<SUP>T</SUP> I<SUB>0</SUB> = (M0<SUP>k</SUP>)<SUP>T</SUP> (I<SUB>0</SUB><SUP>T</SUP> M0<SUP>m</SUP>)<SUP>T</SUP> = (M0<SUP>k</SUP>)<SUP>T</SUP> C<SUB>m</SUB> =
 * [sum over n] (I<SUB>n</SUB><SUP>T</SUP> (M0<SUP>k</SUP>)<SUP>T</SUP> C<SUB>m</SUB> I<SUB>n</SUB>) =
 * [sum over n] (P<SUB>n</SUB><SUP>T</SUP> C<SUB>k</SUB> C<SUB>m</SUB> I<SUB>n</SUB>) =
 * [sum over n] (I<SUB>n</SUB><SUP>T</SUP> P<SUP>T</SUP> C<SUB>k</SUB> C<SUB>m</SUB> I<SUB>n</SUB>) =
 * P<SUP>T</SUP> C<SUB>k</SUB> C<SUB>m</SUB>
 * </PRE>
 *
 * from which we conclude that bit n of <TT>C<SUB>k+m</SUB></TT> is
 *
 * <PRE>
 * I<SUB>n</SUB><SUP>T</SUP> C<SUB>k+m</SUB> =
 * I<SUB>n</SUB><SUP>T</SUP> P<SUP>T</SUP> C<SUB>k</SUB> C<SUB>m</SUB> =
 * P<SUB>n</SUB><SUP>T</SUP> C<SUB>k</SUB> C<SUB>m</SUB>
 * </PRE>
 *
 * Note that <TT>P<SUB>n</SUB><SUP>T</SUP></TT> is a row of rows, hence
 * <TT>P<SUB>n</SUB><SUP>T</SUP> C<SUB>k</SUB></TT> is a row
 * which can also be written as <TT>C<SUB>k</SUB><SUP>T</SUP> Q</TT>.&nbsp;
 * The n-th bit of <TT>C<SUB>k+m</SUB></TT> then becomes
 *
 * <PRE>
 * I<SUB>n</SUB><SUP>T</SUP> C<SUB>k+m</SUB> = C<SUB>k</SUB><SUP>T</SUP> Q<SUB>n</SUB> C<SUB>m</SUB>
 * </PRE>
 *
 * where <TT>Q<SUB>n</SUB></TT> is a matrix consisting of the n-th columns of the elements of <TT>P</TT>,
 * in other words
 *
 * <PRE>
 * Q<SUB>n</SUB> = Q I<SUB>n</SUB>
 * </PRE>
 *
 * where
 *
 * <PRE>
 * Q = (M0<SUP>0</SUP> M0<SUP>1</SUP> M0<SUP>2</SUP> ... M0<SUP>p-1</SUP>)
 * </PRE>
 *
 * Note that Q<SUB>n</SUB> is symmetric so that <TT>Q<SUB>n</SUB><SUP>T</SUP> = Q<SUB>n</SUB></TT> and therefore
 * <TT>C<SUB>k</SUB><SUP>T</SUP> Q<SUB>n</SUB> C<SUB>m</SUB> = C<SUB>m</SUB><SUP>T</SUP> Q<SUB>n</SUB> C<SUB>k</SUB></TT>,
 * as it should be since <TT>M0<SUP>k</SUP> M0<SUP>m</SUP> = M0<SUP>m</SUP> M0<SUP>k</SUP></TT>.
 *
 * Using this method it was possible to write a program that checks the period of the RNG
 * in a time of the order O(p<SUP>2</SUP> * f<SUP>3</SUP>), for pxp matrices with f feedback points, 
 * instead of the order O(p<SUP>4</SUP>).  Note that the following observations have been used
 * as well; the period of a RNG with feedback points f1, f2, f3 ... is equal to the period
 * of a RNG with feedback points p - f1, p - f2, p - f3, ...  This allowed us to drastically reduce
 * the order of the number of feedback points.  Finally note the observation that a RNG only has a
 * period of 2<SUP>p</SUP> - 1 solution when using an odd number of feedback points.
 *
 * Considering that it took me something of the order of an hour to find working feedback points
 * for a RNG with 521 bits, it would now take about 9 weeks to find feedback points for a RNG
 * of 19937 bits instead of 262 year as would be the case when using ordinairy matrices.
 *
 * <b>Implementation</b>
 *
 * The implemented Random Number Generator of libecc uses 521 bits and 9 feedback points
 * at 2, 3, 7, 13, 31, 61, 131, 151 and 251.&nbsp; These feedback points are chosen to be primes
 * in order to garantee the least interference.&nbsp; The distance between the feedback points
 * is every time increased by a factor of two (except for the feedback point at 151 which was
 * added in order to make the RNG have its maximum period).&nbsp; The reason for this is again
 * to have the least interference between feedback frequencies, this way the different feedback
 * points nicely supplement each other in achieving bit mangling over the full range.
 */

static unsigned int const feed1 = rng::S_pool_size - 2;
static unsigned int const feed2 = rng::S_pool_size - 3;
static unsigned int const feed3 = rng::S_pool_size - 7;
static unsigned int const feed4 = rng::S_pool_size - 13;
static unsigned int const feed5 = rng::S_pool_size - 31;
static unsigned int const feed6 = rng::S_pool_size - 61;
static unsigned int const feed7 = rng::S_pool_size - 131;
static unsigned int const feed8 = rng::S_pool_size - 151;
static unsigned int const feed9 = rng::S_pool_size - 251;

/**
 * \brief Constructor.
 *
 * \param seed A bitset of 521 bits, any value is allowed except all zeroes.
 */
rng::rng(pool_type const& seed) : M_out_cnt(0), M_entropy_ptr(M_pool), M_entropy_ptr_end(M_pool + sizeof(M_pool) / 4 - 1),
    M_head(M_pool, 0), M_fbp1(M_pool, feed1), M_fbp2(M_pool, feed2), M_fbp3(M_pool, feed3), M_fbp4(M_pool, feed4),
    M_fbp5(M_pool, feed5), M_fbp6(M_pool, feed6), M_fbp7(M_pool, feed7), M_fbp8(M_pool, feed8), M_fbp9(M_pool, feed9)
{
  for (unsigned int d = 0; d < sizeof(M_pool) / 4; ++d)
    M_pool[d] = seed.digit32(d);
}

/** \brief Generate 512 bits.
 *
 * Fills the internal output buffer with 512 new random bits.
 * You can retrieve this output with the member function rng::get_512_bits().
 */
void rng::generate_512_bits(void)
{
  do
  {
    bitset_digit_t c = 0;
    for (bitset_digit_t out_mask = 1; out_mask != 0; out_mask <<= 1)
    {
      uint32_t a = M_head.increment_and_test(M_pool);
      a ^= M_fbp1.increment_and_test(M_pool);
      a ^= M_fbp2.increment_and_test(M_pool);
      a ^= M_fbp3.increment_and_test(M_pool);
      a ^= M_fbp4.increment_and_test(M_pool);
      a ^= M_fbp5.increment_and_test(M_pool);
      a ^= M_fbp6.increment_and_test(M_pool);
      a ^= M_fbp7.increment_and_test(M_pool);
      a ^= M_fbp8.increment_and_test(M_pool);
      a ^= M_fbp9.increment_and_test(M_pool);
      uint32_t b = a >> 16;
      a ^= b;
      b = a >> 8;
      a ^= b;
      if (libecc::oddnumberofbits[a & 0xff])
      {
	M_head.set();
	c |= out_mask;
      }
      else
	M_head.clear();
    }
    M_out.rawdigit(M_out_cnt++) = c;
    M_out_cnt %= (512 / bitset_digit_bits);
  }
  while(M_out_cnt != 0);
}

/** \brief Add entropy to the random number generator pool.
 *
 * By adding entropy to the random number generator, the output
 * becomes more unpredictable.  Its not really necessary to do
 * this however, its hard enough to 'guess' the 521 bits of
 * seed.  It is advisable to use SHA-1 on the output though,
 * in order to make it harder to reverse engineer the internal
 * state of the rng from its output.
 */
void rng::add_entropy(uint32_t const* noise, unsigned int number_of_ints)
{
  for (unsigned int cnt = 0; cnt < number_of_ints; ++cnt)
  {
    *M_entropy_ptr ^= noise[cnt];
    if (++M_entropy_ptr == M_entropy_ptr_end)
      M_entropy_ptr = M_pool;
  }
}

unsigned int const rng::S_pool_size;

} // namespace libecc
