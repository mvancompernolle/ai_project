#include <more/io/binary_io.h>
#include <more/io/cmdline.h>
#include <fstream>
#include <cstring>

namespace io = more::io;

int
main(int argc, char** argv)
try {
    std::string name;
    bool dov = false;
    bool no_sync = false;
    bool no_hdr = false;
    more::io::cmdline cmd;
    cmd.insert_reference("", "file#Input file.", name);
    cmd.insert_setter("-v", "Print values.", dov, true);
    cmd.insert_setter("--no-syncstream",
		      "Do not look for the syncstream header.",
		      no_sync, true);
    cmd.insert_setter("--no-header",
		      "Do not print the heander with the syncstream version.",
		      no_hdr, true);
    cmd.parse(argc, argv);
    std::ifstream is(name.c_str(),
		     std::ios_base::in | std::ios_base::binary);
    if (!no_sync) {
	static const int n_magic = 17;
	static char const magic[n_magic] = "more::syncstream";
	char buf[n_magic];
	is.read(buf, n_magic);
	if (std::strncmp(buf, magic, n_magic))
	    throw std::runtime_error("Not a syncstream file. "
				     "Try --no-syncstream.");
	int i0p, i1p, i2p;
	io::binary_read(is, i0p);
	io::binary_read(is, i1p);
	io::binary_read(is, i2p);
	if (!no_hdr)
	    std::cout << "<?more::lang::syncstream version=\""
		      << i0p << '.' << i1p << '.' << i2p << "\"?>\n\n";
    }

    io::binary_dump(is, std::cout, dov);
    return 0;
}
catch (more::io::cmdline::relax) {
    std::cout << "NOTE. Floating point numbers will appear as two integers.\n";
    return 0;
}
catch (std::exception const& xc) {
    std::cerr << "exception: " << xc.what() << std::endl;
    return 1;
}
