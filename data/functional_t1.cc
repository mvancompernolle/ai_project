//  Copyright 1998 Petter Urkedal.  Permission is hereby granted to
//  use, modify and redistribute this file under the conditions of
//  the GNU General Public License version 2 and later.  THIS FILE IS
//  PROVIDED AS IS WITH NO EXPLICIT OR IMPLIED WARRANY OF ANY KIND.

//! test functional-t1 using more

#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <more/gen/functional.h>
#include <assert.h>

using namespace std;
using namespace more::gen;

int f(int x) { return x*x; }
double f(double x) { return 3.3; }

struct one {
    one() {}
    one(int a_) : a(a_) {}
    int f() { return a+1; }
    int f(int x) { return a*x; }
    int f(int x, int y) { return a*x + y; }
    int f(int x, int y, int z) { return a*(x+y+z); }
    double f(double x) { return x/a; }
private:
    int a;
};

int main() {
    vector<int> v(3), v1(3), v2(3);
    v[0] = 2; v[1] = 3; v[2] = 5;
    v1[0] = -7; v1[1] = 10; v1[2] = -10;

    transform(v.begin(), v.end(), ostream_iterator<int>(cout, " "),
	      adapt((int(*)(int))&f));
    cout << endl;
    transform(v.begin(), v.end(), ostream_iterator<double>(cout, " "),
	      adapt((double (*)(double))&f));
    cout << endl;

    // bound mem_fun1
    one oneobj(10);
    transform(v.begin(), v.end(), ostream_iterator<double>(cout, " "),
	      bind_adapt(&oneobj, (int(one::*)(int))&one::f));
    cout << endl;

    // bound mem_fun2
    transform(v.begin(), v.end(), v1.begin(),
	      ostream_iterator<int>(cout, " "),
	      bind_adapt(&oneobj, (int(one::*)(int, int))&one::f));
    cout << endl;

    // bound mem_fun3
    assert(bind_adapt(&oneobj, (int(one::*)(int, int, int))&one::f)(2, 2, 2)
	   == 60);


    // mem_fun
    vector<one> vo(3);
    vo[0] = 10; vo[1] = 100; vo[2] = 1000;
    transform(vo.begin(), vo.end(), ostream_iterator<int>(cout, " "),
	      adapt((int(one::*)())&one::f));
    cout << endl;

    // mem_fun1
    transform(vo.begin(), vo.end(), v.begin(),
	      ostream_iterator<int>(cout, " "),
	      adapt((int(one::*)(int))&one::f));
    cout << endl;

    // mem_fun2
    assert(adapt((int(one::*)(int, int))&one::f)(oneobj, 4, 1) == 41);
}

