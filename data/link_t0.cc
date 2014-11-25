#include <more/gen/link.h>
#include <iostream>
#include <iterator>


struct A : more::gen::linked {
    A(more::gen::link<A>& gr, const char* s_) : more::gen::linked(gr), s(s_) {}
    const char* s;
};

std::ostream& operator<<(std::ostream& os, A const& a) {
    return os << a.s;
}


int main() {
    more::gen::link<A> gr;
    A a(gr, "a"), b(gr, "b");
    {
	A c(gr, "c");
	std::copy(gr.begin(), gr.end(),
		  std::ostream_iterator<A>(std::cout, " "));
    }
    std::cout << '\n';
    std::copy(gr.begin(), gr.end(),
	      std::ostream_iterator<A>(std::cout, " "));
    std::cout << '\n';
    return 0;
}
