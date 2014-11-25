//! autotest using more

#include <more/gen/functional.h>
#include <more/gen/closure.h>
#include <string>

#include <iostream>
#include <sstream>

#include <cassert>

using namespace std;
using namespace more::gen;



int f() { static int n = 0; return n++; }
int f(int x) { return x*x; }
double f(double x) { return 1.2; }
string f(int x, char* s) { ostringstream sm; sm << x << s; return sm.str(); }

string f(int x, double y, char* b) {
    ostringstream sm;
    sm << "[" << x << ' ' << y << ' ' << b << "]";
    return sm.str();
}

struct C {
    C(int a_) : a(a_) {}
    int f() { return 333+a; }
    int f(int x) { return 2*x+a; }
    int f(int x, int y) { return x*y+a; }
    string f(int x, char* s, int y = -1) {
	ostringstream sm;
	sm << x << s << y;
	return sm.str();
    }
private:
    int a;
};


int main() {

    // functions

    generator_closure<int> gen = f;
    assert(gen() == 0);
    assert(gen() == 1);
    assert(gen() == 2);

    unary_closure<int, int> fun;
    unary_closure<double, double> fun1 = f;
    binary_closure<int, char*, string> fun2;
    ternary_closure<int, double, char*, string> fun3;
    fun = f;
//    connect(fun2, f);
//    connect(fun3, f);
    fun2 = f;
    fun3 = f;

    assert(fun(10) == 10*10);
    assert(fun1(1000) == 1.2);
    assert(fun2(10, "ten") == string("10ten"));
    assert(fun3(1, 1.1, "?") == "[1 1.1 ?]");

    connect(fun, (int(*)(int))f);
    connect(fun1, (double(*)(double))f);
    fun2 = f;
    fun3 = f;

    assert(fun(10) == 10*10);
    assert(fun1(1000) == 1.2);
    assert(fun2(10, "ten") == string("10ten"));
    assert(fun3(1, 1.1, "?") == "[1 1.1 ?]");


    // bound member functions

    C c(10000);

    generator_closure<int> g0;
    unary_closure<int, int> g1;
    binary_closure<int, int, int> g2;
    ternary_closure<int, char*, int, string> g3;
    connect(g0, c, (int(C::*)())&C::f);
    connect(g1, c, (int(C::*)(int))&C::f);
    connect(g2, c, (int(C::*)(int,int))&C::f);
    connect(g3, c, (string(C::*)(int,char*,int))&C::f);
    assert(g0() == 10000+333);
    assert(g1(3) == 10000+2*3);
    assert(g2(9, -9) == 10000-9*9);
    assert(g3(9, "nine", 99) == string("9nine99"));

    unary_closure<C&, int> f1;
    binary_closure<C&, int, int> f2;
    ternary_closure<C&, int, int, int> f3;


    // member functions

    connect(f1, (int(C::*)())&C::f);
    connect(f2, (int(C::*)(int))&C::f);
    connect(f3, (int(C::*)(int,int))&C::f);
    assert(f1(c) == 10000+333);
    assert(f2(c, 3) == 10000+2*3);
    assert(f3(c, 3, 7) == 10000+3*7);

//    f1 = &C::f;
//    binary_closure<C, int, int> ff2 = &C::f;
//    ternary_closure<C, int, char*, string> ff3 = &C::f;
//    assert(f1(c) == 10000+333);
//    assert(f2(c, 3) == 10000+2*3);
//    assert(f3(c, 3, "three") == string("3three-1"));

}
