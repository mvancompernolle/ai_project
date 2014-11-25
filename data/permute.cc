#include "sys.h"
#include "debug.h"
#include <libecc/point.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <boost/shared_array.hpp>
#include "utils.h"

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::fstream;
using libecc::bitset;
using libecc::bitset_digit_t;

unsigned int const hd = (m - 1) / 4 + 1;	// The number of hexadecimal digits needed to print a polynomial.

// The elliptic curve (we can only have ONE curve at a time!) and the type of the points on them.
// Note in the larger part of the program we are not updating a and b, therefore the arithmetic
// of 'point' won't work. Not a problem because are not doing any point arithmetic here.
poly_t a, b;
typedef libecc::point<poly_t, a, b> point;

// A class to represent more than one curve.
// The values of 'a' and 'b' of our curves (x^3 + ax^2 + b = y^2 + xy) are
// stored here instead, in this program.
//
class Curve {
private:
  poly_t M_a;
  poly_t M_b;
  int M_zeroes;
  int M_order;
  std::vector<point> M_points;

public:
  Curve(poly_t const& a, poly_t const& b) : M_a(a), M_b(b) { }

  void add_point(point const& pt) { M_points.push_back(pt); }
  void set_zeroes(int zeroes) { M_zeroes = zeroes; }
  void set_order(int order) { M_order = order; }

  std::vector<point> const& points(void) const { return M_points; }
  std::vector<point>& points(void) { return M_points; }

  poly_t const& get_a(void) const { return M_a; }
  poly_t const& get_b(void) const { return M_b; }
};

// The type of the container will all our curves in it.
typedef std::vector<Curve> curves_type;

// All curves.
curves_type curves;

// This class represents a permutation operator on q elements.
// The internal representation is by storing the result of
// the operation on an intial value of (1 2 3 4 ... q).
class PermutationOp {
private:
  bitset_digit_t M_permutation[q + 1];	// Index 0 is not used.
public:
  PermutationOp(void);
  void interchange(bitset_digit_t p1, bitset_digit_t p2)
  {
    std::swap(M_permutation[p1], M_permutation[p2]);
  }
};

PermutationOp::PermutationOp(void)
{
  for (unsigned int i = 0; i <= q; ++i)
    M_permutation[i] = i;
}

// This class represents the solutions of an elliptic curve
// permuted according to some reversible operation.
class Matrix {
private:
  boost::shared_array<bitset<q> > M_matrix;
  PermutationOp M_permutation;

public:
  Matrix(Curve const& curve);
  void print_on(std::ostream& os) const;
  void interchange_polynomials(bitset_digit_t p1, bitset_digit_t p2);
};

// Construct a new Matrix from a given curve.
Matrix::Matrix(Curve const& curve) : M_matrix(new bitset<q>[q])
{
  // Reset the whole matrix.
  for (unsigned int y = 0; y < q; ++y)
    M_matrix[y].reset();
  // Read in the solutions of the elliptic curve.
  for (std::vector<point>::const_iterator iter = curve.points().begin(); iter != curve.points().end(); ++iter)
    M_matrix[iter->get_y().get_bitset().digit(0)].set(iter->get_x().get_bitset().digit(0));
}

void Matrix::print_on(std::ostream& os) const
{
  for (int y = q - 1; y >= 0; --y)
  {
    os << std::setw(2) << y << " ";
    for (unsigned int x = 0; x < q; ++x)
    {
#if 0
      if (x == 0 || x == y)
        os << " o";	// Hide
      else
#endif
      if (M_matrix[y].test(x))
	os << " X";
      else
	os << " -";
    }
    os << '\n';
  }
  os << '\n';
  if (q > 10)
  {
    os << "   ";
    for (unsigned int x = 0; x < q; ++x)
      os << ' ' << (x % 100) / 10;
    os << '\n';
  }
  os << "   ";
  for (unsigned int x = 0; x < q; ++x)
    os << ' ' << (x % 10);
  os << '\n';
}

