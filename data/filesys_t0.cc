#include <more/io/filesys.h>
#include <more/io/cmdline.h>
#include <more/gen/iterator.h>
#include <iostream>
#include <iomanip>

int main(int argc, char** argv) {
    std::string patt;
    more::io::cmdline cl;
    cl.insert_reference("", "pattern#Globbing pattern.", patt);
    cl.parse(argc, argv);
    more::io::file_glob_list lst(patt);
    if (lst.good()) {
	std::cout << "Files matching \"" << patt << "\": ";
	std::copy(lst.begin(), lst.end(),
		  more::gen::ostream_iterator<char const*>(std::cout, ", "));
	std::cout << std::endl;
	return 0;
    }
    else {
	std::cerr << "Globbing failed." << std::endl;
	return 1;
    }
}
