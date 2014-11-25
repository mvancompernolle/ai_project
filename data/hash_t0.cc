#include <more/math/hash.h>
#include <more/diag/debug.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>


template <typename Hash, typename T>
  void
  check_consistency()
  {
      for (int i = 0; i < 1000; ++i) {
	  Hash h0(0);
	  Hash h1(0);
	  const std::size_t n_ins = rand() % 40;
	  T data[n_ins];
	  for (std::size_t j = 0; j < n_ins; ++j)
	      data[j] = rand();
	  int j0 = n_ins? rand() % n_ins : 0;
	  int j1 = n_ins? rand() % n_ins : 0;
	  if (j0 > j1) {
	      j0 += j1;
	      j1 = j0 - j1;
	      j0 -= j1;
	  }
	  h0.insert(data + 0, data + j0);
	  h0.insert(data + j0, data + j1);
	  h0.insert(data + j1, data + n_ins);
	  h1.insert(data + 0, data + n_ins);
	  MORE_CHECK(h0 == h1);
	  MORE_CHECK(!(h0 < h1 || h1 < h0));
      }
  }



int
main(int argc, char** argv)
{
    typedef more::math::hash<32>  hash32;
    typedef more::math::hash<64>  hash64;
    typedef more::math::hash<96>  hash96;
    typedef more::math::hash<128> hash128;
    typedef more::math::hash<192> hash192;

    typedef hash192 hash_type;

    int data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    hash_type h(0);
    h.insert(data+0, data+16);
    char str[hash_type::c_name_strlen + 1];
    h.strncpy_to_c_name(str);
    str[hash_type::c_name_strlen] = 0;
    std::cout << str << '\n';

    hash_type h0;
    h0.assign_from_c_name(str);
    h0.strncpy_to_c_name(str);
    std::cout << str << '\n';
    std::cout << h << '\n' << h0 << '\n';
    assert(h == h0);

    MORE_ECHO((check_consistency<hash32, char>()));
    MORE_ECHO((check_consistency<hash64, char>()));
    MORE_ECHO((check_consistency<hash96, char>()));
    MORE_ECHO((check_consistency<hash96, int>()));
    MORE_ECHO((check_consistency<hash64, more::lang::uint_least32_t>()));
    MORE_ECHO((check_consistency<hash64, more::lang::uint_least64_t>()));
    MORE_ECHO((check_consistency<hash128, more::lang::uint_least64_t>()));
    MORE_ECHO((check_consistency<hash192, char>()));
    MORE_ECHO((check_consistency<hash192, more::lang::uint_least64_t>()));

    return more::diag::check_exit_status();
}
