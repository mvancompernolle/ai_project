#include <iostream>
#include <string>
#include <more/io/cmdline.h>
#include <more/sys/date.h>

namespace sys = more::sys;
namespace io = more::io;

int
main(int argc, char** argv)
try {
    std::string s, s_opt = "unspecified";
    double x;
    int n;
    sys::date when;
    bool yes = false;
    bool n_isdef = false;
    bool have_when = false;
    io::cmdline cl;
    cl.insert_reference("", "x#A real number", x);
    cl.insert_reference("", "s#A string", s);
    cl.insert_reference("--some-long-opt", "s_opt#Another string", s_opt);
    cl.insert_reference("-n|--number", "number#A number.", n, n_isdef);
    cl.insert_reference("--date", "YYYY-MM-DD#A date.", when, have_when);
    cl.insert_setter("-y|--yes", "#Print `Yes!'", yes, true);
    cl.insert_setter("-n|--no", "Print `No!'", yes, false);
    cl.parse(argc, argv);
    std::cout << "s = " << s << ", x = " << x
	      << ", s_opt = " << s_opt << std::endl;
    if (n_isdef)
	std::cout << "n = " << n << ", ";
    else
	std::cout << "n is not given, ";
    std::cout << (yes? "Yes!" : "No!")
	      << std::endl;
    if (have_when)
	std::cout << "The date " << when << " was mentioned.\n";
    return 0;
}
catch (io::cmdline::relax const&) {
    return 0;
}
catch (io::cmdline::failure& xc) {
    std::cout << xc.what() << std::endl;
    return 1;
}
catch (std::exception& xc) {
    std::cout << "exception: " << xc.what() << std::endl;
    return 1;
}
