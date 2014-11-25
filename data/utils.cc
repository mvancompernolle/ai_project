#include "sys.h"
#include "debug.h"
#include <cassert>
#include <libecc/polynomial.h>
#include "utils.h"
#include "MultiLoop.h"
#include <gmp.h>
#include <algorithm>
#include <limits>

using std::cout;
using std::flush;
using std::endl;

poly_t const one(1);					// Multiplicative field identity.
std::vector<unsigned long> factors_of_q_minus_one;	// The factors of q - 1.

unsigned long gcd(unsigned long x, unsigned long y)
{
  if (y <= x)
  {
    if (y == 0)
    {
      assert(x);
      return x;
    }
    x = x % y;
  }
  for(;;)
  {
    if (x == 0)
      return y;
    y %= x;
    if (y == 0)
      return x;
    x %= y;
  }
}

// Choose the size of the sieve such that prime_sieve will include all primes <= sqrt(2^(m+1)).
// sieve_size = sqrt(2^(m+1))/2 (+ 1 for rounding up).
int const sieve_size = (int)((1 << ((max_m + 1) / 2 - 1)) * ((max_m & 1) ? 1 : 1.414213562373096) + 1);
int prime_sieve[sieve_size];			// Corresponding prime == 2 * index + 3

int const largest_prime = 2 * (sieve_size - 1) + 3;		// Actually this is larger than exp(log_largest_prime).
// Use a macro because using a 'double const' shouldn't compile according to the C++ standard(?!?).
// See http://gcc.gnu.org/bugzilla/show_bug.cgi?id=29905
#define log_largest_prime ((max_m + 1) * 0.34657359027997265471)	// (m + 1) * ln(2)/2
// See http://primes.utm.edu/howmany.shtml#1, + 1 for rounding up.
int const max_number_of_primes = (int)((largest_prime / log_largest_prime) * (1 + 1.2762 / log_largest_prime) + 1);
unsigned int primes[max_number_of_primes];
int number_of_primes;

void generate_primes(void)
{
  cout << "Generating about " << max_number_of_primes << " primes... " << flush;

  primes[0] = 2;
  number_of_primes = 1;
  int last_index = -1;
  while(number_of_primes < max_number_of_primes)
  {
    do
    {
      ++last_index;
      if (last_index == sieve_size)
      {
	cout << "done (" << number_of_primes << " primes generated)" << endl;
        return;
      }
    }
    while(prime_sieve[last_index]);
    int prime = last_index * 2 + 3;
    primes[number_of_primes++] = prime;
    for(int i = last_index + prime; i < sieve_size; i += prime)
      prime_sieve[i] = 1;
  }

  cout << "done" << endl;
}

void initialize_utils(void)
{
  generate_primes();
  factorize(q - 1, factors_of_q_minus_one); 
}

// Returns a sorted list (low to high) of primes factors on the even indices
// of product (product[0], product[2], product[4], ...) and the number of
// times that this prime occurs in the odd indices.
void factorize(unsigned long n, std::vector<unsigned long>& product)
{
  assert(n <= (unsigned long)primes[number_of_primes - 1] * (unsigned long)primes[number_of_primes - 1]);
  for (int i = 0; i < number_of_primes; ++i)
  {
    if (n % primes[i] == 0)
    {
      int count = 0;
      do
      {
        n /= primes[i];
	++count;
      }
      while (n % primes[i] == 0);
      product.push_back(primes[i]);
      product.push_back(count);
    }
  }
  if (n > 1)
  {
    product.push_back(n);
    product.push_back(1);
  }
}

void generate_divisors(unsigned long n, std::vector<unsigned long> const& product, std::vector<unsigned long>& divisors)
{
  if (n == 1)
  {
    divisors.push_back(1);
    return;
  }
  unsigned int number_of_prime_factors = product.size() / 2;
  unsigned long prime_power[number_of_prime_factors];
  for (MultiLoop ml(number_of_prime_factors); !ml.finished(); ml.next_loop())
  {
    for(; ml() <= product[*ml * 2 + 1]; ++ml)
    {
      if (ml() == 0)
        prime_power[*ml] = 1;
      else
        prime_power[*ml] *= product[*ml * 2];
      if (*ml == number_of_prime_factors - 1)
      {
        unsigned long divisor = 1;
	for (unsigned int i = 0; i < number_of_prime_factors; ++i)
	  divisor *= prime_power[i];
        divisors.push_back(divisor);
      }
    }
  }
}

