// There is a loss of efficiency of 1.7--1.8 independependent of
// whether binary_closure<> uses instance<> or not, as long
// as NDEBUG is defined.  This is probably due to the additional
// pointer-dereference needed since binary_closure<> must
// contain a polymorphic class.  Without this polymorphic class is
// seems impossible to represent a general functional expression,
// which may contain data as well as the function-pointer itself.

#define NDEBUG

#include <iostream>
#include <iomanip>
#include <more/gen/functional.h>
#include <more/gen/closure.h>
#include <more/diag/stopwatch.h>

#include "functional_b1.h"

using namespace std;
using namespace more::gen;

#define repeat for(int _i = 0; _i < num_repeat; ++_i)

const int num_repeat = 100000;


int f(int i, int j) { return i+j; }

int main()
{
    more::diag::stopwatch sw1a, sw1b;

    binary_closure<int, int, int> fa = f;

    for(int i = 0; i < 10; i++) {
	int sum = 0;

	sw1a.start();
	repeat sum += fa(i, i);
	sw1a.stop();

	sum = 0;

	sw1b.start();
	repeat sum += f(i, i);
	sw1b.stop();
    }
    cout << sw1a << '/' << sw1b << " = " << sw1a/sw1b << endl;
}

