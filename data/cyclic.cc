#include <iostream>
#include <cassert>
#include <vector>
#include <utility>
#include <cstring>

static int pwr(int base, int exponent)
{
  int result = 1;
  while (exponent-- > 0)
    result *= base;
  return result;
}

extern std::string decode_group_structure(int const* pt, int np, std::vector<std::pair<int, int> > const& v);

int main(void)
{
  int const p[] = { 2, 3, 5, 7 };	// Primes.
  int const m[] = { 2, 4, 1 };
  int const a[] = { 2, 3, 4 };		// Each value must be larger than the previous one.

  int const n = sizeof(a) / sizeof(int);

  // Sanity check
  assert(sizeof(a) == sizeof(m));
  assert(a[0] > 0);
  for (int i = 0; i < n; ++i)
  {
    assert(m[i] > 0);
    assert(i == 0 || a[i - 1] < a[i]);
  }

  std::cout << "Generating the orders of ";
  bool first_time = true;
  for (int i = 0; i < n; ++i)
  {
    for (int j = 0; j < m[i]; ++j)
    {
      if (!first_time)
	std::cout << " X ";
      else
	first_time = false;
      std::cout << "C_{" << p[0] << "^" << a[i] << "}";
    }
  }
  std::cout << std::endl;
  
  // Dimension
  int d = 0;
  for (int i = 0; i < n; ++i)
    d += m[i];
  std::cout << "Dimension: " << d << std::endl;

  // Vector type
  typedef int vector_type[d];

  // Modulo values
  vector_type mod;
  for (int i = 0, k = 0; i < n; ++i)
    for (int j = 0; j < m[i]; ++j, ++k)
      mod[k] = pwr(p[0], a[i]);
  std::cout << "Modulo values: "; 
  for (int k = 0; k < d; ++k)
    std::cout << mod[k] << " ";
  std::cout << std::endl;

  // Total number of elements
  int t = 1;
  for (int i = 0; i < d; ++i)
    t *= mod[i];
  std::cout << "Total number of elements: " << t << std::endl;

  // Largest a
  int ma = a[n - 1];

  // Frequency of occurance.
  int f[ma + 2];
  memset(f, 0, sizeof(f));

  // Run over all elements
  for (int e = 0; e < t; ++e)
  {
    vector_type P;
    int s = 1;
    for (int i = 0; i < d; ++i)
    {
      P[i] = (e / s) % mod[i];
      s *= mod[i];
    }
#if 0
    int static lastPi = -1;
    if (P[d - 1] != lastPi)
    {
      std::cout << P[d - 1] << std::endl;
      lastPi = P[d - 1];
    }
#endif

    // Determine order
    int k = 0;
    for(;;)
    {
      bool is_zero = true;
      for (int i = 0; i < d; ++i)
      {
        if (P[i] != 0)
	{
	  is_zero = false;
	  break;
	}
      }
      if (is_zero)
        break;
      for (int i = 0; i < d; ++i)
      {
        P[i] *= p[0];
	P[i] %= mod[i];
      }
      ++k;
    }
    assert(k <= ma);
    f[k]++;
  }

  // Prepare (order, count) pairs.
  std::vector<std::pair<int, int> > orders;
  int order = 1;
  assert(f[0] == 1);
  orders.push_back(std::pair<int, int>(order, f[0]));
  for (int i = 1; i <= ma; ++i)
  {
    order *= p[0];
    orders.push_back(std::pair<int, int>(order, f[i]));
  }

  std::cout << "Structure: " << decode_group_structure(p, sizeof(p) / sizeof(int), orders) << std::endl;

  return 0;
}

