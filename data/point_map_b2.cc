#include <iostream>
#include <iomanip>
#include <more/num/point_map.h>
#include <more/diag/stopwatch.h>


void benchit(int n_points, int n_find)
{
    more::diag::stopwatch sw;
    typedef double real_type;
    typedef more::num::vectin<real_type, 2> position_type;

    typedef more::num::point_map<position_type, real_type> point_map_type;
    typedef point_map_type::iterator iterator;

    point_map_type c;

    srand48(1L);
    for (int i = 0; i < n_points; ++i) {
	position_type x(.1*drand48(), .1*drand48());
	c.insert(std::make_pair(x, 0.0));
    }

    sw.start();
    for (int ipos = 0; ipos < n_find; ++ipos) {
	position_type x(.1*drand48(), .1*drand48());
	iterator it = c.find_closest(x);
    }
    sw.stop();

    std::cout << std::setw(8) << n_points
	      << std::setw(12) << sw/n_find << std::endl;
}

int main()
{
    for (int n_points = 2000; n_points < 200000; n_points += 2000)
	benchit(n_points, 10000);
    return 0;
}
