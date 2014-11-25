#include <stdexcept>
#include <string>
#include <more/math/math.h>
#include <limits>
#include <more/bits/precint.h>
#include <more/diag/debug.h>


namespace more {
namespace math {

  //  --- basic algorithms ---
  //
  //  abc bitpow 2 = a0b0c
  //  abc bitpow 3 = a00b00c etc.
  //  abc bitpow -1 = (int)a.bc = a

  template<typename T>
    inline T
    bitpow_algo(T x, int y)
    {
	T r = 0;
	unsigned int bitno = 0;
	while (x) {
	    r |= (x & 1) << bitno;
	    x >>= 1;
	    bitno += y;
	}
	return r;
    }

  template<typename T>
    inline T
    bitroot_algo(T x, int y)
    {
	y = -y;
	T r = 0;
	unsigned int bitno = 0;
	while (x) {
	    r |= (x & 1) << bitno;
	    x >>= y;
	    ++bitno;
	}
	return r;
    }

  template<typename T>
    inline T
    bitpow_fp_algo(T x, int y)
    {
	if (x == (T)0) return (T)0;
	T r = 0;
	int n;
	x = std::frexp(x, &n);
	int last_xp = n - std::numeric_limits<T>::digits/y;
	while (x != (T)0 && n != last_xp) {
	    int i;
	    r += std::ldexp((T)1, (n-1)*y);
	    x = std::frexp(x-(T)0.5, &i);
	    n += i;
	}
	return r;
    }

  template<typename T>
    T*
    mkbitpow(int sz, int y)
    {
	T *bp = new T[sz];
	for (int x = 0; x < sz; ++x)
	    bp[x] = bitpow_fp_algo((T)x, y);
	return bp;
    }

  template<typename T, int N>
    inline T
    bitpow_fp_algo_n(T x)
    {
	const int bits = 12;  // Tuning
	const int size = 1<<bits;
	static T const* bp = mkbitpow<T>(size, N);
	int repeat = (std::numeric_limits<T>::digits+N*bits-1)/(N*bits);
	T r = 0;
	int n;
	x = std::frexp(x, &n);
	while (repeat--) {
	    T x_int;
	    x = std::modf(x*size, &x_int);
	    n -= bits;
	    r += std::ldexp(bp[(int)x_int], n*N);
	}
	return r;
    }


  //  --- accelerator ---

#ifdef MORE_CONF_CXX_HAVE_LONG_LONG
  typedef unsigned long long big_uint;
#else
  typedef unsigned long big_uint;
#endif

  namespace {
    struct stretcher_type
    {
	typedef big_uint array_value_type;

	static const int bits = 8;
	static const int size = 1<<bits;
	static const int mask = size - 1;
	static const int max_bitexpt = sizeof(big_uint)*8/bits;

	stretcher_type();
	template<typename T>
	  T operator()(T, unsigned int);
	template<typename T>
	  T FP_bitpow(T, unsigned int);

	array_value_type array[max_bitexpt+1][size];
    };

    stretcher_type::stretcher_type()
    {
	for (int y = 0; y <= max_bitexpt; ++y) {
	    for (int i = 0; i < size; ++i) {
		array[y][i] = bitpow_algo(i, y);
	    }
	}
    }

    template<typename T>
      inline T
      stretcher_type::operator()(T i, unsigned int y)
      {
	  const array_value_type (&subarray)[size] = array[y];
	  T r = 0L;
	  unsigned int bitno = 0;
	  while (i) {
	      r |= subarray[i & mask] << bitno;
	      i >>= bits;
	      bitno += bits*y;
	  }
	  return r;
      }

    template<typename T>
      inline T
      stretcher_type::FP_bitpow(T x, unsigned int y)
      {
	  // This is slow!
	  const array_value_type (&subarray)[size] = array[y];
	  const int shift = y*bits;
	  T r = 0;
	  int bitno;
	  x = std::frexp(x, &bitno);
	  bitno *= y;
	  int repeat = (std::numeric_limits<T>::digits + shift - 1)/shift;
	  while (repeat--) {
	      T x_int;
	      x = std::modf(std::ldexp(x, bits), &x_int);
	      bitno -= shift;
	      r += std::ldexp(T(subarray[(int)x_int]), bitno);
	  }
	  return r;
      }

