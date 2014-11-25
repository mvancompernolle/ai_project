//! test lambda-t5 using more

#include <iostream>
#include <iomanip>
#include <more/gen/lambda.h>
#include <more/gen/lambda_math.h>
#include <more/gen/functional.h>
#include <more/gen/closure.h>
#include <more/gen/iterator.h>
#include <more/io/cmdline.h>


using namespace std;
using namespace more::gen;

enum interaction { gaussian, Yukawa, harmonic };


unary_closure<double, double>
gen_fun(interaction i, double strength, double range) {

    placeholder<0, double> r;

    switch(i) {
    case gaussian:
	return lambda(r, strength*exp(-r*r/(range*range)));
    case Yukawa:
	return lambda(r, strength*exp(-r/range));
    case harmonic:
    default:
	return lambda(r, strength*r*r);
    }
}




int main(int argc, char* argv[]) try {
    interaction iact = gaussian;
    double V0, r0;

    more::io::cmdline cmd;
    cmd.insert_setter("-g|--gauss|--gaussian", "Select gaussian interaction.",
		iact, gaussian);
    cmd.insert_setter("-y", "Select Yukawa interaction.",
		iact, Yukawa);
    cmd.insert_setter("-o", "Select harmonic oscillator interaction.",
		iact, harmonic);
    cmd.insert_reference("", "strength", V0);
    cmd.insert_reference("", "range", r0);
    cmd.parse(argc, argv);

    unary_closure<double, double> f = gen_fun(iact, V0, r0);
    for(double r = .2; r < 3.0; r += .4)
	cout << setw(6) << r << setw(12) << f(r) << endl;
    return 0;
} catch(const more::io::cmdline::relax&) {
    return 0;
} catch(const exception& xc) {
    cout << "exception: " << xc.what() << endl;
    return 1;
}