void get_divisors(unsigned long n, std::vector<unsigned long>& divisors)
{
  std::vector<unsigned long> product;
  factorize(n, product);
  generate_divisors(n, product, divisors);
}

unsigned long euler_phi(unsigned long n)
{
  std::vector<unsigned long> product;
  factorize(n, product);
  unsigned int number_of_prime_factors = product.size() / 2;
  unsigned long result = n;
  for (unsigned int i = 0; i < number_of_prime_factors; ++i)
  {
    unsigned long prime = product[i * 2];
    result /= prime;
    result *= (prime - 1);
  }
  return result;
}

// Function and types used by euler_phi_inv.
namespace {

typedef std::vector<unsigned long> Lq_type;
typedef std::vector<unsigned long> Lp_type;
typedef std::vector<std::vector<unsigned int> > Mdiv_type;

struct MdivPred {
  Mdiv_type const& M_Mdiv;
  MdivPred(Mdiv_type const& Mdiv) : M_Mdiv(Mdiv) { }
  bool operator()(unsigned int a, unsigned int b) { return M_Mdiv[a].size() < M_Mdiv[b].size(); }
};

#if 1
struct wrk_element_type {
  std::vector<uint8_t> Lstate;
  unsigned long quo;

  void set_used(int j) { Lstate[j] = 1; }
  void set_incompatible(int j) { Lstate[j] = 2; }
  bool is_used(int j) const { return Lstate[j] == 1; }
  bool is_candidate(int j) const { return Lstate[j] == 0; }

  void init_as_all_candidates(int r) { Lstate.resize(r, 0); }
};
#else	// This is NOT faster-- even though it's memory footprint is four times smaller!
struct wrk_element_type {
  typedef uint8_t word_type;		// Using uint8_t is as fast as using unsigned long. uint32_t and uint16_t are slower.
  std::vector<word_type> Lmask;
  unsigned long quo;

  static unsigned int const elements_per_word = 4 * sizeof(word_type);
  static unsigned int index(int unsigned j) { return j / elements_per_word; }
  static word_type const candidate_flag = 1;
  static word_type const used_flag = 2;
  static inline word_type candidate_mask(unsigned int j) { return candidate_flag << ((j % elements_per_word) << 1); }
  static inline word_type used_mask(unsigned int j) { return used_flag << ((j % elements_per_word) << 1); }
  static inline word_type candidate_used_mask(unsigned int j) { return (candidate_flag|used_flag) << ((j % elements_per_word) << 1); }

  void set_used(unsigned int j) { Lmask[index(j)] |= candidate_used_mask(j); }
  void set_incompatible(unsigned int j) { unsigned int indx = index(j); Lmask[indx] |= candidate_mask(j); Lmask[indx] &= ~used_mask(j); }
  bool is_used(unsigned int j) const { return (Lmask[index(j)] & used_mask(j)) != 0; }
  bool is_candidate(unsigned int j) const { return (Lmask[index(j)] & candidate_mask(j)) == 0; }

