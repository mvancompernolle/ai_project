#include <iostream>
#include <more/gen/lambda.h>
#include <cmath>
#include <cstdlib>
#include <functional>

namespace gen = more::gen;

struct A {
    double f() {
	gen::placeholder<1, double> x_;
	gen::placeholder<2, double> y_;
	return gen::lambda(x_, y_, x_*y_)(1.1, 40);
    }
};

int main() {
    using gen::lambda;
    using gen::adapt;
    try {
	gen::placeholder<1, double> x;
	gen::placeholder<2, int> y;
	if (std::abs(lambda(x, apply(adapt<double>(static_cast<double
						   (*)(double)>(&std::sin)), x))(3.3) -
		     std::sin(3.3)) > 1e-9) throw 0;
	if (std::abs(lambda(x, y,
		      apply(adapt((double(*)(double,double))&std::pow),
			    x, y))(1.1, 4) -
	       std::pow(1.1, 4)) > 1e-9) throw 1;

	gen::placeholder<3, int> i_;
	gen::placeholder<4, int> j_;
	gen::placeholder<5, int> k_;

	if (lambda(i_, j_, i_+j_)(3, 9) != 12) throw 2;
	A a;
	if ((int)(a.f()+.5) != 44) throw 3;
	if (lambda(i_, i_+3)(5) != 8) throw 4;
	if (lambda(i_, j_, 12*i_ - 6*j_)(1, 2) != 0) throw 5;

	if (lambda(i_, j_, i_==j_)(3, 5)) throw 6;            // PH==PH
	if (lambda(i_, j_, (i_+j_)==(i_-j_))(1, 2)) throw 9;  // E==E
	if (!lambda(i_, i_==9)(9)) throw 7;               // PH==V
	if (lambda(k_, 10==k_)(3)) throw 8;               // V==PH
	if (lambda(i_, j_, i_==(i_-j_))(1, 2)) throw 10;  // PH==E
	if (lambda(i_, j_, (i_+j_)==j_)(1, 2)) throw 11;  // E==PH
	if (lambda(i_, j_, (i_+j_)==1)(3, 3)) throw 12;   // E==V
	if (lambda(i_, j_, 1==(i_+j_))(3, 3)) throw 13;   // V==E

	// same for `operator!=' which is encoded by the macro.
	if (!lambda(i_, j_, i_!=j_)(3, 5)) throw 6;            // PH!=PH
	if (!lambda(i_, j_, (i_+j_)!=(i_-j_))(1, 2)) throw 9;  // E!=E
	if (lambda(i_, i_!=9)(9)) throw 7;               // PH!=V
	if (!lambda(k_, 10!=k_)(3)) throw 8;               // V!=PH
	if (!lambda(i_, j_, i_!=(i_-j_))(1, 2)) throw 10;  // PH!=E
	if (!lambda(i_, j_, (i_+j_)!=j_)(1, 2)) throw 11;  // E!=PH
	if (!lambda(i_, j_, (i_+j_)!=1)(3, 3)) throw 12;   // E!=V
	if (!lambda(i_, j_, 1!=(i_+j_))(3, 3)) throw 13;   // V!=E
	return 0;
    } catch (int err) {
	std::cerr << "catched error #" << err << std::endl;
	return 1;
    }
}


