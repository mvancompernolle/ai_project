// Factorize 2^m - 1
// Copyright (c) 2007, Carlo Wood

#include <iostream>
#include <fstream>
#include <gmpxx.h>
#include <set>
#include <vector>
#include <cassert>

//=================================================================================================
// NumberCache

// In order to save space, large numbers are stored by index.

// The index struct. Needed to overload for number_index_t and integer types seperately.
struct number_index_t {
  unsigned long M_value;

  number_index_t(void) { }
  number_index_t(unsigned long index) : M_value(index) { }
  friend bool operator==(number_index_t i1, number_index_t i2) { return i1.M_value == i2.M_value; }
  friend bool operator!=(number_index_t i1, number_index_t i2) { return i1.M_value != i2.M_value; }
  friend bool operator<(number_index_t i1, number_index_t i2) { return i1.M_value < i2.M_value; }
  number_index_t operator++(void) { ++M_value; return *this; }
  number_index_t operator++(int) { number_index_t tmp(*this); ++M_value; return tmp; }
};

// The type of an index / large number pair.
typedef std::pair<number_index_t, mpz_class> index_number_pair_t;

// A functor to order index_number_pair_t by number.
struct index_number_compare {
  bool operator()(index_number_pair_t const& p1, index_number_pair_t const& p2) const;
};

inline bool index_number_compare::operator()(index_number_pair_t const& p1, index_number_pair_t const& p2) const
{
  // Only order on number.
  return p1.second < p2.second;
}

// This class stores numbers by index.
class NumberCache {
  public:
    number_index_t store(mpz_class const& number);
    mpz_class const& retrieve(number_index_t index);

  private:
    typedef std::vector<mpz_class> index_number_pairs_t;
    typedef std::set<index_number_pair_t, index_number_compare> number_index_pairs_t;

    static number_index_t S_next_index;

    index_number_pairs_t M_index_number_pairs;	// Index to number.
    number_index_pairs_t M_number_index_pairs;	// Number to index.
};

number_index_t NumberCache::S_next_index = 0;

number_index_t NumberCache::store(mpz_class const& number)
{
  number_index_pairs_t::value_type tmp = number_index_pairs_t::value_type(0, number);
  std::pair<number_index_pairs_t::iterator, bool> result = M_number_index_pairs.insert(tmp);
  if (result.second)
  {
    // New number? Set the index of the pair to a new index.
    const_cast<index_number_pair_t&>(*result.first).first = S_next_index++;
    // Also insert this pair in the index_number_pairs lookup table.
    M_index_number_pairs.push_back(number);
  }
  return result.first->first;
}

inline mpz_class const& NumberCache::retrieve(number_index_t index)
{
  assert(M_index_number_pairs.size() > index.M_value);
  return M_index_number_pairs[index.M_value];
}

NumberCache number_cache;

// End NumberCache
//=================================================================================================

//=================================================================================================
// Number

// The Number class is to get rid of the number_index_t / mpz_class duality.
// It should not be used everywhere, especially not when speed is important,
// but it should be used where space considerations outweight speed. Otherwise
// use mpz_class.

class Number {
  public:
    // Create an uninitialized Number.
    Number(void) { }
    // Create a number from a mpz_class.
    Number(mpz_class const& mpz) : M_number_index(number_cache.store(mpz)) { }
    // Create a number from an unsigned long.
    Number(unsigned int n) : M_number_index(number_cache.store(n)) { }
    // Create a number from an existing index.
    Number(number_index_t index) : M_number_index(index) { }
    // Assignment operator.
    Number& operator=(mpz_class const& mpz) { M_number_index = number_cache.store(mpz); return *this; }

    // Compare a Number with another Number.
    friend bool operator==(Number const& n1, Number const& n2) { return n1.M_number_index == n2.M_number_index; }
    friend bool operator!=(Number const& n1, Number const& n2) { return n1.M_number_index != n2.M_number_index; }
    // Compare size.
    friend bool operator< (Number const& n1, Number const& n2) { return number_cache.retrieve(n1.M_number_index) < number_cache.retrieve(n2.M_number_index); }
    friend bool operator<=(Number const& n1, Number const& n2) { return n1 == n2 || n1 < n2; }
    friend bool operator> (Number const& n1, Number const& n2) { return n2 < n1; }
    friend bool operator>=(Number const& n1, Number const& n2) { return n1 == n2 || n2 < n1; }
    // Compare with 0.
    bool operator!(void) { return number_cache.retrieve(M_number_index) == 0; }
    // Arthimetic operators.
    Number& operator+=(Number const& n)
      { M_number_index = number_cache.store(number_cache.retrieve(M_number_index) + number_cache.retrieve(n.M_number_index));
        return *this; }
    Number& operator+=(unsigned int n)
      { M_number_index = number_cache.store(number_cache.retrieve(M_number_index) + n);
        return *this; }
    Number& operator-=(Number const& n)
      { M_number_index = number_cache.store(number_cache.retrieve(M_number_index) - number_cache.retrieve(n.M_number_index));
        return *this; }
    Number& operator-=(unsigned int n)
      { M_number_index = number_cache.store(number_cache.retrieve(M_number_index) - n);
        return *this; }
    Number& operator*=(Number const& n)
      { M_number_index = number_cache.store(number_cache.retrieve(M_number_index) * number_cache.retrieve(n.M_number_index));
        return *this; }
    Number& operator*=(unsigned int n)
      { M_number_index = number_cache.store(number_cache.retrieve(M_number_index) * n);
        return *this; }
    Number& operator/=(Number const& n)
      { M_number_index = number_cache.store(number_cache.retrieve(M_number_index) / number_cache.retrieve(n.M_number_index));
        return *this; }
    Number& operator/=(unsigned int n)
      { M_number_index = number_cache.store(number_cache.retrieve(M_number_index) / n);
        return *this; }
    Number& operator%=(Number const& n)
      { M_number_index = number_cache.store(number_cache.retrieve(M_number_index) % number_cache.retrieve(n.M_number_index));
        return *this; }
    Number& operator%=(unsigned int n)
      { M_number_index = number_cache.store(number_cache.retrieve(M_number_index) % n);
        return *this; }
    Number& operator&=(Number const& n)
      { M_number_index = number_cache.store(number_cache.retrieve(M_number_index) & number_cache.retrieve(n.M_number_index));
        return *this; }
    Number& operator|=(Number const& n)
      { M_number_index = number_cache.store(number_cache.retrieve(M_number_index) | number_cache.retrieve(n.M_number_index));
        return *this; }
    Number& operator^=(Number const& n)
      { M_number_index = number_cache.store(number_cache.retrieve(M_number_index) ^ number_cache.retrieve(n.M_number_index));
        return *this; }
    friend Number operator+(Number const& n1, Number const& n2) { Number result(n1); result += n2; return result; }
    friend Number operator+(Number const& n1, unsigned long n2) { Number result(n1); result += n2; return result; }
    friend Number operator-(Number const& n1, Number const& n2) { Number result(n1); result -= n2; return result; }
    friend Number operator-(Number const& n1, unsigned long n2) { Number result(n1); result -= n2; return result; }
    friend Number operator*(Number const& n1, Number const& n2) { Number result(n1); result *= n2; return result; }
    friend Number operator*(Number const& n1, unsigned long n2) { Number result(n1); result *= n2; return result; }
    friend Number operator/(Number const& n1, Number const& n2) { Number result(n1); result /= n2; return result; }
    friend Number operator/(Number const& n1, unsigned long n2) { Number result(n1); result /= n2; return result; }
    friend Number operator%(Number const& n1, Number const& n2) { Number result(n1); result %= n2; return result; }
    friend Number operator%(Number const& n1, unsigned long n2) { Number result(n1); result %= n2; return result; }
    friend Number operator&(Number const& n1, Number const& n2) { Number result(n1); result &= n2; return result; }
    friend Number operator|(Number const& n1, Number const& n2) { Number result(n1); result |= n2; return result; }
    friend Number operator^(Number const& n1, Number const& n2) { Number result(n1); result ^= n2; return result; }