  void init_as_all_candidates(unsigned int r) { Lmask.resize((2 * r - 1) / (8 * sizeof(word_type)) + 1, 0UL); }
};
#endif

typedef std::vector<wrk_element_type> wrk_type;

#if 0
std::ostream& operator<<(std::ostream& os, wrk_element_type const& wrk_element)
{
  bool first = true;
  os << "{{";
  unsigned int j = 0;
  for (Lused_type::const_iterator iter = wrk_element.Lused.begin(); iter != wrk_element.Lused.end(); ++iter, ++j)
  {
    if (first)
      first = false;
    else
      os << ", ";
    if (wrk.element[j].is_candidate())
      os << '?';
    else if (wrk.element[j].is_used())
      os << '*';
    else
      os << '-';
  }
  return os << "}, " << wrk_element.quo << '}';
}

std::ostream& operator<<(std::ostream& os, wrk_type const& wrk)
{
  bool first = true;
  os << '{';
  for (wrk_type::const_iterator iter = wrk.begin(); iter != wrk.end(); ++iter)
  {
    if (first)
      first = false;
    else
      os << ", ";
    os << *iter;
  }
  return os << '}';
}
#endif

void genans(wrk_type::iterator l_begin, wrk_type::iterator l_end, unsigned long m, unsigned int r0,
    Lq_type const& Lq, Lp_type const& Lp, std::vector<unsigned long>& result)
{
  for (wrk_type::iterator li = l_begin; li != l_end; ++li)
  {
    wrk_element_type& wrk_element(*li);
    unsigned long const quo(li->quo);

    for (int add2 = 0; add2 <= 1; ++add2)
    {
      if (add2 == 1)
        wrk_element.set_used(0);
      unsigned int j = 0;
      Lq_type::const_iterator qi = Lq.begin();
      while (qi != Lq.end())
      {
        if ((j >= r0 || !wrk_element.is_used(j)) && quo % *qi == 0)
	  break;
        ++qi;
	++j;
      }
      if (qi != Lq.end())
        continue;
      unsigned long res = m;
      for (unsigned int i = 0; i < Lp.size(); ++i)
      {
        if (wrk_element.is_used(i))
	{
	  res /= (Lp[i] - 1);				// \product_{i=0}^{Lp.size()-1}{Lp[i] - 1} divides m, so this is safe.
	  res *= Lp[i];
        }
      }
      result.push_back(res);
    }
  }
}

}

