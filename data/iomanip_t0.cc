#include <iostream>
#include <stdexcept>
#include <more/io/iomanip.h>
#include <more/io/cmdline.h>

namespace io = more::io;

int
main(int argc, char** argv)
try {
    io::cmdline cl;
    cl.parse(argc, argv);
    std::cout << "If you have terminfo "
	      << io::set_bold << "this should be bold" << io::clr_bold
	      << ", " << io::set_underline << "this should be underlined"
	      << io::clr_underline
	      << " and\n"
	      << io::set_italic << "this should be italic" << io::clr_italic
	      << ".  If this is HTML, also "
	      << io::beg_sub << "subscript" << io::end_sub << " and "
	      << io::beg_super << "superscript" << io::end_super
	      << "\nshould work.\n";
    return 0;
}
catch (io::cmdline::relax) {
    return 0;
}
catch (std::runtime_error const& xc) {
    std::cout << "runtime_error: " << xc.what() << std::endl;
    return 1;
}
