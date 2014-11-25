//! test lambda-b1 using more

#include <iostream>
#include <iomanip>
#include <vector>
#include <iterator>
#include <algorithm>
#include <more/gen/functional.h>
#include <more/gen/lambda.h>
#include <more/diag/stopwatch.h>

using namespace std;
using namespace more::gen;

#define repeat for(int _i = 0; _i < num_repeat; ++_i)


struct square_sum : binary_function<int, int, int> {
    int operator()(int i, int j) {
	int k = i+j;
	return k*k;
    }
};
struct square_sum_mod : binary_function<int, int, int> {
    square_sum_mod(int a_, int b_) : a(a_), b(b_) {}
    int operator()(int i, int j) {
	int k = b*i+j;
	return k*k+a;
    }
    const int a, b;
};

struct more_complicated : binary_function<int, int, int> {
    more_complicated(int a_, int b_)
	: a(a_), b(b_) {}
    int operator()(int i, int j) {
	int n = i+j*a;
	int m = j%b+i*i;
	return n*n + n*m + m*m;
    }
private:
    const int a, b;
};

struct more_complicated_consts_in_code : binary_function<int, int, int> {
    int operator()(int i, int j) {
	int n = i+j*3;
	int m = j%7+i*i;
	return n*n + n*m + m*m;
    }
};



const int dim = 1000;
const int num_repeat = 1000;
const int num_calls = dim*num_repeat;
const int unit = 1000;


double eff(double t) {
    return num_calls/(t*unit);
}



int main(int argc, char** argv) {
    vector<int> vec1(dim), vec2(dim), vec3(dim);
    generate(vec1.begin(), vec1.end(), counter<int>());
    generate(vec1.begin(), vec1.end(), counter<int>(10));

    more::diag::stopwatch sw1, sw1l, sw2, sw2l, sw3, sw3l, sw4, sw4x, sw4l;

    for(int irep = 0; irep < 10; ++irep) {
	placeholder<0, int> i;
	placeholder<1, int> j;
	placeholder<2, int> k;
	placeholder<3, int> n;
	placeholder<4, int> m;

	sw1.start();
	repeat
	    transform(vec1.begin(), vec1.end(), vec2.begin(), vec3.begin(),
		      plus<int>());
	sw1.stop();

	sw1l.start();
	repeat
	    transform(vec1.begin(), vec1.end(), vec2.begin(), vec3.begin(),
		      lambda(i, j, i+j));
	sw1l.stop();

	sw2.start();
	repeat
	    transform(vec1.begin(), vec1.end(), vec2.begin(), vec3.begin(),
		      square_sum());
	sw2.stop();

	sw2l.start();
	repeat
	    transform(vec1.begin(), vec1.end(), vec2.begin(), vec3.begin(),
		      lambda(i, j, apply(lambda(k, k*k), i+j)));
	sw2l.stop();

	sw3.start();
	repeat
	    transform(vec1.begin(), vec1.end(), vec2.begin(), vec3.begin(),
		      square_sum_mod(8, 11));
	sw3.stop();

	sw3l.start();
	repeat
	    transform(vec1.begin(), vec1.end(), vec2.begin(), vec3.begin(),
		      lambda(i, j,
			     apply(lambda(k, k*k+8), 11*i+j)));
	sw3l.stop();

	sw4.start();
	repeat
	    transform(vec1.begin(), vec1.end(), vec2.begin(), vec3.begin(),
		      more_complicated_consts_in_code());
	sw4.stop();

	sw4x.start();
	repeat
	    transform(vec1.begin(), vec1.end(), vec2.begin(), vec3.begin(),
		      more_complicated(3, 7));
	sw4x.stop();

	sw4l.start();
	repeat
	    transform(vec1.begin(), vec1.end(), vec2.begin(), vec3.begin(),
		      lambda(i, j, apply(lambda(n, m, n*n+n*m+m*m),
					 i+j*3, j%7+i*i)));
	sw4l.stop();



    }
    cout << setw(16) << ""
	 << setw(12) << setprecision(4) << "i+j"
	 << setw(12) << setprecision(4) << "(i+j)^2"
	 << setw(12) << setprecision(4) << "(11i+j)^2+8"
	 << setw(12) << setprecision(4) << "complicated" << endl;
    cout << "functional:\t"
	 << setw(12) << setprecision(4) << eff(sw1)
	 << setw(12) << setprecision(4) << eff(sw2)
	 << setw(12) << setprecision(4) << eff(sw3)
	 << setw(12) << setprecision(4) << eff(sw4x) << endl;
    cout << "lambda:\t\t"
	 << setw(12) << setprecision(4) << eff(sw1l)
	 << setw(12) << setprecision(4) << eff(sw2l)
	 << setw(12) << setprecision(4) << eff(sw3l)
	 << setw(12) << setprecision(4) << eff(sw4l) << endl;
    cout << "ratios:\t\t"
	 << setw(12) << setprecision(4) << eff(sw1)/eff(sw1l)
	 << setw(12) << setprecision(4) << eff(sw2)/eff(sw2l)
	 << setw(12) << setprecision(4) << eff(sw3)/eff(sw3l)
	 << setw(12) << setprecision(4) << eff(sw4x)/eff(sw4l) << endl;
    cout << "measued in " << 10*unit << " calls/s" << endl;
    cout << "In the last, " << eff(sw4)/eff(sw4x)
	 << " can be gained in functional version by encoding the\n"
	 << "constants into the member function of the functional." << endl;
}