    // Printing a Number.
    friend std::ostream& operator<<(std::ostream& os, Number const& n) { return os << number_cache.retrieve(n.M_number_index); }

    // Accessors.
    number_index_t index(void) const { return M_number_index; }
    mpz_class const& evaluate(void) const { return number_cache.retrieve(M_number_index); }

  private:
    number_index_t M_number_index;	// The index of this number into the number_cache.

    // A functor used to order Number by index.
    friend struct number_compare;
};

// A functor to order Number by index.
struct number_compare {
  bool operator()(Number const& n1, Number const& n2) const { return n1.M_number_index < n2.M_number_index; }
};

// End of Number
//=================================================================================================

//=================================================================================================
// Power

// This class represents a factor of the form p^n.
class Power {
  public:
    // Create an uninitialized Power.
    Power(void) { }
    // Create a Power with base p and exponent n.
    Power(Number const& p, unsigned int n = 1) : M_base(p), M_exponent(n) { }
    // Assignment operator.
    Power& operator=(Power const& pn) { M_base = pn.M_base; M_exponent = pn.M_exponent; return *this; }
    // Explicitely set the exponent.
    Power& set_exponent(unsigned int n) { M_exponent = n; return *this; }

    // Arithmetic operators.
    Power& operator*=(Power const& pn) { M_exponent += pn.M_exponent; return *this; }
    Power& operator/=(Power const& pn) { M_exponent -= pn.M_exponent; return *this; }
    friend Power operator*(Power const& pn1, Power const& pn2) { Power result(pn1); result *= pn2; return result; }
    friend Power operator/(Power const& pn1, Power const& pn2) { Power result(pn1); result /= pn2; return result; }
    // Overload operator^ with meaning "raise to the power".
    Power& operator^=(unsigned int n) { M_exponent *= n; return *this; }
    friend Power operator^(Power const& pn, unsigned int n) { Power result(pn); result ^= n; return result; }

    // Evaluate expression.
    mpz_class evaluate(void) const;

    // Print the factor.
    friend std::ostream& operator<<(std::ostream& os, Power const& pn)
      { if (pn.M_exponent == 0) { os << "1"; } else { os << pn.M_base; if (pn.M_exponent != 1) os << "^" << pn.M_exponent; } return os; }

    // Accessors.
    Number base(void) const { return M_base; }
    unsigned int exponent(void) const { return M_exponent; }

  private:
    Number M_base;
    unsigned int M_exponent;

    // A functor used to order Number by index.
    friend struct power_compare;
};

// A functor to order Power by the index of the base.
struct power_compare {
  number_compare M_number_compare;
  bool operator()(Power const& pn1, Power const& pn2) const { return M_number_compare(pn1.M_base, pn2.M_base); }
};

mpz_class Power::evaluate(void) const
{
  if (M_exponent == 1)
    return M_base.evaluate();
  else if (M_exponent == 0)
    return 1;
  mpz_class result;
  mpz_pow_ui(result.get_mpz_t(), M_base.evaluate().get_mpz_t(), M_exponent);
  return result;
}

// End of Power
//=================================================================================================

//=================================================================================================
// Factor

typedef std::set<Power, power_compare> factors_t;

class Factor {
  public:
    // Create an object of type Factor representing 1.
    Factor(void) { }

    // Arithmetic.
    Factor& operator*=(Power const& pn);
    Factor operator*(Power const& pn) { Factor result(*this); result *= pn; return result; }

    // Retrieve a reference to the factor with base p.
    Power& factor(Number const& p);
    //Power const& factor(Number const& p) const;

    // Evaluate expression.
    mpz_class evaluate(void) const;

    // Print a Power.
    friend std::ostream& operator<<(std::ostream& os, Factor const& factor);

    // Accessors.
    factors_t& factors(void) { return M_factors; }
    factors_t const& factors(void) const { return M_factors; }
    bool empty(void) const { return M_factors.empty(); }

  private:
    factors_t M_factors;
};

Power& Factor::factor(Number const& p)
{
  Power tmp(p, 0);
  std::pair<factors_t::iterator, bool> result = M_factors.insert(tmp);
  return const_cast<Power&>(*result.first);
}

Factor& Factor::operator*=(Power const& pn)
{
  std::pair<factors_t::iterator, bool> result = M_factors.insert(pn);
  if (result.second)
    const_cast<Power&>(*result.first) = pn;
  else
    const_cast<Power&>(*result.first) *= pn;
  return *this;
}

mpz_class Factor::evaluate(void) const
{
  if (M_factors.empty())
    return 1;
  mpz_class result(1);
  for (factors_t::const_iterator iter = M_factors.begin(); iter != M_factors.end(); ++iter)
    result *= iter->evaluate();
  return result;
}

std::ostream& operator<<(std::ostream& os, Factor const& factor)
{
  bool first = true;
  for (factors_t::const_iterator iter = factor.M_factors.begin(); iter != factor.M_factors.end(); ++iter)
  {
    if (first)
      first = false;
    else
      os << " * ";
    os << *iter;
  }
  if (first)
    os << "1";
  return os;
}

