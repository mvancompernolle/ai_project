#include "sys.h"
#include "debug.h"
#include <iostream>
#include <cassert>
#include <iomanip>
#include <sstream>
#include <libecc/point.h>
#include <libecc/fieldmath.h>
#include "utils.h"

#define VERBOSE

using std::cout;
using std::endl;
using std::flush;
using libecc::bitset_base;
using libecc::bitset_digit_t;

typedef libecc::bitset<m> bitset;

poly_t a;
poly_t b;
typedef libecc::point<poly_t, a, b> point;

// Returns the number of elements with foc n and trace 1.
mpz_class tr1c(unsigned int n)
{
  mpz_class result = foc(n);
  while (n % 2 == 0)
  {
    n /= 2;
    result += foc(n);
  }
  result /= 2;
  return result;
}

class F2Ring {
private:
  bool M_val;

public:
  F2Ring(void) : M_val(false) { }  
  F2Ring(F2Ring const& r) : M_val(r.M_val) { }
  F2Ring& operator=(F2Ring const& r) { M_val = r.M_val; return *this; }
  F2Ring(unsigned int constant) : M_val((constant & 1) != 0) { }
  F2Ring& operator=(unsigned int constant) { M_val = (constant & 1) != 0; return *this; }
  F2Ring(bool val) : M_val(val) { }
  F2Ring& operator=(bool val){ M_val = val; return *this; }
  friend std::ostream& operator<<(std::ostream& os, F2Ring const& r) { os << (r.M_val ? 1 : 0); return os; }
  F2Ring& operator+=(F2Ring const& r) { M_val = (M_val != r.M_val); return *this; }
  friend F2Ring operator+(F2Ring const& r1, F2Ring const& r2) { F2Ring result(r1); result += r2; return result; }
  friend F2Ring operator*(F2Ring const& r1, F2Ring const& r2) { F2Ring result(r1.M_val && r2.M_val); return result; }
  F2Ring& operator*=(F2Ring const& r) { M_val = M_val && r.M_val; return *this; }
  bool val(void) const { return M_val; }
  bool operator!=(F2Ring const& r) const { return M_val != r.M_val; }
  bool operator==(F2Ring const& r) const { return M_val == r.M_val; }
};

class SymbolicF2Ring {
private:
  libecc::bitset<q> M_terms;

public:
  SymbolicF2Ring(void) { M_terms.reset(); }  
  SymbolicF2Ring(SymbolicF2Ring const& r) : M_terms(r.M_terms) { }
  SymbolicF2Ring& operator=(SymbolicF2Ring const& r) { M_terms = r.M_terms; return *this; }
  SymbolicF2Ring(unsigned int constant)
  {
    if ((constant & 1))
      M_terms.setall();
    else
      M_terms.reset();
  }
  SymbolicF2Ring& operator=(unsigned int constant)
  { 
    if ((constant & 1))
      M_terms.setall();
    else
      M_terms.reset();
    return *this;
  }
  SymbolicF2Ring(bitset const& factors)
  {
    M_terms.reset();
    M_terms.set(factors.digit(0));
  }
  SymbolicF2Ring& operator=(bitset const& factors)
  {
    M_terms.reset();
    M_terms.set(factors.digit(0));
    return *this;
  }
  friend std::ostream& operator<<(std::ostream& os, SymbolicF2Ring const& r);
  SymbolicF2Ring& operator+=(SymbolicF2Ring const& r)
  {
    M_terms ^= r.M_terms;
    return *this;
  }
  friend SymbolicF2Ring operator+(SymbolicF2Ring const& r1, SymbolicF2Ring const& r2)
      { SymbolicF2Ring result(r1); result += r2; return result; }
  friend SymbolicF2Ring operator*(SymbolicF2Ring const& r1, SymbolicF2Ring const& r2);
  SymbolicF2Ring& operator*=(SymbolicF2Ring const& r)
      { SymbolicF2Ring tmp(*this); *this = tmp * r; return *this; }
  libecc::bitset<q> const& terms(void) const {return M_terms; }
};

SymbolicF2Ring operator*(SymbolicF2Ring const& r1, SymbolicF2Ring const& r2)
{
  SymbolicF2Ring result;
  libecc::bitset<q>::const_iterator iter1 = r1.M_terms.begin();
  for (++iter1, iter1.find1(); iter1 != r1.M_terms.end(); ++iter1, iter1.find1())
  {
    libecc::bitset<q>::const_iterator iter2 = r2.M_terms.begin();
    for (++iter2, iter2.find1(); iter2 != r2.M_terms.end(); ++iter2, iter2.find1())
      result.M_terms.flip(iter1.get_index() | iter2.get_index());
  }
  return result;
}

