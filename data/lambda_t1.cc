//  Copyright 1998 Petter Urkedal.  Permission is hereby granted to
//  use, modify and redistribute this file under the conditions of
//  the GNU General Public License version 2 and later. THIS FILE IS
//  PROVIDED AS IS WITH NO EXPLICIT OR IMPLIED WARRANY OF ANY KIND.

#include <iostream>
#include <functional>
#include <iterator>
#include <list>
#include <algorithm>
#include <more/gen/lambda.h>


using namespace std;
using namespace more::gen;


int main() {

    // Setup some containers

    int data[8] = { 7, 3, 5, 2, 10, -4, -2, 0 };
    int data2[8]= { 1, 2, 3, 4, 5, 6, 7, 8 };
    double data3[8] = { 1.88, 3.0, -1.0, 22.22, -1e6, .01, .0002, 1.0 };
    list<int> l1(8), l2(8);
    copy(data, data+8, l1.begin());
    copy(data2, data2+8, l2.begin());

    // Define placeholders

    placeholder<42, int> x; // the first int is used only to distinguish
    placeholder<1, int> y;  // placeholder of the same type.
    placeholder<2, double> z;

    // Test it!

    transform(l1.begin(), l1.end(),
	      ostream_iterator<bool>(cout, " "),
	      lambda(y, y < 0));

    cout << endl;

    transform(l1.begin(), l1.end(), l2.begin(),
	      ostream_iterator<bool>(cout, " "),
	      lambda(x, y, x < y));
    cout << endl;

    // test where and apply(var, var)
    transform(l1.begin(), l1.end(), l2.begin(),
	      ostream_iterator<int>(cout, " "),
	      lambda(x, y,
		     where(x < y, x, apply(plus<int>(), x, y))));
    cout << endl;

    transform(l1.begin(), l1.end(), l2.begin(),
	      ostream_iterator<int>(cout, " "),
	      lambda(x, y, x ^ (~y)));
    cout << endl;

    // test apply(var, const)
    cout << lambda(x, apply(plus<int>(), x, 12))(100) << endl;

    list<double> l3(8);
    copy(data3, data3+8, l3.begin());
    copy(l3.begin(), l3.end(), ostream_iterator<double>(cout, " "));

    cout << endl;

    // Replace all values outside the interval (0.0, 10.0] with 0.0
    transform(l3.begin(), l3.end(),
	      ostream_iterator<double>(cout, " "),
	      lambda(z, where(z > 0.0 && z <= 10.0,
			      z, 0.0)));
    cout << endl;
    return 0;
}