// End of Factor
//=================================================================================================

typedef unsigned short m_t;	// Should be capable of containing m + 2.

m_t const maxm = 1000;
Factor factors[maxm + 1];

#undef DEBUG_ADD_FACTOR

// CONJECTURE 1, by Carlo Wood, 13 januari 2007.
//
// Let k, m be non-negative integers, and p prime.
// If p | 2^m - 1 and p^n | k, then p^(n+1) | 2^(k*m) - 1.
//
//
// Note that p^0 = 1, and 1 | k for any k, so
// if p | 2^m - 1, then p | 2^(k*m) - 1 for any k > 0.
//
// PROOF
//
// Proof for n = 0,           ___k-1
//                            \ .
// 2^(k*m) - 1 = (2^m - 1) *  /__    [2^(m*i)]
//                               i=0
//
// Mathematica: (2^m - 1) * Sum[(2^m)^i, {i, 0, k - 1}] // Simplify
//
// Thus, whatever prime devides (2^m - 1) will devide 2^(k*m) - 1.
//
// Proof for n = 1,             ___k-1
//                              \ .
// 2^(k*m) - 1 = (2^m - 1)^2 *  /__    [(k - 1 - i) * 2^(im)]  +  k * (2^m - 1)
//                                 i=0
//
// Mathematica: Simplify[(2^m - 1)^2 * Sum[(k - 1 - i) * (2^m)^i, {i, 0, k - 1}] + k * (2^m - 1), {n, k} \[Element] Integers]
// Note that if you use the sum to k-2, which should be the same, then Mathematica doesn't see it.
//
// Thus, if p | 2^m - 1 and p | k, then p^2 | 2^(k*m) - 1.
//
// Proof for arbitrary n
//
// (x^(k*m) - 1) = (x^m - 1)^(n+1) * Sum[Binomial[k-1-i,n] * x^(m*i), {i,0,k-1}] +
//                 (x^m - 1)       * Sum[P[i,n,k]          * x^(m*i), {i,0,n-1}]		[1]
//
// where P[i,n,k], the "Pink functions" as I called them, are defined below.
//
// Binomial[k-1-i,n] = (k-1-i)! / (n! * (k-1-i - n)!, if k-1-i >= n, and 0 otherwise.
//
// Thus, the last n terms of the first Sum are all zero, so it is sufficient
// to sum till i = k-1-n instead of till i=k-1. If k <= n then the first sum
// completely disappears. The second sum disappears if n = 0.
//
// It follows that if p | 2^m - 1 then p^(n+1) | 2^(k*m) - 1
// if p^n | Sum[P[i,n,k] * x^(m*i), {i,0,n-1}].
//                               
// The Pink functions are defined for 0 <= i < n as follows.
// P[i,n,k] are polynomials (hence the P) in k of degree n that
// are defined by: P[i,n,k] = 0 if k <= i, and 1 if i < k <= n.
// The value of |P[i,n,k]| for k > n rapidly grows to large numbers, of course.
//
// Here is a list of all Pink polynomials for n <= 6,
//
// n=1
//
// P(0, 1, k) = k
//
// n=2
//
// P(0, 2, k) = k * (-k + 3) / 2
// P(1, 2, k) = k * ( k - 1) / 2
//
// n=3
//
// P(0, 3, k) = k * (  k^2 - 6k + 11) / n!
// P(1, 3, k) = k * (-2k^2 + 9k -  7) / n!
// P(2, 3, k) = k * (  k^2 - 3k +  2) / n!
//
// n=4
//
// P(0, 4, k) = k * ( -k^3 + 10k^2 - 35k + 50) / n!
// P(1, 4, k) = k * ( 3k^3 - 26k^2 + 69k - 46) / n!
// P(2, 4, k) = k * (-3k^3 + 22k^2 - 45k + 26) / n!
// P(3, 4, k) = k * (  k^3  - 6k^2 + 11k -  6) / n!
//
// n=5
//
// P(0, 5, k) = k * (  k^4 - 15k^3 +  85k^2 - 225k + 274) / n!
// P(1, 5, k) = k * (-4k^4 + 55k^3 - 270k^2 + 545k - 326) / n!
// P(2, 5, k) = k * ( 6k^4 - 75k^3 + 320k^2 - 525k + 274) / n!
// P(3, 5, k) = k * (-4k^4 + 45k^3 - 170k^2 + 255k - 126) / n!
// P(4, 5, k) = k * (  k^4 - 10k^3 +  35k^2 -  50k +  24) / n!
//
// n=6
//
// P(0, 6, k) = k * (  -k^5 +  21k^4 -  175k^3 +  735k^2 - 1624k + 1764) / n!
// P(1, 6, k) = k * (  5k^5 -  99k^4 +  755k^3 - 2745k^2 + 4640k - 2556) / n!
// P(2, 6, k) = k * (-10k^5 + 186k^4 - 1300k^3 + 4170k^2 - 5890k + 2844) / n!
// P(3, 6, k) = k * ( 10k^5 - 174k^4 + 1120k^3 - 3270k^2 + 4270k - 1956) / n!
// P(4, 6, k) = k * ( -5k^5 +  81k^4 -  485k^3 + 1335k^2 - 1670k +  744) / n!
// P(5, 6, k) = k * (   k^5 -  15k^4 +   85k^3 -  225k^2 +  274k -  120) / n!
//
// The factor k in every polynomial is a direct result of the
// definition that says that P[i,n,0] = 0 for every i and n.
// If the divisor n! does not contain a factor p, then the sum is divisable
// by k and thus by p^n and the conjecture follows. Therefore, if p > n then
// we are done.
//
// If p <= n we have to consider the second sum more closely.
//
// A way to "brute force" construct the polynomials is as follows.
// 
// P(i,n,k) = k/n! * (-1)^n * (k - 1) * (k - 2) * ... * (k - i) *
//            ((-1)^(i+1) * Binomial[n, i+1] * (k - (i+2)) * (k - (i+3)) *   . . .   * (k - (n-1)) * (k - n) +
//             (k - (i+1)) * (-1)^(i+2) * Binomial[n, i+2] * (k - (i+3)) *   . . .   * (k - (n-1)) * (k - n) +
//             (k - (i+1)) * (k - (i+2)) * (-1)^(i+3) * Binomial[n, i+3] *   . . .   * (k - (n-1)) * (k - n) +
//             .                           .                                                            .
//             .                                 .                                                      .
//             .                                       .                                                .
//             (k - (i+1)) * (k - (i+2)) *...* (k - (n-2)) * (-1)^(k-(n-1)) * Binomial[n, k-(n-1)] * (k - n) +
//             (k - (i+1)) * (k - (i+2)) *...* (k - (n-2)) * (k - (n-1)) * (-1)^(k-n) * Binomial[n, k-n])
//
// You can express this in Mathematica with:
// P[i_, n_, k_] := (-1)^n * Product[k - j, {j, 0, i}] *
//                           Sum[ Product[k - z, {z, i + 1, j - 1}] *
//                                (-1)^j * Binomial[n, j] *
//                                Product[k - l, {l, j + 1, n}], {j, i + 1, n}
//                              ] / n!
// Unfortunately, Mathematica is not capable of simplifying the full expression of [1],
// but you can check it for fixed values of n. For example,
// n = 12; Simplify[(x^m - 1)^(n + 1) * Sum[Binomial[k - 1 - i, n] * x^(m*i), {i, 0, k - 1}] +
//                  (x^m - 1)         * Sum[P[i, n, k]             * x^(m*i), {i, 0, n - 1}],
//                  {k, m, n} \[Element] Integers]
// will return x^(k*m) - 1. The computation time for this is exponentially in n, but checking it like
// this for say n=0, n=1, ..., n=19 should give you a very strong indication that the formula is correct.
//
// For n = 2 the second sum of [1] equals
//
// k/2 * ((-k + 3) + (k - 1) * x^m)
//
// Note that if p | 2^m - 1, then p is odd, therefore the conjecture holds for n = 2 as well.
//
// For n = 3 this sum is,
//
// k/6 * ((11 - 6k + k^2) + (-7 + 9k - 2k^2) * x^m + (2 - 3k + k^2) * x^(2m))
//
// Since p is odd the factor 2 is not important. The factor 3 does not devide x (x=2)
// and thus it must devide the coefficients, the Pink functions. If 3 does not
// devide p then we're done, so we can assume that p = 3. p^n | k, therefore 3 | k.
// Hence, (11 - 6k + k^2 - (-7 + 9k - 2k^2) * x^m + (2 - 3k + k^2) * x^(2m)) = 11 - 7 + 2 = 0 (mod 3),
// and thus it is divisible by 3 and we're done.
//
// In fact, as long as the odd factor of n! is square free (that is, up till and including n=5)
// it will at most contain a factor p once and the same argument holds. We only need to
// check that the sum of the terms of the Pink functions without k is divisible by p.
// As this sum is P[i,n,0] with the factor k/n! removed, it is,
//        Sum[(-1)^n *
//            Product[-j, {j, 1, i}] *
//            Sum[ Product[-z, {z, i + 1, j - 1}] *
//                 (-1)^j * Binomial[n, j] *
//                 Product[-l, {l, j + 1, n}],
//               {j, i + 1, n}],
//           { i, 0, n-1}] =
//        n! * Sum[ Sum[ (-1)^(j - 1) * Binomial[n, j] / j, {j, i + 1, n}], {i, 0, n - 1} ] = n!
//
// (That last equivalence is far from trivial, but prove is omitted).
// And thus p will divide it if n! contains p as factor, and we are done for n < 6.
//
// From a practical point of view (and as far as this program is concerned) we are done now
// because the first time it could go wrong is when n=6 and p=3, and even when that
// corresponds to the lowest possible m (2^m - 1 = 3 --> m=2), we still get a very
// large value for k*m, namely at least p^n * m = 1458.
//
// Nevertheless, lets have a look at n=6. In this case n! = 2^4 * 3^2 * 5, so there is
// a possible problem for p=3, corresponding to a 'base' of m=2. Furthermore k has to be
// a multiple of p^n = 729. The second sum of [1] now equals
//
// k/n! * [(  -k^5 +  21k^4 -  175k^3 +  735k^2 - 1624k + 1764)
//         (  5k^5 -  99k^4 +  755k^3 - 2745k^2 + 4640k - 2556) * x^m
//         (-10k^5 + 186k^4 - 1300k^3 + 4170k^2 - 5890k + 2844) * x^2m
//         ( 10k^5 - 174k^4 + 1120k^3 - 3270k^2 + 4270k - 1956) * x^3m
//         ( -5k^5 +  81k^4 -  485k^3 + 1335k^2 - 1670k +  744) * x^4m
//         (   k^5 -  15k^4 +   85k^3 -  225k^2 +  274k -  120) * x^5m]
//
// Now, p^n | k, but n! contains p^2. Obviously, p^(n+2) | k^2, and
// p^2 devides (Sum[last terms of the Pink functions]) = n!, therefore
// the conjecture still holds.
//
// Finally, for arbitrary n. Assume that n! contains a factor p^z then
// the conjecture will thus still hold if p^z | k, which will be the
// case when z <= n, which is the case for all p (including p=2),
// because Sum[Floor[n/p^i], {i,1,Floor[Log[p,n]]}] equals the highest
// power of p in n! (see http://mathworld.wolfram.com/Factorial.html
// formula (4)) and this series is less than
// Sum[n/p^i, {i,1,\Infinity]}] = n / (p - 1) <= n.		QED
//
void add_factor(m_t m_start, mpz_class p)
{
#ifdef DEBUG_ADD_FACTOR
  std::cout << "\nEntering add_factor(" << m_start << ", " << p << ")\n";
#endif
  m_t m = m_start;
  m_t c = 1;							// A counter
  m_t ip = (p < (maxm + 1)) ? (m_t)p.get_ui() : (maxm + 1);	// p as integer, or "large".
  do
  {
    int e = 1;
    if (c % ip == 0)
    {
      m_t cf = c;
      do { ++e; cf /= ip; } while (cf % ip == 0);
    }
    factors[m].factor(p).set_exponent(e);    
    m += m_start;
    ++c;
  }
  while(m <= maxm);
#ifdef DEBUG_ADD_FACTOR
  std::cout << "factors is now\n";
  for (m = 2; m <= maxm; ++m)
    std::cout << m << " : " << factors[m] << '\n';
#endif
}

