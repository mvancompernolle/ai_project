#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <more/io/socket.h>
#include <more/io/cmdline.h>
#include <more/diag/errno.h>

namespace diag = more::diag;

int main(int argc, char** argv) try {
    more::io::cmdline cl;
    std::string s_addr;
    cl.insert_reference("", "addr#address to bind to", s_addr);
    cl.parse(argc, argv);

    more::io::socket_address addr, from_addr;
    std::istringstream iss_addr(s_addr);
    iss_addr >> addr;
    std::cout << "I think you typed the address " << addr << std::endl;

    more::io::socket sock;
    std::cout << "Connecting." << std::endl;
    if (!sock.connect(addr))
	throw std::runtime_error(std::string("Could not connect: ") +
				 diag::strerror(sock.the_errno()));

    std::cout << "Writing to socket." << std::endl;
    sock << "Do you read me?" << std::endl;
    std::cout << "Reading replay\n<message>";
    std::string msg;
    getline(sock, msg);
    std::cout << msg;
    std::cout << "</message>\nDone writing. Then destruct and exit."
	      << std::endl;

    return 0;

} catch (std::exception const& xc) {
    std::cerr << "exception: " << xc.what() << std::endl;
    return 1;
} catch (...) {
    std::cerr << "unkown execption" << std::endl;
    return 1;
}
