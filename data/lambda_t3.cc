//  Copyright 1998 Petter Urkedal.  Permission is hereby granted to
//  use, modify and redistribute this file under the conditions of
//  the GNU General Public License version 2 and later.  THIS FILE IS
//  PROVIDED AS IS WITH NO EXPLICIT OR IMPLIED WARRANY OF ANY KIND.

//! test

#include <iostream>
#include <iterator>
#include <functional>
#include <vector>
#include <algorithm>
#include <more/gen/lambda.h>

using namespace std;
using namespace more::gen;


int main() {
    placeholder<0, int> i;
    placeholder<1, int> j;
    placeholder<2, int> k;
    placeholder<3, int> l;

    vector<int> v(9), w(9);

    counter<int> cnt;

    // generate integers
    generate(v.begin(), v.end(), cnt);

    // generate squared integers
    // Note:  apply(cnt) means `call cnt' which transforms the generator
    // cnt into an expression.  The outer lambda-expression has no
    // parameters, and transforms the whole expression into a generator.
    generate(w.begin(), w.end(),
	     lambda(apply(lambda(i, i*i), apply(cnt))));

    copy(v.begin(), v.end(), ostream_iterator<int>(cout, "\t"));
    cout << endl;
    copy(w.begin(), w.end(), ostream_iterator<int>(cout, "\t"));
    cout << endl;

    // visibility of variables in an inner scope:

    transform(v.begin(), v.end(), w.begin(),
	      lambda(i, apply(lambda(j, i*j), i+100)));

    copy(w.begin(), w.end(), ostream_iterator<int>(cout, "\t"));
    cout << endl;

    transform(v.begin(), v.end(), w.begin(), w.begin(),
	      lambda(i, j, apply(lambda(k, l, i+j-k*l), i+j, i-j)));

    copy(w.begin(), w.end(), ostream_iterator<int>(cout, "\t"));
    cout << endl;
    return 0;
}
