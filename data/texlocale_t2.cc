#include "texlocale.h"
#include <locale>
#include <iostream>
#include <cstdlib>


int
main()
{
    double xx[] = { 0.0, 1e6, 100.0, 1.0, 0.01, 1e-6, 1.23456501 };
    int failcnt = 50;
    std::locale loc = std::locale(std::locale(),
				  new more::io::tex_num_put<char>);
    for (unsigned int i = 0; i < 10000; ++i) {
	std::ostringstream oss1, oss2;
	oss2.imbue(loc);
	double x;
	if (i < sizeof(xx)/sizeof(xx[0]))
	    x = xx[i];
	else {
	    x = drand48();
	    int k = lrand48() % 100 - 50;
	    x = std::ldexp(x, k);
	}
	oss1 << x;
	oss2 << x;
	std::string s1 = oss1.str();
	std::string s2 = oss2.str();
	std::string::size_type j = s2.find('$');
	if (j != std::string::npos) {
	    s2.erase(j+1, 9);
	    s2[j] = 'e';
	}
	std::istringstream iss1(s1), iss2(s2);
	double x1 = 1.1, x2 = 2.2;
	iss1 >> x1;
	iss2 >> x2;
	if (x1 != x2) {
	    std::cout << "*** " << s1 << " != " << s2 << std::endl;
	    if (!failcnt--) return 1;
	}
	else if (i < 20)
	    std::cout << s1 << " = " << s2 << std::endl;
    }
    return 0;
}
