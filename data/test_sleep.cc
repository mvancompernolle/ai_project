#include <more/sys/time.h>
#include <iostream>

namespace sys = more::sys;

int
main()
{
    sys::time_t t = -sys::world_time();
    sys::sleep(1.23);
    t += sys::world_time();
    std::cout << "Requested 1.23 s sleep, measured " << t << " s\n";
    return 0;
}
