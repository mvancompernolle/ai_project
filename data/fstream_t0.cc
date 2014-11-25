#include <more/io/cmdline.h>
#include <more/io/fstream.h>
#include <iostream>

int main(int argc, char** argv)
{
    if (argc != 2) {
	std::cerr << "** need one arg" << std::endl;
	return 1;
    }
    more::io::ifstream is(argv[1]);
    while (is.good()) {
	char c = is.get();
	if (is.eof())
	    return 0;
	std::cout << c;
    }
    if (is.bad()) {
	std::cerr << "** file error" << std::endl;
	return 1;
    }
}
