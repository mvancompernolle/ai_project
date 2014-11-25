#include "sys.h"
#include "debug.h"
#include <libecc/point.h>
#include <cassert>
#include <vector>
#include <fstream>
#include <libecc/fieldmath.h>
#include <math.h>
#include <sstream>
#include <map>
#include <boost/utility.hpp>
#include "graph.h"
#include "utils.h"

// Define this to write to cout instead of a file.
//#define STDOUT

using std::cout;
using std::endl;
using libecc::bitset_digit_t;

poly_t a;
poly_t b;
typedef libecc::point<poly_t, a, b> point;

static const poly_t g(2);	// Generator (t).

#ifdef STDOUT
std::ostream& out(std::cout);
#else
std::ofstream out;
#endif

class curve : public boost::noncopyable {
private:
  poly_t M_a;
  poly_t M_b;
  int M_y0;			// Number of points with y=0.
  int M_cardinality;
  int M_factors[m];		// The factors of the cardinality of the current curve.
  int M_number_of_factors;	// Number of prime factors in M_factors.
  int M_number_of_points_with_order;	// The number of points with different x coordinate for
					// which the order has been determined.
  std::string M_structure;	// A string describing the structure of the group formed by the points.
  				// Curves with the same M_structure are isomorphic.

  // Arrays of size q:
  char* M_nx;			// First two bits: Number of points with given x coordinate (as index).
  point* M_point1;		// The first point, if any.
  point* M_point2;		// The second point, if any.
  int* M_order;			// The order of the point(s) with given x coordinate (as index).

public:
  curve(poly_t const& a, poly_t const& b) :
      M_a(a), M_b(b), M_y0(0), M_cardinality(1), M_number_of_points_with_order(0),
      M_nx(new char[q]), M_point1(new point[q]), M_point2(new point[q]), M_order(new int[q])
      { std::memset(M_nx, 0, q); }
  ~curve() { delete [] M_nx; delete [] M_point1; delete [] M_point2; delete [] M_order; }
  void add_point(poly_t const& x, poly_t const& y);
  void calculate_order_of_points(void);
  void determine_structure(void);
  int cardinality(void) const { return M_cardinality; }
  int zeroes(void) const { return M_y0; }
  std::string const& get_structure(void) const { return M_structure; }

private:
  void set_order(int x, int order);
  bool has_order(int x) const { return (M_nx[x] & 4); }
  void init_factorize(void);
};

extern std::string decode_group_structure(int const* pt, int np, std::vector<std::pair<int, int> > const& v);

void curve::determine_structure(void)
{
  this->init_factorize();

  int vi[2 * q];				// Index into vector with order [counts] as function of order.
  std::memset(vi, -1, (2 * q) * sizeof(int));	// Fill it with -1.
  std::vector<std::pair<int, int> > v;		// Vector with (order, order count) pairs.

  // Add the zero element.
  vi[1] = 0;						// Added at index 0.
  v.push_back(std::make_pair(1, 1));			// Only one point with this order exists.

  // Add the order of the point with x coordinate 0.
  vi[M_order[0]] = 1;					// Added at index 1.
  v.push_back(std::make_pair(M_order[0], 1));		// Only one point with this order so far.

  // Run over the remaining x coordinates. 
  for (unsigned int xd = 1; xd < q; ++xd)
  {
    int count = M_nx[xd] & 3;
    if (count == 0)
      continue;						// No points with this x coordinate.
    unsigned int order = M_order[xd];
    assert(order < 2 * q);
    if (vi[order] == -1)				// First time we see this order?
    {
      vi[order] = v.size();				// The place where we'll put this order.
      v.push_back(std::make_pair(order, 2));		// Two points with this order so far.
    }
    else
      v[vi[order]].second += 2;	// Register two more points with this order.
  }

  // Do the real decoding.
  M_structure = decode_group_structure(M_factors, M_number_of_factors, v);
}

void curve::init_factorize(void)
{
  // Called after the cardinality was determined.
  // This function puts the prime factors of the cardinality in M_factors.
  M_number_of_factors = 0;
  int cardinality_factor = M_cardinality;
  for (int i = 0; i < number_of_primes; ++i)
  {
    int prime = primes[i];
    if (cardinality_factor % prime == 0)
    {
      M_factors[M_number_of_factors++] = prime;
      do
      {
	cardinality_factor /= prime;
      }
      while (cardinality_factor % prime == 0);
      if (cardinality_factor == 1)
        return;
    }
  }
  // The remaining factor must be a prime because it cannot be devided
  // anymore by any number less than or equal sqrt(2q).  Now suppose that the
  // remaining factor is not a prime, then the smallest possible value
  // would be (sqrt(2q) + 1)^2 (assuming sqrt(2q) + 1 is prime), which is
  // 2q + 2sqrt(q) + 1.  But at all times the cardinality is
  // less than 2q (in fact, it is less than q + 1 + sqrt(q)).
  M_factors[M_number_of_factors++] = cardinality_factor;
}

