#include <more/sys/resource.h>
#include <more/sys/time.h>
#include <iostream>
#include <iomanip>

#define SHOW(ex) (std::cout << #ex" = " << (ex) << std::endl)

namespace sys = more::sys;

void
show_state()
{
    SHOW(sys::current_spatial_energy(1));
    SHOW(sys::current_temporal_energy());
}

int
main()
{
    show_state();
    std::cout << "Allocating.\n";
    char* ptr = new char[500000000];
    show_state();
    std::cout << "Freeing.\n";
    delete[] ptr;
    show_state();
    return 0;
}

