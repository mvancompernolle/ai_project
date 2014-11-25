#define MORE_BACKWARD 20000531
#include <more/num/point_map.h>
#include <iostream>
#include <stdlib.h>


int main() {
    typedef double real_type;
    typedef more::vectin<real_type, 2> position_type;

    typedef more::point_map<position_type, real_type> point_map_type;
    typedef point_map_type::vicinity_view vicinity_view;

    double R = 100.0;

    point_map_type c(R);

    for (int i = 0; i < 10000; ++i) {
	position_type x(R*(2.1*drand48()-1.05), R*(2.1*drand48()-1.05));
	c.insert(x, 0.0);
    }

    vicinity_view vz[2];
    vz[0] = c.vicinity(position_type(-0.1*R, 0.7*R), more::pow2(0.6*R));
    vz[1] = c.vicinity(position_type(-0.94*R, -0.3*R), more::pow2(0.1*R));
    vz[2] = c.vicinity(position_type(0.32*R, -0.8*R), more::pow2(0.03*R));
    std::cout
      << "$ data=curve2d\n"
      << "% xmin=" << -R*1.1 << " xmax=" << R*1.1
      << "  ymin=" << -R*1.1 << " ymax=" << R*1.1 << "\n";
    for (int ivz = 0; ivz < 3; ++ivz) {
	std::cout << "% linetype=0 markertype=2\n";
	for (vicinity_view::iterator it = vz[ivz].begin();
	     it != vz[ivz].end(); ++it) {
	    position_type x = it->first;
	    std::cout << x[0] << ' ' << x[1] << '\n';
	}
	std::cout << "\n% linetype=0 markertype=1\n";
	for (vicinity_view::subiterator it = vz[ivz].sub_begin();
	     it != vz[ivz].sub_end(); ++it) {
	    position_type x = it->second.first;
	    std::cout << x[0] << ' ' << x[1] << '\n';
	}
	std::cout << "\n";
    }
    std::cout << "$ end\n";
    return 0;
}