void curve::set_order(int x, int order)
{
  assert(!has_order(x));
  M_order[x] = order;
  M_nx[x] |= 4;
  int multiplicity = (x == 0) ? 1 : 2;
  M_number_of_points_with_order += multiplicity;
}

void curve::add_point(poly_t const& x, poly_t const& y)
{
  bitset_digit_t xd = x.get_bitset().digit(0);
  bitset_digit_t yd = y.get_bitset().digit(0);
  out << "Solution: (x, y) = (" << x.get_bitset() << ", " << y.get_bitset() << ")" << " = (" << xd << ", " << yd << ")\n";
  int count = M_nx[xd] & 3;	// Number of points with this x coordinate that we already saw.
  assert(count < 2);
  M_nx[xd]++;
  if (count == 0)
    M_point1[xd] = point(x, y);
  else
    M_point2[xd] = point(x, y);
  if (!y.get_bitset().any())	// y = 0
    ++M_y0;
  ++M_cardinality;
}

void curve::calculate_order_of_points(void)
{
  // We get here after all points are known.
  assert(a == M_a && b == M_b);		// We are going to do point arithmetic.

  // Run over all points by running over all x coordinates and
  // then handling the points (if any) with that x coordinate.

  // The first point (with x=0) always has order 2
  // because it is (0, sqrt(b)).  Doubling this point gives 0 per definition.
  assert((M_nx[0] & 3) == 1);	// There is exactly one point with coordinate 0.
  point p0 = M_point1[0];
  p0 += p0;
  assert(p0.is_zero());
  set_order(0, 2);
  // Run over the remaining x coordinates. 
  for (unsigned int xd = 1; xd < q; ++xd)
  {
    int count = M_nx[xd] & 3;
    if (count == 0)
      continue;			// No points with this x coordinate.
    assert(count == 2);
    // Those points should be eachothers additive inverse.
    // Which means that their y coordinate XOR-ed gives the x coordinate:
    bitset_digit_t y1 = M_point1[xd].get_y().get_bitset().digit(0);
    bitset_digit_t y2 = M_point2[xd].get_y().get_bitset().digit(0);
    assert((y1 ^ y2) == xd);
    // Check if the library agrees with this.
    point p = M_point1[xd];
    p += M_point2[xd];
    assert(p.is_zero());
    // Check that both points are indeed on the curve according to the library.
    assert(M_point1[xd].check());
    assert(M_point2[xd].check());
    // If we already calculated the order of this point because it is an integer
    // times another point that we already had, then skip it.
    if (has_order(xd))
      continue;
    // The order of P and -P is of course the same and nP cycles over the same points.
    // Therefore we can speed up with a factor of two by only calculating the order of
    // one of the points.  Lets always take the one with the smallest y coordinate.
    p = (y1 < y2) ? M_point1[xd] : M_point2[xd];
    // Now calculate nP, for n = 1 and larger until we find the order of the point for which nP = 0.
    point np = p;
    int order = 1;
    std::vector<bitset_digit_t> cyclic_group;
    do
    {
      ++order;
      np += p;
      // Check that the result is indeed on the curve again.
      assert(np.check());
      if (np.is_zero())
        break;
      // Temporarily remember (the x coordinate of) all points on this cyclic group.
      cyclic_group.push_back(np.get_x().get_bitset().digit(0));
    }
    while(!np.is_zero());
    out << "#" << p << " = " << order << '\n';
    set_order(xd, order);
    // Now fill in the order of all other points we encountered as well.
    int n = 2;	// The first point that was stored is 2P.
    // It makes no sense to go behond order/2 because the second half of the loop
    // has the exact same values of x coordinates: nP + (order - n) P = order P = 0
    // and thus nP = -((order - n) P) which means that they have the same x coordinate (and same order).
    np = p;
    for (std::vector<bitset_digit_t>::const_iterator iter = cyclic_group.begin(); n < order / 2; ++iter, ++n)
    {
      if (has_order(*iter))
        continue;
      set_order(*iter, order / gcd(order, n));
    }
  }
  // Make sure we calculated the order of every point (we didn't count 'zero').
  assert(M_number_of_points_with_order == M_cardinality - 1);
}

