#include <iostream>
#include <algorithm>
#include <more/gen/multiclosure.h>
#include <more/gen/lambda.h>

using namespace std;
using namespace more::gen;

int f1(int x) { cout << "f1(" << x << ")"; return 7; }
int f2(int x) { cout << "f2(" << x << ")"; return 8; }
int f3(int x) { cout << "f3(" << x << ")"; return 9; }

int
main(int, char**)
{
    placeholder< 0, unary_closure<int, int> > f_ph;
    unary_multiclosure<int, int> f;
    connection c1(f, adapt(f1));
    connection c2(f, adapt(f2));
    {
	connection c3(f, adapt(f3));
	cout << " 10 --> ";
	transform(f.begin(), f.end(), ostream_iterator<int>(cout),
		  lambda(f_ph, apply(f_ph, 10)));
	cout << endl;
    }
    cout << " 100 --> ";
    transform(f.begin(), f.end(), ostream_iterator<int>(cout),
	      lambda(f_ph, apply(f_ph, 100)));
    cout << endl;
    return 0;
}
