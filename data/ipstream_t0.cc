#include <more/io/ipstream.h>

int main() {
    more::io::ipstream is("fortune");
    while (is.good())
	std::cout << (char)is.get();
    return is.bad();
}
