#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <more/io/socket.h>
#include <more/io/cmdline.h>
#include <more/diag/errno.h>

namespace diag = more::diag;

int main(int argc, char** argv)
try {
    more::io::cmdline cl;
    std::string s_addr;
    cl.insert_reference("", "addr#address to bind to", s_addr);
    cl.parse(argc, argv);

    more::io::socket_address addr, from_addr;
    std::istringstream iss_addr(s_addr);
    iss_addr >> addr;
    std::cout << "I think you typed the address " << addr << std::endl;

    more::io::socket sock;
    if (!sock.bind(addr))
	diag::errno_exception(sock.the_errno());
    std::cout << "Calling listen()" << std::endl;
    if (!sock.listen())
	diag::errno_exception(sock.the_errno());
    std::cout << "Calling accept()" << std::endl;
    more::io::socket sock_accepted(sock.accept());
    std::cout << "Got connection from " << sock_accepted.address() << std::endl;
    if (!std::cout.good())
	throw std::runtime_error("Accepted stream is not good.");
    std::cout << "<message>";
    std::string msg;
    getline(sock_accepted, msg);
    std::cout << msg;
    std::cout << "</message>" << std::endl;
    std::cout << "Sending back." << std::endl;
    sock_accepted << "Yeah, steady and clear." << std::endl;
    std::cout << "All done." << std::endl;
    return 0;
} catch (std::exception const& xc) {
    std::cerr << "exception: " << xc.what() << std::endl;
    return 1;
} catch (...) {
    std::cerr << "unknown exception" << std::endl;
    return 1;
}


