#include <more/math/math.h>
#include <more/diag/stopwatch.h>
#include <limits>
#include <iostream>
#include <more/diag/debug.h>
#include <more/math/spinor.h>

using namespace more;
using math::half;

typedef double real_type;
typedef std::complex<real_type> complex_type;
typedef math::spinor<complex_type> spinor_type;
typedef math::spinopr<complex_type> spinopr_type;

namespace more {
namespace math {
  template<typename T>
    math::spinopr<T>
    exp_series(math::spinopr<T> const& x)
    {
	typedef typename math::norm_type_<T>::eval real_type;
	int n = 1000;
	math::spinopr<T> res, term;
	res = 1;
	term = 1;
	int i = 1;
	while(i < n) {
	    do {
		term *= x;
		term /= i;
		res += term;
		++i;
	    } while(i%4 != 0);
	    if (norm(term) <
		norm(std::numeric_limits<real_type>::epsilon())*norm(res))
		break;
	}
// 	std::clog << i << " iter." << std::endl;
	return res;
    }
}}


int
main()
{
    using math::exp_series;

    int stat = 0;
    more::diag::stopwatch sw1, sw2;
    spinopr_type op;
    std::cout
	<< "epsilon(R) = " << std::numeric_limits<real_type>::epsilon()
	<< "\nepsilon(C) = " << std::numeric_limits<complex_type>::epsilon()
	<< std::endl;
    for (int count = 0; count < 1000; ++count) {
	for (int i = 0; i < 4; ++i) {
	    math::pm_half m1 = half, m2 = half;
	    if (i >= 2) m1 = -m1;
	    if (i % 2) m2 = -m2;
	    op(m1, m2) = exp(2.0*drand48() +
			     2*math::numbers::pi*math::onei*drand48());
	}
	sw1.start();
	spinopr_type op1 = exp(op);
	sw1.stop();
	sw2.start();
	spinopr_type op2 = exp_series(op);
	sw2.stop();
	if (norm(op1 - op2) > std::max(1e-4*norm(op1), 1e-7)) {
	    static int cnt = 0;
	    if (cnt++ > 20) {
		std::cerr << "*** More errors, exit.\n";
		return 1;
	    }
	    std::cerr << "*** fail:"
		      << "\n  X = " << op
		      << "\n  exp(X)   = " << op1
		      << "\n  exp_s(X) =" << op2
		      << "\n  exp(X)*exp(-X)     = " << exp(op)*exp(-op)
		      << "\n  exp_s(X)*exp_s(-X) = "
		      << exp_series(op)*exp_series(-op)
		      << "\n\n";
	    stat = 1;
	}
    }
    MORE_SHOW(op);
    MORE_SHOW(exp(op));
    MORE_SHOW(exp_series(op));
    std::clog << "\nCheck if exp is precise for small negative exponents.\n";
    MORE_SHOW(-1e-8*op);
    MORE_SHOW(exp(-1e-8*op));
    MORE_SHOW(exp_series(-1e-8*op));
    std::cout
	<< "\neigenvalue based: " << sw1 << '\n'
	<< "series based:     " << sw2 << '\n' << std::endl;
//     MORE_SHOW(exp_series(op, 2));
//     MORE_SHOW(exp_series(op, 5));
//     MORE_SHOW(exp_series(op, 10));
//     MORE_SHOW(exp_series(op, 100));
//     MORE_SHOW(exp_series(.001*op, 5));
//     MORE_SHOW(exp(0.001*op));
//     MORE_SHOW(exp(op)*exp(-op));
//     MORE_SHOW(exp_series(op, 100)*exp_series(-op, 100));
    return stat;
}
