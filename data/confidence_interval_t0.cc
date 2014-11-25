#include <more/phys/confidence_interval.h>
#include <more/io/cmdline.h>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <math.h>

namespace io = more::io;
namespace phys = more::phys;

int
main(int argc, char** argv)
{
    io::cmdline cl;
    cl.parse(argc, argv);

    typedef phys::confidence_interval<double> confiv;
    std::cout << "\nSymmetric values:\n";
    for (int i = 0; i < 20; ++i) {
	double cent = rand() / (double)RAND_MAX;
	double rdev = pow(0.7, rand() % 8);
	double fxpt = pow(10.0, rand() % 6 - 3);
	cent *= fxpt;
	confiv x(cent, cent*rdev);
//	std::cout << x << " = " << cent << " ± " << rdev*cent << std::endl;
	std::cout << std::setw(14) << x << " = "
		  << x.cent() << " ± " << x.dev_below() << '\n';
    }
    std::cout << "\nAsymmetric values:\n";
    for (int i = 0; i < 10; ++i) {
	double cent = rand() / (double)RAND_MAX;
	double rdev0 = pow(0.7, rand() % 8);
	double rdev1 = pow(0.7, rand() % 8);
	double fxpt = pow(10.0, rand() % 6 - 3);
	cent *= fxpt;
	confiv x(cent, cent*rdev0, cent*rdev1);
	std::cout << std::setw(15) << x << " ∈ ["
		  << std::setw(11) << x.cent() << " - "
		  << std::setw(11) << x.dev_below() << ", "
		  << std::setw(11) << x.cent() << " + "
		  << std::setw(11) << x.dev_above() << "]\n";
    }
    std::cout << "\nSpecial cases:\n"
              << "    " << confiv(0.0, 0) << " = 0\n"
              << "    " << confiv(1.0, 0) << " = 1\n"
              << "    " << confiv(1000.0, 0) << " = 1000\n"
              << "    " << confiv(.0001, 0) << " = .0001\n"
              << "    " << confiv(1000.0, 1.0) << " = 1000(1)\n"
              << "    " << confiv(30.0, confiv::unknown) << " ≅ 30\n"
              << "    " << confiv(3.0, confiv::infinite, 0.0) << " ≤ 3\n\n";
    return 0;
}

// Local Variables:
// coding: utf-8
// End:
