#include <more/gen/generic.h>
#include <more/diag/debug.h>
#include <string>
#include <iostream>
#include <iterator>
#include <list>


struct A { // will be inlined
    A() { std::cout << this << " A::A()" << std::endl; }
    A(int i) { std::cout << this << " A::A(" << i << ")" << std::endl; }
    A(A const& o) {
	x = o.x;
	std::cout << this << " A::A(A const&)" << std::endl;
    }
    ~A() { std::cout << this << " A::~A()" << std::endl; }
    int x; 
};

struct B { // will be referred
    B() { std::cout << "B::B()" << std::endl; }
    B(B const& o) {
	x = o.x; y = o.y; z = o.z;
	std::cout << "B::B(B const&)" << std::endl;
    }
    ~B() { std::cout << "B::~B()" << std::endl; }
    int x;
    int y;
    int z;
};

int main() {
    namespace gen = more::gen;
    gen::generic vv;
    MORE_CHECK(vv.is<void>());
    gen::generic i(10);
    MORE_SHOW(i.to<int>());
    MORE_CHECK(i.to<int>() == 10);
    gen::generic x(11.1);
    MORE_SHOW(x.to<double>());
    MORE_CHECK(x.to<double>() == 11.1);
    gen::generic s(std::string("ezzit"));
    MORE_SHOW(s.to<std::string>());
    MORE_CHECK(s.to<std::string>() == "ezzit");

    A a0;
    a0.x = 444;
    gen::generic a1(a0);
    gen::generic a2(a1);
    gen::generic a3;
    a3 = a2;
    MORE_SHOW(a3.to<A>().x);
    MORE_CHECK(a3.to<A>().x == 444);

    B b0;
    b0.x = 444;
    gen::generic b1(b0);
    gen::generic b2(b1);
    gen::generic b3;
    b3 = b2;
    MORE_SHOW(b3.to<B>().x);
    MORE_CHECK(b3.to<B>().x == 444);

    std::list<int> l;
    l.push_back(1);
    l.push_back(2);
    l.push_back(3);
    gen::generic g(l.begin(), l.end());
    std::pair<int*, int*> r = g.to_range<int>();
    std::copy(r.first, r.second, std::ostream_iterator<int>(std::cout, " "));
    MORE_CHECK(std::equal(r.first, r.second, l.begin()));
    return more::diag::check_exit_status();
}
