#include <more/bits/fdstreambuf.h>
#include <iostream>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstdio>

struct fdstream : std::iostream {
    explicit fdstream(int fd)
	: std::iostream(new more::io::bits::fdstreambuf(fd, fd)) {}
    more::io::bits::fdstreambuf* rdbuf() {
	return static_cast<more::io::bits::fdstreambuf*>
	    (std::iostream::rdbuf());
    }
};

int main(int argc, char** argv) {
    {
	int fd = open("tmp.test_fdstreambuf.txt", O_CREAT | O_WRONLY, 0644);
	if (fd == -1) {
	    std::perror(argv[0]);
	    std::exit(1);
	}
	fdstream fdout(fd);
	std::cerr << "Stream created. Now, writing to stream." << std::endl;
	fdout << "Testing testing.\n";
	std::cout << "Done. Then, flush it." << std::endl;
	fdout << std::flush;
	std::cout << "Good, then destruct it." << std::endl;
    }
    {
	int fd = open("tmp.test_fdstreambuf.txt", O_RDONLY);
	if (fd == -1) {
	    std::perror(argv[0]);
	    std::exit(1);
	}
	fdstream fdin(fd);
	std::cerr << "Created input stream. Now, read it.\n"
		  << "<contents>" << std::endl;
	char c;
	while (c = fdin.get(), fdin.good())
	    std::cerr.put(c);
	std::cerr << "</contents>\nDone." << std::endl;
    }
    return 0;
}
