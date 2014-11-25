#include <more/gen/functional.h>
#include <more/gen/closure.h>
#include <iostream>

struct func : std::unary_function<double, int> {
    int operator()(double x) { return int(x*10); }
};
struct gen : more::gen::generator<int> {
    int operator()() { return 1111; }
};

int main(int, char**) {
    gen g;
    more::gen::generator_closure<int> fcl1, fcl2(g);
    fcl1 = g;
    std::cout << fcl1() << ' ' << fcl2() << '\n';

    bool xc_ok = false;
    try {
	more::gen::unary_closure<int, int> cl;
	cl(1);
    } catch (std::logic_error const& xc) {
	xc_ok = true;
    }
    if (!xc_ok) {
	std::cerr << "Undefined closure did not throw exception."
		  << std::endl;
	return 1;
    }

    func f;
    more::gen::unary_closure<double, int> f1, f2(f);
    f1 = f;
    std::cout << f1(3.2) << ' ' << f2(3.2) << '\n';

    return 0;
}
