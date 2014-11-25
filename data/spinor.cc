#include <more/math/complex.h>
#include <more/math/spinor.h>
#include <more/math/cospinor.h>
#include <more/io/iomanip.h>
#include <more/io/syncstream.h>
#include <algorithm>
#include <ios>
#include <iostream>
#include <cassert>

namespace more {
namespace math {

  // -- alpha, beta --

  alpha_tag alpha;
  beta_tag beta;
  coalpha_tag coalpha;
  cobeta_tag cobeta;


  // -- Pauli matrices --

  sigma_x_tag sigma_x;
  sigma_y_tag sigma_y;
  sigma_z_tag sigma_z;


  // -- IO --

  template<typename T>
    std::ostream&
    operator<<(std::ostream& os, const spinor<T>& x)
    {
#  ifdef MORE_EXTENSIVE_SPINOR_OUTPUT
	os << x(half) << "|1/2> + " << x(-half) << "|-1/2>";
#  else
	std::streamsize w = std::max(os.width()-3, std::streamsize(0));
	std::streamsize p = os.precision();
	os << '[' << io::setwp((w+1)/2, p) << x(half)
	   << ';' << io::setwp(w/2, p) << x(-half) << ']';
#  endif
	return os;
    }

  template<typename T>
    std::ostream&
    operator<<(std::ostream& os, const cospinor<T>& x)
    {
#  ifdef MORE_EXTENSIVE_SPINOR_OUTPUT
	os << x(half) << "<1/2| + " << x(-half) << "<-1/2|";
#  else
	std::streamsize w = std::max(os.width()-3, std::streamsize(0));
	std::streamsize p = os.precision();
	os << '[' << io::setwp((w+1)/2, p) << x(half)
	   << ',' << io::setwp(w/2, p) << x(-half) << ']';
#  endif
	return os;
    }

  template<typename T>
    std::ostream&
    operator<<(std::ostream& os, const spinopr<T>& x)
    {
#  ifdef MORE_EXTENSIVE_SPINOR_OUTPUT
	os << x(half, half) << "|1/2><1/2| + "
	   << x(-half, half) << "|-1/2><1/2| + "
	   << x(half, -half) << "|1/2><-1/2| + "
	   << x(-half, -half) << "|-1/2><-1/2|";
#  else
//	int w = 10, p = 4;
	os << '['
//	   << std::setw(w) << std::setprecision(p)
	   << x(half, half) << " "
//	   << std::setw(w) << std::setprecision(p)
	   << x(half, -half) << "; "
//	   << std::setw(w) << std::setprecision(p)
	   << x(-half, half) << " "
//	   << std::setw(w) << std::setprecision(p)
	   << x(-half, -half) << ']';
#  endif
	return os;
    }

  template <typename T>
    void
    spinor<T>::sync(io::syncstream& sio)
    {
	sio | u | d;
    }

  template <typename T>
    void
    spinopr<T>::sync(io::syncstream& sio)
    {
	sio | uu | du | ud | dd;
    }

  template<typename T>
    std::istream& operator>>(std::istream&, spinor<T>&) { assert(0); }
  template<typename T>
    std::istream& operator>>(std::istream&, cospinor<T>&) { assert(0); }
  template<typename T>
    std::istream& operator>>(std::istream&, spinopr<T>&) { assert(0); }

  template std::ostream&
    operator<<(std::ostream&, const spinor<float>&);
  template std::ostream&
    operator<<(std::ostream&, const spinor< complex<float> >&);
  template std::ostream&
    operator<<(std::ostream&, const cospinor<float>&);
  template std::ostream&
    operator<<(std::ostream&, const cospinor< complex<float> >&);
  template std::ostream&
    operator<<(std::ostream&, const spinopr<float>&);
  template std::ostream&
    operator<<(std::ostream&, const spinopr< complex<float> >&);

  template std::ostream&
    operator<<(std::ostream&, const spinor<double>&);
  template std::ostream&
    operator<<(std::ostream&, const spinor< complex<double> >&);
  template std::ostream&
    operator<<(std::ostream&, const cospinor<double>&);
  template std::ostream&
    operator<<(std::ostream&, const cospinor< complex<double> >&);
  template std::ostream&
    operator<<(std::ostream&, const spinopr<double>&);
  template std::ostream&
    operator<<(std::ostream&, const spinopr< complex<double> >&);

  template std::ostream&
    operator<<(std::ostream&, const spinor<long double>&);
  template std::ostream&
    operator<<(std::ostream&, const spinor< complex<long double> >&);
  template std::ostream&
    operator<<(std::ostream&, const cospinor<long double>&);
  template std::ostream&
    operator<<(std::ostream&, const cospinor< complex<long double> >&);
  template std::ostream&
    operator<<(std::ostream&, const spinopr<long double>&);
  template std::ostream&
    operator<<(std::ostream&, const spinopr< complex<long double> >&);

  template void spinor<float>::sync(io::syncstream&);
  template void spinopr<float>::sync(io::syncstream&);
  template void spinor< complex<float> >::sync(io::syncstream&);
  template void spinopr< complex<float> >::sync(io::syncstream&);

  template void spinor<double>::sync(io::syncstream&);
  template void spinopr<double>::sync(io::syncstream&);
  template void spinor< complex<double> >::sync(io::syncstream&);
  template void spinopr< complex<double> >::sync(io::syncstream&);

  template void spinor<long double>::sync(io::syncstream&);
  template void spinopr<long double>::sync(io::syncstream&);
  template void spinor< complex<long double> >::sync(io::syncstream&);
  template void spinopr< complex<long double> >::sync(io::syncstream&);

}}
