#include "sys.h"
#include "debug.h"
#include <iostream>
#include <iomanip>
#include <set>
#include <libecc/point.h>
#include "utils.h"

using std::cout;
using std::endl;
using std::flush;

// g^n
struct PowerOfg {
  poly_t gn;
  unsigned long n;

  PowerOfg(poly_t const& poly, unsigned long exponent) : gn(poly), n(exponent) { }
};

struct Compare {
  bool operator()(PowerOfg const& a, PowerOfg const& b) { return a.n < b.n; }
};

class FrobeniusEquivalenceClass {
  private:
    std::set<PowerOfg, Compare> M_elements;
  public:
    FrobeniusEquivalenceClass(poly_t representative, unsigned long exponent);

    unsigned int size(void) const { return M_elements.size(); }
    unsigned long representative(void) const { return M_elements.begin()->n; }
    int trace(void) const { return M_elements.begin()->gn.trace(); }

    friend std::ostream& operator<<(std::ostream& os, FrobeniusEquivalenceClass const& fec);
    friend bool operator==(FrobeniusEquivalenceClass const& fec1, FrobeniusEquivalenceClass const& fec2)
        { return fec1.M_elements.begin()->n == fec2.M_elements.begin()->n; }
    friend bool operator!=(FrobeniusEquivalenceClass const& fec1, FrobeniusEquivalenceClass const& fec2)
        { return fec1.M_elements.begin()->n != fec2.M_elements.begin()->n; }

    bool is_normal_basis(void) const;
};

