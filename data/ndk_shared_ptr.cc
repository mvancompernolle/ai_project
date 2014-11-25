#include <iostream>

#include <ndk++/support.hh>

struct A
{
	A() { std::cerr << "A constructor\n"; }
	~A() {std::cerr << "A destructor\n"; }
};

typedef ndk::shared_ptr<A> a_ptr;

void func(a_ptr a)
{
	a_ptr b = a;
}

int main()
{
	a_ptr ap;
	{
		a_ptr	bp(new A);
		ap = bp;
	}
	func(ap);
}
