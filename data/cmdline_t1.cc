#include <iostream>
#include <more/io/cmdline.h>


int
main(int argc, char** argv) try {
    int i = -1, j = -1;
    more::io::cmdline cl;
    cl.insert_reference("", "i", i);
    cl.insert_reference("", "j", j);
    cl.parse(argc, argv);
//     cl.print_help(std::cout);
    std::cout << "i = " << i << ", j = " << j << std::endl;
    return 0;
}
catch (more::io::cmdline::relax const& xc) {
    return 0;
}
catch (more::io::cmdline::failure const& xc) {
    std::cout << xc.what() << '\n';
    return 1;
}
catch (std::exception const& xc) {
    std::cout << "exception: " << xc.what() << '\n';
    return 1;
}
catch (...) {
    std::cerr << "unknown exception.\n";
    return 1;
}
