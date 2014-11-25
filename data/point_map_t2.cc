#include <iostream>
#include <more/num/point_map.h>

int main() {
    typedef double real_type;
    typedef more::num::vectin<real_type, 2> position_type;

    typedef more::num::point_map<position_type, real_type> point_map_type;
    typedef point_map_type::vicinity_view vicinity_view;
    typedef point_map_type::iterator iterator;

    point_map_type c;

    std::cout << "$ data=curve2d\n"
	      << "% equalscale\n"
	      << "% linetype=0\n"
	      << "% markertype=12\n";
    srand48(1L);
    for (int i = 0; i < 800; ++i) {
	position_type x(.1*drand48(), .1*drand48());
	c.insert(std::make_pair(x, 0.0));
	std::cout << x[0] << ' ' << x[1] << '\n';
    }
    std::cout << '\n';

    for (int ipos = 0; ipos < 800; ++ipos) {
	std::cout << "% linetype=1\n";
	std::cout << "% markertype=0\n";
	position_type x(.1*drand48(), .1*drand48());
	iterator it = c.find_closest(x);
	std::cout << x[0] << ' ' << x[1] << '\n'
		  << it->first[0] << ' ' << it->first[1] << "\n\n";
    }
    std::cout << std::endl;

    c.clear();
//     std::cout << "% linetype=3\n";
//     for (double x = 0.0; x < 1.0; x += 0.02)
// 	for (double y = 0.0; y < 1.0; y += 0.02)
// 	    c.insert(position_type(x, y), 0.0);
//     for (iterator it = c.begin(); it != c.end(); ++it)
// 	std::cout << it->first[0] << ' ' << it->first[1] << '\n';

    std::cout << "$ end\n";
    return 0;
}
