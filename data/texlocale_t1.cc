#include "texlocale.h"
#include <locale>
#include <iostream>

int
main()
{
    double x[] = {
	0.0,
	1e-5, 1e-4, 1e-3, 1e-2, 0.1, 1.0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7,
	0.00999, 0.0999, 0.999, 9.99, 99.9, 999.0, 9990.0,
	0.009999, 0.09999, 0.9999, 9.999, 99.99, 999.9, 9999.0, 99990.0,
	0.2224, 2.224, 2224,
	0.02225, 2.225, 2225,
	0.0222501, 2.22501, 2225.01,
    };
    const int n = sizeof(x)/sizeof(x[0]);
    std::locale loc = std::locale(std::locale(),
				  new more::io::tex_num_put<char>);
    std::cout.imbue(loc);
//     std::cout << 2.3e-6 << std::endl;
//     std::use_facet< std::num_put<char> >(loc)
//       .put(std::cout, std::cout, std::cout.fill(), 1.3e-7);
//     std::cout << std::endl;
    std::cout.precision(3);
    for (int i = 0; i < n; ++i)
	std::cout << x[i] << std::endl;
    return 0;
}
