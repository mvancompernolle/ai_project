#include <algorithm>
#include "tabledecoder.h"
#include <vector>
#include <iostream>
#include <cassert>

int main(int argc, char* argv[])
{
  initialize_tables();
  bool print_usage = false;
  bool complete_list = (argc == 1);
  if (argc != 2 && argc != 1)
    print_usage = true;
  else
  {
    unsigned int n;
    if (!complete_list)
      n = std::atoi(argv[1]);
    if (!complete_list && (n <= 1 || n > 1200))
      print_usage = true;
    else
    {
      for(unsigned int k = complete_list ? 2 : n; k <= 1200 && (complete_list || k == n); ++k)
      {
	mpz_class total, two(2);
	mpz_pow_ui(total.get_mpz_t(), two.get_mpz_t(), k);
	total -= 1;
	std::vector<factor_ct> factors;
	list_factors_minus(factors, k);
	sort(factors.begin(), factors.end());
	mpz_class product(1);
	if (complete_list)
	  std::cout << "2^" << k << " - 1 = ";
	bool first = true;
	bool is_composite = false;
	for (std::vector<factor_ct>::iterator iter = factors.begin(); iter != factors.end(); ++iter)
	{
	  if (complete_list)
	  {
	    if (first)
	      first = false;
	    else
	      std::cout << " * ";
	  }
	  is_composite = (iter->get_type() == composite);
	  assert( is_composite == (mpz_probab_prime_p(iter->get_factor().get_mpz_t(), 40) == 0) );
	  if (is_composite && complete_list)
	    std::cout << "\e[31m";
	  std::cout << iter->get_factor();
	  if (!complete_list)
	    std::cout << std::endl;
	  if (is_composite && complete_list)
	    std::cout << "\e[0m" << std::flush;
	  product *= iter->get_factor();
	  assert( !is_composite || (iter == factors.end() - 1) );	// 'irreducible.cc' assumes that only the last factor can be composite.
	}
	if (complete_list)
	  std::cout << std::endl;
	else if (is_composite)
	  std::cout << "1" << std::endl;
	assert( total == product );
      }
    }
  }
  if (print_usage)
  {
    std::cout << "Factorize V2.0.  Outputs the factors of 2^n - 1 for 1 < n <= 1200.\nUsage: " << argv[0] << " <n>\n";
  }
  return 0;
}