int main(void)
{
  Debug(check_configuration());
#if CWDEBUG_ALLOC
  libcwd::make_all_allocations_invisible_except(NULL);
#endif
  Debug(if (getenv("SUPPRESS_DEBUG_OUTPUT") == NULL) libcw_do.on());
  Debug(libcw_do.set_ostream(&std::cout));
  Debug(read_rcfile());
  Debug(list_channels_on(libcw_do));

  initialize_utils();
#if 0
  {
    int i = 0;
    while (i < number_of_primes && i < 10)
    {
      cout << "Found prime: " << primes[i] << '\n';
      ++i;
    } 
    if (i < number_of_primes - 10)
    {
      cout << "[...]\n";
      i = number_of_primes - 10;
    }
    while (i < number_of_primes)
    {
      cout << "Found prime: " << primes[i] << '\n';
      ++i;
    }
  }
#endif

  std::ostringstream out_filename;
  out_filename << "testPoint" << m << ".out";

#ifndef STDOUT
  out.open(out_filename.str().c_str());
#endif
  out << "m = " << m << "; k = " << k;
  if (k1 != 0)
    out << "; k1 = " << k1 << "; k2 = " << k2;
  out << '\n';

  std::map<std::string, std::pair<int, int> > isomorphic_curves;

  for (unsigned int ag = 0; ag < (1 << m); ++ag)
  {
    a = poly_t(ag);
    for (unsigned int bg = 1; bg < (1 << m); ++bg)
    {
      b = poly_t(bg);
      curve c(a, b);

      out << "a = " << a.get_bitset() << "; b = " << b.get_bitset() << '\n';

      // x = 0 has always exactly one solution: (0, sqrt(b)).
      poly_t y(b);
      y.sqrt();
      c.add_point(poly_t(0), y);

      for (unsigned int xg = 1; xg < (1 << m); ++xg)
      {
        poly_t x(xg);
	// x^3 + a*x^2 + b = y^2 + xy
	libecc::bitset_digit_t x2buf[poly_t::square_digits];
        poly_t& x2 = x.square(x2buf);	// x2 = x^2
	poly_t left = (x + a) * x2 + b;	// left = x^3 + ax^2 + b
	try
	{
	  poly_t y(x, left);		// Solve y from y^2 + xy = x^3 + ax^2 + b
	  libecc::bitset_digit_t y2buf[poly_t::square_digits];
	  poly_t& y2 = y.square(y2buf);	// y2 = y^2
	  if (left == y2 + x * y)
	  {
	    for(int y_sign = 0; y_sign <= 1; ++y_sign)
	    {
	      c.add_point(x, y);
	      y += x;
	    }
	  }
        }
	catch (std::domain_error const& error)
	{
	  continue;
	}
      }
      c.calculate_order_of_points();
      c.determine_structure();
      out << "z = " << c.zeroes() << "; #E = " << c.cardinality() <<
          "; (a, b) = (" << a.get_bitset().digit(0) << ", " << b.get_bitset().digit(0) << ')' <<
	  "; Structure: " << c.get_structure() << std::endl;
      std::pair<std::string, std::pair<int, int> > tmp(c.get_structure(), std::make_pair(c.cardinality(), 1));
      std::pair<std::map<std::string, std::pair<int, int> >::iterator, bool> res = isomorphic_curves.insert(tmp);
      if (!res.second)
        res.first->second.second++;
      out << "-------------------------------------------------\n";
    }
  }

  std::ostringstream title;
  title << "Curves over GF(2^" << m << ")/<t^" << m << "+t";
  if (k > 1)
    title << "^" << k;
  if (k1 > 0)
  {
    title << "+t^" << k1 << "+t";
    if (k2 > 1)
      title << "^" << k2;
  }
  title << "+1>";
  std::ostringstream basename;
  basename << m;
  Graph graph(basename.str(), title.str());
  int cyclic_column = -1;
  for (std::map<std::string, std::pair<int, int> >::const_iterator iter = isomorphic_curves.begin();
       iter != isomorphic_curves.end(); ++iter)
  {
    std::string label = iter->first;
    int x = iter->second.first;
    int y = iter->second.second;
    bool cyclic = (label.find("X") == std::string::npos);
    if (cyclic)
      label = "Cyclic";
    std::cout << "Adding " << x << ", " << y;
    if (label != "Cyclic")
      std::cout << ", \"" << label << '"';
    std::cout << '\n';
    if (cyclic)
    {
      if (cyclic_column == -1)
	cyclic_column = graph.add_point(x, y, label, 9);
      else
        graph.add_point(x, y, cyclic_column);
    }
    else
      graph.add_point(x, y, label);
  }
  int last_column = graph.get_colors().size() - 1;
  graph.swap_columns(last_column, cyclic_column);
  graph.plot();

#ifndef STDOUT
  out.close();
#endif
  return 0;
}