void euler_phi_inv(unsigned long m, std::vector<unsigned long>& result)
{
  switch(m)
  {
    case 0:
      result.push_back(0UL);	// Convention, euler_phi(0) = 0.
      return;
    case 1:
      result.push_back(1UL);	// Convention, euler_phi(1) = 1.
      result.push_back(2UL);
    default:
      if ((m & 1) == 1)		// No solutions for odd m != 1.
        return;
  }

  // We need to find all divisors of m that are equal to some prime minus one.
  // We know that m is even, so no need to include that in the factorization and generation of the divisors.
  // Find the factorization of m / 2.
  typedef std::vector<unsigned long> product_type;
  product_type product;
  factorize(m / 2, product);
  unsigned int number_of_prime_factors = product.size() / 2;
  // Did we remove the only factor of two?
  unsigned int only_factor_of_two = (number_of_prime_factors == 0 || product[0] != 2) ? 1 : 0;
  number_of_prime_factors += only_factor_of_two;

  // Put the prime factors in Lq.
  Lq_type Lq(number_of_prime_factors);
  Lq[0] = 2;
  for (unsigned int i = only_factor_of_two; i < number_of_prime_factors; ++i)
    Lq[i] = product[(i - only_factor_of_two) * 2];
  // Remember what is the largest value in Lq.
  unsigned long q_max = *Lq.rbegin();
  // Let s be the size of Lq.
  unsigned int const s = Lq.size();

  // Generate all divisors of m / 2.
  std::vector<unsigned long> divisors;
  generate_divisors(m / 2, product, divisors);
  
  // Generate Lp to be a list of all primes equal to one plus a divisor of m.
  // Let r0 be the number of elements in the intersection of Lq and Lp; those
  // elements are put in the first r0 elements of Lp.
  unsigned int r0 = 0;
  // Add 2 seperately, corresponding with the divisor 1.
  Lp_type Lp(1, 2); for (unsigned int r1 = r0; r1 < s; ++r1) if (2 == Lq[r1]) { std::swap(Lq[r0++], Lq[r1]); break; }
  // Then add the odd primes.
  mpz_t tmp;
  mpz_init(tmp);
  for (std::vector<unsigned long>::iterator iter = divisors.begin(); iter != divisors.end(); ++iter)
  {
    unsigned long divisor_plus_one = (*iter * 2) + 1UL;		// Here we add back the factor 2; (*iter * 2) is an even divisor of m.
    					// divisor_plus_one is now odd and >= 3.
    bool probab_prime;
    if (divisor_plus_one < 8)		// All odd numbers 3 <= p < 8 are primes.
      probab_prime = true;
    else if (divisor_plus_one % 3 == 0 || divisor_plus_one % 5 == 0 || divisor_plus_one % 7 == 0)
      continue;
    else if (divisor_plus_one < 121)	// The first non-prime that doesn't have 2, 3, 5 or 7 as factors is 11^2 = 121.
      probab_prime = true;
    else
    {
      mpz_set_ui(tmp, divisor_plus_one);
      probab_prime = mpz_probab_prime_p(tmp, 10);
    }
    if (probab_prime)
    {
      Lp.push_back(divisor_plus_one);
      // If divisor_plus_one is also in Lq, put it upfront.
      if (divisor_plus_one <= q_max)
        for (unsigned int r1 = r0; r1 < s; ++r1)
	  if (divisor_plus_one == Lq[r1])
	  {
	    std::swap(Lp[r0], *Lp.rbegin());
	    std::swap(Lq[r0++], Lq[r1]);
	    break;
	  }
    }
  }
  // Clean up.
  mpz_clear(tmp);
  // Let r be the size of Lp.
  unsigned int const r = Lp.size();

#if 0
  // We rely on the following.
  for (unsigned int j = 0; j < r0; ++j)
    assert(Lp[j] == Lq[j]);
#endif

  // Let Mdiv be a list of lists, one list for each value of q in Lq,
  // containing the indices k of Lp such that Lp[k] = 1 (mod q).
  Mdiv_type Mdiv(s);
  for (unsigned int j = 0; j < s; ++j)
    for (unsigned int k = 0; k < r; ++k)
      if (Lp[k] % Lq[j] == 1)
        Mdiv[j].push_back(k);
  // Let Mdiv_index be a list of indices for Mdiv, such that the size
  // of Mdiv[Mdiv_index[j]] runs from small to large.
  unsigned int Mdiv_index[s];
  for (unsigned int j = 0; j < s; ++j)
    Mdiv_index[j] = j;
  std::sort(Mdiv_index, Mdiv_index + s, MdivPred(Mdiv));

#if 0
  cout << "Lq:\n";
  for (Lq_type::iterator iter = Lq.begin(); iter != Lq.end(); ++iter)
    cout << *iter << ", ";
  cout << endl;
  cout << "Lp:\n";
  for (Lp_type::iterator iter = Lp.begin(); iter != Lp.end(); ++iter)
    cout << *iter << ", ";
  cout << endl;
  cout << "Mdiv:\n";
  for (unsigned int j = 0; j < s; ++j)
  {
    cout << j << ": ";
    for (std::vector<unsigned int>::iterator iter = Mdiv[j].begin(); iter != Mdiv[j].end(); ++iter)
      cout << (*iter + 1) << ", ";
    cout << endl;
  }
  cout << "Mdiv_index:\n";
  for (unsigned int j = 0; j < s; ++j)
    cout << j << ": " << Mdiv_index[j] << endl;
#endif

  // Initialize wrk.
  wrk_type wrk;
  wrk_element_type wrk_element;
  wrk_element.quo = m;
  wrk.push_back(wrk_element);
  wrk_type::iterator wrk_iterator = wrk.begin();
  wrk_iterator->init_as_all_candidates(r);
  wrk_iterator->set_incompatible(0);							// Mark prime 2 as 'incompatible', it is handled in genans.

  // Main loop.
  unsigned int const threshold = 500;
  for (unsigned int i = 0; i < wrk.size(); ++i)
  {
    //cout << "At top of main loop; i = " << i << "; wrk[i] = " << wrk[i] << endl;

    // If wrk grows a certain size, process it before continuing.
    if (i == threshold)
    {
      genans(wrk.begin(), wrk.begin() + threshold, m, r0, Lq, Lp, result);
      wrk.erase(wrk.begin(), wrk.begin() + threshold);
      i = 0;
    }

    // Select best candidate: the index j such that the current quotient has a prime
    // factor Lq[j], that wasn't added before to L, and Mdiv[j].size() is as small as possible.
    // Note that for values j < r0, Lq[j] == Lp[j] and Lp[j] was added to L if wrk[i].is_used(j).
    unsigned int indx = std::numeric_limits<unsigned int>::max();
    for (unsigned int l = 0; l < s; ++l)
    {
      unsigned int const j = Mdiv_index[l];	// This orders j such that the smallest Mdiv[j].size() comes first.
      if ((j >= r0 || !wrk[i].is_used(j)) && wrk[i].quo % Lq[j] == 0)
      {
	indx = j;
	break;
      }
    }
    // If indx == -1, then apparently wrk[i].quo does not contain prime factors from Lq,
    // or such a prime factor is also element of Lp (j < r0) and was already added (is_used(j)).
    // Therefore, no new elements from Lp that weren't already added are valid since
    // nothing needs to be eliminated anymore. Hence, at this point there are no
    // candidates anymore (!is_candidate) and we might as well abort right here.
    if (indx == std::numeric_limits<unsigned int>::max())
      continue;

    // Generate candidates.
    Mdiv_type::value_type::iterator cand_iter = Mdiv[indx].begin();
    if ((unsigned int)indx >= r0 && cand_iter == Mdiv[indx].end())
      continue;
    // indx is only a candidate if indx < r0.
    unsigned int cand = (indx < r0) ? indx : *cand_iter++;
    for(;;)
    {
      if (wrk[i].is_candidate(cand))
      {
	// Add candidates.
	wrk.push_back(wrk[i]);
	wrk_element_type& wrk_element(*wrk.rbegin());
	wrk_element.set_used(cand);
	wrk_element.quo /= (Lp[cand] - 1);
	wrk[i].set_incompatible(cand);
	for (unsigned int j = 0; j < r; ++j)
	  if (wrk_element.is_candidate(j) && wrk_element.quo % (Lp[j] - 1) != 0)
	    wrk_element.set_incompatible(j);
      }
      if (cand_iter == Mdiv[indx].end())
        break;
      cand = *cand_iter++;
    }
  }

  // Process (remainder of) wrk.
  genans(wrk.begin(), wrk.end(), m, r0, Lq, Lp, result);

#if 0
  for (std::vector<unsigned long>::iterator ri = result.begin(); ri != result.end(); ++ri)
    cout << *ri << ", ";
  cout << endl;
#endif
}

