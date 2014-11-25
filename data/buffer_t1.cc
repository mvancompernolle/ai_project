#include <more/io/buffer.h>
#include <iostream>


int main() {
    more::io::buffer<char> ibuf(more::io::new_readline_driver<char>("> "));
    std::cout << "This should come before the prompt." << std::endl;
    assert(!ibuf.is_eof());
    while (!ibuf.is_eof()) {
	ibuf.get(); // skip the inserted newline to cause a prompt
	std::cout << "\nYou typed: " << std::flush;
	while (!ibuf.is_eof() && ibuf.peek() != '\n')
	    std::cout << ibuf.get();
	std::cout << std::endl;
    }
}
