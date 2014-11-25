#include <more/io/binary_io.h>
#include <string>
#include <fstream>
#include <cstdlib>
#include <assert.h>

int main() {
    std::string str1, str2;
    {
	std::ofstream os("tmp.binary_io_t1.bin");
	for(int i = 0; i < 32; ++i) {
	    more::io::bit_writer< std::ostream > bitos(os);
	    for(int j = 0; j < i; ++j) {
		int bit = rand() & 1;
		std::cout << bit;
		str1.append(1, (bit? '1' : '0'));
		bitos.put(bit);
	    }
	    std::cout << ' ';
	}
    }
    std::cout << std::endl;
    {
	std::ifstream is("tmp.binary_io_t1.bin");
	for(int i = 0; i < 32; ++i) {
	    more::io::bit_reader< std::istream > bitis(is);
	    for(int j = 0; j < i; ++j) {
		int bit = bitis.get();
		std::cout << bit;
		str2.append(1, (bit? '1' : '0'));
	    }
	    std::cout << ' ';
	}
    }
    std::cout << std::endl;
    assert(str1 == str2);
    return 0;
}