unsigned long num_normal(unsigned long n)
{
  unsigned long odd_n = n;
  while (!(odd_n & 1))
    odd_n >>= 1;
  unsigned long pp = 1;
  std::vector<unsigned long> divisors;
  get_divisors(odd_n, divisors);
  for (std::vector<unsigned long>::iterator iter = divisors.begin(); iter != divisors.end(); ++iter)
  {
    unsigned long d = *iter;
    unsigned long r = 1;
    while(((1UL << r) - 1) % d != 0)
      ++r;
    unsigned long i = euler_phi(d) / r;
    unsigned long f = (1UL << r) - 1;
    while (i--)
      pp *= f;
  }
  pp <<= n - odd_n;
  pp /= n;
  return pp;
}

// Return the number of elements with Frobenius order of m and trace 1.
// That is, m times the number of Lyndon words length m.
unsigned long ect1(unsigned int m)
{
  static unsigned long cache[max_m];
  assert(m <= max_m);

  if (cache[m])
    return cache[m];

  unsigned long result = (1UL << (m - 1));
  std::vector<unsigned long> divisors;
  get_divisors(m, divisors);
  for (std::vector<unsigned long>::iterator iter = divisors.begin(); iter != divisors.end(); ++iter)
  {
    if (*iter == 1)
      continue;
    if ((*iter & 1))
      result -= ect1(m / *iter);
  }

  cache[m] = result;
  return result;
}

// Slow but correct.
mpz_class foc_reference(unsigned int n)
{
  if (n == 1)
    return 2;
  mpz_class result(1);
  result <<= n;
  for (unsigned int k = 1; k < n; ++k)
    if (n % k == 0)
      result -= foc_reference(k);
  return result;
}

