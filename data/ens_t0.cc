#include <more/phys/ens.h>
#include <sstream>
#include <iostream>

int
main(int argc, char const** argv) try
{
    more::phys::nucleus nucl0;
    if (argc == 1)
	nucl0 = more::phys::nucleus(40, 36);
    if (argc == 2) {
	std::istringstream iss(argv[1]);
	iss >> nucl0;
    }
    std::cout << "Datasets for Nucleus " << nucl0 << "\n\n";
    more::phys::ens::nucleus nucl(nucl0);
    nucl.dump(std::cout);
    return 0;
}
catch (std::runtime_error const& xc) {
    std::cerr << "runtime_error: " << xc.what() << std::endl;
    return 1;
}
#if 0
catch (std::exception const& xc) {
    std::cerr << "exception: " << xc.what() << std::endl;
    return 1;
}
#endif