#undef DEBUG_FACTORIZE
#define BRUTEFORCE_r 0
#define TABLELOOKUP 1
#define FACTORIZEMETHOD BRUTEFORCE_r

#if FACTORIZEMETHOD == BRUTEFORCE_r

char* sda = NULL;

void initialize_perfect_square(m_t m)
{
  if (sda)
    delete [] sda;

  unsigned long m2 = m; m2 *= m2;
  sda = new char[m2];
  memset(sda, 0, m2 * sizeof(char));

  for (unsigned long i = 0; i < m2; ++i)
    sda[(i * i) % m2] = 1;
}

#define is_not_perfect_square(ps_mod_m2) (sda[(ps_mod_m2)] == 0)

#elif FACTORIZEMETHOD == TABLELOOKUP

#include "tabledecoder.h"

#endif

void factorize(unsigned long m, mpz_class c, bool& first)
{
  // CONJECTURE 2, by Carlo Wood, 13 jan 2007.
  //
  // The remaining factors of 2^m - 1 (that this function
  // is called for) will (very likely) have the form:
  //
  //            ____
  //             ||  
  // c * m + 1 = || (c_i * m + 1)
  //              i
  //
  // PROOF
  //
  // Proof for m is prime,
  //             _m_
  //             \ .
  // (a + b)^m = /__ a^(i) * b^(m-i) * m! / (i! * (m-i)!)
  //             i=0
  // 
  // Fill in a = b = 1 and bring the first and last terms,
  // both 1, outside of the sum,
  //           _m-1_
  //           \ .
  // 2^m = 2 + /___ m! / (i! * (m-i)!)
  //            i=1
  //
  // If m is prime, then none of the factors in the divisor
  // divides m, so m divides each of the terms of the sum
  // and we can bring m outside of the sum to get:
  //                   _m-1_
  //                   \ .  
  // 2^m - 1 = 1 + m * /___ (m-1)! / (i! * (m-i)!)
  //                    i=1 
  //
  // It is possible that this cannot be factorized, but
  // if it can be factorized, then this is MOST likely the
  // result of being able to partition the sum into two
  // (or more) parts where one part can be divided by m.
  //
  // For example, consider the smallest case m = 11 (prime).
  //
  // 2^11 - 1 = 1 + 11 * (1 + 5 + 15 + 30 + 42 + 42 + 30 + 15 + 5 + 1)
  //
  // Then notice that if we have to solve 2^11 - 1 = 2047 = (11 * 186 + 1) =
  // (11 * a + 1) * (11 * b + 1), we have (a * b) * 11 + (a + b) = 186,
  // and 186 = 10 (mod 11), so a + b = 10 (mod 11).
  // a and b are therefore composed of elements of the bionomial series
  // whose sum is 10 (mod 11),... for example 5 + 5, and the remaining sum
  // is divisible by 11: (1 + 15 + 30 + 42 + 42 + 30 + 15 + 1) = 176 = 16 * 11
  // Hence, a * b = 16, and a + b = 10. Solving gives a = 2 and b = 8, and
  // thus we factorized 2047 = (11 * 2 + 1) * (11 * 8 + 1) = 23 * 89.

  // The real factor.
  mpz_class n = c * m + 1;

  // Lets first assume that there are two factors, then
  // (c * m + 1) = (a * m + 1) * (b * m + 1) = (a * b * m + a + b) * m + 1 -->
  // (a * b * m + a + b) = c.
  //
  // Let k = a + b mod m, and r = (a + b) / m, so that a + b = r * m + k,
  // Substituting a + b we find that a * b * m + r * m + k = c,
  // thus k = c mod m, and a * b = (c - k) / m - r.
  mpz_class kmpz = c % m;
  unsigned long const k = kmpz.get_ui();

  // If a and b can be solved, then we can also solve d, e for dm + e = c.
  // We can therefore conclude that if m|c there is no solution
  // because otherwise we'd have: (d+f)m = c, where fm = e. Then we can
  // solve d+f, but can never find d and f seperately, and thus never find
  // an a and b. Since that is a contradiction with the above conjecture
  // (which states that if n can be factorized then a and b exist), we
  // can conclude that if k == 0 then n is a prime.
  // Also, the absolute minimum factors would be a = b = 1 for
  // n = (m + 1)^2 = (m + 2)*m + 1 (not sure if we ever get here for a
  // square, but that's not really relevant). So, if c < m + 2 then we
  // also have a certain prime.
  int is_prime = 0;
  if (c < m + 2 || k == 0)
    is_prime = 2;
  else
    is_prime = mpz_probab_prime_p(n.get_mpz_t(), 20);

  // If n is a prime, then we will have only one factor of course.
  if (is_prime)
  {
    add_factor(m, n);
    if (!first) std::cout << " * "; else first = false;
    if (is_prime == 1)	// Probably prime. 2 means certainly prime.
      std::cout << '{';
    std::cout << "(" << n << " = " << m << " * \e[31m" << c << "\e[0m + 1)";
    if (is_prime == 1)
      std::cout << '}';
    return;
  }

  // b = r * m + k - a, thus
  // a * (r * m + k - a) * m + a + r * m + k - a = c -->
  // m * a^2 - m * (r * m + k) * a - r * m - k + c = 0 -->
  // a = (m * (r * m + k) +- sqrt(m^2 * (r * m + k)^2 - 4 * m * (c - r * m - k))) / (2 * m) =
  //   = ((r * m + k) +- sqrt((r * m + k)^2 - 4 * ((c - k) / m - r))) / 2
  // where the +- gives the two solutions a and b respectively, so that
  // a + b = sqrt((r * m + k)^2 - 4 * ((c - k) / m - r)).
  //
  // So, if a solution exists then (r * m + k)^2 - 4 * ((c - k) / m - r) has
  // to be a perfect square. Writing it out we obtain,
  // m^2 * r^2 + (2 * m * k + 4) * r + k^2 - 4 * ((c - k) / m)
  //
  // Define the following constants,
  unsigned long const z1 = m * m;
  unsigned long const z2 = 2 * m * k + 4;
  mpz_class const z3 = (c / m) * 4 - k * k;
  // then z1 * r^2 + z2 * r - z3 has to be perfect square.

  // Calculate the minimum value of r, by assuming that a = b.
  mpz_class rmin;
  rmin = c; rmin *= 4 * m; rmin = sqrt(rmin + 4) - 2 - k * m; rmin /= z1;
  // Calculate the maximum value of r, by assuming a = 1.
  mpz_class rmax;
  rmax = c; rmax += m; rmax -= k * (m + 1); rmax /= (z1 + m);
  // Initialize s = z1 * r^2 + z2 * r - z3.
  // Note that we can get here such that (z1 * rmin + z2) * rmin - z3 < 0.
  // To avoid that, correct rmin.
  mpz_class s;
  while(1)
  {
    s = (z1 * rmin + z2) * rmin - z3;
    if (s >= 0)
      break;
    ++rmin;
  }

  // Note that the following relationships concerning odd/even hold.
  //
  // m    a    b    c = (abm+a+b) s=y^2=(a-b)^2
  // even even even even          even
  // even even odd  odd           odd
  // even odd  even odd           odd
  // even odd  odd  even          even
  // odd  even even even          even
  // odd  even odd  odd           odd
  // odd  odd  even odd           odd
  // odd  odd  odd  odd           even
  //
  // So, unless m and c are both odd, we can conclude that c = s = y (mod 2).
  //
  // Note that if m is even, then z1 is even, thus sinc is even (z2 is always even),
  // and then the initial odd/even-ness of s_r doesn't change, since
  // we only add sinc_r to it, whose odd-ness doesn't change as we only add an
  // even number (2 * z1) to it. Therefore, there is no solution when
  // m is even and c != s (mod 2). However, when there is no solution then
  // n is prime and we don't get here.
  assert(m % 2 == 1 || (c - s) % 2 == 0);
  // If m is odd, then z1 is odd, thus sinc is odd and the odd/even-ness of s_r
  // changes each time r is incremented by one. Therefore if c is even, we might
  // as well add 2 to r every time. The initial value of rmin might need to be
  // adjusted in that case however (this is only really interesting for the
  // brute force method, but it won't hurt others).
  if (m % 2 == 1 && (c - s) % 2 == 1)
  {
    ++rmin;
    s = (z1 * rmin + z2) * rmin - z3;
  }
  // Verify the table.
  assert((m * c) % 2 == 1 || (c - s) % 2 == 0);
  // Initialize sinc.
  mpz_class const sinc = 2 * z1 * rmin + z1 + z2;

  // Define D and N. See below for an explanation.
  unsigned long D = 4 * z1;
  mpz_class N = 2 * z1 * rmin + z2; N *= N; N -= 4 * z1 * s;

  // Declare x and y.
  mpz_class x, y;

  std::cout << "\nm = " << m << "; Need to solve x^2 - " << D << " y^2 = " << N << std::endl;

#if FACTORIZEMETHOD == BRUTEFORCE_r
  // Find x and y by brute force running over all sensible r values.
  //
  // This method works well till m=59.
  // Every number 2^m - 1 with 1<m<59 are factorized in ~0.25 seconds (on a P4 1.7 GHz).
  //
  mpz_class sinc_r = sinc, s_r = s, r_brute = rmin;
  unsigned long const sinc_r_modm2 = mpz_class(sinc_r % z1).get_ui();
  unsigned long s_r_modm2 = mpz_class(s_r % z1).get_ui();
  if ((m % 2) == 0 || (c % 2) == 1)
    while (r_brute <= rmax)
    {
      // If r_brute is correct (meaning that there IS a solution) then s_r is a perfect square.
      bool fail = is_not_perfect_square(s_r_modm2);	// Obviously, this test is faster than calculating the square root.
      if (!fail)
      {
	y = sqrt(s_r);
	fail = y * y != s_r;
      }
      if (fail)
      {
	s_r += sinc_r;
	s_r_modm2 += sinc_r_modm2;
	if (s_r_modm2 > z1)
	  s_r_modm2 -= z1;
	sinc_r += 2 * z1;
	++r_brute;
	continue;
      }
      // Got it (see below for an explanation of this formula).
      x = 2 * z1 * (r_brute - rmin) + (2 * z1 * rmin + z2);
      break;
    }
  else // If m is odd and c is even then we can increment r with 2 each step.
    while (r_brute <= rmax)
    {
      // If r_brute is correct (meaning that there IS a solution) then s_r is a perfect square.
      bool fail = is_not_perfect_square(s_r_modm2);	// Obviously, this test is faster than calculating the square root.
      if (!fail)
      {
	y = sqrt(s_r);
	fail = y * y != s_r;
      }
      if (fail)
      {
	s_r += 2 * sinc_r;
	s_r += 2 * z1;
	s_r_modm2 += 2 * sinc_r_modm2;
	if (s_r_modm2 > z1)
	  s_r_modm2 %= z1;
	sinc_r += 4 * z1;
	r_brute += 2;
	continue;
      }
      // Got it (see below for an explanation of this formula).
      x = 2 * z1 * (r_brute - rmin) + (2 * z1 * rmin + z2);
      break;
    }
  // We should always find a solution, because if there were no factors
  // then we'd have exited above after detecting that n is prime.
  assert(r_brute <= rmax);
#elif FACTORIZEMETHOD == TABLELOOKUP
  // Get all factors of 2^m - 1.
  std::vector<factor_ct> factors;
  list_factors_minus(factors, m);
  bool is_composite;
  // Find any factor in n.
  std::vector<factor_ct>::iterator iter = factors.begin();
  for(;;)
  {
    is_composite = (iter->get_type() == composite);
    assert(!is_composite || (iter == factors.end() - 1));
    if (mpz_divisible_p(n.get_mpz_t(), iter->get_factor().get_mpz_t()))
      break;
    ++iter;
    assert(iter != factors.end());
  }
#ifdef DEBUG_FACTORIZE
  std::cout << "Table lookup: " << n << " can be divided by " << iter->get_factor() << std::endl;
#endif
  // Now we have to determine x and y (we're faking solving THAT).
  mpz_class a_fake = (iter->get_factor() - 1) / m;
  assert((a_fake * m + 1) == iter->get_factor());
  mpz_class b_fake = (n / iter->get_factor() - 1) / m;
  if (a_fake == 0 || b_fake == 0)
  {
    // Composite. Add it anyway.
    add_factor(m, n);
    if (!first) std::cout << " * "; else first = false;
    std::cout << "{(" << n << " = " << m << " * \e[32m" << c << "\e[0m + 1)}";
    return;
  }
  mpz_class r_fake = ((a_fake + b_fake) - k) / m;
  x = (r_fake - rmin) * (2 * z1) + (2 * z1 * rmin + z2);
  y = 2 * b_fake - r_fake * m - k;
  if (y < 0)	// Swap a and b, so that y is positive.
    y = -y;
#endif

  // Found a solution!

  mpz_class a, b, r;

  // Solve r, a and b.
  r = (x - (2 * z1 * rmin + z2)) / (2 * z1) + rmin;
  a = (r * m + k - y) / 2;
  b = (r * m + k + y) / 2;

  // Make sure the solution we found is correct.
  assert((a * m + 1) * (b * m + 1) == (c * m + 1));
  assert(a > 0 && b > 0);

#ifdef DEBUG_FACTORIZE
  // The input variables.
  std::cout << "\n=============================================================\n";
  std::cout << "m = " << m << std::endl;
  std::cout << "c = " << c << std::endl;

  // The initialization.
  std::cout << "k = " << k << std::endl;
  assert((c/m)*m + k == c);
  std::cout << "z1 = " << z1 << std::endl;
  assert(sqrt((mpz_class)z1) == m);
  std::cout << "z2 = " << z2 << std::endl;
  assert((z2 - 4)/2/k == m);
  std::cout << "z3 = " << z3 << std::endl;
  assert((z3 + k*k)/4 * m == c - k);
  std::cout << "rmin = " << rmin << std::endl;
  assert(rmin == (sqrt(c * 4 * m + 4) - 2 - k * m) / z1 || (z1 * (rmin - 1) + z2) * (rmin - 1) - z3 < 0 ||
         ((m % 2) == 1 && (c % 2) == 0 && (z1 * (rmin - 2) + z2) * (rmin - 2) - z3 < 0));
#endif

  // Instead of running through the above loop, we can also try to
  // solve the following problem.
  //
  // We know that s is the smallest d^2: if there is a solution then
  // the corresponding d^2 will be equal or larger.
#ifdef DEBUG_FACTORIZE
  std::cout << "s = " << s << std::endl;
  assert(s == (z1 * rmin + z2) * rmin - z3 && s >= 0);
  std::cout << "sinc = " << sinc << std::endl;

  // The values that we actually add to s (see the above loop) is
  // (r - rmin) * (sinc + (r - rmin - 1) * z2),
  // where sinc = 2 * z1 * rmin + z1 + z2 and r >= rmin some integer.
#if 0
  std::cout << "Value that needs to be added to the initial s is d^2 - s = " << (d*d - s) << std::endl;
  mpz_class s_test = s;
  mpz_class sinc_test = sinc;
  for (mpz_class r_test = rmin; r_test <= r; ++r_test)
  {
    std::cout << "Value added for r = " << r_test << ": " << (s_test - s) << std::endl;
    assert((r_test-rmin) * (sinc + (r_test-rmin - 1) * z1) == s_test - s);
    s_test += sinc_test;
    sinc_test += 2 * z1;
  }
#endif
#endif

  // Let w = sqrt(s), so that s - w^2 >= 0, then
  // a value that we can add to s to instantly get a perfect square is,
  // (w + n)^2 - s, where n is some positive integer.
  mpz_class w = sqrt(s);
#ifdef DEBUG_FACTORIZE
  std::cout << "w = " << w << std::endl;
  assert(s - w*w > 0 || r == rmin);
  mpz_class n_test = y - w;
  assert((w + n_test)*(w + n_test) - y * y == 0);

  // Therefore, we would have found a solution when these two are equal.
  // (r - rmin) * (sinc + ((r - rmin) - 1) * z1) = (w + n_test)^2 - s
  std::cout << "r = " << r << std::endl;
  assert((r - rmin) * (sinc + (r - rmin - 1) * z1) == (w + n_test)*(w + n_test) - s);

  // The brute force results.
  std::cout << "a = " << a << std::endl;
  std::cout << "b = " << b << std::endl;
#endif

  // Rearranging this a bit we get,
  //
  // (w + n)^2 - z1 * (r - rmin)^2 + (z1 - sinc) * (r - rmin) - s = 0
  //
  // Multiply with -4 * z1, invert (z1 - sinc) and replace sinc with 2 * z1 * rmin + z1 + z2 to get
  //
  // -4 * z1 * (w + n)^2 + 4 * z1^2 * (r - rmin)^2 + 4 * z1 * (2 * z1 * rmin + z2) * (r - rmin) + 4 * z1 * s = 0
  //
  // Let x' = 2 * z1 * (r - rmin) + (2 * z1 * rmin + z2)
  //     y' = w + n
  //     D' = 4 * z1
  //     N' = (2 * z1 * rmin + z2)^2 - 4 * z1 * s
  //
  // Note that x'^2 = (2 * z1 * (r - rmin) + (2 * z1 * rmin + z2))^2 =
  // 4 * z1^2 * (r - rmin)^2 + 4 * z1 * (r - rmin) * (2 * z1 * rmin + z2) + (2 * z1 * rmin + z2)^2 =
  // 4 * z1^2 * (r - rmin)^2 + 4 * z1 * (2 * z1 * rmin + z2) * (r - rmin) + N' + 4 * z1 * s
  //
  // so that we can rewrite the equation as
  //
  // -D' * y'^2 + x'^2 - N' = 0
  //
  // or
  //
  // x'^2 - D' * y'^2 = N'
  //
  // A paper that describes how to solve this type of equation is
  // http://www.numbertheory.org/pdfs/talk_2004.pdf from which we
  // can conclude that we cannot solve it because D' is a perfect
  // square and in our case |N'| > D.
  //
  // We can write out D' and N' a bit more,
  //
  // D' = 4 z1 = (2m)^2 and is thus a perfect square.
  // N' = (2 * z1 * rmin + z2)^2 - 4 * z1 * s = ...very long derivation... = 16 n
  //
  // Note that if there is a solution then y' equals the perfect square
  // that we were looking for, so that y' = |a-b|
  assert(y*y == (a - b)*(a - b));
  // Therefore, after dividing both sides by 4 and moving the m^2 into y,
  // we really have the equation:
  //
  // x^2 - y^2 = 4n
  //
  // where x = m * (a + b) + 2
  //       y = m * (a - b)
  //       n = (m * a + 1) * (m * b + 1)
  //
  // Then, if we could solve x and y, we would have a and b. But we
  // can't solve x and y without factorizing n...

  //-----------------------------------------------------------------------------------------------------
  // Binomial attack.
  //

  mpz_class tpn;
  mpz_ui_pow_ui(tpn.get_mpz_t(), 2, m);
  tpn -= 1;
  tpn /= n;

  std::vector<unsigned long> binmodm;

  mpz_class bm = 1;
  unsigned int i = 1;
  do
  {
    std::cout << bm << " mod " << m << " = " << (bm % m) << std::endl;
    bm *= (m - i);
    ++i;
    bm /= i;
  }
  while (i <= (m - 1) / 2);

  // 2^23 - 1 = 1 + 23 * 2 * (1 + 11 + 77 + 385 + 1463 + 4389 + 10659 + 21318 + 35530 + 49742 + 58786)
  //                 Mod 23:  1   11    8    17     14     19      10      20      18      16      21
  //              *2 Mod 23:  2   22   16    11      5     15      20      17      13       9      19
  // (2^23 - 2) / 23 = 11
  // If now we pick a + b = 2 * 385 = 770 (= 11 mod 23) and a * b = (2^23 - 2) / 23 - 770 = 363952,
  // then it doesn't work. So we need to do more work.
  //
  // First, lets group the terms in pairs whose sum mod m is 0:
  //
#ifdef DEBUG_FACTORIZE
  std::cout << "x = " << x << std::endl;
  std::cout << "y = " << y << std::endl;

  // The variables to be checked.
  std::cout << "N0 = " << N << std::endl;
  std::cout << "D0 = " << D << std::endl;
  assert(x * x - D * y * y == N);
#endif

  // Print some debugging info.
  std::cout << "Solution: x = " << x << "; y = " << y << std::endl;

  // As a result of how a anb b have been calculated, it follows directly that,
  // a + b = r * m + k
  // a * b * m = (((r * m + k)^2 - d^2) / 4) * m = 
  //       = ((r^2 * m^2 + 2 * r * m * k + k^2 - s) / 4) * m =
  //       = ((r^2 * m^2 + 2 * r * m * k + k^2 - (m^2 * r + 2 * m * k + 4) * r + (c / m) * 4 - k^2) / 4) * m =
  //       = (-r + (c / m)) * m = 
  //       = (-r + ((c - k) / m)) * m =
  //       = c - k - r * m
  // Thus a * b * m + a + b = c
  // Thus c * m + 1 = (a * b * m + a + b) * m + 1 = (a * m + 1) * (b * m + 1)
  // Therefore (a * m + 1) and (b * m + 1) are factors of the
  // the correct form and we can call factorize again for each of them.

  // Factorize each further, add them and print them.
  factorize(m, a, first);
  factorize(m, b, first);
}

int main()
{
#if 0	// Test NumberCache.
  number_index_t i1 = number_cache.store(1234);
  number_index_t i2 = number_cache.store(1234);
  assert(i1 == i2);
  number_index_t i3 = number_cache.store(11);
  assert(i3 != i1);
  number_index_t i4 = number_cache.store(23);
  assert(i4 != i1 && i4 != i3);
  number_index_t i5 = number_cache.store(11);
  assert(i5 == i3);
  number_index_t i6 = number_cache.store(33);
  assert(i6 != i1 && i6 != i3 && i6 != i4);
  number_index_t i7 = number_cache.store(1234);
  assert(i7 == i1);
  assert(number_cache.retrieve(i1) == 1234);
  assert(number_cache.retrieve(i3) == 11);
  assert(number_cache.retrieve(i4) == 23);
  assert(number_cache.retrieve(i6) == 33);
  std::cout << number_cache.retrieve(i1) << std::endl;
  std::cout << number_cache.retrieve(i3) << std::endl;
  std::cout << number_cache.retrieve(i4) << std::endl;
  std::cout << number_cache.retrieve(i6) << std::endl;
#elif 0	// Testing Number
  //Number n = 1;	// Should not compile.
  Number n1 = mpz_class(0xffffffff);
  Number n2 = n1;
  assert(n2 == n1);
  std::cout << n2 << std::endl;
  Number n3;
  n3 = n1 * n2;
  std::cout << n3 << std::endl;
  Number n4 = Number(0);	// Retrieve first Number stored.
  assert(n4 == n1);
  Number n5(1);			// Retrieve second Number stored.
  assert(n5 == n3);
  n3 /= n4;
  assert(n3 == n1);
  Number n6 = n5 - 1;
  std::cout << n6 << std::endl;
  assert(n6 < n5);
#elif 0
  Power pn1(3, 5);
  Power pn2(3, 7);
  Power pn3;
  pn3 = pn1 * pn2;
  std::cout << pn3 << std::endl;
  std::cout << pn3.evaluate() << std::endl;
  assert(pn3.evaluate() == 531441);
#elif 0
  Factor f;
  std::cout << f << " = " << f.evaluate() << std::endl;
  f *= Power(3, 2);
  std::cout << f << " = " << f.evaluate() << std::endl;
  f *= Power(5);
  std::cout << f << " = " << f.evaluate() << std::endl;
  f *= Power(7);
  std::cout << f << " = " << f.evaluate() << std::endl;
  f *= Power(13);
  std::cout << f << " = " << f.evaluate() << std::endl;
  assert(f.evaluate() == 4095);
#elif 0
  int drm = 64;
  unsigned char a[64];
  memset(a, 0, 64);
  for (unsigned long i = 0; i < drm; ++i)
    a[(i * i) % drm] = 1;
  int c = 0;
  unsigned int long mask1 = 0, mask2 = 0;
  for (int i = 0; i < drm; ++i)
  {
    if (a[i])
    {
      if (i < 32)
        mask1 |= (1UL << i);
      else
        mask2 |= (1UL << (i - 32));
      ++c;
    }
  }
  std::cout << std::hex << mask1 << std::endl;
  std::cout << std::hex << mask2 << std::endl;
  std::cout << drm << " : " << (1.0 * c) / drm << std::endl;
#else
  //========================================================================
  // Main program.
  //
#if FACTORIZEMETHOD == TABLELOOKUP
  initialize_tables();
#endif
  // Run over all values of m.
  for (m_t m = 2; m <= maxm; ++m)
  {
#if FACTORIZEMETHOD == BRUTEFORCE_r
    // Initialize the table need for is_probable_perfect_square().
    initialize_perfect_square(m);
#endif

    // Calculate our next to-be-factorized number.
    mpz_class tpn;
    mpz_ui_pow_ui(tpn.get_mpz_t(), 2, m);
    tpn -= 1;
    // Print it.
    std::cout << "2^" << m << " - 1 = " << tpn << " = ";
    // Print all factors known so far.
    bool first = true;
    if (!factors[m].empty())
    {
      std::cout << factors[m] << std::flush;
      first = false;
      // Find the remaining factor.
      tpn /= factors[m].evaluate();
    }
    if (tpn != 1)
    {
      mpz_class c = (tpn - 1) / m;
      if (m == 728)
        continue; // Conjecture fails :((
      // Check that the remaining factor is of the form a*m + 1.
      assert(c * m == tpn - 1);
      // And see if it exists of more factors, add those and print them.
      factorize(m, c, first);
    }
    std::cout << std::endl;
  }
#endif
}
