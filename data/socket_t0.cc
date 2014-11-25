#include <more/io/socket.h>
#include <more/io/cmdline.h>
#include <iostream>

namespace io = more::io;

int
main(int argc, char** argv)
{
    std::string str_haddr;

    io::cmdline cl;
    cl.insert_reference("", "host#Enter a host name or an IP number.",
			str_haddr);
    cl.parse(argc, argv);

    io::host_address haddr(str_haddr);
    std::cout << "Official name: " << haddr.host_name() << '\n';
    bool is1st = true;
    for (io::host_address::host_alias_range rng = haddr.host_aliases();
	 rng.first != rng.second; ++rng.first) {
	if (is1st) {
	    std::cout << "Aliases: ";
	    is1st = false;
	}
	else
	    std::cout << ", ";
	std::cout << *rng.first;
    }
    if (!is1st) {
	std::cout << '\n';
	is1st = true;
    }
    for (io::host_address::ip_address_range rng = haddr.ip_addresses();
	 rng.first != rng.second; ++rng.first) {
	if (is1st) {
	    std::cout << "IP addresses: ";
	    is1st = false;
	}
	else
	    std::cout << ", ";
	std::cout << *rng.first;
    }
    if (!is1st)
	std::cout << '\n';
    return 0;
}
