//  Copyright 1998 Petter Urkedal.  Permission is hereby granted to
//  use, modify and redistribute this file under the conditions of
//  the GNU General Public License version 2 and later.  THIS FILE IS
//  PROVIDED AS IS WITH NO EXPLICIT OR IMPLIED WARRANY OF ANY KIND.

//! autotest functional-t3 using more

//  This shows that when connecting to a virtual functor, we can have
//  template specializations, but not conversion to best viable
//  overloaded function.
//
//  Intrusive workaround:  The user supplies a member-function
//  template which selects the best viable function.  The analog for
//  functions is not always desirable.
//
//  Nonintrusive workaround?  Probably none.

#include <more/gen/functional.h>
#include <more/gen/closure.h>
#include <complex>
#include <cmath>

using namespace std;
using namespace more::gen;

struct base {};
struct derived : base {};

struct C {
    int f(const base&) { return 1; }
    template<typename T> complex<T> g(T x) { return sqrt(complex<T>(x)); }
};


int main() {
    unary_closure<const derived&, int> fun;
    C c;
    unary_closure< double, complex<double> > fct;
    fct = bind_adapt(&c,
		     (complex<double>(C::*)(double))&C::g<double>);
    assert(abs(imag(fct(-4.0)) - 2.0) < 1e-8);
}