    stretcher_type stretcher;
  }


  //  --- bitpow ---

  unsigned int
  bitpow(unsigned int x, int y)
  {
      if (y > stretcher_type::max_bitexpt)
	  return bitpow_algo(x, y);
      else
	  return stretcher(x, y);
  }

  unsigned long
  bitpow(unsigned long x, int y)
  {
      if (y > stretcher_type::max_bitexpt)
	  return bitpow_algo(x, y);
      else
	  return stretcher(x, y);
  }

#ifdef MORE_CONF_CXX_HAVE_LONG_LONG
  unsigned long long
  bitpow(unsigned long long x, int y)
  {
      if (y > stretcher_type::max_bitexpt)
	  return bitpow_algo(x, y);
      else
	  return stretcher(x, y);
  }
#endif

  float
  bitpow(float x, int y)
  {
      if (y <= 3) {
	  switch(y) {
	    case 3:
	      return bitpow_fp_algo_n<float, 3>(x);
	    case 2:
	      return bitpow_fp_algo_n<float, 2>(x);
	    case 1:
	      return x;
	    case 0:
	      return x == 0.0f? 0.0f : 1.0f;
	    default:
	      return std::numeric_limits<float>::signaling_NaN();
	  }
      }
      return bitpow_fp_algo(x, y);
  }

  double
  bitpow(double x, int y)
  {
      if (y <= 3) {
	  switch(y) {
	    case 3:
	      return bitpow_fp_algo_n<double, 3>(x);
	    case 2:
	      return bitpow_fp_algo_n<double, 2>(x);
	    case 1:
	      return x;
	    case 0:
	      return x == 0.0? 0.0 : 1.0;
	    default:
	      return std::numeric_limits<double>::signaling_NaN();
	  }
      }
      return bitpow_fp_algo(x, y);
  }

#ifdef MORE_CONF_CXX_HAVE_LONG_DOUBLE
  long double
  bitpow(long double x, int y)
  {
      if (y <= 3) {
	  switch(y) {
	    case 3:
	      return bitpow_fp_algo_n<long double, 3>(x);
	    case 2:
	      return bitpow_fp_algo_n<long double, 2>(x);
	    case 1:
	      return x;
	    case 0:
	      return x == 0.0? 0.0 : 1.0;
	    default:
	      return std::numeric_limits<long double>::signaling_NaN();
	  }
      }
      return bitpow_fp_algo(x, y);
  }
#endif


  //  --- bitlace ---

  unsigned int
  bitlace(unsigned int x0, unsigned int x1)
  {
      return stretcher(x0, 2) | (stretcher(x1, 2)<<1);
  }

  unsigned long
  bitlace(unsigned long x0, unsigned long x1)
  {
      return stretcher(x0, 2) | (stretcher(x1, 2)<<1);
  }

#ifdef MORE_CONF_CXX_HAVE_LONG_LONG
  unsigned long long
  bitlace(unsigned long long x0, unsigned long long x1)
  {
      return stretcher(x0, 2) | (stretcher(x1, 2)<<1);
  }
#endif

  unsigned long
  bitlace(unsigned long x0, unsigned long x1, unsigned long x2)
  {
      return stretcher(x0, 3)
	  | (stretcher(x1, 3)<<1) | (stretcher(x2, 3)<<2);
  }

//   double bitlace(double x0, double x1) {
//       // XXX works only for positive values
//       int const half_max_bits = sizeof(big_uint)*4;
//       int i_exp;
//       double x_max = std::max(std::fabs(x0), std::fabs(x1));
//       double scale =
// 	  std::ldexp(std::frexp(x_max, &i_exp)/x_max, half_max_bits);
//       i_exp -= half_max_bits;
//       x0 *= scale;
//       x1 *= scale;
//       big_uint i0 = (big_uint)x0;
//       big_uint i1 = (big_uint)x1;
//       double r = bitlace(i0, i1);
//       return std::ldexp(r, i_exp*2);
//   }

  double
  bitlace(double x0, double x1)
  {
      return bitpow(x0, 2) + 2*bitpow(x1, 2);
  }

  double
  bitlace(double x0, double x1, double x2)
  {
      return bitpow(x0, 3) + 2*bitpow(x1, 3) + 4*bitpow(x2, 3);
  }

}}