std::ostream& operator<<(std::ostream& os, SymbolicF2Ring const& r2)
{
  SymbolicF2Ring r(r2);
  bool all_zeroes = (r.M_terms.digit(libecc::bitset<q>::digits - 1) & ~((libecc::bitset<q>::digits == 1) ? 1 : 0)) == 0;
  bool all_ones = (r.M_terms.digit(libecc::bitset<q>::digits - 1) | ((libecc::bitset<q>::digits == 1) ? 1 : 0)) == libecc::bitset<q>::valid_bits;
  for (int d = libecc::bitset<q>::digits - 2; d >= 0 && (all_zeroes || all_ones); --d)
  {
     if (r.M_terms.digit(d) != 0 && (d > 0 || r.M_terms.digit(d) != 1))
       all_zeroes = false;
     if (r.M_terms.digit(d) != 0xffffffff && (d > 0 || r.M_terms.digit(d) != 0xfffffffe))
       all_ones = false;
  }
  if (all_zeroes)
    os << '0';
  else if (all_ones)
    os << '1';
  else
  {
    bool first = true;
    libecc::bitset<q>::const_iterator iter = r.M_terms.begin();
    for (++iter, iter.find1(); iter != r.M_terms.end(); ++iter, iter.find1())
    {
      if (!first)
        os << "+";
      bitset product(iter.get_index());
      for (unsigned int bit = 0; bit < m; ++bit)
      {
	if (product.test(bit))
	  os << (char)('a' + bit);
      }
      first = false; 
    }
  }
  return os;
}

template<typename T>
class Vector {
private:
  T M_elements[m];
public:
  Vector(void) { }
  Vector(Vector const& v) : M_elements(v.M_elements) { }
  Vector& operator=(Vector const& v) { for (unsigned int i = 0; i < m; ++i) M_elements[i] = v.M_elements[i]; return *this; }
  T& operator[](unsigned int index) { return M_elements[index]; }
  T const& operator[](unsigned int index) const { return M_elements[index]; }
  template<typename T2>
    friend std::ostream& operator<<(std::ostream& os, Vector<T2> const& v);
  template<typename T2>
    friend T2 operator*(Vector<T2> const& v1, Vector<T2> const& v2);
  Vector& operator+=(Vector const& v) { for (unsigned int i = 0; i < m; ++i) M_elements[i] += v.M_elements[i]; return *this; }
  T trace(void) const;
  bool operator!=(Vector const& v) const
  {
    for (unsigned int i = 0; i < m; ++i)
      if (M_elements[i] != v.M_elements[i])
        return true;
    return false;
  }
  bool operator==(Vector const& v) const { return !this->operator!=(v); }
};

template<typename T>
T Vector<T>::trace(void) const
{
  T result;
  if ((m & 1))
    result = M_elements[0];
  if (((m - k) & 1))
    result += M_elements[m - k];
  if (k1 && ((m - k1) & 1))
    result += M_elements[m - k1];
  if (k2 && ((m - k2) & 1))
    result += M_elements[m - k2];
  return result;
}

template<typename T>
T operator*(Vector<T> const& v1, Vector<T> const& v2)
{
  T result(v1[0] * v2[0]);
  for (unsigned int i = 1; i < m; ++i)
    result += v1[i] * v2[i];
  return result;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, Vector<T> const& v)
{
  os << '(' << v.M_elements[0];
  for (unsigned int i = 1; i < m; ++i)
  {
    os << ", " << v.M_elements[i];
  }
  os << ")^T";
  return os;
}

