#include "sys.h"
#include "debug.h"
#include <iostream>
#include "utils.h"

using std::cout;
using std::endl;

std::vector<bitset_t> convert;

void get_normal_basis(std::vector<poly_t>& normal_basis)
{
  poly_t normal(poly_t::normal());
  bitset_t::const_reverse_iterator tt = normal.get_bitset().rbegin();
  tt.find1();
  poly_t b;
  for (unsigned long i = 0; i < q; ++i)
  {
    poly_t x(i);
    if (x.trace() == 0)
      x.get_bitset().flip(tt);
    if (is_normal_basis(x))
    {
      b = x;
      break;
    }
  }
  std::vector<poly_t> matrix;
  polynomial_square square;
  for (unsigned int i = 0; i < m; ++i)
  {
    normal_basis.push_back(b);
    matrix.push_back(b);
    convert.push_back(bitset_t(1UL << i));
    square(b);
  } 
  for (unsigned int mask = 0; mask < m; ++mask)
  {
    for (unsigned int b1 = 0; b1 < m; ++b1)
    {
      if (!matrix[b1].get_bitset().test(mask))
	continue;
      std::swap(matrix[b1], matrix[m - 1 - mask]);
      std::swap(convert[b1], convert[m - 1 - mask]);
      for (unsigned int b2 = 0; b2 < m; ++b2)
      {
        if (b2 == m - 1 - mask)
	  continue;
	if (matrix[b2].get_bitset().test(mask))
	{
	  matrix[b2] += matrix[m - 1 - mask];
	  convert[b2] ^= convert[m - 1 - mask];
        }
      }
    }
  }
}

bitset_t convert_to_normal_basis(poly_t const& poly)
{
  bitset_t result;
  result.reset();
  for (unsigned int b1 = 0; b1 < m; ++b1)
  {
    if (poly.get_bitset().test(b1))
      result ^= convert[m - 1 - b1];
  }
  return result;
}

std::vector<poly_t> normal_basis;

poly_t convert_from_normal_basis(bitset_t const& x)
{
  poly_t result(0);
  for (unsigned int b1 = 0; b1 < m; ++b1)
  {
    if (x.test(b1))
      result += normal_basis[b1];
  }
  return result;
}

int lyndon_code_length(bitset_t const& bs_in)
{
  int result = 1;
  bitset_t bs1(bs_in), bs2;
  for(;;)
  {
    bs1.rotate<1, libecc::left>(bs2);
    if (bs2 == bs_in)
      break;
    ++result;
    bs2.rotate<1, libecc::left>(bs1);
    if (bs1 == bs_in)
      break;
    ++result;
  }
  return result;
}

int number_of_bits(bitset_t const& bs_in)
{
  int result = 0;
  for (unsigned int digit = 0; digit < bitset_t::digits; ++digit)
  {
    unsigned long x = bs_in.digit(digit);
    x = ((x >> 1) & 0x5555555555555555UL) + (x & 0x5555555555555555UL);
    x = ((x >> 2) & 0x3333333333333333UL) + (x & 0x3333333333333333UL);
    x = ((x >> 4) + x) & 0x0f0f0f0f0f0f0f0fUL;
    x += x >> 8;
    x += x >> 16;
    x += x >> 32;
    result += x & 0xff;
  }
  return result;
}

int main()
{
  Debug(debug::init());
  initialize_utils();

  get_normal_basis(normal_basis);

  unsigned int n = 1;
  cout << "Normal basis:\n";
  for (std::vector<poly_t>::iterator iter = normal_basis.begin(); iter != normal_basis.end(); ++iter, n *= 2)
  {
    cout << "b^" << n << " = " << *iter << " = ";
    print_poly_on(cout, *iter, false);
    cout << '\n';
  }
  cout << '\n';

  for (unsigned long n = 1; n < q; ++n)
  {
    bitset_t nx(n);
    poly_t x(convert_from_normal_basis(nx));
    poly_t y = x + one / x;
    bitset_t ny(convert_to_normal_basis(y));
    cout << "x = " << poly_t(nx) << " (" << number_of_bits(nx) << ", " << lyndon_code_length(nx) <<
       "); x + 1/x = " << poly_t(ny) << " (" << number_of_bits(ny) << ", " << lyndon_code_length(ny) << ")";
    if (y.trace() == 0)
      cout << " *";
    else
      cout << " -";
    cout << '\n';
  }
}
