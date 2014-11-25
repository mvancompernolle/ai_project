#include <exception>
#include <iostream>
#include <iomanip>
#include <more/math/special_functions.h>

int main() try {
    double x = 0.0, dx = .1;
    for(int i = 0; i < 100; ++i) {
	x += dx;
	std::cout << std::setw(12) << x;
	for(int n = 0; n < 5; ++n)
	    std::cout << std::setw(12) << more::math::besj(n, x);
	std::cout << std::endl;
    }
    return 0;
} catch(std::exception const& xc) {
    std::cerr << "exception: " << xc.what() << std::endl;
    return 1;
}