template<typename T>
class Matrix {
private:
  Vector<T> M_columns[m];
  static Matrix S_unity;
public:
  Matrix(void) { }
  Matrix(Matrix const& matrix) : M_columns(matrix.M_columns) { }
  Matrix& operator=(Matrix const& matrix) { for (unsigned int col = 0; col < m; ++col) M_columns[col] = matrix.M_columns[col]; return *this; }
  Matrix(Vector<T> const& v);
  Vector<T>& operator[](unsigned int column) { return M_columns[column]; }
  Vector<T> const& operator[](unsigned int column) const { return M_columns[column]; }
  Vector<T> row(unsigned int row)
      { Vector<T> result; for (unsigned int col = 0; col < m; ++col) result[col] = M_columns[col][row]; return result; }
  T& element(unsigned int row, unsigned int col) { return M_columns[col][row]; }
  T const& element(unsigned int row, unsigned int col) const { return M_columns[col][row]; }
  template<typename T2>
    friend std::ostream& operator<<(std::ostream& os, Matrix<T2> const& matrix);
  Vector<T> operator*(Vector<T> const& v) const;
  template<typename T2>
    friend Matrix<T2> operator*(Matrix<T2> const& m1, Matrix<T2> const& m2);
  Matrix<T>& operator*=(Matrix<T> const& m1) { Matrix<T> tmp = *this * m1; return (*this = tmp); }
  T trace(void);
  void invert(void);
  Matrix(unsigned int) { for (unsigned int i = 0; i < m; ++i) M_columns[i][i] = 1U; }
  static Matrix const& unity(void) { return S_unity; }
  bool operator!=(Matrix const& matrix) const
  {
    for (unsigned int i = 0; i < m; ++i)
      if (M_columns[i] != matrix.M_columns[i])
        return true;
    return false;
  }
  bool operator==(Matrix const& matrix) const { return !this->operator!=(matrix); }
};

template<typename T>
Matrix<T> Matrix<T>::S_unity(1U);

template<typename T>
void
Matrix<T>::invert(void)
{
  Matrix<T> result;
  Matrix<T> tmp(*this);
  do
  {
    result = tmp;
    tmp *= *this;
  }
  while (tmp != unity());
  *this = result;
}

template<typename T>
T Matrix<T>::trace(void)
{
  T result(M_columns[0][0]);
  for (unsigned int i = 1; i < m; ++i)
    result += M_columns[i][i];
  return result;
}

template<typename T>
Matrix<T>::Matrix(Vector<T> const& v)
{
  M_columns[0] = v;
  for (unsigned int i = 1; i < m; ++i)
  {
    for (unsigned int j = 1; j < m; ++j)
      M_columns[i][j] = M_columns[i - 1][j - 1];
    M_columns[i][0] = M_columns[i - 1][m - 1];
    M_columns[i][k] += M_columns[i - 1][m - 1];
    if (k1)
    {
      M_columns[i][k1] += M_columns[i - 1][m - 1];
      M_columns[i][k2] += M_columns[i - 1][m - 1];
    }
  }
}

template<typename T>
Matrix<T> operator*(Matrix<T> const& m1, Matrix<T> const& m2)
{
  Matrix<T> result;
  for (unsigned int col = 0; col < m; ++col)
  {
    for (unsigned int i = 0; i < m; ++i)
    {
      result.M_columns[col][i] = m2.M_columns[col][0] * m1.M_columns[0][i];
      for (unsigned int j = 1; j < m; ++j)
	result.M_columns[col][i] += m2.M_columns[col][j] * m1.M_columns[j][i];
    }
  }
  return result;
}

template<typename T>
Vector<T> Matrix<T>::operator*(Vector<T> const& v) const
{
  Vector<T> result;
  for (unsigned int i = 0; i < m; ++i)
  {
    result[i] = v[0] * M_columns[0][i];
    for (unsigned int j = 1; j < m; ++j)
      result[i] += v[j] * M_columns[j][i];
  }
  return result;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, Matrix<T> const& matrix)
{
  size_t max_width[m];
  for (unsigned int col = 0; col < m; ++col)
  {
    max_width[col] = 0;
    for (unsigned int row = 0; row < m; ++row)
    {
      std::ostringstream buf;
      buf << ' ' << matrix.M_columns[col][row];
      if (buf.str().length() > max_width[col])
        max_width[col] = buf.str().length();
    }
  }
  for (unsigned int row = 0; row < m; ++row)
  {
    os << "|";
    for (unsigned int col = 0; col < m; ++col)
    {
      std::ostringstream buf;
      buf << ' ' << matrix.M_columns[col][row];
      os << std::setw(max_width[col]) << buf.str() << " |";
    }
    os << endl;
  }
  return os;
}

