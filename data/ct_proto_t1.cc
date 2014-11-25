#include <more/lang/ct_proto.h>
#include <more/diag/stopwatch.h>

namespace lang = more::lang;
namespace diag = more::diag;

double
my_mult(double x, double y)
{
    return x*y;
}

int
main()
{
    diag::stopwatch sw_direct;
    diag::stopwatch sw_indir;

    int const N = 1000000;
    double summy = 0;

    double x = 1.0;
    double y = 1.0;
    sw_direct.start();
    for (int i = 0; i < N; ++i) {
	summy += my_mult(x, y);
	x *= .5;
	y *= .8;
    }
    sw_direct.stop();

    lang::ct_proto* t
	= lang::make_proto(lang::ct_type_of<double>(),
			   lang::ct_type_of<double>(),
			   lang::ct_type_of<double>(),
			   (lang::ct_type*)0);
    summy = 0;
    x = 1.0;
    y = 1.0;
    sw_indir.start();
    for (int i = 0; i < N; ++i) {
	double res;
	void* args[2] = { &x, &y };
	t->call((lang::fn_ptr_t)my_mult, &res, args);
	summy += res;
	x *= .5;
	y *= .8;
    }
    sw_indir.stop();

    std::cout << "sw_direct = " << sw_direct << '\n'
	      << "sw_indir  = " << sw_indir << '\n';
}