unsigned int foc_table[] = {
    0, 2, 2, 6, 12, 30, 54, 126, 240, 504, 990, 2046, 4020, 8190, 16254, 32730,
    65280, 131070, 261576, 524286, 1047540, 2097018, 4192254, 8388606, 16772880,
    33554400, 67100670, 134217216, 268419060, 536870910, 1073708010, 2147483646 };

// Really, we only need a cache for every divisor of m, but well - m is rather small usually.
mpz_class foc_cache[10000 /* m */];

// Frobenius order count.
// foc(n), n > 0,  returns the number of elements x in a field with
// characteristic 2 such that x^(2^n) = x and x^(2^k) != x for any 0 < k < n.
// In other words, n is the number of times one has to apply Frobenius
// in order to get x again.
mpz_class const& foc(unsigned int n)
{
  mpz_class& result(foc_cache[n]);
  if (result == 0)
  {
    if (n < 32)
      result = foc_table[n];
    else
    {
      result = 1;
      result <<= n;				// Calculate 2^n - 2, if n would be prime then this
      result -= 2;				// would be the number of elements having order n.

      for (unsigned int k = 2; k < 32; ++k)
	if (n % k == 0)
	  result -= foc_table[k];

      for (unsigned int k = 32; k < n; ++k)
	if (n % k == 0)
	  result -= foc(k);
    }
  }
  return result;
}

void print_poly_on(std::ostream& os, poly_t const& p, bool html)
{
  bool first = true;
  for (int k = m - 1; k >= 0; --k)
  {
    if (p.get_bitset().test(k))
    {
      if (!first)
        os << "+";
      else 
        first = false;
      if (k == 0)
        os << "1";
      else if (k == 1)
	os << "t";
      else if (html)
        os << "t<sup>" << k << "</sup>";
      else
        os << "t^" << k;
    }
  }
  if (first)
    os << "0";
}

// Return x^n
poly_t pow(poly_t const& x, unsigned long n)
{
  libecc::bitset_digit_t x2buf[poly_t::square_digits];
  poly_t result(one), xk(x);
  while (n > 0)
  {
    if ((n & 1UL))
      result *= xk;
    n >>= 1;
    xk = xk.square(x2buf);
    if (xk == one)
      break;
  }
  return result;
}

// Return a generator of the multiplicative group GF(2^m)^*.
poly_t generator(void)
{
  unsigned int number_of_primes = factors_of_q_minus_one.size() / 2;
  poly_t g(one);
  for (unsigned int i = 0; i < number_of_primes; ++i)
  {
    unsigned long prime = factors_of_q_minus_one[2 * i];
    unsigned int power = factors_of_q_minus_one[2 * i + 1];
    unsigned long prime_power = 1;
    for (unsigned int j = 0; j < power; ++j)
      prime_power *= prime;
    bool is_root = false;
    unsigned int j = 1;
    poly_t yi;
    do
    {
      yi = bitset_t(++j);
      is_root = pow(yi, (q - 1) / prime) == one;
    }
    while(is_root);
    poly_t zi = pow(yi, (q - 1) / prime_power);
    g *= zi;
  }
  return g;
}

// Returns true if g is a generator of the multiplicative group GF(2^m)^*.
bool is_generator(poly_t const& g)
{
  unsigned int number_of_primes = factors_of_q_minus_one.size() / 2;
  for (unsigned int i = 0; i < number_of_primes; ++i)
    if (pow(g, (q - 1) / factors_of_q_minus_one[i * 2]) == one)
      return false;
  return true;
}

bool is_normal_basis(poly_t const& b)
{
  if (b.trace() == 0)
    return false;
  polynomial_square square;
  bitset_t bs[m];
  unsigned int i = 0;
  poly_t x(b);
  do
  {
    bs[i++] = x.get_bitset();
    square(x);
  }
  while (x != b);
  if (i != m)
    return false;
  for (unsigned int bit = 0; bit < m; ++bit)
  {
    bitset_t* found_bit = NULL;
    for (unsigned int i = 0; i < m; ++i)
    {
      if (!bs[i].test(bit))
        continue;
      if (!found_bit)
        found_bit = &bs[i];
      else
        bs[i] ^= *found_bit; 
    }
    if (!found_bit)
      return false;
    found_bit->reset();
  }
  return true;
}