int main()
{
  initialize_utils();

  libecc::bitset<q> h;
  h.reset();
  for (unsigned int n = 1; n <= m; ++n)
  {
    int nn = 0;
    for (unsigned int v = 0; v < q; ++v)
    {
      if (h.test(v))
	continue;
      unsigned int mask = q >> 1;
      unsigned int count = 0;
      while(mask)
      {
        if ((mask & v))
	  ++count;
        mask >>= 1;
      }
      if (count != n)
        continue;
      cout << poly_t(v) << endl;
      ++nn;
      bitset p(v), result;
      for (unsigned int i = 0; i < m; ++i)
      {
	h.set(p.digit(0));
	p.rotate<1, libecc::left>(result);
	p = result;
      }
    }
    cout << nn << " patterns with " << n << " bits set." << endl;
  }

#ifdef VERBOSE
  cout << "The cardinality of the field (q) is " << q << " (m = " << m << ")." << endl;
  cout << "n = " << poly_t(poly_t::normal()) << endl;
#endif

  poly_t nb[m];
  for (bitset_digit_t v = 1; v < q; ++v)
  {
    poly_t g(v);
    if (g.trace() == 0)
      continue;
    // Try to create the normal basis.
    nb[0] = g;
    unsigned int i;
    for (i = 1; i < m; ++i)
    {
      nb[i] = nb[i - 1] * nb[i - 1];
      if (nb[i].get_bitset().digit(0) < v)
        break;
      if (nb[i] == g)
        break;
    }
    if (i != m)
      continue;
    libecc::bitset<q> h;
    h.reset();
    bitset_digit_t v2;
    for (v2 = 0; v2 < q; ++v2)
    {
      bitset b2(v2);
      poly_t x(0);
      for (unsigned int i = 0; i < m; ++i)
      {
        if (b2.test(i))
	  x += nb[i];
      }
      if (h.test(x.get_bitset().digit(0)))
        break;
      h.set(x.get_bitset().digit(0));
    }
    if (v2 != q)
      continue;
    cout << "Found normal basis = { " << nb[0];
    for (unsigned int i = 1; i < m; ++i)
      cout << ", " << nb[i];
    cout << " }" << endl;

    poly_t x(g);
    unsigned int n;
    for (n = 1; n < q; ++n)
    {
      x *= g;		// x = g^(n + 1)
      if (x == g)
        break;
    }
    assert((q - 1) % n == 0);
    cout << "g^((q - 1)/" << ((q - 1) / n) << ") = 1" << endl;

    Matrix<F2Ring> M_b;
    for (unsigned int col = 0; col < m; ++col)
      for (unsigned int row = 0; row < m; ++row)
	M_b[col][row] = nb[col].get_bitset().test(row);

    //cout << "M_b = \n" << M_b << endl;
    Matrix<F2Ring> M_b_inv(M_b);
    M_b_inv.invert();
    //cout << "M_b^(-1) = \n" << M_b_inv << endl;

    Matrix<SymbolicF2Ring> M_n2t;
    Matrix<SymbolicF2Ring> M_t2n;
    for (unsigned int col = 0; col < m; ++col)
    {
      for (unsigned int row = 0; row < m; ++row)
      {
	M_n2t[col][row] = M_b[col][row].val() ? 1U : 0U;
	M_t2n[col][row] = M_b_inv[col][row].val() ? 1U : 0U;
      }
    }
//    cout << "M_n2t = \n" << M_n2t << endl;
//    cout << "M_t2n = \n" << M_t2n << endl;

    // Define an _arbitrary_ vector.
    Vector<SymbolicF2Ring> v;
    for (unsigned int i = 0; i < m; ++i)
      v[i] = bitset(1 << i);	// Each variable is represented by a different bit.
				  // Thus, this way we assign a different variable to each bit.

    if (0)
    {
      // Limit this vector to those vectors for which Tr(v) = 0.
      // The trace is the sum of each set bit in the normal.
      bitset::const_iterator iter = poly_t::normal().begin();
      // Find the first bit that is set, this is the variable that we will replace with the other bits.
      // For example, if Tr(x) = Tr(c_x t^x + c_y t^y + c_z t^z) then we set c_x = c_y + c_z by
      // adding 'cx + cy + c_z' to the place in the vector that now holds c_x.
      iter.find1();						
      int eliminated_variable = iter.get_index();
      // Eliminate the variable that corresponds to that bit.
      do
      {
	v[eliminated_variable] += SymbolicF2Ring(bitset(1 << iter.get_index()));
	++iter;
	iter.find1();
      }
      while(iter != poly_t::normal().end());
    }

//    cout << "v = " << v << endl;
//    cout << "M_n2t v = " << (M_n2t * v) << endl;

    Matrix<SymbolicF2Ring> M_v(M_n2t * v);	// Create the matrix corresponding with this vector.

//    cout << "M_v = \n" << M_v << endl;

    M_v = M_t2n * (M_v * M_n2t);

//    cout << "M_t2n * M_v * M_n2t = \n" << M_v << endl;

    Matrix<SymbolicF2Ring> M2n[m];
    M2n[0] = M_v;
    for (unsigned int i = 1; i < m; ++i)
      M2n[i] = M2n[i - 1] * M2n[i - 1];	// Calculate M_v^(2^n)
    Matrix<SymbolicF2Ring> M_v_inverse(M2n[1]);
    for (unsigned int i = 2; i < m; ++i)
      M_v_inverse *= M2n[i];		// Calculate M_v^(2^1) * M_v^(2^2) * ... * M_v^(2^(m-1)) = M_v^(2^m - 2) = M_v^(-1).

//    cout << "M_v^(-1) = \n" << M_v_inverse << endl;

    Vector<SymbolicF2Ring> v_inverse = M_v_inverse[0];
    for (unsigned int i = 1; i < m; ++i)
      v_inverse += M_v_inverse[i];
    cout << "v^(-1) = " << v_inverse << " (normal basis)" << endl;

#if 0
    cout << "Tr(v)  = " << (M_n2t * v).trace() << endl;
    cout << "Tr(M_v) = " << M_v.trace() << endl;
    cout << "Tr(v^-1)  = " << (M_n2t * v_inverse).trace() << endl;
    cout << "Tr(M_v^(-1)) = " << M_v_inverse.trace() << endl;
#endif

    cout << "Tr(v + v^-1) = ";
    libecc::bitset<q> terms2((M_v.trace() + M_v_inverse.trace()).terms());
    bool first_time = true;
    for (unsigned int number_of_bits_in_pattern = 1; number_of_bits_in_pattern <= m; ++number_of_bits_in_pattern)
    {
      //cout << "number_of_bits_in_pattern = " << number_of_bits_in_pattern << endl;
      for (bitset_digit_t pattern = 1; pattern < q; ++pattern)
      {
	unsigned int bit_count = 0;
	for (bitset_digit_t mask = 1; mask < q; mask <<= 1)
	  if ((pattern & mask))
	    ++bit_count;
	if (bit_count != number_of_bits_in_pattern)
	  continue;
	//cout << "pattern = " << poly_t(pattern) << endl;
	if (terms2.test(pattern))
	{
	  //cout << "pattern is set in M_terms" << endl;
	  bitset product(pattern);
	  bitset rotate_result;
	  bitset_digit_t min_pattern = pattern;
	  for (unsigned int shift = 1; shift < m; ++shift)
	  {
	    product.rotate<1, libecc::left>(rotate_result);
	    product = rotate_result;
	    if (product.digit(0) < min_pattern)
	      min_pattern = product.digit(0);
	    //cout << "Rotated pattern is " << poly_t(product) << endl;
	    assert(terms2.test(product.digit(0)));
	    terms2.clear(product.digit(0));
	    //cout << "Cleared pattern " << poly_t(product) << endl;
	    if (product.digit(0) == 0 || product.digit(0) == q - 1)
	      break;
	  }
	  if (!first_time)
	    cout << " + ";
	  else
	  {
	    cout << "\n   ";
	    first_time = false;
	  }
	  cout << "C_" << number_of_bits_in_pattern << "(" << poly_t(min_pattern) << ")" << endl;
	}
      }
    }
    cout << endl;

    // Next, count the non-zero number of elements for which Tr(v + v^1) is 0.
    // Create a list of all non-zero product terms.
    std::vector<bitset> products;
    libecc::bitset<q> const terms((M_v.trace() + M_v_inverse.trace()).terms());
    libecc::bitset<q>::const_iterator iter = terms.begin();
    ++iter;	// The first bit is nonsense.
    for (iter.find1(); iter != terms.end(); ++iter, iter.find1())
      products.push_back(bitset(iter.get_index()));
    unsigned int count = 0;
    for (unsigned int v = 1; v < q; ++v)
    {
      unsigned int trace = 0;
      // Find all product terms that are 1.
      for (std::vector<bitset>::iterator iter = products.begin(); iter != products.end(); ++iter)
	if ((v & iter->digit(0)) == iter->digit(0))
	  trace ^= 1;
      //cout << "Tr(" << poly_t(v) << ") = " << trace << endl;
      if (trace == 0)
	++count;
    }
    cout << "Count = " << count << endl;
  }

  return 0;
}
