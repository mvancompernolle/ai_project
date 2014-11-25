//  Copyright 1998 Petter Urkedal.  Permission is hereby granted to
//  use, modify and redistribute this file under the conditions of
//  the GNU General Public License version 2 and later. THIS FILE IS
//  PROVIDED AS IS WITH NO EXPLICIT OR IMPLIED WARRANY OF ANY KIND.

#include <iostream>
#include <functional>
#include <iterator>
#include <vector>
#include <algorithm>
#include <more/gen/lambda.h>
#include <assert.h>

using namespace std;
using namespace more::gen;

int main() {
    vector<int> u(9), v(9), w(9);
    for(int i = 0; i < 9; i++) {
	u[i] = i;
	v[i] = 13*(7-i)&i-7*i;
    }

    placeholder<0, int> i;
    placeholder<1, int> j;
    placeholder<2, int> k;

    transform(u.begin(), u.end(), w.begin(),
	      lambda(i, 13*(7-i)&i-7*i));
    assert(v == w);

    copy(u.begin(), u.end(), ostream_iterator<int>(cout, "\t"));
    cout << endl;
    copy(v.begin(), v.end(), ostream_iterator<int>(cout, "\t"));
    cout << endl;

    transform(u.begin(), u.end(), v.begin(),
	      ostream_iterator<int>(cout, "\t"),
	      lambda(i, j, apply(lambda(k, k*k), i + j - 10) + j));
    cout << endl;
    return 0;
}
