#include <fstream>
#include <iomanip>
#include <more/io/binary_io.h>
#include <cstdlib>
#include <assert.h>

bool test_unsigned() {
    const int n0 = 5, n = 100;
    unsigned int v[n] = { 0x00, 0x3f, 0x40, 0x1fff, 0x2000 };
    for(int i = n0; i < n; ++i) v[i] = rand();
    {
	std::ofstream os("tmp.binary_io_t2.bin");
	for(int i = 0; i < n; ++i)
	    more::io::binary_write(os, v[i]);
    }
    {
	std::ifstream is("tmp.binary_io_t2.bin");
	assert(is.good());
	for(int i = 0; i < n; ++i) {
	    unsigned int x;
	    if(more::io::binary_read(is, x), x != v[i]) {
		std::cerr
		    << "Failed comparison #" << i
		    << std::setiosflags(std::ios_base::hex)
		    << ": expected 0x" << v[i] << ", got 0x" << x
		    << std::resetiosflags(std::ios_base::hex)
		    << std::endl;
		return false;
	    }
	    assert(is.good());
	}
    }
    return true;
}

bool test_signed() {
    const int n = 100;
    int v[n];
    for(int i = 0; i < n; ++i) v[i] = rand() - RAND_MAX/2;
    {
	std::ofstream os("tmp.binary_io_t2.bin");
	for(int i = 0; i < n; ++i)
	    more::io::binary_write(os, v[i]);
    }
    {
	std::ifstream is("tmp.binary_io_t2.bin");
	assert(is.good());
	for(int i = 0; i < n; ++i) {
	    int x;
	    if(more::io::binary_read(is, x), x != v[i]) {
		std::cerr
		    << "Failed comparison #" << i
		    << ": expected 0x"
		    << std::setiosflags(std::ios_base::hex)
		    << v[i] << ", got 0x" << x
		    << std::resetiosflags(std::ios_base::hex)
		    << std::endl;
		return false;
	    }
	    assert(is.good());
	}
    }
    return true;
}

int main() {
    return !(test_unsigned() && test_signed());
}