// Cause the polynomials p1 and p2 to be interchanged.
void Matrix::interchange_polynomials(bitset_digit_t p1, bitset_digit_t p2)
{
  cout << "Interchanging " << p1 << " with " << p2 << "\n\n";
  if (p1 == p2)
    return;

  std::swap(M_matrix[p1], M_matrix[p2]);
  for (int y = q - 1; y >= 0; --y)
  {
    if (M_matrix[y].test(p1) != M_matrix[y].test(p2))
    {
      M_matrix[y].flip(p1);
      M_matrix[y].flip(p2);
    }
  }
  // Also keep the permutation operator up to date.
  M_permutation.interchange(p1, p2);
}

int main()
{
  // Initialization.

  std::ostringstream in_filename;
  in_filename << "testPoint" << m << ".out";

  std::ifstream in;
  in.open(in_filename.str().c_str());

  std::string line;
  if (!in || !getline(in, line))
  {
    cerr << "Could not open file " << in_filename.str() << ".  First run: ./testPoint" << endl;
    return 1;
  }

  std::string::size_type k_start;
  if (line.compare(0, 4, "m = ") == 0 && (k_start = line.find("k = ")) != std::string::npos)
  {
    unsigned int mo = atoi(line.substr(4).c_str());
    unsigned int ko = atoi(line.substr(k_start + 4).c_str());
    if (mo != m || ko != k)
    {
      cerr << "Value of (m = " << m << ", k = " << k << ") does not correspond with "
              "value in " << in_filename.str() << " (m = " << mo << ", k = " << ko << ").\n"
              "Please rerun ./testPoint.  In order to change values of m and k, recompile with: make M=m." << endl;
      return 1;
    }
  }
  else
  {
    cerr << "Unexpected start of file testPoint.out (\"" << line << "\").\nPlease rerun ./testPoint" << endl;
    return 1;
  }

  curves_type::iterator iter;
  while(getline(in, line))
  {
    static std::string::size_type const prefix = sizeof("Solution: (x, y)");	// Minor speed up: skip the prefix.
    char first_char = line[0];
    if (first_char == 'S')	// Solution: Solution: (x, y) = (...
    {
      std::string::size_type const x_start = prefix + 3; // line.find("= (", offset) + 3;
      std::string::size_type const y_start = line.find(", ", x_start) + 2;

      // Add this solution to the curve.
      iter->add_point(point(line.substr(x_start, hd), line.substr(y_start, hd)));
    }
    else if (first_char == 'a')	// a = ...
    {
      static std::string::size_type const a_start = 4;
      std::string::size_type const b_start = line.find("b = ", a_start) + 4;

      poly_t a =  line.substr(a_start, hd);
      poly_t b =  line.substr(b_start, hd);

      cout << "a = " << a << "; b = " << b << '\n';

      // Create a new curve with parameters a and b and insert it the curves container.
      // Make iter point to this last inserted element.
      curves.push_back(Curve(a, b));
      iter = curves.end() - 1;
    }
    else if (first_char == 'z') // z = ...
    {
      static std::string::size_type const z_start = 4;
      std::string::size_type const p_start = line.find("#E = ", z_start) + 5;

      int z = atoi(line.substr(z_start).c_str());
      int p = atoi(line.substr(p_start).c_str());

      cout << "z = " << z << "; #E = " << p << '\n';
      iter->set_zeroes(z);	// Number of points with a y-coordinate equal to 0.
      iter->set_order(p);	// Number of points on the curve.
    }
    // Skip other lines.
  }

  Curve* c;
  for (int curve = 0;; ++curve)
  {
    c = &curves[curve];
    if (c->get_a() == poly_t(11) && c->get_b() == poly_t(2))
      break;
  }

  Matrix matrix(*c);

  // Always swap sqrt(b) with column 1.
  poly_t sqrt_b(c->get_b());
  sqrt_b.sqrt();
//  matrix.interchange_polynomials(1, sqrt_b.get_bitset().digit(0));

  for(unsigned int swapcol = 2; swapcol < q - 1; ++swapcol)
  {
    matrix.print_on(cout);

    unsigned int e1;
    do
    {
      cout << "\nSwap " << swapcol << " with? " << std::flush;
      cin >> e1;
    }
    while (e1 < swapcol || e1 >= q);
    matrix.interchange_polynomials(swapcol, e1);
  }

  in.close();
  return 0;
}