bool FrobeniusEquivalenceClass::is_normal_basis(void) const
{
  if (M_elements.size() != m)
    return false;
  if (M_elements.begin()->gn.trace() == 0)
    return false;
  bitset_t bs[m];
  unsigned int i = 0;
  for (std::set<PowerOfg, Compare>::iterator iter = M_elements.begin(); iter != M_elements.end(); ++iter, ++i)
    bs[i] = iter->gn.get_bitset();
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

std::ostream& operator<<(std::ostream& os, FrobeniusEquivalenceClass const& fec)
{
  os << '{';
  bool first = true;
  for (std::set<PowerOfg, Compare>::iterator iter = fec.M_elements.begin(); iter != fec.M_elements.end(); ++iter)
  {
    if (!first)
      os << ", ";
    else
      first = false;
    os << std::setw(3) << iter->n;
  }
  return os << '}';
}

FrobeniusEquivalenceClass::FrobeniusEquivalenceClass(poly_t const representative, unsigned long exponent)
{
  polynomial_square square;
  unsigned long n = exponent;
  poly_t next(representative);
  do
  {
    M_elements.insert(PowerOfg(next, n));
    square(next);
    n = (2 * n) % (q - 1);
  }
  while(next != representative);
  assert(n == exponent);
}

// Precalcualted values of w(m, 1).
unsigned long wm1[] = {
  0, 2,
  1, 0, 1, 4, 3, 8, 16,
  28, 45, 96, 167, 308, 579, 1100, 2018,
  3852, 7280, 13776, 26133, 49996, 95223, 182248, 349474,
  671176, 1289925, 2485644, 4793355, 9255700, 17894421, 34638296
};

#define HTML 0
#define OUTPUT 1

#if HTML
#define WRITE_OUTPUT(str, data) do { cout << "<tr><td>" << str << "</td><td>" << (data) << "</td></tr>\n"; } while(0)
#else
#define WRITE_OUTPUT(str, data) do { cout << str << ' ' << (data) << '\n'; } while(0)
#endif

int main()
{
  Debug(debug::init());

  initialize_utils();

  poly_t const mg(generator());

  for (poly_t g(mg * mg); g != mg;  g *= mg)
  {
    if (!is_generator(g))
      continue;

#if !HTML && OUTPUT
    cout << "g = " << g << '\n';
#endif

    poly_t const ginv(one / g);

    libecc::bitset<q>* mask = new libecc::bitset<q>;
    mask->reset();

    unsigned int fecm_count = 0;
    unsigned int fecm_trace1_count = 0;
    unsigned int fecm_own_inverse_count = 0;
    unsigned int fecm_own_inverse_trace1_count = 0;
    unsigned int generators = 0;
    unsigned int generators_trace1_count = 0;
    unsigned int normals = 0;
    unsigned int generator_normals = 0;
    unsigned int solutions = 0;
    unsigned int solutions_trace1_count = 0;
    unsigned int solutions_generators = 0;
    unsigned int solutions_generators_trace1_count = 0;
    unsigned int solutions_normals = 0;
    unsigned int solutions_generator_normals = 0;

#if HTML
    cout << "<h3>m = " << m << "</h3>\n\n";

#if OUTPUT
    cout << "<table cellspacing=\"0\" border=\"1\" cellpadding=\"3\">\n";
    cout << "<tr><th colspan=6>F<sub>2<sup>" << m << "</sup></sub>, t<sup>" << m << "</sup>+t";
    if (k > 1)
      cout << "<sup>" << k << "</sup>";
    if (k1 > 0)
    {
      cout << "+t";
      if (k1 > 1)
	cout << "<sup>" << k1 << "</sup>";
    }
    if (k2 > 0)
    {
      cout << "+t";
      if (k2 > 1)
	cout << "<sup>" << k2 << "</sup>";
    }
    cout << "+1=0, g=";
    print_poly_on(cout, poly_t(g));
    cout << "</th></tr>\n";

    cout << "<tr><th>x</th><th>Tr(x)</th><th>Generator</th><th>Normal</th><th>x<sup>-1</sup></th><th>Tr(x<sup>-1</sup>)</th></tr>\n";
#endif
#endif

    poly_t x(g);
    poly_t xinv(ginv);
    for(unsigned long n = 1; n <= (q - 1) / 2; ++n, x *= g, xinv *= ginv)
    {
      if (mask->test(n))
	continue;
      FrobeniusEquivalenceClass fec(x, n);
      if (fec.size() == m)
      {
	FrobeniusEquivalenceClass fecinv(xinv, q - 1 - n);
	int fecm_trace = fec.trace();
	int fecminv_trace = fecinv.trace();
	int generator = is_generator(x);
	bool normal = fec.is_normal_basis();
#if OUTPUT
#if HTML
	cout << "<tr><td>" << fec << "</td><td>" << fecm_trace << "</td><td>" << generator << "</td><td>" << normal << "</td><td>" << fecinv << "</td><td>" << fecminv_trace << "</td></tr>" << endl;
#else
	cout << fec << " " << fecm_trace << " " << generator << " " << normal << " " << fecinv << " " << fecminv_trace << endl;
#endif
#endif
        // The number of FEC with size m.
	++fecm_count;
	// The number thereof that is a solution to the Elliptic Curve equation.
	if (fecm_trace == fecminv_trace)
	{
	  ++solutions;
	  // The number of solutions with trace 1.
	  if (fecm_trace)
	    ++solutions_trace1_count;
	  // Solutions that are generators.
	  if (generator)
	    ++solutions_generators;
	  // Solutions that are generators with trace 1.
	  if (generator && fecm_trace)
	    ++solutions_generators_trace1_count;
	  // Solutions that are a normal basis.
	  if (normal)
	    ++solutions_normals;
	  // Solutions that are a normal basis and generator.
	  if (normal && generator)
	    ++solutions_generator_normals;
	}
	// The number of generators.
	if (generator)
	  ++generators;
	// The normal of normal basis.
	if (normal)
	{
	  ++normals;
	  // The normal of normal basis that are also generator.
	  if (generator)
	    ++generator_normals;
	}
	// The number of generators with trace 1.
	if (generator && fecm_trace)
	  ++generators_trace1_count;
	// The number with Tr(x) == 1.
	if (fecm_trace)
	  ++fecm_trace1_count;
	if (fec == fecinv)
	{
	  // The number of FEC with size m that is its own inverse.
	  ++fecm_own_inverse_count;
	  // The number of with Tr(x) == 1 thereof.
	  if (fecm_trace)
	    ++fecm_own_inverse_trace1_count;
	  // Conjecture: An element whose inverse is in the same FEC is not a generator, unless m == 2.
	  assert(m == 2 || !generator);
	}
	// Conjecture: A normal base always had trace 1.
	assert(!normal || fecm_trace);
      }
      unsigned long nn = n;
      do
      {
	mask->set(nn);
	nn = (2 * nn) % (q - 1);
      }
      while(nn != n);
    }
#if HTML && OUTPUT
    cout << "</table>\n\n";
#endif
#if HTML
    cout << "<table cellspacing=\"0\" border=\"0\">\n";
#endif

    WRITE_OUTPUT("Number of Frobenius equivalence classes: ", fecm_count);
    WRITE_OUTPUT("Number of Frobenius equivalence classes with trace 1: ", fecm_trace1_count);
    WRITE_OUTPUT("Number of Frobenius equivalence classes that are their own inverse: ", fecm_own_inverse_count);
    WRITE_OUTPUT("Number of Frobenius equivalence classes that are their own inverse with trace 1 (depends on w(m/2, 1)): ", fecm_own_inverse_trace1_count);
    WRITE_OUTPUT("Number of generators: ", generators);
    WRITE_OUTPUT("Number of generators with trace 1 (no formula): ", generators_trace1_count);
    WRITE_OUTPUT("Number of normal basis: ", normals);
    WRITE_OUTPUT("Number of normal basis that exist of generators (no formula, A107222): ", generator_normals);
    WRITE_OUTPUT("Number of solutions, w(" << m << ", 1): ", solutions);
    WRITE_OUTPUT("Number of solutions with trace 1 (no formula): ", solutions_trace1_count);
    WRITE_OUTPUT("Number of solutions that are generators (no formula): ", solutions_generators);
    WRITE_OUTPUT("Number of solutions that are generators with trace 1 (no formula): ", solutions_generators_trace1_count);
    WRITE_OUTPUT("Number of solutions that are normal basis (no formula): ", solutions_normals);
    WRITE_OUTPUT("Number of solutions that are normal basis and generators (no formula): ", solutions_generator_normals);

#if HTML
    cout << "</table>\n";
#endif

    cout << flush;

    // Check that the table wm1[] contains the right values.
    assert(solutions == wm1[m]);

    // Conjecture: The total number of FEC with size m is equal to the number of elements with Frobenious Order m, devided by m.
    // That is logical - so this just checks if the empirical count indeed matches the formula calculation by 'foc'.
    // This is also equal to http://oeis.org/A001037
    assert(fecm_count == foc(m) / m);

    // Proposition 1: The number of FEC with size m and trace 1 is equal to the number of elements with trace 1 and FEC size m, divided by m.
    // That is logical - so this just checks if the empirical count indeed matches the formula calculation by 'ect1'.
    // This is also equal to http://oeis.org/A000048
    assert(fecm_trace1_count == ect1(m) / m);

    // Propositions 2 and 5: The number of FEC with size m that are their own inverse is zero if m is odd, or equal to the
    // number of FEC with size m/2 and trace 1 in a field with half the degree.
    assert(fecm_own_inverse_count == (m % 2 != 0) ? 0 : ect1(m / 2) / (m / 2));

    // Conjecture: The number of FEC with size m that are their own inverse and have trace 1 is zero if m is odd,
    // equal to w(m/2, 1)/2 if m/2 is odd, or equal to (w(m/2, 1) + ect1(m/4)/(m/4))/2 is m/2 is even.
    assert(fecm_own_inverse_trace1_count == ((m % 2 != 0) ? 0 : (m % 4 != 0) ? wm1[m / 2] / 2 : (wm1[m / 2] + 4 * ect1(m / 4) / m) / 2));

    // The number of generators is equal to Phi(q - 1).
    assert(generators == euler_phi(q - 1) / m);

    // The number of generators with trace 1.
    // generators_trace1_count ?

    // The number of normal basis is equal to sequence A027362.
    // http://oeis.org/A027362
    assert(normals == num_normal(m));

    // The number of normal basis that exist of generators is equal to sequence A107222.
    // http://oeis.org/A107222
    static unsigned int A107222[] = {
      0,
      1, 1, 1, 1, 3, 3, 7, 7, 19, 29, 87, 52, 315, 291, 562, 1017, 3825, 2870, 13797, 11255, 23579, 59986,
      178259, 103680, 607522, 859849, 1551227, 1815045, 9203747, 5505966, 28629151, 33552327, 78899078
    };
    assert(generator_normals == A107222[m]);

    // Remove this break to test other g.
    break;
  }
}

