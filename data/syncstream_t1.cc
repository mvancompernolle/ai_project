#include <more/io/syncstream.h>


struct A {
    A() {}
    A(int i_, std::string s_, double x_, int j_)
	: i(i_), s(s_), x(x_), j(j_) {}

    friend bool
    operator==(A const& a, A const& b) {
	return a.i == b.i && a.s == b.s && a.x == b.x && a.j == b.j;
    }
    void sync(more::io::syncstream& sio) {
	sio | i | s | x | j;
    }
    friend std::ostream& operator<<(std::ostream& os, A const& a) {
	os << '{' << a.i << ',' << a.s << ',' << a.x << ',' << a.j << '}';
	return os;
    }
  private:
    int i;
    std::string s;
    double x;
    int j;
};    


int
main() try {
    A a(-15, "Plan 9", .3, 10);
    A b;
    {
	more::io::fsyncstream sout("tmp.syncstream_t1.bin",
				   std::ios_base::out | std::ios_base::binary);
	sout | a;
    }
    {
	more::io::fsyncstream sinput("tmp.syncstream_t1.bin",
				     std::ios_base::in | std::ios_base::binary);
	sinput | b;
    }
    if (!(a == b)) {
	std::cerr << "Re-read object differs from original:\n"
		  << "  a = " << a << '\n'
		  << "  b = " << b << '\n';
	return 1;
    }
    return 0;
}
catch (std::exception const& xc) {
    std::cerr << "exception: " << xc.what() << std::endl;
    return 1;
}
