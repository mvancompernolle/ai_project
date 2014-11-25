#include <more/num/point_map.h>
#include <iostream>
#include <stdlib.h>

int main() {
    typedef double real_type;
    typedef more::num::vectin<real_type, 2> position_type;

    typedef more::num::point_map<position_type, real_type> point_map_type;
    typedef point_map_type::vicinity_view vicinity_view;

    point_map_type c;

    for (int i = 0; i < 10000; ++i) {
	position_type x(drand48(), drand48());
	c.insert(std::make_pair(x, 0.0));
    }

    std::cout << "$ data=curve2d\n";
    for (int ipos = 0; ipos < 4; ++ipos) {
	position_type x(drand48(), drand48());
	std::cout
	  << "% label='" << x << "'\n"
	  << "% linestyle=" << ipos+1 << "\n";
	real_type dmax = 1.0;

	for (int i = 0; i < 2; ++i)
	    dmax = std::min(dmax, std::min(x[i], 1.0-x[i]));

	for (real_type d = 0.002; d < dmax; d += 0.002) {
	    vicinity_view vz = c.vicinity(x, more::math::pow2(d));
	    int n1 = std::distance(vz.begin(), vz.end());
	    int n2 = std::distance(vz.sub_begin(), vz.sub_end());
	    std::cout << d << ' ' << n2/(real_type)n1 << '\n';
	}
	std::cout << '\n';
    }
    std::cout << "$ end\n";
    return 0;
}

